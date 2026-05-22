/*********************************************************************
 *
 *  $Header:$
 *
 *  NAME
 *      yahr_conduit.c
 *
 *  DESCRIPTION
 *      This is the conduit of communication between the yahrzeit
 *      front-end elements (such as the web pages), the backend elements
 *      (such as the yahrzeitd), and certain command-line scripts
 *      like "alloff" or "yizkor".
 *  
 *      Specifically this application conduits commands to the
 *      8051 LED controller board.
 *      
 *
 *  CONTENTS
 *
 *      line    Function Declarations
 *      ----    ---------------------------------------------------------
 *     	  49    #define PERROR_EXIT(format,arg)  ...
 *     	  57    #define VDEBUG(arg)              ...
 *     	  86    main (argc, argv)
 *     	 236    void  queue_init ( struct queue *Q ) {  
 *     	 240    void  enqueueIObyte ( struct queue *Q, char c ) {
 *     	 249    char  dequeueIObyte ( struct queue *Q ) {   
 *     	 258    isQueueEmpty ( struct queue *Q ) { return ( Q->q_count == 0 ); }
 *     	 266    void  serial_process()
 *     	 393    void  termiomagic()
 *     	 430    void  socket_process()

 *
 *  HISTORY
 *
 *
 *  BUGS
 *
 *
 *  TODO
 *
 *  COPYRIGHT NOTICE
 *      Copyright (c) 2008, Allan M. Schwartz
 *      All rights reserved.
 *
 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <strings.h>
#include <assert.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define FD  int


#define PERROR_EXIT(format,arg)  { \
              sprintf( errstring, "%s: %d %s\n", __FILE__, __LINE__, "error detected" ); \
              if ( verbose ) \
                  printf( errstring ); \
              sprintf( errstring, format, arg ); \
              perror (errstring); exit (errno); \
              }

#define VDEBUG(arg)              { \
              sprintf( errstring, "%s: %d %s\n", __FILE__, __LINE__, arg ); \
              if ( verbose ) \
                  printf( errstring ); \
              }

extern int errno;
int     verbose = 0;
int     exitcode = 0;
char   *progname = NULL;
char    buffer[80];
char    errstring[80];
FD      serialdevFD = 0;
FD      inputcommandFD = 0;
FD      sockFD = 0;

char   *serial_dev_name = NULL;
char   *ifname = NULL;
char   *netaddress = NULL;
char   *portnum = "23";

// forward define these functions, to help out the compiler
void  serial_process();
void  termiomagic();
void  socket_process();

/**
 * main entry to the process
 */
main (int argc, char **argv)
{
    VDEBUG( "main" );
    char   *rindex ();
    extern char *optarg;
    extern int optind;
    register int c;
    int     errflg = 0;


    /* Initialization */
    if ((progname = rindex (*argv, '/')) == NULL)
        progname = *argv;
    else
        progname++;

    /* Argument Processing */
    while ((c = getopt (argc, argv, "vi:s:n:p:")) != EOF) {
        switch (c) {
            case 'v':
                verbose = 1;
                break;
            case 'i':
                if ((ifname = optarg) == NULL) {
                    fprintf (stderr, "Missing input file name\n");
                    exit (1);
                }
                break;
            case 's':           /* serial device file name */
                if ((serial_dev_name = optarg) == NULL) {
                    fprintf (stderr, "Missing output file name\n");
                    exit (1);
                }
                break;
            case 'n':           /* network address */
                if ((netaddress = optarg) == NULL ) {
                    fprintf (stderr, "Mising host socket\n");
                    exit (1);
                }
                break;
            case 'p':           /* port */
                if ((portnum = optarg) == NULL ) {
                    fprintf (stderr, "Mising port number\n");
                    exit (1);
                }
                break;
            case '?':
                errflg++;
        }
    }

    argc -= optind;
    argv += optind;

    if (errflg) {
        printf ("Usage: %s [-v] -i filename [-s serialport] | [-n netaddress -p port] \n", progname);
        exit (2);
    }

    VDEBUG( "arguments have been handled" );

    // two case ... serial connection of socket based tcp-ip connection

    if ( serial_dev_name != NULL ) {

        // Open LED controller (the serial port connected to this board)  
        //    ... both for read or write
        if ( (serialdevFD = open( serial_dev_name, O_RDWR|O_NONBLOCK|O_NOCTTY ) ) == -1 ) {
            PERROR_EXIT( "on open %s", serial_dev_name );   // exit on any serious error
        }
    } 
    else if ( netaddress ) { 
        struct sockaddr_in digiOneSp_addr;
        struct hostent *he;

        // translate the hostname into a address structure
        if (( he = gethostbyname( netaddress ) ) == NULL ) {
            PERROR_EXIT( "on gethostbyname %s", netaddress );    // exit on any serious error
        }

        if ((sockFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            PERROR_EXIT( "on socket creation %s", "SOCK_STREAM" );   // exit on any serious error
        }

        //VDEBUG( "after socket creation" );

        bzero ( &digiOneSp_addr, sizeof digiOneSp_addr );
        digiOneSp_addr.sin_family = AF_INET;
        digiOneSp_addr.sin_addr = *((struct in_addr *)he->h_addr);
        digiOneSp_addr.sin_port = htons( atoi( portnum ? portnum : "23" ) );

        //VDEBUG( "before connect " );

        // connect to the digiOneSp
        if (connect( sockFD, (struct sockaddr *)&digiOneSp_addr, sizeof(struct sockaddr)) == -1 ) {
            PERROR_EXIT( "on connect %s", netaddress );          // exit on any serious error
        }
        //VDEBUG( "connect succeeded" );

    } else {
        fprintf (stderr, "no controller file name or IP address\n");
        exit (1);
    }

    /* Open input command fifo or file */
    if (ifname) {
        if ( (inputcommandFD = open( ifname, O_RDONLY|O_NONBLOCK) ) == -1 ) {
            PERROR_EXIT( "on open %s", ifname );                // exit on any serious error
        }
    } else {
        fprintf (stderr, "no input file name\n");
        exit (1);
    }

    /* BEGIN ACTION HERE */
    if ( serial_dev_name != NULL ) {
        VDEBUG( "starting serial_process" );
        termiomagic();
        serial_process();
    }
    else if ( netaddress ) { 
        VDEBUG( "starting socket_process" );
        socket_process();
    }

    exit (exitcode);
}


/**
 * define a Queue package, ... 
 *
 * because we are going to queue characters read from the input FIFO
 * to them be written to the LED controller
 *
 * We queue them, because we are doing byte-at-a-time IO
 */

#define Q_SIZE  1024
#define Q_MASK  1023

struct queue {
    char q_store[Q_SIZE];
    int  q_count;
    int  q_input_index;         // index to store next byte, iff q_count < QSIZE
    int  q_output_index;        // index to retrieve next byte, iff q_count > 0
};

void  queue_init ( struct queue *Q ) {  
    bzero( Q, sizeof (struct queue) );
}

void  enqueueIObyte ( struct queue *Q, char c ) {
    if ( Q->q_count < sizeof Q->q_store ) {
        Q->q_count++;
        Q->q_store[Q->q_input_index] = c;
        Q->q_input_index++;
        Q->q_input_index &= Q_MASK;
    }
}

char  dequeueIObyte ( struct queue *Q ) {   
    if ( Q->q_count == 0 ) return 0;
    char c = Q->q_store[Q->q_output_index];
    Q->q_output_index++;
    Q->q_output_index &= Q_MASK;
    Q->q_count--;
    return c;
}

isQueueEmpty ( struct queue *Q ) { return ( Q->q_count == 0 ); }

/**
 * serial_process
 *
 * this is a our infinite event loop process
 *
 */
void  serial_process()
{
    char *rc;
    struct timeval timeout;
    int rc_i;
    fd_set read_set;
    //fd_set write_set;

    char inputcmd[80];
    char inputcmd_index = 0;
    bzero( inputcmd, sizeof inputcmd );

    struct queue controller_write_q;
    queue_init( &controller_write_q );

    VDEBUG( "serial_process" );

    int nfildes = ( inputcommandFD > serialdevFD ? inputcommandFD : serialdevFD ) + 1;

    while ( 1) {
        // wait 1 millisecond at the top of this loop
        usleep( 1000 );

        FD_ZERO( &read_set );
        FD_SET( serialdevFD, &read_set);
        FD_SET( inputcommandFD, &read_set);
        //FD_ZERO( &write_set );
        //FD_SET( serialdevFD, &write_set);

        // ... blocks until some fd is ready to read
        if ( select( nfildes, &read_set, /* &write_set */ NULL, NULL, NULL ) == -1 ) {
            PERROR_EXIT( "select error", 0 );       // exit on any serious error
        }

        // ... read a byte from the serialdevFD, and echo it
        //   these are typically response lines like "OK"
        if (FD_ISSET( serialdevFD, &read_set ) ) {
            char readbuf[10];
            bzero( readbuf, sizeof readbuf );
        
            // ... read a byte from the serialdevFD
            if ( read ( serialdevFD, readbuf, 1 ) != 1 ) {
                PERROR_EXIT( "on read %d", serial_dev_name ); // exit on any serious error
            }
            // just echo the response
            printf( "%c", readbuf[0] );
            continue;               // continue to read all of these bytes...
        }

        // ... write a byte to the serialdevFD
        //  should we check !isQueueEmpty( &controller_write_q) && FD_ISSET( serialdevFD, &write_set );
        if ( !isQueueEmpty( &controller_write_q ) ) {
            char c = dequeueIObyte( &controller_write_q );
            int rc;

            int timeout;
            for ( timeout = 10; --timeout; ) {
                // ... write a byte from the serialdevFD
                int rc;
                rc = write ( serialdevFD, &c, 1 );
                if ( rc == -1 && errno == EAGAIN ) {
                    usleep( 1000 ) ;
                    continue;
                }
                else if ( rc == -1 ) {
                    PERROR_EXIT( "on write %s", serial_dev_name ); // exit on any serious error
                }
                else if ( rc == 0 ) {
                    printf("debug write 0 ,%c,\n", c );
                }
                else {
                    break;
                }
            }
            if ( timeout == 0 ) {
                printf( "debug timeout .. can't write ,%c,\n", c );
            }
            continue;               // continue to write all of these bytes...
        }

        // ... read a byte from the input command fifo or file
        //   these are typically command lines like "pixel on 2 3 4"
        if (FD_ISSET( inputcommandFD, &read_set ) ) {

            char c;
        
            int rc; 
            if ( (rc = read( inputcommandFD, &c, 1 ) )  == -1 ) {
                PERROR_EXIT( "on read %s", ifname );    // exit on any serious error
            }

            if ( rc == 0 ) {
                //printf("debug read rc 0 \n", c );
                continue;
            }

            assert ( c != '\0' );
            inputcmd[inputcmd_index++] = c;
            assert ( inputcmd_index < sizeof inputcmd );

            if ( c == '\n' || c == '\r' ) {

                // enqueue this line to be written to the LED controller
                char *p;
                for ( p = inputcmd; *p; p++ ) {
                    if ( *p == '\n' ) *p = '\r';
                    enqueueIObyte( &controller_write_q, *p );
                }

                // pause a couple of milliseconds  (4000 usec is 4 msec)
                usleep(4000);

                bzero( inputcmd, sizeof inputcmd );
                inputcmd_index = 0;
            }
        }
    }
}


#include <termios.h>

/**
 * termiomagic
 *
 * do the magic necessary to set the baud rate, ignore modem control lines
 */
void  termiomagic()
{
    VDEBUG( "termiomagic" );
    int my_flags;
    if ( ( my_flags = fcntl( serialdevFD, F_GETFL ) ) == -1 )  {
        PERROR_EXIT( "on fcntl %s", serial_dev_name );      // exit on any serious error
    }

    int rc;
    rc = fcntl( serialdevFD, F_SETFL, my_flags & ~O_NONBLOCK );
    if ( rc == -1 ) {
        PERROR_EXIT( "on fnctl %s", serial_dev_name );      // exit on any serious error 
    }

    struct termios tios;
    rc = tcgetattr( serialdevFD, &tios );
    if ( rc == -1 ) {
        PERROR_EXIT( "on tcgetattr %s", serial_dev_name );  // exit on any serious error 
    }

    cfsetispeed ( &tios, 9600 );
    cfsetospeed ( &tios, 9600 );
    tios.c_cflag |= CLOCAL;            // ignore all modem flow control signals

    rc = tcsetattr( serialdevFD, TCSAFLUSH, &tios ) ;
    if ( rc == -1 ) {
        PERROR_EXIT( "on tcsetattr %s", serial_dev_name );  // exit on any serious error 
    }
}


/**
 * socket_process
 *
 * this is a our infinite event loop process when socket connected
 *
 */
void  socket_process()
{
    fd_set read_set;

    char inputcmd[100];
    char inputcmd_index = 0;
    bzero( inputcmd, sizeof inputcmd );

    VDEBUG( "socket_process" );

    int nfildes = ( inputcommandFD > sockFD ? inputcommandFD : sockFD ) + 1;

    while ( 1) {

        FD_ZERO( &read_set );
        FD_SET( sockFD, &read_set);
        FD_SET( inputcommandFD, &read_set);

        // ... blocks until some fd is ready to read
        if ( select( nfildes, &read_set, /* &write_set */ NULL, NULL, NULL ) == -1 ) {
            PERROR_EXIT( "select error", 0 );       // exit on any serious error
        }

        // ... read a line from the sockFD, and echo it
        //   these are typically response lines like "OK"
        if (FD_ISSET( sockFD, &read_set ) ) {
            char readbuf[100];
            bzero( readbuf, sizeof readbuf );
        
            // ... read a byte from the sockFD
            if ( read ( sockFD, readbuf, 1 ) != 1 ) {
                PERROR_EXIT( "on read %d", netaddress ); // exit on any serious error
            }
            // just echo the response
            printf( "%s", readbuf );
            continue;               // continue to read all of these bytes...
        }

        // ... read a byte from the input command fifo or file
        //   these are typically command lines like "pixel on 2 3 4"
        if (FD_ISSET( inputcommandFD, &read_set ) ) {

            char c;
            int rc; 

            if ( (rc = read( inputcommandFD, &c, 1 ) )  == -1 ) {
                PERROR_EXIT( "on read %s", ifname );    // exit on any serious error
            }

            if ( rc == 0 ) {
                // End-of-file, our work here is done
                exitcode = 0;
                return;
            }

            assert ( c != '\0' );
            inputcmd[inputcmd_index++] = c;
            assert ( inputcmd_index < sizeof inputcmd );

            if ( c == '\n' || c == '\r' ) {

                if ( inputcmd[0] == '#' ) {
                    // this is a comment, echo it to stdout
                    printf( "%s", inputcmd );
                }
                else {

                    rc = write ( sockFD, inputcmd, strlen(inputcmd) );

                    if ( rc == -1 ) {
                        PERROR_EXIT( "on write %s", netaddress ); // exit on any serious error
                    }

                    // each line, pause 50 milliseconds  (50000 usec is 50 msec)
                    // giving the 8051 board enough time to do the operation.
                    usleep(50000);
                }
                bzero( inputcmd, sizeof inputcmd );
                inputcmd_index = 0;
            }
        }
    }
}

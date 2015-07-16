/**
 * @file        yahrzeit_v2.c
 *
 * @brief       top-level C code the Yahrzeit Embedded Controller
 *
 * @history     version 0.4 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 *
 * @notes       see project notes in file yahrzeit_v2.h
 */

/**
 * ASSEMBLY
 *      Using the "TEST_16x64_LED_MATRIX"
 *          LED Matrix panel has 64x16 pixels.
 *                  connections to Arduino Uno
 *
 *          LED Matrix PCB:      LA, LB, LC, LD,    EN, R1, LT, SK
 *          Arduino Digital pin: 3,  4,  5,  6,     2, 11,  8,  13
 *
 *      Using the YYZ PIXEL boards 
 *                  connections to an Arduino Mega
 *
 *          YYZ PIXEL BOARD:     DI, OE, CP, ST
 *          Arduino Digital pin: 48, 46, 42, 44
 *
 *      Ethernet shield attached to pins 50, 51, 52, 53 (on the Arduino Mega)
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <SPI.h>
#include "yahrzeit_v2.h"


// ----------------------------------------------------------------------------
//            L E D   A R R A Y S
// ----------------------------------------------------------------------------

// we are supporting two different physical interfaces to LED ARRAYS
#if TEST_16x64_LED_MATRIX
#include "LEDMatrix.h"
#endif

#if CBS_56x40_WALL || TEST_2x24_HARNESS
#include "yyz_pixel.h"
#endif


#define WIDTH  NCOLS        // 2 in our home lab
#define HEIGHT NROWS        // 24 in our home lab


#if TEST_16x64_LED_MATRIX
// constructor for the LedMatrix, an instance of the 'LEDMatrix' class
// LEDMatrix    ( a, b, c, d, oe, r1, stb, clk);
LEDMatrix LedMatrix( 3, 4, 5, 6, 2,  11, 8,   13);
#endif

#if CBS_56x40_WALL || TEST_2x24_HARNESS
// constructor for the LedMatrix, an instance of the 'yyz_pixel' class
//yyz_pixel     ( DI, OE, CP, ST )
yyz_pixel LedMatrix(   48, // blue ->  DI,
                       46, // green->  OE,
                       42, // yellow-> CP
                       44);// orange-> ST
#endif


// ----------------------------------------------------------------------------
//            E T H E R N E T
// ----------------------------------------------------------------------------

// Make up a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
    0x11, 0x09, 0x19, 0x55, 0x06, 0x13
};
IPAddress ipaddr(192, 168, 3, 5);
IPAddress dnsaddr(192, 168, 3, 1);
IPAddress gateway(192, 168, 3, 1);
IPAddress subnet(255, 255, 255, 0);

// telnet defaults to port 23, however we will list on port 2001
EthernetServer socket(2001);


// ----------------------------------------------------------------------------
//            G L O B A L   S T O R A G E
// ----------------------------------------------------------------------------

// Display Buffer up to 64 rows and 64 columns, 512 bytes = 64 * 64 / 8
byte Displaybuf[MAXDATA] = { 0 };
char CmdOutput[ 512 ];            // the output of the command, in one packet
const char VersionString[] =
    "LED Controller, V2.0, 2015-07-13\n"
    "Copywrite (c) 2008-15, AMS Consulting\n"
    "----------\n";
/*
enum error_ids {
    NO_ERROR = 0,
    ERR_ROW,
    ERR_COL,
    ERR_PANEL,
    ERR_BRIGHT,
    ERR_TESTNUM,
};
*/

const char *error_strings[6] = {
    "OK", "ERR Row", "ERR Col", "ERR Panel", "ERR Brightness", "ERR Test Number"
};
const char *OK = "OK";

// ----------------------------------------------------------------------------
//           M A I N
// ----------------------------------------------------------------------------

/**
 * setup ... initializatin code, to run once:
 */
void setup()
{
    // initialize the LED matrix package
    LedMatrix.begin( Displaybuf, WIDTH, HEIGHT );

    // initialize the USB serial port
    Serial.begin( 9600 );

    // initialize the Ethernet
    Ethernet.begin( mac, ipaddr, dnsaddr, gateway, subnet );

    // start listening for clients
    socket.begin();

    // initialize the console logic
    console_init( CONSOLE );

    // restore the last pattern stored yesterday
    led_loaddata();
}


/**
 * loop ... main code, to run repeatedly:
 */
void loop()
{
    console_thread();
    socket_thread();
    sleep_ms( false, 1 );
}


/**
 * stream_thread .. implement the socket stream I/O gets(), puts() loop.
 *    in each call to stream_thread, we read one line, and execute one command.
 *
 * @returns
 *      returns after every command line is executed
 */
void  socket_thread( void )
{
    // the command from the host
#define MAXCMDLINE 64
    // these four variables need to be static, so we carry the state
    static char          cmd_input[ MAXCMDLINE ];
    static int           cmd_pos2 = 0;                 // 0..MAXCMDLINE-1
    static unsigned int  cmd_number = 0;
    static boolean       ignore_next_blank_line = false;

    int rc = socket_gets( cmd_input, sizeof cmd_input, &cmd_pos2 );
    // return codes from stream_gets:
    enum {
        GETS_NOCONNECTION = -2,
        GETS_NOCHAR = -1,
        GETS_PARTIAL = 0,
        GETS_FULLCMD = 1,
    };
    switch ( rc ) {
        case GETS_NOCONNECTION: // no available byte or no connection
        case GETS_NOCHAR:       // no data is available
        case GETS_PARTIAL:      // recorded a partial command line
            break;
        case GETS_FULLCMD:      // complete command line, to interpret
            char *t_rendered = display_time(millis());
            if ( (cmd_number > 0) && (strlen( cmd_input ) > 0) ) {
                // suppress the first line of the connection .. because it is garbage.
                const char *result = shell_execute( SOCKET, cmd_input );
                if ( strlen(result) ) {
                    snprintf( CmdOutput, sizeof CmdOutput, "%s | %s  ---- %s ---- \n",
                        t_rendered, cmd_input, result );
                }
                else {
                    snprintf( CmdOutput, sizeof CmdOutput, "%s | %s\n",
                        t_rendered, cmd_input );
                }
                my_puts( SOCKET, CmdOutput );
                ignore_next_blank_line = false;
            }
            else {
                // some oddity of the socket code, we get blank lines twice
                snprintf( CmdOutput, sizeof CmdOutput, "%s |\n", t_rendered );
                if ( ignore_next_blank_line ) {
                    ignore_next_blank_line = false;
                } else {
                    my_puts( SOCKET, CmdOutput );
                    ignore_next_blank_line = true;
                }
            }

            cmd_number++;
            cmd_pos2 = 0;
            memset( cmd_input, 0, sizeof cmd_input );
            break;
    }
}


/**
 * socket_gets ... this is our fgets() routine which reads a single line from
 *       the network socket
 *
 *    console_gets() reads in at most one less than size characters from the
 *    serial UART stream and and stores them into the buffer pointed to by str.
 *    Reading stops after a newline or cr or the str is filled.
 *    If a newline or cr is read, it is not stored into the buffer.
 *    A terminating null byte ('0' is stored after the last character.
 *
 *    This non-threaded version of console_gets() does not block, rather it
 *    returns immediately, that is returns after the current packet is read.
 *    The boolean result code indicates whether a full line was read
 *
 * @param str       the resulting null-terminated string
 * @param size      max size we can store, including a NULL
 * @param iLen      pointer to a count of the number of bytes read into str
 *
 * @returns int,    an enumeration of the gets-state { -2, -1, 0, 1 }
 */
int socket_gets(char *str, const int size, int *piIndex)
{
    // socket_gets can return the following, representing connection state:
    enum {
        GETS_NOCONNECTION = -2,
        GETS_NOCHAR = -1,
        GETS_PARTIAL = 0,
        GETS_FULLCMD = 1,
    };
    EthernetClient client = socket.available();
    if ( client ) {
        int c;
        while ( (c = client.read()) > 0 ) {
            // Handle backspace
            if (c == '\b') {
                if (*piIndex > 0) {
                    *piIndex--;
                }
                continue;
            }

            // Handle CR or LF
            if ((c == '\r') || (c == '\n')) {
                return GETS_FULLCMD;
            }

            // Handling potential overflow of buffer, by just returning
            if (*piIndex >= (size - 1)) {
                return GETS_FULLCMD;
            }

            // normal character
            str[*piIndex] = c;
            *piIndex = *piIndex + 1;
            str[*piIndex] = '\0';
        }
        if ( c < 0 ) {
            return GETS_NOCHAR;
        }
        if ( c == 0 ) {
            return GETS_PARTIAL;
        }
    }
    return GETS_NOCONNECTION;
}


/**
 * sleep_ms ... delays the specified number of milliseconds, however, 
 *         the display is refreshed periodically.
 *
 * @param doRefresh   should we do a refresh, before we wait?
 * @param ms        number of milliseconds to delay
 */
void sleep_ms( boolean doRefresh, unsigned int ms )
{
    unsigned long t0 = millis();

    if ( doRefresh) {
        LedMatrix.refresh();
    }

    while ( (unsigned int)(millis() - t0) < ms ) {
        delayMicroseconds(100);

#if TEST_16x64_LED_MATRIX
        LedMatrix.refresh();
#endif
    }
}


/**
 * display_time ... displays the uptime, in the format hh:mm:ss.mmm  (with millisecond precision)
 *
 * @note:        returns a static string (is not thread safe)
 *
 * @param t      number of milliseconds since the arduino microcontroller was booted
 */
char *display_time(unsigned long t)
{
	int msec = t % 1000;
	unsigned long s = t / 1000;
	int mm = (s / 60 % 60);
	int hh = (s / 3600);
	int ss = s % 60;
	static char buf[20];        // caution ... static, not thread-safe
	snprintf(buf, sizeof buf, "%02d:%02d:%02d.%03d", hh, mm, ss, msec);
	return buf;
}


/**
 * console_log ... displays a time-stamped line, typically a diagnostic line
 *        on the serial console
 *
 * @param str      the line to display
 */
void console_log( char *str )
{
    char temp[80];
    snprintf( temp, sizeof temp, "%s | %s\n", display_time( millis() ), str );
    Serial.print( temp );
}


/**
 * my_puts ... prints a single string to the stream 
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param str         the line to display
 */
void my_puts(int streamID, const char *str)
{
    switch ( streamID ) {
        case SOCKET:
            socket.write(str);
            //break;
        case CONSOLE:
            Serial.write(str);
            break;
    }
}


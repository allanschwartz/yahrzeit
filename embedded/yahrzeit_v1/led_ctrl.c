/**
 * NAME
 *      led_ctrl.c
 *
 * SYNOPSIS
 *      These C files are the code for the LED CONTROLLER:
 *          led_ctrl.c
 *          led_device.c
 *          read_switch.c
 *          ser_ir.c
 *
 * ARCHITECTURE
 *      Yahrzeit Wall - The LED array representing that many 
 *          souls to be remembered by illuminating the LEDs at
 *          at the appropriate time.
 *      Yahrzeit Appliance - LAMP (or equiv.) machine which presents
 *          a browsable GUI to the the end-user.
 *      LED Controller - The 8051-based micro-controller card
 *          which sits behind the Yahrzeit wall and controls
 *          the LED array.
 *      Yahrzeit Pixel board (YPX) - the YPX board is a small
 *          number of LEDs on a single narrow board.  We have
 *          implemented 6-pixel, 8-pixel and 10-pixel YPX boards.
 *          A string of 7 YPX boards comprise each column,
 *          and 40 columns (a total 280 YPX boards) are connected 
 *          to comprise the entire LED array and the Yahrzeit Wall.
 *          The YPX is sometimes referred to as the "light engine."
 *      Digi One SP -- The Digit One SP is a communications device
 *          which provides a TCP/IP bridge between the 
 *          LED Controller and the Yahrzeit Appliance.
 *          With this device, the Yahrzeit Wall (through the
 *          LED Controller) is a TCP/IP device, reachable with
 *          "telnet".
 *          
 * FURTHER DETAILS
 *      The LED CONTROLLER board is implemented with an Atmel 82C51AC2 chip.
 *
 *      The 89C51AC2 is an 8051-compatible 8bit microprocessor with 32K Flash.
 *      The AT89C51AC2 provides the following standard features: 32K
 *      bytes of Flash, 2KB EEPROM, 1280 bytes of RAM, 32 I/O lines, three 16-bit
 *      timer/counters, a six-vector two-level interrupt architecture,
 *      a full-duplex serial port, on-chip oscillator, and clock circuitry.
 *
 * CLI
 *      The LED CONTROLLER implements the following console commands,
 *      to facilitate the yahrzeit appliance controlling the LED array.
 *          ROws <value>
 *          COl <value>
 *          All on|off [<panel>]
 *          DAta <address> <nbytes> <hexified data>
 *          PIxel on|off <row> <col> [<panel>]
 *          TEst <testnumber> [<panel>]
 *          REfresh
 *          DEbug
 *          HElp
 *          ?
 *      
 *
 * NOTES
 *
 * HISTORY
 *      version 0.7 created for Congregation Beth Sholom, 2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * COPYRIGHT NOTICE
 *      copyright (c) 2008, by Allan M. Schwartz
 *      All rights reserved.
 *
 * BUGS
 *
 *
 * TODO
 *     [ ]  the led data array should be recalled from EEPROM
 *          on power-up; and stored in EEPROM on refresh.
 *
 *
 *
 * FUNCTION CONTENTS
 *
 *  line    Funtion Declarations
 *  ----    ------------------------------------
 *          #define STRMATCH2(s1,s2)      (strncmp(s1,s2,2)==0)
 *          #define REQUIRED_ARG(i)       (arguments[i]!=NULL)
 *          #define OPTIONAL_ARG(i)       (arguments[i]!=NULL)
 *          int  main()
 *          void  console_loop()
 *          void  parse_cmd_buffer()
 *          uint8_t  match_cmd_verb()
 *          bool    onoff_bool( char *s )
 *          uint8_t  xdigitvalue ( char c )
 *          uint8_t  xdigits_decoded( char c1, char c2 )
 *          void  led_data_cmd( unsigned int offset, int nbytes, char *he...
 *          void  hexdump( unsigned char *p, unsigned int len )
 *          void  led_debug()
 *          void  led_selftest( uint8_t testnumber, uint8_t panel, uint8_t k );
 *          void  led_selftest_corners( uint8_t panel )
 *          void  led_selftest_all_on( BIT pixelbit, uint8_t panel )
 *          void  led_selftest_flashes( uint8_t panel )
 *          void  led_selftest_marching_row( uint8_t panel )
 *          void  led_selftest_marching_col( uint8_t panel )

 */

#ifdef SDCC
#include "ser_ir.h"
#include "mcs51/at89x52.h"
#define XDATA __xdata
#define CONST  __code static const
#define NOP __asm nop __endasm;
#define BIT bit
#define SLEEP_1SEC  sleep_ms( 1000 )
#else
#define XDATA
#define CONST  static const
#define NOP
#define BIT char
#define SLEEP_1SEC  sleep( 1 )
#endif  /* SDCC */

#include "ctype.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define uint8_t      unsigned char
#define STRMATCH2(s1,s2)      (strncmp(s1,s2,2)==0)
#define REQUIRED_ARG(i)       (arguments[i]!=NULL)
#define OPTIONAL_ARG(i)       (arguments[i]!=NULL)
typedef enum { false = 0, true = 1 } bool;


// forward define functions ... to help the compiler
    int  main();
    void  console_loop();
    uint8_t  match_cmd_verb();
    void  parse_cmd_buffer();
    int     enforce_range ( int desiredval, int minlimit, int maxlimit, char *itemname );
    bool    onoff_bool( char *s );
    uint8_t  xdigitvalue ( char c );
    uint8_t  xdigits_decoded( char c1, char c2 );
    void  led_data_cmd( unsigned int offset, int nbytes, char *hexdata );
    void  led_debug();
    void  led_selftest( uint8_t testnumber, uint8_t panel, uint8_t k );
    void  led_selftest_corners( uint8_t panel );
    void  led_selftest_all_on( BIT singlebit, uint8_t panel );
    void  led_selftest_flashes( uint8_t panel );
    void  led_selftest_marching_row( uint8_t panel );
    void  led_selftest_marching_col( uint8_t panel );
    void  led_selftest_cylon( uint8_t panel );

// define as extern those things from "led_driver.c"

extern    void  led_store8_offset( uint8_t databits, unsigned int offset );
extern    void  led_store8( uint8_t databits, uint8_t row, uint8_t col);
extern    uint8_t led_data8( uint8_t row, uint8_t col );
extern    void  led_store1( BIT singlebit, uint8_t row, uint8_t col);
extern    uint8_t led_data1( uint8_t row, uint8_t col );
extern    void  led_store_in_panel( BIT singlebit, uint8_t panel, uint8_t row, uint8_t col );
extern    void  led_datarefresh( int n );
extern    void  gate_pixels_out ( unsigned char XDATA *pixeldata, int nbytes );
extern    void  putchar( char c );
extern    char  getchar();
extern    void  sleep_ms( unsigned int n );

extern    void  read_switch();

/*
 * ----------------------------------------------------------------------------
 *            G L O B A L   S T O R A G E
 * ----------------------------------------------------------------------------
 */

// global variables, these are in the __near/__data or directly addressable internal 8051 registers
#define MAXNROWS    64
uint8_t     nrows = 56;                  // number of rows
uint8_t     nrows_div_8 = 56 / 8;        // number of bytes or pixel boards in a column
#define MAXNCOLS    64
uint8_t     ncols = 40;                  // a column is 56 rows of pixel data
#define MAXBOARDS   ( MAXNROWS * MAXNCOLS / 8 )
int        nled_boards = 40*7;          // number of LED pixel boards connected to this controller
                                        // each LED pixel board is 8 pixels, or 1 byte of data
            // assert  nled_boards == ncols * ( nrows / 8 )

// not so big ... this should still be in the __near/__data
#define MAXARGS 5
char *arguments[ MAXARGS ];

// two big structures ... these must be in xdata, which is the 8052's
// on chip Expeanded Ram (XRAM).   There is just 1024 bytes of XRAM on the AT89C51
XDATA  unsigned char led_data[ MAXBOARDS ];

#define MAXCMDLINE 80
XDATA  char          cmd_buffer[ MAXCMDLINE ];

// constant character data is placed in the CODE segment
CONST char version_string[] =
"LED Controller, V0.7, 2008-06-08\n\
Copywrite (c) 2008, AMS Consulting\n\
----------";

CONST char help_string[] =
"ROws <n>\n\
COlumns <n>\n\
All  on|off\n\
DAta <byteoffset> <nbytes> <hexified data>\n\
PIxel 0|1 <row> <col> [<panel>]\n\
TEst <testnumber> [<panel>]\n\
REfresh";

CONST char test_help_string[] =
"TEst 1 [<panel>]     --   4 corners ON\n\
TEst 2 [<panel>]     --   all pixels ON\n\
TEst 3 [<panel>]     --   all pixels OFF\n\
TEst 4 [<panel>] [k] --   all ON / all OFF\n\
TEst 5 [<panel>] [k] --   marching row pattern\n\
TEst 6 [<panel>] [k] --   marching column pattern\n\
TEst 7 [<panel>] [k] --   Cylon pattern\n\
TEst 8 [<panel>] [k] --   lots of tests";

extern CONST  uint8_t led_row_of_panel[22];
extern CONST  uint8_t led_col_of_panel[22];
extern CONST  uint8_t nrows_perpanel[22];
extern CONST  uint8_t ncols_perpanel[22];


/*
 * ----------------------------------------------------------------------------
 *            M A I N
 * ----------------------------------------------------------------------------
 */

/**
 * MAIN ... execution starts here.
 *
 * @TODO
 *      - [DONE] experiment with other serial port settings
 *      - [DONE] make sure interrupt driven character I/O works
 *      - [  ]   experiment with a watchdog timer
 *      - [DONE] experiment with a microsleep() function
 *      - [DONE] does the XRAM have to be software selected to full size?
 *              (crt does this)
 *      - [  ]  play with LCD display, make that work.
 *
 * @NOTE
 *      uses the following library functions:()
 *          ser_init(), memset(), puts()
 *
 * @returns n/a
 */
int  main()
{
#ifdef SDCC
    // initialize 232 serial port
    ser_init();
#endif

    // initialize global data structures
    memset( led_data, 0, sizeof led_data );

    // splash
    puts( version_string );

    // some initial pattern, on power-up (as a self-test)
    led_selftest_all_on( 0, /* panel */ 0);         // all off
    led_selftest_marching_row( /* panel */ 0 );
    led_selftest_marching_col( /* panel */ 0 );

    // drop into main loop.  never return.
    console_loop();
    return 0;
}


/*
 * ----------------------------------------------------------------------------
 *            C O N S O L E
 * ----------------------------------------------------------------------------
 */

enum cmd_ids  {
    CMD_ROWS    = 1,
    CMD_COLS    = 2,
    CMD_DATA    = 3,
    CMD_PIXEL   = 4,
    CMD_TEST    = 5,
    CMD_REFRESH = 6,
    CMD_DEBUG   = 7,
    CMD_HELP    = 8,
    CMD_NOP     = 9,
    CMD_ALL     = 10,
};

const struct cmd_s {
    uint8_t  cmd_identifier;            // see enum cmd_ids above
    uint8_t  cmd_required_args;
    const char * cmd_string;
} commands[] = {
    {   CMD_ROWS, 1, "rows" },
    {   CMD_COLS, 1, "cols" },
    {   CMD_DATA, 3, "data" },
    {   CMD_PIXEL, 3, "pixel" },
    {   CMD_TEST, 1, "test" },
    {   CMD_REFRESH, 0, "refresh" },
    {   CMD_DEBUG, 0, "debug" },
    {   CMD_HELP, 0, "help" },
    {   CMD_NOP, 0, "nop" },    {   CMD_ALL, 1, "all" },
//    {   CMD_HELP, 0, "?" },
    {   0, 0, 0 }
};


/**
 * console_loop .. implement the console i/o gets(), puts() loop.
 *      Execution falls into here from main and never returns.
 *
 * @COMMAND LANGUAGE
 *      We have implemented a simple command interpreter that recognizes
 *      several commands (which can be abbrieviated to 2 characters).
 *      The commands supported are:
 *
 *          ROws <value>
 *          COl <value>
 *          DAta <address> <nbytes> <hexified data>
 *          All on|off [<panel>]
 *          PIxel on|off <row> <col> [<panel>]
 *          TEst <testnumber> [<panel>]
 *          REfresh
 *          DEbug
 *          HElp
 *          ?
 *
 * @NOTE
 *      uses the following library functions:
 *          memset(), atoi(), gets(), putchar(), puts(), printf(), strncmp()
 *
 * @RETURN
 *      no return from console_loop
 */
void  console_loop()
{
    int arg1 = 0;
    int arg2 = 0;
    int arg3 = 0;

    while (1) {
        putchar('>'); putchar(' ');
        memset( cmd_buffer, 0, sizeof cmd_buffer );
        //
        // check the buttons
        // the buttons are only checked between command lines
        read_switch();

        ser_gets( cmd_buffer, sizeof cmd_buffer );
        parse_cmd_buffer();         // array arguments[] assigned pointers to tokens

        // as an code-size optimization, lets convert argv[1] & argv[2] from ascii to an int
        if ( OPTIONAL_ARG(1) ) {
            arg1 = atoi( arguments[1] );
        }
        if ( OPTIONAL_ARG(2) ) {
            arg2 = atoi( arguments[2] );
        }
        if ( OPTIONAL_ARG(3) ) {
            arg3 = atoi( arguments[3] );
        }

        if ( arguments[0] == NULL ) {
            //puts("eh?");
            continue;
        }
        if ( arguments[0][0] == '?' ) {
            goto help_user;
        }


        switch ( match_cmd_verb() ) {
            case CMD_ROWS:
                // rows <value>
                nrows = enforce_range( arg1, 1, MAXNROWS, "rows" );
                nrows_div_8 = nrows / 8;
                nled_boards = ncols * nrows_div_8;
                break;

            case CMD_COLS:
                // columns <value>
                ncols = enforce_range( arg1, 1, MAXNCOLS, "columns" );
                nled_boards = ncols * nrows_div_8;
                break;

            case CMD_ALL:
                // all on|off [<panel>]
                led_selftest_all_on( 
                            onoff_bool( arguments[1] ),  /* "on"|"off" or "0"|"1" */
                            OPTIONAL_ARG(2) ? arg2 : 0  /* optional <panel> */
                            );
                break;

            case CMD_DATA:
                // data <address> <length> <hex-data>
                led_data_cmd( arg1,                 /* address */
                              arg2,                 /* length */
                              arguments[3] );       /* hexified data */
                break;

            case CMD_PIXEL:
                // pixel 0|1 <row> <col> [<panel>]
                if ( OPTIONAL_ARG(4) ) {
                    led_store_in_panel(
                            onoff_bool( arguments[1] ),  /* "on"|"off" or "0"|"1" */
                            atoi( arguments[4] ),   /* panel */
                            arg2,                   /* row */
                            arg3 );                 /* column */
                } else {
                    led_store1(
                            onoff_bool( arguments[1] ),  /* "on"|"off" or "0"|"1" */
                            arg2,                   /* row */
                            arg3 );                 /* column */
                }
                break;

            case CMD_TEST:
                // test <testnumber> [<panel>]
                if ( REQUIRED_ARG(1) ) {
                    led_selftest( 
                            arg1,                           /* test number */
                            OPTIONAL_ARG(2) ? arg2 : 0 ,    /* optional <panel> */
                            OPTIONAL_ARG(3) ? arg3 : 3 );   /* optional repeat */
                } else {
                    puts ( test_help_string );
                }
                break;

            case CMD_REFRESH:
                // refresh [<nbytes>]
                led_datarefresh( OPTIONAL_ARG(1) ? arg1 : 0 );
                break;

            case CMD_DEBUG:
                // debug
                led_debug();
                break;

            case CMD_HELP:
                // help or ?
help_user:
                puts( version_string );
                puts( help_string );
                break;

            case CMD_NOP:
                break;

            case 0:
            default:
                puts("eh?");
                break;
        }
    }
}


/**
 * match command verb
 *    uses the globals arguments and the table commands
 *
 * @returns unique command identifier
 */
uint8_t  match_cmd_verb()
{
    struct cmd_s *p;
    for( p = &commands[0]; *p->cmd_string; p++ ) {
        if ( STRMATCH2( arguments[0], p->cmd_string ) ) {
            if ( REQUIRED_ARG(p->cmd_required_args) ) {
                return p->cmd_identifier;
            } else {
                puts( "missing arg" );
                return 0;
            }
        }
    }
    return 0;
}


/**
 * parse_cmd_bufer ... function to break apart the arguments
 *    uses the globals cmd_buffer and arguments
 *
 * @returns n/a
 */
void  parse_cmd_buffer()
{
    uint8_t i;
    char XDATA *p = cmd_buffer;

    memset( arguments, 0, sizeof arguments );
    for( i = 0; i < MAXARGS; i++ ) {
        // skip white space
        while ( *p && isspace(*p) ) {
            p++;
        }
        if ( *p == 0 )
            return;
        arguments[i] = p;       // record where arg starts
        // walk over this arg
        while( *p && !isspace(*p) ) {
            p++;
        }
        if ( *p == 0 )
            return;
    }
}


/**
 * enforce_range -- set an integer within a bound minlimit..maxlimit
 *
 * @returns the desired value or the max
 */
int     enforce_range ( int desiredval, int minlimit, int maxlimit, char *itemname )
{
    if ( desiredval < minlimit ) {
        printf ( "%s must be between %d and %d.  Setting to %d\n",
            itemname, minlimit, maxlimit, minlimit );
        return minlimit;
    }
    if ( desiredval > maxlimit ) {
        printf ( "%s must be between %d and %d.  Setting to %d\n",
            itemname, minlimit, maxlimit, maxlimit );
        return maxlimit;
    }
    return desiredval;
}


/**
 * onoff_bool -- convert a string representing on/off, or true/false, or 1/0 
 * to a bool
 *
 * @returns true or false
 */
bool    onoff_bool( char *s )
{
    if ( s == NULL ) 
        return false;
    if ( STRMATCH2( s, "true" ) ) 
        return true;
    if ( STRMATCH2( s, "on" ) ) 
        return true;
    if ( STRMATCH2( s, "false" ) ) 
        return false;
    if ( STRMATCH2( s, "off" ) ) 
        return false;
    if ( isdigit( *s ) ) 
        return( atoi( s ) );
        
    return false;
}

/**
 * xdigitvalue -- dehexify an ascii character representing 4-bits
 *
 * @returns the 4-bit value of the hex digit
 */
uint8_t  xdigitvalue( char c )
{
    uint8_t a;

    if ( isdigit( c ) )
        a = c - '0';
    else if ( 'a' <= c && c <= 'f' )
        a = c - 'a' + 10;
    else if ( 'A' <= c && c <= 'F' )
        a = c - 'A' + 10;
    else
        a = 0;
    return ( a );
}


/*
 * xdigits_decoded -- 2 hex digits decoded into a single byte
 *
 * @returns the 8-bit value of the 2 hex digits
 */
uint8_t  xdigits_decoded( char c1, char c2 )
{
    uint8_t a, b;

    a = xdigitvalue( c1 );
    b = xdigitvalue( c2 );
    return ( a << 4 | b );
}


/**
 * led_data_cmd ... performs the action of the "DATA" command.
 *    The given data is decoded (dehexified, stored into led data XRAM data,
 *    then the display is refreshed.
 *
 * @param offset    offset in pixels into the LED array
 * @param bytes     number of bytes (each representing 8 pixels) provided
 * @param hexdata   hexified data. i.e., hex data encoded in ascii chars
 *
 * @returns n/a
 */
void  led_data_cmd( unsigned int offset, int nbytes, char *hexdata )
{
    char *p;
    char c1, c2;
    uint8_t x;

    if ( offset >= (nrows*ncols) ) {
        puts( "ERR addr" );
        return;
    }
    if ( nbytes < 1 || (offset + nbytes) >= (ncols*nrows_div_8) ) {
        puts( "ERR len" );
        return;
    }

    for ( p = hexdata; nbytes > 0; nbytes--, offset++ ) {
        c1 = *p++;
        c2 = *p++;
        if ( !isxdigit(c1)  || !isxdigit(c2) ) {
            puts( "ERR non-hex" );
            return;
        }
        x = xdigits_decoded( c1, c2 );
        led_store8_offset( x, offset );
    }
    //led_datarefresh( 0 );
    puts( "OK" );
}


#if 0
/**
 * hexdump -- a general purpose routine to dump a section of memory in hex
 */
void  hexdump( unsigned char *p, unsigned int len )
{
    for( offset = 0; len; offset++, len-- ) {
        if ( (offset & 31 )  == 0 )  {
            printf( "%04x: ", offset );
        }
        printf ( "%02x ", p[offset] );
    }
    printf( "\n" );
}
#endif


/**
 * led_debug ... debug routine to display most of the key global variables,
 *     and especially the LED data area
 */
void  led_debug()
{
    uint8_t row, col;

    printf( "DEBUG data nrows %d nrows_div_8 %d ncols %d nled_boards %d\n" ,
                        nrows,   nrows_div_8,   ncols,   nled_boards );
    puts( "led_data:" );

    // dump_led_data ... display the LED data in hex
    for( col = 1; col <= ncols; col++ ) {
        if ( col > 1 ) {
            puts("");
        }
        printf( "%04d: ", col );
        for( row = 1; row <= nrows; row += 8 ) {
            printf ( "%02x ", led_data8( row, col ) );
        }
        col++;
        printf( "  %04d: ", col );
        for( row = 1; row <= nrows; row += 8 ) {
            printf ( "%02x ", led_data8( row, col ) );
        }
    }
    printf( "\n" );
    puts( "OK" );
}


/*
 * ----------------------------------------------------------------------------
 *            S E L F    T E S T
 * ----------------------------------------------------------------------------
 */


/**
 * SELF TEST DISPATCH
 *
 * @TEST PATTERNS
 *      We have implemented the following 7 test patterns.
 *
 *      The commands supported are:
 *
 *      TEst 1 [<panel>] --   4 corners ON
 *      TEst 2 [<panel>] --   all pixels ON
 *      TEst 3 [<panel>] --   all pixels OFF
 *      TEst 4 [<panel>] --   all ON / all OFF
 *      TEst 5 [<panel>] --   marching row pattern
 *      TEst 6 [<panel>] --   marching column pattern
 *      TEst 7 [<panel>] --   Cylon pattern
 *
 *      <panel> omitted means apply the test to the entire pixel array
 *
 * @param testnumber    testnumber, see above
 * @param panel     which panel 1..21 (0 means using entire LED arrary)
 * @param k         repeat count
 *
 * @returns n/a
 */
void  led_selftest( uint8_t testnumber, uint8_t panel, uint8_t k )
{
    for ( ; k; k-- ) {
        switch ( testnumber ) {
            case 1:
                led_selftest_corners( panel);
                break;
            case 2:
                led_selftest_all_on( 1, panel);
                break;
            case 3:
                led_selftest_all_on( 0, panel);
                break;
            case 4:
                led_selftest_flashes( panel );
                break;
            case 5:
                led_selftest_marching_row( panel );
                break;
            case 6:
                led_selftest_marching_col( panel );
                break;
            case 7:
                led_selftest_cylon( panel);
                break;
            case 8:
                led_selftest_corners( panel);
                led_selftest_marching_row( panel );
                led_selftest_marching_col( panel );
                led_selftest_cylon( panel);
                break;
            default:
            case 0:
                puts("eh?");
                break;
        }
    }
    puts( "OK" );
}


/**
 * SELF TEST 1, for a panel ... turn corner LEDs on
 */
void  led_selftest_corners( uint8_t panel )
{
    printf( "selftest 1 - corners LEDs on,  panel %d\n", panel );
    led_datarefresh( 0 );
    if ( panel == 0 ) {
                       // bit, row, col
        led_store1( 1, 1, 1 );
        led_store1( 1, nrows, 1 );
        led_store1( 1, 1, ncols );
        led_store1( 1, nrows, nrows );
    } else {
                       // bit, panel, row, col
        led_store_in_panel( 1, panel, 1, 1 );
        led_store_in_panel( 1, panel, nrows_perpanel[panel], 1 );
        led_store_in_panel( 1, panel, 1,  ncols_perpanel[panel] );
        led_store_in_panel( 1, panel, nrows_perpanel[panel], ncols_perpanel[panel] );
    }
    led_datarefresh( 0 );
}


/**
 * SELF TEST 2, 3: for a panel, turn all LEDs on / off
 */
void  led_selftest_all_on( BIT pixelbit, uint8_t panel )
{
    uint8_t row, col;

    //printf( "selftest %d - turn all pixels %s, panel %d\n",
    //        pixelbit ? 2 : 3, pixelbit ? "ON" : "OFF", panel );
    if ( panel == 0 ) {
        for ( col = 1; col <= ncols; col++ ) {
            for ( row = 1; row <= nrows; row++ ) {
                led_store1( pixelbit, row, col );
            }
        }
    } else {
        for ( col = 1; col <= ncols_perpanel[panel]; col++ ) {
            for ( row = 1; row <= nrows_perpanel[panel]; row++ ) {
                led_store_in_panel( pixelbit, panel, row, col );
            }
        }
    }
    led_datarefresh( 0 );
}


/**
 * SELF TEST 4: all LEDs on, then off
 */
void  led_selftest_flashes( uint8_t panel )
{
    printf( "selftest 4 - all pixels ON then OFF, panel %d\n", panel );

    led_selftest_all_on( 1, panel );
    SLEEP_1SEC;

    led_selftest_all_on( 0, panel );
    SLEEP_1SEC;
}


/**
 * SELF TEST 5: marching row pattern
 */
void  led_selftest_marching_row( uint8_t panel )
{
    uint8_t row, col;

    printf( "selftest 5 - marching row pattern, panel %d\n", panel );

        if ( panel == 0 ) {
            for ( row = 1; row <= nrows; row++ ) {
                for ( col = 1; col <= ncols; col++ ) {
                    led_store1( 1, row, col );
                }
                led_datarefresh( 0 );
                sleep_ms( 100 );            // 250 ms

                for ( col = 1; col <= ncols; col++ ) {
                    led_store1( 0, row, col );
                }
                led_datarefresh( 0 );
            }
        } else {
            for ( row = 1; row <= nrows_perpanel[panel]; row++ ) {
                for ( col = 1; col <= ncols_perpanel[panel]; col++ ) {
                    led_store_in_panel( 1, panel, row, col );
                }
                led_datarefresh( 0 );
                sleep_ms( 100 );            // 250 ms

                for ( col = 1; col <= ncols_perpanel[panel]; col++ ) {
                    led_store_in_panel( 0, panel, row, col );
                }
                led_datarefresh( 0 );
            }
        }
}


/**
 * SELF TEST 6: marching column pattern, repeat 3 times
 */
void  led_selftest_marching_col( uint8_t panel )
{
    uint8_t row, col;

    printf( "selftest 6 - marching column pattern, panel %d\n", panel );

        if ( panel == 0 ) {
            for ( col = 1; col <= ncols; col++ ) {
                for ( row = 1; row <= nrows; row++ ) {
                    led_store1( 1, row, col );
                }
                led_datarefresh( 0 );
                sleep_ms( 100 );            // 250 ms

                for ( row = 1; row <= nrows; row++ ) {
                    led_store1( 0, row, col );
                }
                led_datarefresh( 0 );
            }
        } else {
            for ( col = 1; col <= ncols_perpanel[panel]; col++ ) {
                for ( row = 1; row <= nrows_perpanel[panel]; row++ ) {
                    led_store_in_panel( 1, panel, row, col );
                }
                led_datarefresh( 0 );
                sleep_ms( 100 );            // 250 ms

                for ( row = 1; row <= nrows_perpanel[panel]; row++ ) {
                    led_store_in_panel( 0, panel, row, col );
                }
                led_datarefresh( 0 );
            }
        }

}


/**
 * SELF TEST 7: Cylon pattern
 */
void  led_selftest_cylon( uint8_t panel )
{
    uint8_t row, col;

    printf( "selftest 7 - Cylon pattern, panel %d\n", panel );
#if 0
        if ( panel == 0 ) {
            for ( col = 1; col <= ncols; col++ ) {
                for ( row = 1; row <= (nrows-1); row++ ) {
                    led_store1( 1, row, col );
                    led_store1( 1, row+1, col );
                    led_datarefresh( 0 );
                    sleep_ms( 50 );            // 50 ms
                    led_store1( 0, row, col );
                    led_store1( 0, row+1, col );
                }
                for ( row = nrows; row > 1; row-- ) {
                    led_store1( 1, row, col );
                    led_store1( 1, row-1, col );
                    led_datarefresh( 0 );
                    sleep_ms( 50 );            // 50 ms
                    led_store1( 0, row, col );
                    led_store1( 0, row-1, col );
                }
                led_datarefresh( 0 );
                sleep_ms( 250 );            // 250 ms
            }
        } else {
#endif
            for ( col = 1; col <= ncols_perpanel[panel]; col++ ) {
                for ( row = 1; row <= (nrows_perpanel[panel] - 1); row++ ) {
                    led_store_in_panel( 1, panel, row, col );
                    led_store_in_panel( 1, panel, row+1, col );
                    led_datarefresh( 0 );
                    sleep_ms( 50 );            // 50 ms
                    led_store_in_panel( 0, panel, row, col );
                    led_store_in_panel( 0, panel, row+1, col );
                }
                for ( row = nrows_perpanel[panel]; row > 1; row-- ) {
                    led_store_in_panel( 1, panel, row, col );
                    led_store_in_panel( 1, panel, row-1, col );
                    led_datarefresh( 0 );
                    sleep_ms( 50 );            // 50 ms
                    led_store_in_panel( 0, panel, row, col );
                    led_store_in_panel( 0, panel, row-1, col );
                }
                led_datarefresh( 0 );
                sleep_ms( 250 );            // 250 ms
            }
#if 0
        }
#endif

}



/**
 * @file        yahrzeit_r2.h
 *
 * @brief       overall C header file for the Yahrzeit Embedded Controller
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 */

/**
 * ARCHITECTURE
 *      Yahrzeit Wall - The LED array representing the many souls to
 *          be remembered, by illuminating LEDs at at the appropriate time.
 *
 *      Yahrzeit Appliance - LAMP (or equiv.) machine which presents
 *          a browsable GUI to the the end-user.
 *
 *      Yahrzeit Embedded Controller - The Arduino based microcontroller 
 *          which sits behind the Yahrzeit wall and controls the LED array.
 *        
 *      Yahrzeit Pixel board (YPX) - the YPX board is a small
 *          number of LEDs on a single narrow board.  We have implemented
 *          6-pixel, 8-pixel and 10-pixel YPX boards.  A string of 7 YPX
 *          boards comprise each column, and 40 columns (a total 280 YPX
 *          boards) are connected to comprise the entire LED array and the
 *          Yahrzeit Wall.  The YPX is referred to in this code as the
 *          "yyz_pixel" board.
 *        
 *      Arduino Ethenet Shield -- The Ethernet communications device
 *          which provides a TCP/IP connection between the Embedded
 *          Controller and the Yahrzeit Appliance.  With this device, the
 *          Yahrzeit Wall (the Yahrzeit Embedded Controller) is reachable with
 *          "telnet" or "nc", at a particular IP address and port number.
 *          
 * FURTHER DETAILS
 *      Version 1 of the Yahrzeit Embedded Controller was implemented with an 
 *      Atmel 89C51ED2 custom board.
 *
 *      The 89C51ED2 is an 8051-compatible 8bit microprocessor with 32K Flash.
 *      The AT89C51ED2 provides the following standard features: 32K
 *      bytes of Flash, 2KB EEPROM, 256 bytes of SRAM, 1792 bytes of XRAM, 
 *      32 I/O lines, three 16-bit timer/counters, full-duplex serial port, 
 *      on-chip oscillator, and clock circuitry.
 *
 *      Version 2 of the Yahrzeit Embedded Controller is implemented with
 *      a Arduino Mega, using a ATmega2560 chip.  This chip has 54 Digital
 *      I/O pins, 256 KB of Flash Memory, 8KB SRAM, 4 KB EEPROM, and runs
 *      at a 16 MHz clock speed.
 *
 *      However, the most important feature of the Arduino family is the
 *      open-source hardware design, the easy-to-use development enviroment,
 *      and the well supported libraries.
 *
 * CLI
 *      The LED CONTROLLER implements the following console commands,
 *      to facilitate the yahrzeit appliance controlling the LED array.
 *          All on|off [<panel>]
 *          BRightness <n> (1:low, 10:high)
 *          DAta <row> <col> <binary data>
 *          DUmp
 *          HElp
 *          LOad
 *          PIxel on|off <row> <col> [<panel>]
 *          REfresh
 *          SAve
 *          TEst <testnumber> [<panel>]
 */

#ifndef __YAHRZEIT_R2_H__
#define __YAHRZEIT_R2_H__

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
//#include "stdlib.h"
//#include "string.h"
#define STRMATCH2(s1,s2)      (strncmp(s1,s2,2) == 0)

enum { PANEL0 = 0 };

enum streamIds { CONSOLE = 1, SOCKET = 2 };

enum commandIds  {
    NONE_OF_THE_ABOVE = 0,
    CMD_ALL = 1,        // turn on/off all LEDs
    CMD_BRIGHT,         // set the brightness
    CMD_DATA,           // set a specific data bit pattern
    CMD_DUMP,           // dump the pixel memory
    CMD_HELP,           // display command help
    CMD_LOAD,           // load the pixel memory from EEPROM
    CMD_PIXEL,          // set a pixel memory on/off
    CMD_REFRESH,        // refresh the LED display from pixel memory
    CMD_SAVE,           // store the pixel memory into EEPROM
    CMD_TEST,           // do one of several LED test pattern
    CMD_NOP,            // (was required on the slower 8051 inplementation)
    MISSING_ARG = 255
};

enum ResultIds {
    NO_ERROR = 0, ERR_SYNTAX, ERR_MISSING, ERR_ROW,
    ERR_COL, ERR_PANEL, ERR_BRIGHT, ERR_TESTNUM,
};

extern const char *ResultStrings[8];
    

    
// ----------------------------------------------------------------------------
//            F U N C T I O N   P R O T O T Y P E S
// ----------------------------------------------------------------------------

// define as extern the public functions defined in yahrzeit_r2.c
void    socket_thread( void );
int     socket_gets( char *input, const unsigned int maxsize, int *piIndex );
void    sleep_ms( boolean, const unsigned int ms );
void    console_log( char *msg );
void    my_puts( byte streamID, char *msg );
char   *display_uptime( void );

// define as extern the public functions defined in "led_console.c"
void    console_init( void );
void    console_thread( void );
boolean console_gets( char *input, const unsigned int maxsize, int *piIndex );
const char *  shell_execute( byte streamID, char *command );
void    parse_command( char *command, char **argv );
boolean onoff_bool( char *token );
int     console_data_cmd( byte row, byte col, char *bindata );
void    hexdump( byte streamID, void *addr, const unsigned int len );
int     console_dump( byte streamID, byte panel );

// define as extern the public functions defined in "led_device.c"
void    led_init( void );
int     led_store_in_array( boolean pixelbit, byte row, byte col );
boolean led_pixel_value( byte row, byte col );
int     led_store_in_panel( boolean pixelbit, byte row, byte col, byte panel );
void    led_savedata( void );
void    led_loaddata( void );
int     led_set_intensity( byte intensity );
int     led_all_on( boolean pixelbit, byte panel );
extern const  byte led_row_of_panel[];
extern const  byte led_col_of_panel[];
extern const  byte nrows_perpanel[];
extern const  byte ncols_perpanel[];

// define as extern the public functions defined in "selftest.h"
void  selftest_description( byte streamID, byte testNumber, const char * testDescription, byte panel );
int   selftest( byte streamID, byte testnumber, byte panel, int repeat );
void  selftest_marching_row( byte panel );


// ----------------------------------------------------------------------------
//            L E D   M A T R I X   S I Z E
// ----------------------------------------------------------------------------


        // one of the following three lines must be uncommented
#define CBS_56x40_WALL    1
//#define TEST_16x64_LED_MATRIX    1
//#define TEST_2x24_HARNESS    1


#if CBS_56x40_WALL

#define NPANELS     21                  // At Beth Shalom, we have 3 * 7 panels
#define NROWS       56                  // number of rows
#define NCOLS       40                  // a column is 56 rows of consecutive pixel data

#endif

#if TEST_16x64_LED_MATRIX

#define NPANELS     10                  // In this test-harness we have 2 * 5 panels
#define NROWS       16
#define NCOLS       40                  // a column is 40 rows of consecutive pixel data

#endif

#if TEST_2x24_HARNESS

#define NPANELS     1                   // In this test-harness we have 1 panels
#define NROWS       24                  // three 8-pixel yyz_pixel boards is 24 rows
#define NCOLS       2                   // and we only have two columns

#endif

#define MAXNROWS    64
#define MAXNCOLS    64
#define MAXDATA     ( MAXNROWS * MAXNCOLS / 8 )       // 512 bytes of data

#endif  // __YAHRZEIT_R2_H__

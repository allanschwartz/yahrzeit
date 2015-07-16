/**
 * NAME
 *      yahrzeit_r2.h
 *
 * SYNOPSIS
 *      These C files are the code for the LED CONTROLLER:
 *          yahrzeit_r2.c
 *          led_device.c
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
 */
 
#ifndef __YAHRZEIT_R2_H__
#define __YAHRZEIT_R2_H__


#define SLEEP_1SEC  sleep_ms( 1000 )
#define SLEEP_5SEC  sleep_ms( 5000 )


#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
//#include "stdlib.h"
//#include "string.h"
#define STRMATCH2(s1,s2)      (strncmp(s1,s2,2) == 0)


// ----------------------------------------------------------------------------
//            F U N C T I O N   P R O T O T Y P E S
// ----------------------------------------------------------------------------

// define as extern those functions defined in yahrzeit_r2.c
void sleep_ms( bool, unsigned int ms );

// define as extern those things from "led_console.c"
void    console_init(void);
void    console_thread(void);
boolean console_gets(char *str, const int size, int *iLen);
void    console_execute(char *str);
void    parse_command(char *str, char **argv);
boolean onoff_bool( char *s );
void    console_data_cmd( byte row, int col, char *bindata );
void    led_debug();

// define as extern those things from "led_device.c"
void    led_init(void);
void    led_store1( boolean singlebit, byte row, byte col);
boolean led_data1( byte row, byte col );
void    led_store_in_panel( boolean singlebit, byte panel, byte row, byte col );;
void    led_savedata( void );
void    led_loaddata( void );
void    led_set_intensity(byte intensity);
extern const  byte led_row_of_panel[];
extern const  byte led_col_of_panel[];
extern const  byte nrows_perpanel[];
extern const  byte ncols_perpanel[];

// define as extern those things from "selftest.h"
void  selftest( byte testnumber, byte panel, byte k );
void  selftest_all_on( boolean singlebit, byte panel );
void  selftest_marching_row( byte panel );


// ----------------------------------------------------------------------------
//            L E D   M A T R I X   S I Z E
// ----------------------------------------------------------------------------

 
        // one of the following three lines must be uncommented
//#define CBS_56x40_WALL    1          
//#define TEST_16x64_LED_MATRIX    1 
#define TEST_2x24_HARNESS    1 


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
#define NROWS       24                  // three 8-pixel yyz_pixel board is 24 rows
#define NCOLS       2                   // and we only have two columns

#endif

#define MAXNROWS    64
#define MAXNCOLS    64
#define MAXDATA     ( MAXNROWS * MAXNCOLS / 8 )       // 512 bytes of data

#endif  // __YAHRZEIT_R2_H__

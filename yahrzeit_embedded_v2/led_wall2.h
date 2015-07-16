/**
 * NAME
 *      led_wall2.h
 *
 * SYNOPSIS
 *      These C files are the code for the LED CONTROLLER:
 *          led_wall2.c
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
 * BUGS
 *
 *
 * TODO
 *     [ ]  the led data array should be recalled from EEPROM
 *          on power-up; and stored in EEPROM on refresh.
 *
 */
 
#ifndef __LED_WALL2_H__
#define __LED_WALL2_H__



#define SLEEP_1SEC  sleep_ms( 1000 )
#define SLEEP_5SEC  sleep_ms( 5000 )


#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
//#include "stdlib.h"
//#include "string.h"

#define STRMATCH2(s1,s2)      (strncmp(s1,s2,2)==0)
#define REQUIRED_ARG(i)       (arguments[i]!=NULL)
#define OPTIONAL_ARG(i)       (arguments[i]!=NULL)

// define as extern those things from "led_console.c"
#include "led_console.h"

// define as extern those things from "led_device.c"
#include "led_device.h"

// define as extern those things from "led_selftest.h"
#include "led_selftest.h"

/*
 * ----------------------------------------------------------------------------
 *            G L O B A L   S T O R A G E
 * ----------------------------------------------------------------------------
 */

#if CBS
// global variables, these are in the __near/__data or directly addressable internal 8051 registers
#define NPANELS     21                   // At Beth Shalom, we have 3 * 7 panels
#define MAXNROWS    64
#define NROWS       56                  // number of rows
#define MAXNCOLS    64
#define NCOLS       40                  // a column is 56 rows of consecutive pixel data
#define MAXBOARDS   ( MAXNROWS * MAXNCOLS / 8 )
#define MAXDATA     ( NROWS * NCOLS / 8 )

#else    // TESTHARNESS

#define NPANELS     10                  // In the test-harness we have 2 * 5 panels      
#define MAXNROWS    64
#define NROWS       16
#define MAXNCOLS    64
#define NCOLS       40                  // a column is 56 rows of consecutive pixel data
#define MAXBOARDS   ( MAXNROWS * MAXNCOLS / 8 )
#define MAXDATA     ( NROWS * NCOLS / 8 )

#endif
            // assert  nled_boards == ncols * ( nrows / 8 )

#define MAXARGS 5
extern  char *arguments[ MAXARGS ];

#define MAXCMDLINE 64
extern  char  cmd_buffer[ MAXCMDLINE ];

#endif


/**
 * @file        yahrzeit_v3.h
 *
 * @brief       Overall project header for the Yahrzeit Embedded Controller
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 3.0 revised in April 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 *
 * @notes       see project notes in file yahrzeit_v3.h
 */

/**
 * ARCHITECTURE
 *      Yahrzeit Wall - The LED array representing the many souls to
 *          be remembered, by illuminating LEDs at the appropriate time.
 *
 *      Yahrzeit Appliance - LAMP (or equiv.) machine which presents
 *          a browsable GUI to the end-user.
 *
 *      Yahrzeit Embedded Controller - The Arduino-based microcontroller 
 *          which sits behind the Yahrzeit wall and controls the LED array.
 *          The V2 implementation was an Arduino Mega.  
 *        
 *      Yahrzeit Pixel board (YPX) - the YPX board is a small
 *          number of LEDs on a single narrow board.  We have implemented
 *          6-pixel, 8-pixel and 10-pixel YPX boards.  A string of 7 YPX
 *          boards comprise each column, and 40 columns (a total 280 YPX
 *          boards) are connected to comprise the entire LED array and the
 *          Yahrzeit Wall.  The YPX is referred to in this code as the
 *          "YyzPixel" board.
 *        
 *      Arduino Ethernet Shield -- The Ethernet communications device
 *          which provides a TCP/IP connection between the Embedded
 *          Controller and the Yahrzeit Appliance.  With this device, the
 *          Yahrzeit Wall (the Yahrzeit Embedded Controller) is reachable with
 *          "telnet" or "nc", at a particular IP address and port number.
 *          
 * FURTHER DETAILS
 *
 *      Version 1 of the Yahrzeit Embedded Controller was implemented
 *      using a custom board based on the Atmel AT89C51ED2.
 *
 *      The AT89C51ED2 is an 8051-compatible 8-bit microcontroller with
 *      32 KB Flash, 2 KB EEPROM, 256 bytes SRAM, 1792 bytes XRAM,
 *      32 I/O lines, three 16-bit timer/counters, and a full-duplex
 *      serial interface.
 *
 *
 *      Version 2 of the Yahrzeit Embedded Controller was implemented
 *      using an Arduino Mega based on the ATmega2560.
 *
 *      The ATmega2560 provides 54 digital I/O pins, 256 KB Flash,
 *      8 KB SRAM, 4 KB EEPROM, and operates at 16 MHz.
 *
 *
 *      Version 3 of the Yahrzeit Embedded Controller is implemented
 *      using an Arduino Uno R4 Minima based on the Renesas RA4M1.
 *
 *      The RA4M1 is a 32-bit ARM Cortex-M4 microcontroller operating
 *      at 48 MHz, with 256 KB Flash, 32 KB SRAM, USB connectivity,
 *      hardware SPI support, and 5-volt-compatible I/O.
 *
 *      However, the most important feature of the Arduino family is the
 *      open-source hardware design, the easy-to-use development environment,
 *      and the well supported libraries.
 *
 * CLI
 *      The LED CONTROLLER implements the following console commands,
 *      to facilitate the yahrzeit appliance controlling the LED array.
 *
	        All  on|off [<panel>]
	        BRightness <n> (1:bright, 254:dim)
	        CLear <panel>
	        DAta <row> <col> <binary data>
	        DUmp [<panel>]
	        HElp
	        LOad
	        PIxel on|off <row> <col> [<panel>]
	        REfresh
	        SAve
	        STatus
	        TEst <testnumber> [<panel>]
	        VErsion
 */

#pragma once
#include <Arduino.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <EEPROM.h>
#include <Ethernet.h>

// include all of our project headers, including our classes 
#include "CmdProc.h"
#include "LedWall.h"
#include "selftest.h"
#include "serial_thread.h"
#include "socket_thread.h"
#include "YyzPixel.h"


// ----------------------------------------------------------------------------
//            L E D   M A T R I X   S I Z E
// ----------------------------------------------------------------------------

        // one of the following two lines must be uncommented
        // this declares the size of Pixel matrix
// #define CBS_56x40_WALL    1
#define TEST_3x24_HARNESS    1


#if CBS_56x40_WALL

constexpr byte NPANELS = 21;                  // At Beth Shalom, we have 3 * 7 panels
constexpr byte NROWS   = 56;                  // number of rows
constexpr byte NCOLS   = 40;                  // a column is 56 rows of consecutive pixel data

#endif

#if TEST_3x24_HARNESS

// constexpr byte NPANELS = 1;                 // In this test-harness we have 1 panel
// constexpr byte NROWS   = 24;                // three 8-pixel YyzPixel boards is 24 rows
// constexpr byte NCOLS   = 3;                 // and we only have three columns
constexpr byte NPANELS = 21;                  // At Beth Shalom, we have 3 * 7 panels
constexpr byte NROWS   = 56;                  // number of rows
constexpr byte NCOLS   = 40;                  // a column is 56 rows of consecutive pixel data

#endif

//  We use these values, so the display buffer is dimensioned maximally
constexpr byte MAXNROWS = 64;
constexpr byte MAXNCOLS = 64;

// ----------------------------------------------------------------------------
//            M I S C   C O N S T A N T S
// ----------------------------------------------------------------------------

constexpr byte PANEL0 = 0;

enum streamIds : byte { CONSOLE = 1, SOCKET = 2 };

enum ResultIds : byte {
    NO_ERROR = 0, ERR_SYNTAX, ERR_MISSING, ERR_ROW,
    ERR_COL, ERR_PANEL, ERR_BRIGHT, ERR_TESTNUM,
};

// ----------------------------------------------------------------------------
//            E X T E R N S
// ----------------------------------------------------------------------------

extern EthernetClient socketClient;
extern const char versionString[];

extern CmdProc cmdProc;

struct DisplayConfig {
    uint8_t nRows;
    uint8_t nCols;
    uint8_t brightness;
    uint8_t nPanels;
};
extern DisplayConfig displayConfig;

struct NetworkConfig {
    byte mac[6];
    IPAddress ipAddr;
    IPAddress dnsAddr;
    IPAddress gateway;
    IPAddress subnet;
    char wifiSsid[33];          // reserved for possible WiFi variant
    char wifiPassword[65];      // reserved for possible WiFi variant  
};
extern NetworkConfig networkConfig;

extern char outputBuf[256];

    
// ----------------------------------------------------------------------------
//            F U N C T I O N   P R O T O T Y P E S
// ----------------------------------------------------------------------------

// declare as extern the public functions in yahrzeit_v3.ino
void    sleep_ms( bool, const unsigned int ms );
void    my_puts( byte streamID, const char *msg );

[[noreturn]] void panic(const char *expr, const char *file, int line);

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            panic(#expr, __FILE__, __LINE__); \
        } \
    } while (0)




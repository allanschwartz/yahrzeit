/**
 * @file        yahrzeit_v3.h
 *
 * @brief       Project-wide configuration and declarations for the Yahrzeit
 *              Embedded Controller.
 *
 * @details
 *              BLUF:
 *                  yahrzeit_v3.ino owns controller setup and the main loop.
 *                  socket_thread and serial_thread collect command lines.
 *                  CmdProc parses and dispatches commands.
 *                  LedWall owns logical wall operations.
 *                  YyzPixel owns low-level pixel hardware access.
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
 * @notes       This file defines project-wide constants, configuration
 *              structures, global externs, and shared utility declarations.
 *              See also 'README.md' in this directory, for more detail.
 */

/**
 * SYSTEM ARCHITECTURE
 *      Yahrzeit Wall 
 *          The physical memorial wall and LED array.
 *
 *      Yahrzeit Appliance
 *          The PHP/Linux system that maintains the memorial database,
 *          computes what should be lit, and sends command streams to the
 *          embedded controller.
 *
 *      Yahrzeit Embedded Controller
 *          This Arduino-based controller. It receives line-oriented commands
 *          over USB serial or TCP socket and drives the LED wall.
 *        
 *      Yahrzeit Pixel board / YyzPixel
 *          The small LED driver boards behind the wall. In this code, the
 *          low-level pixel driver is named YyzPixel. LedWall.cpp defines the
 *          logical geometry and panel mapping presented to this driver.
 *        
 *      Arduino Ethernet Shield
 *          The Ethernet board which provides a TCP/IP connection between
 *          the Embedded Controller and the Yahrzeit Appliance.
 *          
 * IMPLEMENTATION HISTORY
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
 *      The embedded controller accepts simple line-oriented ASCII commands
 *      over either USB serial or TCP socket.
 *
	        All  on|off [<panel>]
	        BRightness <n> (1:bright, 254:dim)
	        DAta <row> <col> <binary data>
	        DUmp [<panel>]
	        HElp
	        LOad
	        PIxel on|off <row> <col> [<panel>]
	        REfresh
	        SAve
	        STatus
	        TEst <testnumber> [<panel>]
            TIming on|off
	        VErsion
 */

#pragma once
#include <Arduino.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
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

// Select exactly one display geometry.
// CBS_56x40_WALL is the installed Congregation Beth Sholom wall.
// TEST_FIXTURE is the bench/test fixture.

// #define CBS_56x40_WALL    1
#define TEST_FIXTURE    1

#if defined(CBS_56x40_WALL) == defined(TEST_FIXTURE)
#error "Select exactly one display geometry"
#endif


// ----------------------------------------------------------------------------
//            M I S C   C O N S T A N T S
// ----------------------------------------------------------------------------

constexpr byte PANEL0 = 0;
constexpr uint16_t SOCKET_LISTEN_PORT = 2001;

constexpr const char *FIRMWARE_RELEASE = "v3.0.1";

#if defined(ARDUINO_NANO_R4)
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Nano R4";
#elif defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOWIFIR4)
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Uno R4 WiFi";
#elif defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_MINIMA)
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Uno R4 Minima";
#elif defined(ARDUINO_PORTENTA_C33)
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Portenta C33";
#elif defined(ARDUINO_ARCH_RENESAS)
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Renesas board";
#elif defined(ARDUINO_AVR_MEGA2560)
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Mega 2560";
#elif defined(ARDUINO_AVR_UNO)
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Uno";
#elif defined(ARDUINO_ARCH_AVR)
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino AVR board";
#else
constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino-compatible board";
#endif

#if CBS_56x40_WALL
constexpr const char *DISPLAY_CONFIGURATION = "CBS_56x40_WALL";
#else
constexpr const char *DISPLAY_CONFIGURATION = "TEST_FIXTURE";
#endif

enum streamIds : byte { CONSOLE = 1, SOCKET = 2 };

// ----------------------------------------------------------------------------
//            E X T E R N S
// ----------------------------------------------------------------------------

extern EthernetClient socketClient;

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

extern char outputBuf[512];

extern bool timingOutputEnabled;

extern bool debugPixel;

    
// ----------------------------------------------------------------------------
//            F U N C T I O N   P R O T O T Y P E S
// ----------------------------------------------------------------------------

/** Optionally refresh the display, then block for the requested interval. */
void    sleepMs( bool, const unsigned int ms );

/** Write the supplied string to the selected console or socket stream. */
void    writeOutput( byte streamID, const char *msg );

/** Format the firmware release identifier and compilation date/time. */
const char *versionText();

/** Report a failed assertion, signal panic for five minutes, then restart. */
[[noreturn]] void panic(const char *expr, const char *file, int line);

/** Report a failed assertion with formatted context, then restart. */
[[noreturn]] void panicContext(const char *expr, const char *file, int line,
                               const char *fmt, ...);

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            panic(#expr, __FILE__, __LINE__); \
        } \
    } while (0)

#define ASSERT_CONTEXT(expr, fmt, ...) \
    do { \
        if (!(expr)) { \
            panicContext(#expr, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
        } \
    } while (0)

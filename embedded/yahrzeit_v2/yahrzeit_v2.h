/**
 * @file        yahrzeit_v2.h
 *
 * @brief       Shared configuration for the Mega Yahrzeit controller.
 *
 *              Selects wall or bench geometry and gathers the module interfaces
 *              and shared application declarations. Wiring and network defaults
 *              are defined in the main sketch. Project history follows below.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 2.1 revised in November 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
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
 *          BRightness <n> (0:full, 255:off)
 *          DAta <row> <col> <binary data>
 *          DUmp
 *          HElp
 *          LOad
 *          PIxel on|off <row> <col> [<panel>]
 *          REfresh
 *          SAve
 *          TEst <testnumber> [<panel>|*]
 */

#pragma once

#include "socket_thread.h"
#include "serial_thread.h"
#include "led_device.h"
#include "CmdProc.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
//#include "stdlib.h"
//#include "string.h"

enum streamIds { CONSOLE = 1, SOCKET = 2 };

constexpr uint16_t SOCKET_LISTEN_PORT = 2001;

#if defined(ARDUINO_AVR_MEGA2560)
    constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Mega 2560";
#elif defined(ARDUINO_AVR_UNO)
    constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino Uno";
#elif defined(ARDUINO_ARCH_AVR)
    constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino AVR board";
#else
    constexpr const char *CONTROLLER_BOARD_MODEL = "Arduino-compatible board";
#endif


// ----------------------------------------------------------------------------
//            F U N C T I O N   P R O T O T Y P E S
// ----------------------------------------------------------------------------

// Top-level controller services defined in yahrzeit_v2.ino.
const char *versionText( void );
void    writeOutput( byte streamID, const char *msg );
void    sleep_ms( boolean doRefresh, const unsigned int ms );
char   *display_uptime( void );
void    panic( const char *expression, const char *file, int line );

#define ASSERT(expression) \
    do { \
        if (!(expression)) { \
            panic(#expression, __FILE__, __LINE__); \
        } \
    } while (0)

// Public self-test functions defined in led_selftest.ino.
void  selftest_marching_row( byte panel );
void  selftest_description( byte streamID, byte testNumber, const char * testDescription, byte panel );
int   selftest( byte streamID, byte testNumber, byte panel );


// ----------------------------------------------------------------------------
//            L E D   M A T R I X   S I Z E
// ----------------------------------------------------------------------------


// Select exactly one display geometry; this also selects site/lab networking.
#define CBS_56x40_WALL    1
//#define TEST_24x6_FIXTURE      1
//#define TEST_24x2_FIXTURE 1

#if (defined(CBS_56x40_WALL) + defined(TEST_24x6_FIXTURE) + defined(TEST_24x2_FIXTURE)) != 1
#error "Select exactly one display geometry"
#endif

#if CBS_56x40_WALL

constexpr const char DISPLAY_CONFIGURATION[] = "CBS 56x40 wall";

constexpr byte NPANELS = 21;                  // At Beth Shalom, we have 3 * 7 panels
constexpr byte NROWS = 56;                  // number of rows
constexpr byte NCOLS = 40;                  // a column is 56 rows of consecutive pixel data

#endif


#if TEST_24x2_FIXTURE

// Test with 6 YYZ-PIXEL boards, arranged as 2 columns of 3 boards.
constexpr const char DISPLAY_CONFIGURATION[] = "YYZ 24x2 test fixture";

constexpr byte NPANELS = 1;                   // one 24-row by 2-column fixture
constexpr byte NROWS = 24;                  // three 8-pixel yyz_pixel boards is 24 rows
constexpr byte NCOLS = 2;                   // two columns

#endif

#if TEST_24x6_FIXTURE

constexpr const char DISPLAY_CONFIGURATION[] = "YYZ 24x6 test fixture";

constexpr byte NPANELS = 2;                   // two 24-row by 3-column fixture boards
constexpr byte NROWS = 24;
constexpr byte NCOLS = 6;

#endif

constexpr byte MAXNROWS = 64;
constexpr byte MAXNCOLS = 64;
constexpr unsigned int MAXDATA = MAXNROWS * MAXNCOLS / 8; // 512 bytes of data

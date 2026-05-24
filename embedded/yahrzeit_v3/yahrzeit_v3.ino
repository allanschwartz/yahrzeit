/**
 * @file        yahrzeit_v3.ino
 *
 * @brief       top-level C code the Yahrzeit Embedded Controller
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
 * ASSEMBLY
 *
 *      Using the TEST_FIXTURE board(s)
 *          Each fixture board contains 24 rows x 3 columns of pixels.
 *
 *          TEST FIXTURE SIGNAL:    DI  OE  CP  ST
 *          Arduino Uno pin:         4   5   6   7
 *
 *
 *      Using the YYZ Pixel board(s)
 *
 *          V2 system (Arduino Mega)
 *
 *              YYZ PIXEL SIGNAL:   DI  OE  CP  ST
 *              Arduino Mega pin:   42  44  46  48
 *
 *              The Ethernet shield uses pins 50, 51, 52, 53
 *              on the Arduino Mega SPI interface.
 *
 *
 *          V3 system (Arduino Uno R4 Minima)
 *
 *              YYZ PIXEL SIGNAL:   DI  OE  CP  ST
 *              Arduino Uno pin:     4   5   6   7
 *
 *              The Ethernet Shield 2 uses pins 10, 11, 12, 13
 *              on the Arduino Uno SPI interface.
 */

#include <Arduino.h>
#include "yahrzeit_v3.h"


// ----------------------------------------------------------------------------
//            L E D   A R R A Y S
// ----------------------------------------------------------------------------

// support the physical interfaces to LED ARRAY

DisplayConfig displayConfig = {
    .nRows = NROWS,
    .nCols = NCOLS,
    .brightness = 128,
    .nPanels = NPANELS
};

// create an instance of the YyzPixels class, giving us the YyzPixels primitives
// note that YyzPixel is called with portIDs    ( DI, OE, CP, ST )

// Port definitions for Controller V3: Arduino Uno R4 Minima
static constexpr byte
     DI_pin = 4,       // DATA IN
     OE_pin = 5,       // Output Enable
     CP_pin = 6,       // Clock Pulse
     ST_pin = 7;       // STORE

// Port definitions for Controller V2: Arduino Mega with tail-shield 
// static constexpr bye
    // DI_pin = 42,     // DATA IN
    // OE_pin = 44,     // Output Enable
    // CP_pin = 46,     // Clock Pulse
    // ST_pin = 48;     // Store


YyzPixel yyzPixels( DI_pin, OE_pin, CP_pin, ST_pin );

// create an instance of the LedWall class, giving us the LedWall primitives
LedWall ledWall(yyzPixels);

// create an instance of the CmdProc class, giving us the CmdProc primitives
CmdProc cmdProc(ledWall);

// ----------------------------------------------------------------------------
//            E T H E R N E T
// ----------------------------------------------------------------------------

// Make up a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:

NetworkConfig networkConfig = {
    .mac = { 0x02, 0x19, 0x55, 0x11, 0x00, 0x09 },          // 1955-11-09
    #if CBS_56x40_WALL
        // Values used at Congregation Beth Sholom
        .ipAddr = IPAddress(192, 168, 13, 9),
        .dnsAddr = IPAddress(8, 8, 8, 8),
        .gateway = IPAddress(192, 168, 13, 8),
        .subnet = IPAddress(255, 255, 255, 0),
    #else 
        // Values used at Allan's home lab
        .ipAddr = IPAddress(192, 168, 86, 240),
        .dnsAddr = IPAddress(8, 8, 8, 8),
        .gateway = IPAddress(192, 168, 86, 1),
        .subnet = IPAddress(255, 255, 255, 0),
    #endif
};

// telnet defaults to port 23, however we will listen on port 2001
//   as port 23 would be too easily "hacked".
EthernetServer socket(2001);
EthernetClient socketClient;

// ----------------------------------------------------------------------------
//            G L O B A L   S T O R A G E
// ----------------------------------------------------------------------------

const char versionString[]  =
    "\tLED Controller, V3.0, built " __DATE__ " " __TIME__ "\n"
    "\tcopyright (c) 2008,2015,2026 AMS Consulting\n";

char outputBuf[256] {};

// ----------------------------------------------------------------------------
//           M A I N
// ----------------------------------------------------------------------------

/**
 * @brief   application startup, called at the end of setup()
 */
static void controller_begin()
{
    yyzPixels.begin();
    ledWall.begin();
    serial_log( "LED wall up\n" );

    // splash
    my_puts( CONSOLE, versionString );

    // some initial pattern, on power-up (as a self-test)
    serial_log( "light test" );
    (void)ledWall.allOn( 1, PANEL0 );
    sleep_ms(true, 1500);
    (void)ledWall.allOn( 0, PANEL0 );
    sleep_ms(true, 1500);
    selftest_marching_row( PANEL0 );
    sleep_ms(true, 1500);

    // restore the last pattern stored yesterday
    ledWall.loadPixels();
    
    // and refresh
    ledWall.refresh();

    serial_log( "ready >" );
}

/**
 * @brief   setup ... initializatin code, to run once:
 */
void setup()
{

    // initialize the serial console logic thread
    serial_init();
    serial_log( "Serial up" );
    
    // initialize the Ethernet
    ethernet_init();
    serial_log( "Ethernet up" );

    // start listening for socket clients to connect
    socket_init();
    controller_begin();
}




/**
 * @brief   loop ... main code, to run repeatedly:
 */
void loop()
{
    serial_thread();
    socket_thread();
    sleep_ms( false, 1 );       // avoid pounding the SPI bus with continuous Ethernet polling
}


/**
 * @brief   my_puts ... prints a single string to the stream
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param msg         the line to display, a C string
 */
void my_puts( byte streamID, const char *msg )
{
    switch ( streamID ) {
        case SOCKET:
            if (socketClient && socketClient.connected()) { 
                socketClient.write( msg );
            }
            break;
            
        case CONSOLE:
            if (Serial) {
                Serial.write( msg );
            }
            break;
    }
}

/** 
 * @brief   sleep_ms ... delays the specified number of milliseconds
 *
 * @param doRefresh   should we do a refresh, before we start the sleep?
 * @param ms          number of milliseconds to delay
 */
void sleep_ms( bool doRefresh, const unsigned int ms )
{
    const unsigned long t0 = millis();

    if ( doRefresh) {
        ledWall.refresh();
    }
        
    delay(ms);
}   

/** 
 * panic ... if a serious firmware error occurs, notice it
 *
 * @param expr   a text description of the expression which failed
 * @param file   where the panic originated from
 * @param line   and on which line number
 */
[[noreturn]] void panic(const char *expr, const char *file, int line)
{
    if (!Serial) {
        Serial.print("\n***PANIC***\nASSERT: ");
        Serial.println(expr);
        Serial.print("FILE: ");
        Serial.println(file);
        Serial.print("LINE: ");
        Serial.println(line);
        Serial.flush();
        delay(1000);
    }

    pinMode(LED_BUILTIN, OUTPUT);

    // Stay here forever, blinking built-in LED as secondary indication.
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}

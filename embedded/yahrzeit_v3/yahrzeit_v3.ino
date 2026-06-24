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
 *       The Yahrzeit Controller stack is built on an Arduino Uno R4
 *       Minima as the base, then the Ethernet Shield 2, and finally
 *       the Yahrzeit Controller Pixel Interface board on top.   The
 *       Yahrzeit Controller Pixel Interface board connects with a
 *       ribbon cable to the YYZ Pixel board(s) which drive the LED
 *       wall.
 *
 *      Using the TEST_FIXTURE board(s)
 *          Each fixture board contains 24 rows x 3 columns of pixels.
 *
 *              TEST FIXTURE SIGNAL: DI  OE  CP  ST
 *              Arduino Uno pin:     4   5   6   7
 *
 *
 *      Using the YYZ Pixel board(s)
 * 	        The CBS Yahrzeit Wall is 56 rows x 40 columns of pixels.
 * 	        Each YYZ Pixel board has 8 or 10 individual LEDs.  The
 * 	        Wall and is built from 2 or 3 YYZ Pixel boards per panel
 * 	        column, and 8 pixel boards per column, times 40 columns,
 * 	        for a total of 320 boards.
 *          
 *          A Panel is at most 28 rows x 20 columns of pixels.
 *
 *          V3 system (Arduino Uno R4 Minima)
 *
 *              YYZ PIXEL SIGNAL:    DI  OE  CP  ST
 *              Arduino Uno pin:     4   5   6   7
 *
 *      The Ethernet Shield 2 uses the Arduino Uno SPI interface:
 *              D10 CS, D11 CIPO, D12 COPI, D13 SCK
 * 
 *      The YAHRZEIT CONTROLLER PIXEL INTERFACE board has a STATUS LED
 *      connected to pin 3.
 */

#include <Arduino.h>
#include <math.h>
#include "yahrzeit_v3.h"


// ----------------------------------------------------------------------------
//            L E D   A R R A Y S
// ----------------------------------------------------------------------------

// support the physical interfaces to LED ARRAY

DisplayConfig displayConfig = {
    .nRows = 0,
    .nCols = 0,
    .brightness = 128,
    .nPanels = 0
};

// the STATUS LED is the LED on the Yahrzeit Controller Pixel Interface board,
// connected to Arduino Uno R4 Minima, pin D3
static constexpr byte STATUS_LED = 3;
static constexpr byte STATUS_LED_BRIGHTNESS = 128; // 50% PWM duty cycle
static constexpr unsigned long ALIVE_PERIOD_MS = 2560;

// note that YyzPixel is called with portIDs    ( DI, OE, CP, ST )

// Port definitions for Controller V3: Arduino Uno R4 Minima
static constexpr byte
     DI_pin = 4,       // DATA IN
     OE_pin = 5,       // Output Enable
     CP_pin = 6,       // Clock Pulse
     ST_pin = 7;       // STORE


// create an instance of the YyzPixels class, giving us the YyzPixels primitives
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

bool timingOutputEnabled = false;

bool debugPixel = false;



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
    pinMode(STATUS_LED, OUTPUT);
    analogWrite(STATUS_LED, STATUS_LED_BRIGHTNESS);

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
    breathing_led();

    delay( 10 );       // avoid pounding the SPI bus with continuous Ethernet polling
}

/**
 * @brief   Update the alive-status LED.
 */
static void breathing_led()
{
    constexpr float twoPi = 6.28318530718f;
    constexpr float halfPi = twoPi / 2.0f;
    const unsigned long elapsedMs = millis() % ALIVE_PERIOD_MS;
    const float phase = twoPi * elapsedMs / ALIVE_PERIOD_MS;
    const float intensity = (sinf(phase - halfPi) + 1.0f) / 2.0f;

    analogWrite(STATUS_LED,
                static_cast<byte>(intensity * STATUS_LED_BRIGHTNESS + 0.5f));
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
    if (Serial) {
        Serial.print("\n***PANIC***\nASSERT: ");
        Serial.println(expr);
        Serial.print("FILE: ");
        Serial.println(file);
        Serial.print("LINE: ");
        Serial.println(line);
        Serial.flush();
    }

    blink_panic_led_then_restart();
}

[[noreturn]] void panic_context(const char *expr, const char *file, int line,
                               const char *fmt, ...)
{
    if (Serial) {
        Serial.print("\n***PANIC***\nASSERT: ");
        Serial.println(expr);
        Serial.print("FILE: ");
        Serial.println(file);
        Serial.print("LINE: ");
        Serial.println(line);
        Serial.print("DETAIL: ");

        va_list args;
        va_start(args, fmt);
        char buffer[128];
        vsnprintf(buffer, sizeof buffer, fmt, args);
        va_end(args);

        Serial.println(buffer);
        Serial.flush();
    }

    blink_panic_led_then_restart();
}


constexpr byte PANIC_MINUTES = 5;

/**
 * @brief   Blink the built-in LED during a panic, then reset the controller.
 */
[[noreturn]] static void blink_panic_led_then_restart()
{
    constexpr unsigned long blinkMs = 100;
    constexpr unsigned long panicMs = PANIC_MINUTES * 60UL * 1000UL;
    const unsigned long startedAt = millis();

    pinMode(STATUS_LED, OUTPUT);

    while (millis() - startedAt < panicMs) {
        analogWrite(STATUS_LED, STATUS_LED_BRIGHTNESS);
        delay(blinkMs);
        analogWrite(STATUS_LED, 0);
        delay(blinkMs);
    }

    // not a standard Arduino function, but works on the Uno R4 Minima
    NVIC_SystemReset();

    while (true) {
    }
}

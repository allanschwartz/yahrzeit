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

// The STATUS LED is the LED on the Yahrzeit Controller Pixel Interface board,
// connected to Arduino Uno R4 Minima, pin D3
static constexpr byte STATUS_LED_PIN = 3;
static constexpr byte STATUS_LED_BRIGHTNESS = 128; // 50% PWM duty cycle
static constexpr unsigned long ALIVE_PERIOD_MS = 2560;

// Controller V3 pin definitions: Arduino Uno R4 Minima.
// Names match the YYZ Pixel Interface board silkscreen: DI, OE, CP, ST.
static constexpr byte
     DI_PIN = 4,       // Data input
     OE_PIN = 5,       // Output enable, active LOW
     CP_PIN = 6,       // Clock pulse
     ST_PIN = 7;       // Store/latch pulse


// create an instance of the YyzPixels class, giving us the YyzPixels primitives
YyzPixel yyzPixels( DI_PIN, OE_PIN, CP_PIN, ST_PIN );

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

// telnet defaults to port 23, however we listen on a less-common control port.
//   as port 23 would be too easily "hacked".
static constexpr uint16_t SOCKET_LISTEN_PORT = 2001;
EthernetServer socket(SOCKET_LISTEN_PORT);
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

static void breathingLed();
[[noreturn]] static void blinkPanicLedThenRestart();


// ----------------------------------------------------------------------------
//           M A I N
// ----------------------------------------------------------------------------

/**
 * @brief   application startup, called at the end of setup()
 */
static void controllerBegin()
{
    yyzPixels.begin();
    ledWall.begin();
    serialLog( "LED wall up\n" );

    // splash
    writeOutput( CONSOLE, versionString );

    // some initial pattern, on power-up (as a self-test)
    serialLog( "light test" );
    (void)ledWall.allOn( 1, PANEL0 );
    sleepMs(true, 1500);
    (void)ledWall.allOn( 0, PANEL0 );
    sleepMs(true, 1500);
    selftestMarchingRow( PANEL0 );
    sleepMs(true, 1500);

    // restore the last pattern stored yesterday
    ledWall.loadPixels();
    
    // and refresh
    ledWall.refresh();

    serialLog( "ready >" );
}

/**
 * @brief   setup ... initialization code, to run once:
 */
void setup()
{
    pinMode(STATUS_LED_PIN, OUTPUT);
    analogWrite(STATUS_LED_PIN, STATUS_LED_BRIGHTNESS);

    // initialize the serial console logic thread
    serialInit();
    serialLog( "Serial up" );
    
    // initialize the Ethernet
    ethernetInit();
    serialLog( "Ethernet up" );

    // start listening for socket clients to connect
    socketInit();
    controllerBegin();
}

/**
 * @brief   loop ... main code, to run repeatedly:
 */
void loop()
{
    serialThread();
    socketThread();
    breathingLed();

    delay(10);          // pause 10ms, to avoid pounding the SPI bus with continuous Ethernet polling
}

/**
 * @brief   Update the alive-status LED.
 */
static void breathingLed()
{
    constexpr float TWO_pi = 6.28318530718f;                    /* keep phase math in float */
    unsigned long elapsedMs = millis() % ALIVE_PERIOD_MS;       /* time:    [0..Period)  */
    float phase = TWO_pi * (float)elapsedMs / ALIVE_PERIOD_MS;  /* radians: [0.0 .. 2π) */
    float sinePhase = sinf(phase);                              /* plotted as a sine wave */
    float intensity = (sinePhase + 1.0f) / 2.0f;                /* scaled to [0.0 .. 1.0] */
    int pwmValue = intensity * STATUS_LED_BRIGHTNESS;           /* int:     [0 .. 255] */

    analogWrite(STATUS_LED_PIN, pwmValue);
}

/**
 * @brief   writeOutput ... prints a single string to the stream
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param msg         the line to display, a C string
 */
void writeOutput( byte streamID, const char *msg )
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
 * @brief   sleepMs ... delays the specified number of milliseconds
 *
 * @param doRefresh   should we do a refresh, before we start the sleep?
 * @param ms          number of milliseconds to delay
 */
void sleepMs( bool doRefresh, const unsigned int ms )
{
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

    blinkPanicLedThenRestart();
}

[[noreturn]] void panicContext(const char *expr, const char *file, int line,
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

    blinkPanicLedThenRestart();
}


constexpr byte PANIC_MINUTES = 5;

/**
 * @brief   Blink the built-in LED during a panic, then reset the controller.
 */
[[noreturn]] static void blinkPanicLedThenRestart()
{
    constexpr unsigned long BLINK_MS = 100;
    constexpr unsigned long PANIC_MS = PANIC_MINUTES * 60UL * 1000UL;
    const unsigned long startedAt = millis();

    pinMode(STATUS_LED_PIN, OUTPUT);

    while (millis() - startedAt < PANIC_MS) {
        analogWrite(STATUS_LED_PIN, STATUS_LED_BRIGHTNESS);
        delay(BLINK_MS);
        analogWrite(STATUS_LED_PIN, 0);
        delay(BLINK_MS);
    }

    // not a standard Arduino function, but works on the Uno R4 Minima
    NVIC_SystemReset();

    while (true) {
    }
}

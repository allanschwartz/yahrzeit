/**
 * @file        yahrzeit_v2.c
 *
 * @brief       top-level C code the Yahrzeit Embedded Controller
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 *
 * @notes       see project notes in file yahrzeit_v2.h
 */

/**
 * ASSEMBLY
 *      Using the YYZ PIXEL boards
 *                  connections to an Arduino Mega
 *
 *          YYZ PIXEL BOARD:     DI, OE, CP, ST
 *          Arduino Digital pin: 42, 44, 46, 48
 *
 *      Ethernet shield attached to pins 50, 51, 52, 53 (on the Arduino Mega)
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "yahrzeit_v2.h"


// ----------------------------------------------------------------------------
//            L E D   A R R A Y S
// ----------------------------------------------------------------------------

// Both the wall and bench harness use the YYZ pixel interface.

#include "yyz_pixel.h"



// constructor for the LedMatrix, an instance of the 'yyz_pixel' class
//yyz_pixel     ( DI, OE, CP, ST )
//enum { DI_pin = 48, OE_pin = 46, CP_pin = 42, ST_pin = 44 };    // for hard-wired prototype board
enum { DI_pin = 42, OE_pin = 44, CP_pin = 46, ST_pin = 48 };    // for fabbed tail-shield board.
yyz_pixel LedMatrix(   DI_pin, OE_pin, CP_pin, ST_pin );


// ----------------------------------------------------------------------------
//            E T H E R N E T
// ----------------------------------------------------------------------------

// MAC address printed on the replacement Ethernet shield.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
    0xA8, 0x61, 0x0A, 0xAE, 0xE4, 0xE2
};
#if CBS_56x40_WALL
// Congregation Beth Sholom installation.
IPAddress ipaddr(192, 168, 13, 9);
IPAddress gateway(192, 168, 13, 8);
#else
// Allan's home lab. Reserve .243 for this shield before connecting.
// No ping/ARP response on 2026-11-22; .240, .241 and .242 are occupied.
// test with 'nc -v 192.168.86.243 2001'
IPAddress ipaddr(192, 168, 86, 143);
IPAddress gateway(192, 168, 86, 1);
#endif
IPAddress dnsaddr(8, 8, 8, 8);
IPAddress subnet(255, 255, 255, 0);

// TCP command port used by the PHP appliance's bin/yahrzeit wrapper through nc.
EthernetServer socket(SOCKET_LISTEN_PORT);
EthernetClient socketClient;

constexpr byte ETHERNET_SD_CS_PIN = 4; // unused SD card shares the SPI bus
constexpr byte STATUS_LED_PIN = 45;
constexpr unsigned long ALIVE_PERIOD_MS = 2560;

static void breathingLed();

[[noreturn]] static void blinkPanicLedThenRestart();

// ----------------------------------------------------------------------------
//            G L O B A L   S T O R A G E
// ----------------------------------------------------------------------------

// Display Buffer up to 64 rows and 64 columns, 512 bytes = 64 * 64 / 8
byte Displaybuf[MAXDATA] = { 0 };
char CmdOutput[ 512 ];            // the output of the command, in one packet

/**
 * @brief   Format the firmware version, compiler-selected Arduino target, and build.
 */
const char *versionText( void )
{
    static char versionOutput[256];



    snprintf(versionOutput, sizeof versionOutput,
             "\tYahrzeit Embedded Controller, V2.1\n"
             "\ttarget=%s\n"
             "\tbuilt %s %s\n"
             "\tcopyright (c) 2008,2015,2026 AMS Consulting\n",
             CONTROLLER_BOARD_MODEL, __DATE__, __TIME__);

    return versionOutput;
}
// ----------------------------------------------------------------------------
//           M A I N
// ----------------------------------------------------------------------------

/**
 * setup ... initializatin code, to run once:
 */
void setup()
{
    // watchdog_startup.cpp disables a surviving watchdog before C++ startup.
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    // initialize the LED matrix package
    LedMatrix.begin( Displaybuf, NCOLS, NROWS );

    // initialize the USB serial port
    serial_init();

    // Deselect the unused SD interface before accessing the shared SPI bus.
    digitalWrite(ETHERNET_SD_CS_PIN, HIGH);
    pinMode(ETHERNET_SD_CS_PIN, OUTPUT);

    // initialize the Ethernet
    ethernet_init();

    // socket_thread() starts the listener when Ethernet link is ready and
    // restarts it after later link disruptions.

    // initialize the console logic
    console_init();

    // restore the last pattern stored yesterday
    led_loaddata();

    // and referesh
    LedMatrix.refresh();
}


/**
 * @brief   Cooperatively service serial input, socket input, and the ALIVE
 *          LED animation.
 */
void loop()
{
    console_thread();
    socket_thread();
    breathingLed();

    delay(10);          // pause 10ms, to avoid pounding the SPI bus with continuous Ethernet polling
}


/**
 * @brief   Breathe the ALIVE LED using a sine-wave lookup table in flash.
 *
 * The animation confirms that the cooperative main loop is running. It does
 * not indicate Ethernet or command-socket health.
 */
static void breathingLed()
{
    // Phase-shifted V3 sine wave: round(64 * (1 - cos(2*pi*i/255))).
    // Mirrored samples start/end at 0; the two center samples peak at PWM 128.
    // PROGMEM stores the 256-byte table in flash rather than RAM.
    // Table samples 0, 4, ... 252: 64 columns, eight rows, one star per sample.
    // 128 |                           ***********
    //     |                       ****           ****
    //     |                   ****                   ***
    //     |                ***                          ***
    //     |             ***                                ***
    //     |          ***                                      ***
    //     |      ****                                            *****
    //   0 |******                                                     *****
    static const byte BREATHING_PWM[256] PROGMEM = {
          0,   0,   0,   0,   0,   0,   1,   1,   1,   2,   2,   2,   3,   3,   4,   4,
          5,   6,   6,   7,   8,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
         19,  20,  21,  22,  24,  25,  26,  27,  29,  30,  31,  33,  34,  35,  37,  38,
         40,  41,  43,  44,  46,  47,  49,  50,  52,  53,  55,  57,  58,  60,  61,  63,
         64,  66,  68,  69,  71,  72,  74,  75,  77,  78,  80,  82,  83,  85,  86,  87,
         89,  90,  92,  93,  95,  96,  97,  99, 100, 101, 103, 104, 105, 106, 107, 109,
        110, 111, 112, 113, 114, 115, 116, 117, 118, 118, 119, 120, 121, 121, 122, 123,
        123, 124, 124, 125, 125, 126, 126, 127, 127, 127, 127, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 127, 127, 127, 127, 126, 126, 125, 125, 124, 124, 123,
        123, 122, 121, 121, 120, 119, 118, 118, 117, 116, 115, 114, 113, 112, 111, 110,
        109, 107, 106, 105, 104, 103, 101, 100,  99,  97,  96,  95,  93,  92,  90,  89,
         87,  86,  85,  83,  82,  80,  78,  77,  75,  74,  72,  71,  69,  68,  66,  64,
         63,  61,  60,  58,  57,  55,  53,  52,  50,  49,  47,  46,  44,  43,  41,  40,
         38,  37,  35,  34,  33,  31,  30,  29,  27,  26,  25,  24,  22,  21,  20,  19,
         18,  17,  16,  15,  14,  13,  12,  11,  10,   9,   8,   8,   7,   6,   6,   5,
          4,   4,   3,   3,   2,   2,   2,   1,   1,   1,   0,   0,   0,   0,   0,   0,
    };

    const unsigned long elapsedMs = millis() % ALIVE_PERIOD_MS;
    // One table entry per 10 ms at the 2560 ms period; delayed loops skip ahead.
    const byte phaseIndex = elapsedMs / (ALIVE_PERIOD_MS / sizeof(BREATHING_PWM));
    const byte pwmValue = pgm_read_byte(&BREATHING_PWM[phaseIndex]);

    analogWrite(STATUS_LED_PIN, pwmValue);
}


/**
 * @brief   Write one string, unchanged, to the selected output stream.
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param msg         null-terminated string; no newline is added
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
            Serial.write( msg );
            break;
    }
}


/**
 * @brief   Optionally refresh the display, then block for a fixed interval.
 *
 * @param doRefresh   should we do a refresh, before we start the sleep?
 * @param ms          number of milliseconds to delay
 */
void sleep_ms( boolean doRefresh, const unsigned int ms )
{
    if ( doRefresh) {
        LedMatrix.refresh();
    }

    delay(ms);
}


/**
 * display_uptime ... displays the uptime, in the format hh:mm:ss.mmm  (with millisecond precision)
 *
 * @note:        returns a pointer to a static string (so it is not thread safe)
 *
 * @param uptime    number of milliseconds since the arduino microcontroller was booted
 */
char *display_uptime( void )
{
    unsigned long msec = millis();
    unsigned long s = msec / 1000;
    int mm = (s / 60 % 60);
    int hh = (s / 3600);
    int ss = s % 60;
    int imsec = msec % 1000;
    static char displayBuf[20];        // caution ... static, not thread-safe
    snprintf(displayBuf, sizeof displayBuf, "%02d:%02d:%02d.%03d", hh, mm, ss, imsec );
    return displayBuf;
}



/**
 * @brief   Report a failed assertion, signal panic for five minutes, then
 *          restart the Mega using the AVR watchdog.
 *
 * @param expression    The expression that failed.
 * @param file          The source file containing the assertion.
 * @param line          The source line containing the assertion.
 */
[[noreturn]] void panic( const char *expression, const char *file, int line )
{
    Serial.println( "\n*** PANIC ***" );
    Serial.print( "ASSERT: " );
    Serial.println( expression );
    Serial.print( "FILE: " );
    Serial.println( file );
    Serial.print( "LINE: " );
    Serial.println( line );
    Serial.print( "Restarting in a few minutes" );
    Serial.flush();

    blinkPanicLedThenRestart();
}


constexpr byte PANIC_MINUTES = 5;

/**
 * @brief   Blink the ALIVE LED for five minutes, then reset the Mega.
 *
 * @note    This function does not return. The watchdog performs the reset
 *          approximately one second after the blinking interval ends.
 */
[[noreturn]] static void blinkPanicLedThenRestart()
{
    constexpr unsigned long BLINK_MS = 100;
    constexpr unsigned long PANIC_MS = PANIC_MINUTES * 60UL * 1000UL;
    const unsigned long startedAt = millis();

    pinMode(STATUS_LED_PIN, OUTPUT);

    while (millis() - startedAt < PANIC_MS) {
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(BLINK_MS);
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(BLINK_MS);
    }

    // Stop interrupt handlers from running while waiting for a hardware reset.
    // Do not feed the watchdog: expiration deliberately resets the MCU.
    noInterrupts();
    wdt_reset();
    wdt_enable(WDTO_1S);
    while (true) {
    }
}

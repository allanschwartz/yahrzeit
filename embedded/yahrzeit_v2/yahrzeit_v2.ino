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
 *      Using the "TEST_16x64_LED_MATRIX"
 *          LED Matrix panel has 64x16 pixels.
 *                  connections to Arduino Uno
 *
 *          LED Matrix PCB:      LA, LB, LC, LD,    EN, R1, LT, SK
 *          Arduino Digital pin: 3,  4,  5,  6,     2, 11,  8,  13
 *
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
#include <avr/wdt.h>
#include "yahrzeit_v2.h"


// ----------------------------------------------------------------------------
//            L E D   A R R A Y S
// ----------------------------------------------------------------------------

// we are supporting two different physical interfaces to LED ARRAYS
#if TEST_16x64_LED_MATRIX
#include "LEDMatrix.h"
#endif

#if CBS_56x40_WALL || TEST_2x24_HARNESS
#include "yyz_pixel.h"
#endif

#define WIDTH  NCOLS
#define HEIGHT NROWS

#if TEST_16x64_LED_MATRIX
// constructor for the LedMatrix, an instance of the 'LEDMatrix' class
// LEDMatrix    ( a, b, c, d, oe, r1, stb, clk);
LEDMatrix LedMatrix( 3, 4, 5, 6, 2,  11, 8,   13);
#endif

#if CBS_56x40_WALL || TEST_2x24_HARNESS
// constructor for the LedMatrix, an instance of the 'yyz_pixel' class
//yyz_pixel     ( DI, OE, CP, ST )
//enum { DI_pin = 48, OE_pin = 46, CP_pin = 42, ST_pin = 44 };    // for hard-wired prototype board
enum { DI_pin = 42, OE_pin = 44, CP_pin = 46, ST_pin = 48 };    // for fabbed tail-shield board.
yyz_pixel LedMatrix(   DI_pin, OE_pin, CP_pin, ST_pin );
#endif


// ----------------------------------------------------------------------------
//            E T H E R N E T
// ----------------------------------------------------------------------------

// Make up a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
    0x11, 0x09, 0x19, 0x55, 0x06, 0x13
};
IPAddress  ipaddr(192, 168, 13, 9);
IPAddress dnsaddr( 8, 8,  8, 8);
IPAddress gateway(192, 168, 13, 8);
IPAddress subnet(255, 255, 255, 0);

// telnet defaults to port 23, however we will list on port 2001
EthernetServer socket(2001);
EthernetClient socketClient;
boolean socketListenerRunning = false;
unsigned long socketLastByteReceivedMs = 0;

static constexpr unsigned long SOCKET_IDLE_TIMEOUT_MS =
    30UL * 60UL * 1000UL;
static constexpr unsigned long LINK_CHECK_INTERVAL_MS = 50UL;
static constexpr byte PANIC_MINUTES = 5;

enum SocketGetsResult {
    GETS_NOCHAR = -1,
    GETS_PARTIAL = 0,
    GETS_FULLCMD = 1,
};


// ----------------------------------------------------------------------------
//            G L O B A L   S T O R A G E
// ----------------------------------------------------------------------------

// Display Buffer up to 64 rows and 64 columns, 512 bytes = 64 * 64 / 8
byte Displaybuf[MAXDATA] = { 0 };
char CmdOutput[ 512 ];            // the output of the command, in one packet

/**
 * Format the firmware version, compiler-selected Arduino target, and build.
 */
const char *versionText( void )
{
    static char versionOutput[256];

#if defined(ARDUINO_AVR_MEGA2560)
    const char *boardTarget = "Arduino Mega 2560";
#elif defined(ARDUINO_AVR_UNO)
    const char *boardTarget = "Arduino Uno";
#elif defined(ARDUINO_ARCH_AVR)
    const char *boardTarget = "Arduino AVR board";
#else
    const char *boardTarget = "Arduino-compatible board";
#endif

    snprintf(versionOutput, sizeof versionOutput,
             "\tYahrzeit Embedded Controller, V2.1\n"
             "\ttarget=%s\n"
             "\tbuilt %s %s\n"
             "\tcopyright (c) 2008,2015,2026 AMS Consulting\n",
             boardTarget, __DATE__, __TIME__);

    return versionOutput;
}
/*
enum ResultIds {
    NO_ERROR = 0, ERR_SYNTAX, ERR_MISSING, ERR_ROW, 
    ERR_COL, ERR_PANEL, ERR_BRIGHT, ERR_TESTNUM,
};*/

const char* ResultStrings[8] = {
    "OK", "Eh?", "ERR Missing Arg", "ERR Row", 
    "ERR Col", "ERR Panel", "ERR Brightness", "ERR Test Number"
};

const char HelpText[]  = 
    "\n"
    "\tAll  on|off [<panel>]\n"
    "\tBRightness <n> (0:full, 255:off)\n"
    "\tDAta <row> <col> <binary data>\n"
    "\tDUmp [<panel>]\n"
    "\tHElp\n"
    "\tLOad\n"
    "\tPIxel on|off <row> <col> [<panel>]\n"
    "\tREfresh\n"
    "\tSAve\n"
    "\tSTatus\n"
    "\tTEst <testnumber> [<panel>|*]\n"
    "\tVErsion\n";
    
const char TestMenu[]  = 
    "\n"
    "\tTEst 1 [<panel>|*] --   4 corners ON\n"
    "\tTEst 2 [<panel>|*] --   all pixels ON\n"
    "\tTEst 3 [<panel>|*] --   all pixels OFF\n"
    "\tTEst 4 [<panel>|*] --   checkerboard test\n"
    "\tTEst 5 [<panel>|*] --   marching row pattern\n"
    "\tTEst 6 [<panel>|*] --   marching column pattern\n"
    "\tTEst 7 [<panel>|*] --   Cylon pattern\n"
    "\t* repeats the test for panels 1 through n\n";
    

// ----------------------------------------------------------------------------
//           M A I N
// ----------------------------------------------------------------------------

/**
 * setup ... initializatin code, to run once:
 */
void setup()
{
    // Ensure a watchdog-initiated restart does not become a reset loop.
    wdt_disable();

    // initialize the LED matrix package
    LedMatrix.begin( Displaybuf, WIDTH, HEIGHT );

    // initialize the USB serial port
    Serial.begin( 9600 );

    // initialize the Ethernet
    Ethernet.begin( mac, ipaddr, dnsaddr, gateway, subnet );

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
 * loop ... main code, to run repeatedly:
 */
void loop()
{
    console_thread();
    socket_thread();
    sleep_ms( false, 1 );
}


/**
 * socket_thread .. implement the socket stream I/O gets(), puts() loop.
 *    in each call to socket_thread, we read one line, and execute one command.
 *
 * @returns
 *      returns after every command line is executed
 */
void  socket_thread( void )
{
    // the command from the client (the Yahrzeit Server)
#define MAXINPUTLINE 64
    // These variables remain static so partial commands survive loop calls.
    static char          inputBuf[ MAXINPUTLINE ] = { 0 };
    static int           inputBufPos = 0;                 // 0..MAXCMDLINE-1
    static unsigned long lastLinkCheckMs = 0;
    static EthernetLinkStatus cachedLinkStatus = Unknown;

    const unsigned long nowMs = millis();
    if (lastLinkCheckMs == 0 ||
        nowMs - lastLinkCheckMs >= LINK_CHECK_INTERVAL_MS) {
        lastLinkCheckMs = nowMs;
        cachedLinkStatus = Ethernet.linkStatus();
    }

    if (cachedLinkStatus == LinkOFF) {
        if (socketClient) {
            socketClient.stop();
        }
        inputBufPos = 0;
        inputBuf[0] = '\0';
        socketListenerRunning = false;
        return;
    }

    if (!socketListenerRunning) {
        socket.begin();
        socketListenerRunning = true;
        console_log( "ethernet: listener started" );
    }

    // Own the TCP client lifecycle here. socket_gets() only reads an already
    // connected client.
    if (!socketClient || !socketClient.connected()) {
        if (socketClient) {
            socketClient.stop();
        }

        socketClient = socket.available();
        inputBufPos = 0;
        inputBuf[0] = '\0';

        if (!socketClient) {
            return;
        }

        socketLastByteReceivedMs = millis();
        console_log( "socket: client connected" );
    }

    if (socketClient.available() == 0 &&
        millis() - socketLastByteReceivedMs >= SOCKET_IDLE_TIMEOUT_MS) {
        socketClient.stop();
        inputBufPos = 0;
        inputBuf[0] = '\0';
        console_log( "socket: idle client closed after 30 minutes" );
        return;
    }

    int rc = socket_gets( inputBuf, sizeof inputBuf, &inputBufPos );
    switch ( rc ) {
        case GETS_NOCHAR:       // no data is available
            break;
        case GETS_PARTIAL:      // recorded a partial command line
            socketLastByteReceivedMs = millis();
            break;
        case GETS_FULLCMD:      // complete command line, to interpret
        {
            socketLastByteReceivedMs = millis();
            char *uptimeRendered = display_uptime();
            if ( strlen( inputBuf ) > 0 ) {
                const char *result = shell_execute( SOCKET, inputBuf );
                snprintf( CmdOutput, sizeof CmdOutput, "%s | %s\n",
                          uptimeRendered, inputBuf );
                my_puts( SOCKET, CmdOutput );
                if ( strlen(result) ) {
                    my_puts( SOCKET, result );
                    if (result[strlen(result) - 1] != '\n') {
                        my_puts( SOCKET, "\n" );
                    }
                }
            }
            else {
                snprintf( CmdOutput, sizeof CmdOutput, "%s |\n", uptimeRendered );
                my_puts( SOCKET, CmdOutput );
            }

            inputBufPos = 0;
            memset( inputBuf, 0, sizeof inputBuf );
            break;
        }
    }
}


/**
 * socket_gets ... read one line from the already-connected TCP client
 *
 *    console_gets() reads in at most one less than size characters from the
 *    serial UART stream and and stores them into the buffer pointed to by str.
 *    NUL and carriage return are ignored. Reading stops after a newline or
 *    when the buffer is filled. The newline is not stored.
 *    A terminating null byte ('0' is stored after the last character.
 *
 *    This function owns no listener, connection, timeout, or reconnection
 *    policy. socket_thread() establishes the client before calling it.
 *
 * @param input    the resulting null-terminated string
 * @param maxsize  max size we can store, including a NULL
 * @param piIndex  pointer to a count of the number of bytes read into input
 *
 * @returns GETS_NOCHAR, GETS_PARTIAL, or GETS_FULLCMD
 */
int socket_gets(char *input, const unsigned int maxsize, int *piIndex)
{
    if (socketClient.available() == 0) {
        return GETS_NOCHAR;
    }

    while (socketClient.available() > 0) {
        int c = socketClient.read();
        if (c < 0) {
            return GETS_NOCHAR;
        }

        // Handle backspace
        if (c == '\b') {
            if (*piIndex > 0) {
                *piIndex--;
            }
            continue;
        }

        // Ignore NUL and CR; LF completes one command. This avoids a
        // second blank command when a client sends CRLF.
        if (c == 0 || c == '\r') {
            continue;
        }
        if (c == '\n') {
            return GETS_FULLCMD;
        }

        // Handling potential overflow of buffer, by just returning
        if (*piIndex >= (maxsize - 1)) {
            return GETS_FULLCMD;
        }

        // normal character
        input[*piIndex] = c;
        *piIndex = *piIndex + 1;
        input[*piIndex] = '\0';
    }
    return GETS_PARTIAL;
}


/**
 * sleep_ms ... delays the specified number of milliseconds, however,
 *         the display is refreshed periodically.
 *
 * @param doRefresh   should we do a refresh, before we start the sleep?
 * @param ms          number of milliseconds to delay
 */
void sleep_ms( boolean doRefresh, const unsigned int ms )
{
    unsigned long t0 = millis();

    if ( doRefresh) {
        LedMatrix.refresh();
    }

    while ( (unsigned int)(millis() - t0) < ms ) {
        delayMicroseconds(100);

#if TEST_16x64_LED_MATRIX
        LedMatrix.refresh();
#endif
    }
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
 * console_log ... displays a time-stamped line, typically a diagnostic line
 *        on the serial console.  A trailing newline is also written.
 *
 * @param msg      the line to display, a C string
 */
void console_log( const char *msg )
{
    snprintf( CmdOutput, sizeof CmdOutput, "%s | %s\n", display_uptime(), msg );
    Serial.print( CmdOutput );
}


/**
 * Print an assertion failure, wait five minutes, then restart the Mega using
 * the AVR watchdog.
 */
void panic( const char *expression, const char *file, int line )
{
    Serial.println( "\n*** PANIC ***" );
    Serial.print( "ASSERT: " );
    Serial.println( expression );
    Serial.print( "FILE: " );
    Serial.println( file );
    Serial.print( "LINE: " );
    Serial.println( line );
    Serial.print( "Restarting in " );
    Serial.print( PANIC_MINUTES );
    Serial.println( " minutes" );
    Serial.flush();

    const unsigned long panicStartedMs = millis();
    const unsigned long panicDelayMs =
        (unsigned long)PANIC_MINUTES * 60UL * 1000UL;
    while (millis() - panicStartedMs < panicDelayMs) {
        delay(1000);
    }

    wdt_enable(WDTO_1S);
    while (true) {
    }
}


/**
 * my_puts ... prints a single string to the stream
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
            Serial.write( msg );
            break;
    }
}

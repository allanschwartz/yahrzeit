/**
 * @file        serial_thread.ino
 *
 * @brief       Console Interface code for the Yahrzeit Project
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

#include "yahrzeit_v3.h"

// ----------------------------------------------------------------------------
//            S E R I A L   C O N S O L E
// ----------------------------------------------------------------------------

/**
 * @brief   Initializes the USB Serial port, which becomes our console
 */
void serial_init()
{
    Serial.begin(9600);

    const unsigned long start = millis();
    // we would hope the serial port comes up in 2 seconds.
    while (!Serial && (millis() - start < 2000)) {
        delay(10);
    }
    Serial.print("\n\n");
    serial_log( "Yahrzeit Wall Embedded Controller" );
}


// ----------------------------------------------------------------------------
//            C O N S O L E   C O M M A N D S
// ----------------------------------------------------------------------------


/**
 * @brief   Print a time-stamped prompt
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 */
static void prompt( byte streamID )
{
    char promptLine[32];
    snprintf(promptLine, sizeof promptLine, "%s >", display_uptime());
    my_puts(streamID, promptLine);
}


/**
 * @brief   Console main loop handling serial input
 *
 *      Implements the console I/O gets(), puts() loop.
 *      in each call to console_thread, we either read one line, and execute one command.
 *      or, if the line is not present, we return immediately, but keep state so we can
 *      resume reading the line
 */
void  serial_thread()
{
    // the command from the host
    constexpr unsigned MAXINPUTLINE = 64;
    static    char     inputBuf[ MAXINPUTLINE ] {};
    static    unsigned inputBufPos = 0;               // 0..MAXINPUTLINE-1

    if ( serial_gets( inputBuf, sizeof inputBuf, inputBufPos ) ) {
        char uptimeLine[80];

        if ( inputBuf[0] != '\0' ) {
            const char *result = cmdProc.execute( CONSOLE, inputBuf );
            snprintf( uptimeLine, sizeof uptimeLine, "%s | %s\n",
                      display_uptime(), inputBuf );
            my_puts( CONSOLE, uptimeLine );
            if (result != nullptr) {
                my_puts( CONSOLE, result );
                my_puts( CONSOLE, "\n");
            }
        }
        else {
            snprintf( uptimeLine, sizeof uptimeLine, "%s |\n", display_uptime() );
            my_puts( CONSOLE, uptimeLine );
        }
        inputBufPos = 0;            // reset to the beginning of the inputBuf
        inputBuf[0] = '\0';
        prompt( CONSOLE );
    }
}


/**
 * @brief   Reads a single line from the Serial UART, (similar to fgets())
 *
 *    console_gets() reads in at most one less than size characters from the
 *    serial UART stream and and stores them into the buffer pointed to by str.
 *    Reading stops after a newline or cr or the str is filled.
 *    If a newline or cr is read, it is not stored into the buffer.
 *    A terminating null byte ('\0') is stored after the last character.
 *
 *    This non-threaded version of console_gets() does not block, rather it
 *    returns immediately, that is returns each character as read.
 *    The bool result code indicates whether a full line was read
 *
 * @param inputBuf    the resulting null-terminated string
 * @param maxsize  max size we can store, including a NULL
 * @param index    reference to a count of the number of bytes read into inputBuf
 *
 * @returns bool, true: a full line was read; false: if not
 */
bool serial_gets( char inputBuf[], const unsigned maxsize, unsigned &index )
{
    while ( Serial.available() > 0 ) {
        const int c = Serial.read();

        if (c == 0 || c == '\r') {
            // remove NUL and CR from the input stream
            continue;
        }
        // handle NL
        if (c == '\n') {
            Serial.write("\r\n");
            return true;
        }

        // If the command is too long, terminate and process the truncated line.
        if (index >= (maxsize - 1)) {
            inputBuf[index] = '\0';
            return true;
        }

        // normal character
        inputBuf[index] = static_cast<char>(c);
        ++index;
        inputBuf[index] = '\0';
    }
    return false;
}

/**
 * @brief   Displays a time-stamped line, typically a diagnostic line
 *        on the serial console.  A trailing newline is also written.
 *
 * @param msg      the line to display, a C string
 */
void serial_log( const char *msg )
{
    if (Serial) {
        char outputBuf[80];
        snprintf( outputBuf, sizeof outputBuf, "%s | %s\n", display_uptime(), msg );
        Serial.print( outputBuf );
    }
}

/**
 * @brief   Displays the uptime, in the format hh:mm:ss.mmm  (with millisecond precision)
 *
 * @note:        returns a pointer to a static string (so it is not thread safe)
 *
 * @param uptime    number of milliseconds since the arduino microcontroller was booted
 */
const char *display_uptime()
{
    unsigned long msec = millis();
    unsigned long seconds = msec / 1000;
    int mm = (seconds / 60 % 60);
    int hh = (seconds / 3600);
    int ss = seconds % 60;
    int imsec = msec % 1000;
    static char displayBuf[20];        // caution ... static, not thread-safe
    snprintf(displayBuf, sizeof displayBuf, "%02d:%02d:%02d.%03d", hh, mm, ss, imsec );
    return displayBuf;
}


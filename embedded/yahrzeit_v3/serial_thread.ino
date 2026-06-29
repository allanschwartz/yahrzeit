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
void serialInit()
{
    // Use a common monitor speed and allow time for the USB/serial port to settle.
    Serial.begin(115200);
    delay(100);

    const unsigned long start = millis();
    // Some boards only become ready after the host opens the port.
    while (!Serial && (millis() - start < 3000)) {
        delay(10);
    }

    Serial.println();
    Serial.println("...");
    Serial.flush();
    serialLog( "Yahrzeit Wall Embedded Controller" );
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
    snprintf(promptLine, sizeof promptLine, "%s >", displayUptime());
    writeOutput(streamID, promptLine);
}


/**
 * @brief   Console main loop handling serial input
 *
 *      Implements the console I/O gets(), puts() loop.
 *      In each call to serialThread, we either read one line and execute one command,
 *      or, if the line is not present, we return immediately, but keep state so we can
 *      resume reading the line
 */
void  serialThread()
{
    // the command from the host
    constexpr unsigned MAX_INPUT_LINE = 64;
    static    char     inputBuf[ MAX_INPUT_LINE ] {};
    static    unsigned inputBufPos = 0;               // 0..MAX_INPUT_LINE-1

    if ( serialGets( inputBuf, sizeof inputBuf, inputBufPos ) ) {
        char uptimeLine[80];

        if ( inputBuf[0] != '\0' ) {
            const char *result = cmdProc.execute( CONSOLE, inputBuf );
            snprintf( uptimeLine, sizeof uptimeLine, "%s | %s\n",
                      displayUptime(), inputBuf );
            writeOutput( CONSOLE, uptimeLine );
            if (result != nullptr) {
                writeOutput( CONSOLE, result );
                writeOutput( CONSOLE, "\n");
            }
        }
        else {
            snprintf( uptimeLine, sizeof uptimeLine, "%s |\n", displayUptime() );
            writeOutput( CONSOLE, uptimeLine );
        }
        inputBufPos = 0;            // reset to the beginning of the inputBuf
        inputBuf[0] = '\0';
        prompt( CONSOLE );
    }
}


/**
 * @brief   Reads a single line from the Serial UART, (similar to fgets())
 *
 *    serialGets() reads in at most one less than size characters from the
 *    serial UART stream and stores them into the buffer pointed to by inputBuf.
 *    Reading stops after a newline or cr or the str is filled.
 *    If a newline or cr is read, it is not stored into the buffer.
 *    A terminating null byte ('\0') is stored after the last character.
 *
 *    This non-threaded version of serialGets() does not block, rather it
 *    returns immediately, that is returns each character as read.
 *    The bool result code indicates whether a full line was read
 *
 * @param inputBuf    the resulting null-terminated string
 * @param maxsize  max size we can store, including a NULL
 * @param index    reference to a count of the number of bytes read into inputBuf
 *
 * @returns bool, true: a full line was read; false: if not
 */
bool serialGets( char inputBuf[], const unsigned maxsize, unsigned &index )
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
void serialLog( const char *msg )
{
    if (Serial) {
        char outputBuf[80];
        snprintf( outputBuf, sizeof outputBuf, "%s | %s\n", displayUptime(), msg );
        Serial.print( outputBuf );
        Serial.flush();
    }
}

/**
 * @brief   Displays the uptime, in the format hh:mm:ss.mmm  (with millisecond precision)
 *
 * @note:        returns a pointer to a static string (so it is not thread safe)
 *
 * @param uptime    number of milliseconds since the arduino microcontroller was booted
 */
const char *displayUptime()
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

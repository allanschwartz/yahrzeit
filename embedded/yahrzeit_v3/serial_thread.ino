/**
 * @file        serial_thread.ino
 *
 * @brief       Nonblocking USB serial command input and shared diagnostic
 *              formatting for the Yahrzeit Embedded Controller.
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
 * @brief   Initialize the USB serial console at 115200 baud.
 *
 * Waits no more than three seconds for a host to open the USB serial port, so
 * an unattended controller can complete startup without a connected monitor.
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
 * @brief   Write the current uptime followed by a command prompt.
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
 * @brief   Service the nonblocking serial command loop once.
 *
 * Accumulates input across calls. When a newline or full buffer completes a
 * command, dispatches it through CmdProc, writes the result, resets the input
 * state, and emits another prompt.
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
 * @brief   Accumulate one newline-terminated command from USB serial input.
 *
 * This function is nonblocking and preserves its position through the index
 * reference. NUL and carriage-return bytes are ignored; newline completes the
 * command but is not stored. A full buffer also completes the truncated
 * command. The buffer is kept null-terminated after every stored character.
 *
 * @param inputBuf  destination buffer for the null-terminated command
 * @param maxsize   buffer capacity, including the null terminator
 * @param index     number of command bytes accumulated across calls
 *
 * @returns         true when a newline or full buffer completes a command;
 *                  false while the command remains incomplete
 */
bool serialGets( char inputBuf[], const unsigned maxsize, unsigned &index )
{
    while ( Serial.available() > 0 ) {
        const int c = Serial.read();

        if (c < 0) {
            break;
        }
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
        inputBuf[index] = c;
        ++index;
        inputBuf[index] = '\0';
    }
    return false;
}

/**
 * @brief   Write one uptime-prefixed diagnostic line to the serial console.
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
 * @brief   Format time since startup as hours, minutes, seconds, and
 *          milliseconds: hh:mm:ss.mmm.
 *
 * @returns pointer to a shared static buffer, overwritten by the next call
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

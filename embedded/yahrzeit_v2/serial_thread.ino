/**
 * @file        serial_thread.ino
 *
 * @brief       USB-connected UART console input loop and shared diagnostics.
 *
 *              The serial thread reads line-oriented commands from the Mega's
 *              serial console and passes complete command lines to the shared
 *              command processor in CmdProc.ino.
 *
 *              This path is useful for bench testing, controller bring-up,
 *              and field diagnostics when a USB cable is connected.
 *
 *              This module also provides startup console behavior and
 *              timestamped logging. Uptime formatting remains in the main
 *              sketch and is declared in yahrzeit_v2.h.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 2.1 reorganized in November 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#include "yahrzeit_v2.h"

/**
 * @brief   Initialize pixel state, print the startup banner, and run the
 *          startup LED test sequence.
 *
 * @note    Sets full brightness and runs the marching-row test before logging
 *          readiness. This function can block while the test is running.
 */
void console_init( void )
{
    led_init();

    // Print the firmware and build banner.
    writeOutput( CONSOLE, versionText() );

    // Run the startup display checks at full brightness.
    console_log( "light test" );
    led_all_on( 1, PANEL0 );
    led_all_on( 0, PANEL0 );
    led_set_intensity( 0 );
    selftest_marching_row( PANEL0 );
    console_log( "ready >" );
}


// ----------------------------------------------------------------------------
//            C O N S O L E   C O M M A N D S
// ----------------------------------------------------------------------------

/**
 * @brief   Collect serial input and execute at most one command per call.
 *
 * Partial input is preserved between calls. A completed command is passed to
 * shell_execute(), followed by timestamped feedback, any result text, and a
 * prompt. Empty lines receive timestamped feedback and a prompt.
 *
 * @note    Input collection does not wait for new bytes. Command execution
 *          and serial output may block, particularly during LED tests.
 */
void  console_thread( void )
{
    // Preserve the partial command between main-loop calls.
    constexpr unsigned int MAXINPUTLINE = 64;
    static char     inputBuf[ MAXINPUTLINE ] = { 0 };
    static int      inputBufPos = 0;               // 0..MAXINPUTLINE-1

    if ( console_gets( inputBuf, sizeof inputBuf, &inputBufPos ) ) {
        char *uptimeRendered = display_uptime();

        if ( strlen( inputBuf ) > 0 ) {
            // TEMPORARY bench command: comment out this block after panic testing.
            // Exact serial-only match; deliberately omitted from the command/help tables.
            if ( strcmp( inputBuf, "panic" ) == 0 ) {
                ASSERT(false);
            }

            const char *result = shell_execute( CONSOLE, inputBuf );
            snprintf( CmdOutput, sizeof CmdOutput, "%s | %s\n",
                      uptimeRendered, inputBuf );
            writeOutput( CONSOLE, CmdOutput );
            if ( strlen(result) ) {
                writeOutput( CONSOLE, result );
                if (result[strlen(result) - 1] != '\n') {
                    writeOutput( CONSOLE, "\n" );
                }
            }
        }
        else {
            snprintf( CmdOutput, sizeof CmdOutput, "%s |\n", uptimeRendered );
            writeOutput( CONSOLE, CmdOutput );
        }
        inputBufPos = 0;            // reset to the beginning of the inputBuf
        memset( inputBuf, 0, sizeof inputBuf );
        prompt( CONSOLE );
    }
}


/**
 * @brief   Drain available serial input until a command is ready.
 *
 * @param input      destination buffer, including space for the terminator
 * @param maxsize    nonzero capacity of the destination buffer
 * @param piIndex    character count, preserved between calls for partial input
 *
 * @returns true on CR, LF, or the full-buffer path; false when available
 *          input ends without completing a command
 *
 * @note    The caller initializes the buffer and count, and resets both after
 *          consuming a command. Stored characters are followed by a NUL.
 *          Ordinary characters are echoed; backspace erases the last one.
 *          CR and LF independently complete a command, so CRLF also produces
 *          an empty command on the following call.
 *
 * @note    If another ordinary character arrives after maxsize - 1 characters
 *          have been stored, it is consumed but not stored. The truncated
 *          command is returned; remaining input is retained in the UART queue.
 */
boolean console_gets( char *input, const unsigned int maxsize, int *piIndex )
{
    char c;

    while ( Serial.available() > 0 ) {
        c = Serial.read();

        // Handle backspace
        if (c == '\b') {
            if (*piIndex > 0) {
                --(*piIndex);
                input[*piIndex] = '\0';
                Serial.write("\b \b");
            }
            continue;
        }

        // Handle CR or LF
        if ((c == '\r') || (c == '\n')) {
            Serial.write("\r\n");
            return true;
        }

        // Return the terminated prefix; this extra character is discarded.
        if (*piIndex >= (maxsize - 1)) {
            return true;
        }

        // Echo and append an ordinary character.
        Serial.write(c);               // echo
        input[*piIndex] = c;
        *piIndex = *piIndex + 1;
        input[*piIndex] = '\0';
    }
    return false;
}


/**
 * @brief   Write one uptime-prefixed diagnostic line to the serial console.
 *
 * @param msg    diagnostic text; a newline is added
 *
 * @note    Uses the shared CmdOutput buffer and display_uptime() helper.
 */
void console_log( const char *msg )
{
    snprintf( CmdOutput, sizeof CmdOutput, "%s | %s\n", display_uptime(), msg );
    Serial.print( CmdOutput );
}



/**
 * @brief   Initialize the Mega's USB-connected UART console at 9600 baud.
 *
 * Allow a short settling interval and separate startup output from earlier
 * monitor text. Unlike native USB Serial, the Mega's UART always reports
 * ready, so checking Serial cannot detect whether a host has opened the port.
 */
void serial_init()
{
    Serial.begin(9600);
    delay(100);

    Serial.println();
    Serial.println("...");
    Serial.flush();
}

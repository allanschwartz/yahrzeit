/**
 * @file        led_console.c
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
//            C O N S O L E
// ----------------------------------------------------------------------------


/**
 * console_init ... initializes global data structures,
 *     display the initial pattern on the LED Matrix
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 */
void console_init( void )
{
    ledWall.begin();

    // splash
    my_puts( CONSOLE, (char *)VersionString );

    // some initial pattern, on power-up (as a self-test)
    console_log( "light test" );
    ledWall.setBrightness( 128 );
    ledWall.allOn( 1, PANEL0 );
    ledWall.allOn( 0, PANEL0 );
    selftest_marching_row( PANEL0 );
    console_log( "ready >" );
}


// ----------------------------------------------------------------------------
//            C O N S O L E   C O M M A N D S
// ----------------------------------------------------------------------------


/**
 * prompt ... print a time-stamped prompt
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 */
static void prompt( byte streamID )
{
      char *uptimeRendered = display_uptime();
      strcat( uptimeRendered, " >" );
      my_puts( streamID, uptimeRendered );
}


/**
 * console_thread ... implements the console I/O gets(), puts() loop.
 *    in each call to console_thread, we either read one line, and execute one command.
 *    or, if the line is not present, we return immediately, but keep state so we can
 *    resume reading the line
 *
 * @returns
 *      returns after every command line is executed
 */
void  console_thread( void )
{
    // the command from the host
#define MAXINPUTLINE 64
    static char     inputBuf[ MAXINPUTLINE ] = { 0 };
    static int      inputBufPos = 0;               // 0..MAXCMDLINE

    if ( console_gets( inputBuf, sizeof inputBuf, &inputBufPos ) ) {
        char *uptimeRendered = display_uptime();

        if ( strlen( inputBuf ) > 0 ) {
            const char *result = cmdProc.execute( CONSOLE, inputBuf );
            if ( strlen(result) ) {
                snprintf( CmdOutput, sizeof CmdOutput, "%s | %s  ---- %s ---- \n",
                          uptimeRendered, inputBuf, result );
            }
            else {
                snprintf( CmdOutput, sizeof CmdOutput, "%s | %s\n",
                          uptimeRendered, inputBuf );
            }
            my_puts( CONSOLE, CmdOutput );
        }
        else {
            snprintf( CmdOutput, sizeof CmdOutput, "%s |\n", uptimeRendered );
            my_puts( CONSOLE, CmdOutput );
        }
        inputBufPos = 0;            // reset to the beginning of the inputBuf
        memset( inputBuf, 0, sizeof inputBuf );
        prompt( CONSOLE );
    }
}


/**
 * console_gets ... this is our fgets() routine which reads a single line from
 *       the Serial UART
 *
 *    console_gets() reads in at most one less than size characters from the
 *    serial UART stream and and stores them into the buffer pointed to by str.
 *    Reading stops after a newline or cr or the str is filled.
 *    If a newline or cr is read, it is not stored into the buffer.
 *    A terminating null byte ('0' is stored after the last character.
 *
 *    This non-threaded version of console_gets() does not block, rather it
 *    returns immediately, that is returns each character as read.
 *    The boolean result code indicates whether a full line was read
 *
 * @param input    the resulting null-terminated string
 * @param maxsize  max size we can store, including a NULL
 * @param piIndex  pointer to a count of the number of bytes read into input
 *
 * @returns boolean, true: a full line was read; false: if not
 */
boolean console_gets( char *input, const unsigned int maxsize, int *piIndex )
{
    char c;

    if ( Serial.available() > 0 ) {
        c = Serial.read();

        // Handle backspace
        if (c == '\b') {
            if (*piIndex > 0) {
                *piIndex--;
                Serial.write("\b \b");
            }
            return false;
        }

        // Handle CR or LF
        if ((c == '\r') || (c == '\n')) {
            Serial.write("\r\n");
            return true;
        }

        // Handling potential overflow of buffer, by just returning
        if (*piIndex >= (maxsize - 1)) {
            return true;
        }

        // normal character
        Serial.write(c);               // echoplex
        input[*piIndex] = c;
        *piIndex = *piIndex + 1;
        input[*piIndex] = '\0';
    }
    return false;
}





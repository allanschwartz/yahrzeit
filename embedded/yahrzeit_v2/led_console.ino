/**
 * @file        led_console.c
 *
 * @brief       Console Interface code for the Yahrzeit Project
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 */

#include "yahrzeit_v2.h"


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
    led_init();

    // splash
    my_puts( CONSOLE, (char *)VersionString );

    // some initial pattern, on power-up (as a self-test)
    console_log( "light test" );
    led_all_on( 1, PANEL0 );
    selftest_marching_row( PANEL0 );
    led_all_on( 0, PANEL0 );
    console_log( "ready >" );
}


// ----------------------------------------------------------------------------
//            C O N S O L E   C O M M A N D S
// ----------------------------------------------------------------------------

/**
 * @COMMAND LANGUAGE
 *      We have implemented a simple command interpreter that recognizes
 *      several commands.
 *      Each command may be abbrieviated to 2 characters.
 */

#if 0
enum commandIds  {
    NONE_OF_THE_ABOVE = 0,
    CMD_ALL = 1,        // turn on/off all LEDs
    CMD_BRIGHT,         // set the brightness
    CMD_DATA,           // set a specific data bit pattern
    CMD_DUMP,           // dump the pixel memory
    CMD_HELP,           // display command help
    CMD_LOAD,           // load the pixel memory from EEPROM
    CMD_PIXEL,          // set a pixel memory on/off
    CMD_REFRESH,        // refresh the LED display from pixel memory
    CMD_SAVE,           // store the pixel memory into EEPROM
    CMD_TEST,           // do one of several LED test pattern
    CMD_NOP,            // (was required on the slower 8051 inplementation)
    MISSING_ARG = 255
};
#endif


static const struct commands {
    const enum commandIds  cmd_identifier;            // see enum cmd_ids above
    const byte  cmd_required_args;
    const char * cmd_verb;
} Commands[] = {
    { CMD_ALL,    1, "all"    },
    { CMD_BRIGHT, 1, "bright" },
    { CMD_DATA,   3, "data"   },
    { CMD_DUMP,   0, "dump"   },
    { CMD_HELP,   0, "help"   },
    { CMD_HELP,   0, "?"      },
    { CMD_LOAD,   0, "load"   },
    { CMD_PIXEL,  3, "pixel"  },
    { CMD_REFRESH, 0, "refresh" },
    { CMD_SAVE,   0, "save"   },
    { CMD_TEST,   0, "test"   },
    { CMD_NOP,    0, "nop"    },
    { CMD_NOP,    0, "# "     },
    { CMD_NOP,    0, "\r"     },
    { CMD_NOP,    0, "\n"     },
    { NONE_OF_THE_ABOVE, 0, 0 },
};


/**
 * match_cmd_verb ...  matches argv[0] as a command verb in the Commands table
 *
 * @param argv    argument array, required arguments are checked
 *
 * @returns an enumerator representing the command identifier
 */
static enum commandIds  match_cmd_verb( char **argv )
{
    for ( const struct commands *p = &Commands[0]; p->cmd_verb != NULL; p++ ) {
        if ( STRMATCH2( argv[0], p->cmd_verb ) ) {
            if ( argv[p->cmd_required_args] != NULL ) {
                return p->cmd_identifier;
            }
            else {
                return MISSING_ARG;
            }
        }
    }
    return NONE_OF_THE_ABOVE;
}


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
            const char *result = shell_execute( CONSOLE, inputBuf );
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


/**
 * shell_execute ... perform one command
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param command     the unparsed command to interpret and execute
 *
 * @returns        a string representing the command output
 */
const char * shell_execute( byte streamID, char *command )
{
#define MAXARGS 5
    char *   arg_string[ MAXARGS ];
    int      arg_value[ MAXARGS ];

    if ( command[0] == 0 || command[0] == '#' ) {
        return "";
    }

#define OPTIONAL_ARG(i)       (arg_string[i] != NULL)

    memset( arg_string, 0, sizeof arg_string );
    parse_command(command, arg_string);         // tokenize the command into arg_string[]

    // as an code-size optimization, lets convert argv[1] ... argv[n] from ascii to an int
    for (byte i = 0; i < MAXARGS; i++ ) {
        if ( OPTIONAL_ARG(i) ) {
            arg_value[i] = atoi( arg_string[i] );
        }
        else {
            arg_value[i] = 0;
        }
    }

    enum commandIds  id = match_cmd_verb( arg_string );
    int rc;                                        // result code
    switch ( id ) {

        case CMD_ALL:
            // all on|off [<panel>]
            rc = led_all_on(
                     onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                     arg_value[2] );               /* <panel> */

            break;

        case CMD_BRIGHT:
            // bright <intensity>
            rc = led_set_intensity( arg_value[1] );
            LedMatrix.refresh();
            break;

        case CMD_DATA:
            //  data <row> <col> <binary data>
            rc = console_data_cmd(
                     arg_value[1],                 /* row */
                     arg_value[2],                 /* col */
                     arg_string[3] );              /* binary data */
            break;

        case CMD_DUMP:
            // dump [<panel>]
            rc = console_dump( streamID,
                     arg_value[1] );               /* optional <panel> */ 
            break;

        case CMD_HELP:
            // help or ? 
            return HelpText;

        case CMD_PIXEL:
            // pixel 0|1 <row> <col> [<panel>]
            if ( OPTIONAL_ARG(4) ) {
                rc = led_store_in_panel(
                         onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                         arg_value[2],              /* row */
                         arg_value[3],              /* column */
                         arg_value[4] );            /* panel */
            }
            else {
                rc = led_store_in_array(
                         onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                         arg_value[2],              /* row */
                         arg_value[3] );            /* column */
            }
            break;

        case CMD_REFRESH:
            LedMatrix.refresh();
            rc = NO_ERROR;
            break;

        case CMD_LOAD:
            led_loaddata();
            rc = NO_ERROR;
            break;

        case CMD_SAVE:
            led_savedata();
            rc = NO_ERROR;
            break;

        case CMD_TEST:
            // test <testnumber> [<panel>]
            if ( !OPTIONAL_ARG(1) ) {
                return TestMenu;
            }
            rc = selftest(
                     streamID,
                     arg_value[1],                 /* test number */
                     arg_value[2],                 /* optional <panel> */
                     OPTIONAL_ARG(3) ? arg_value[3] : 3 );    /* optional repeat count */
            break;

        case CMD_NOP:
            return "";

        case MISSING_ARG:
            rc = ERR_MISSING;
            break;

        case 0:
        default:
            rc = ERR_SYNTAX;
    }
    return ResultStrings[rc];
}


/**
 * parse_cmd_bufer ... function to parse the args in the command input
 *
 * @param command   the unparsed command
 * @param argv      the resulting argument vector
 */
void  parse_command( char *command, char **argv )
{
    char *p = command;

    for ( byte i = 0; i < MAXARGS; i++ ) {
        // skip white space
        while ( *p && isspace(*p) ) {
            p++;
        }
        if ( *p == 0 )
            return;
        argv[i] = p;       // record where arg starts
        // walk over this arg
        while ( *p && !isspace(*p) ) {
            p++;
        }
        if ( *p == 0 )
            return;
    }
}


/**
 * onoff_bool -- convert a string representing on/off, or true/false, or 1/0
 * to a boolean
 *
 * @param token       the unparsed string
 *
 * @returns true or false
 */
boolean    onoff_bool( char *token )
{
    if ( token == NULL )
        return false;
    if ( STRMATCH2( token, "true" ) )
        return true;
    if ( STRMATCH2( token, "on" ) )
        return true;
    if ( STRMATCH2( token, "false" ) )
        return false;
    if ( STRMATCH2( token, "off" ) )
        return false;
    if ( isdigit( *token ) )
        return ( atoi( token ) );

    return false;
}


/**
 * console_data_cmd ... performs the action of the "DATA" command.
 *    The given data is decoded, stored into the led data-store
 *
 * @param row       starting row number
 * @param col       the col number
 * @param bindata   binary-encoded data. i.e., 0s and 1s in ascii chars
 *
 * @returns        NO_ERROR or an error code
 */
int console_data_cmd( byte row, byte col, char *bindata )
{
    if ( row > NROWS ) {
        return ERR_ROW;
    }
    if ( col > NCOLS ) {
        return ERR_COL;
    }

    boolean bitvalue;
    for ( char *p = bindata; *p; p++, row++ ) {
        if ( *p == '0' ) {
            bitvalue = 0;
        }
        else if (*p == '1' ) {
            bitvalue = 1;
        }
        else {
            continue;
        }
        if ( row > NROWS ) {
            return ERR_ROW;
        }
        led_store_in_array( bitvalue, row, col);
    }
    return NO_ERROR;
}


#if 0
/**
 * hexdump -- a general purpose routine to dump a section of memory in hex
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param addr        pointer to memory to dump
 * @param len         number of bytes
 *
 * @returns         n/a
 */
void  hexdump( byte streamID, void *addr, const unsigned int len )
{
    char buf[40];
    byte *p = (byte *)addr;
    for ( unsigned int offset = 0; offset < len; offset++ ) {
        if ( (offset & 31 )  == 0 )  {
            sprintf( buf, "\r\n%04x: ", offset );
            my_puts( streamID, buf );
        }
        sprintf ( buf, "%02x ", p[offset] );
        my_puts( streamID, buf );
    }
    my_puts( streamID, "\n" );
}
#endif


/**
 * console_dump ... debug function to dump the LED data 
 *
 * @param panel     panel number 0 or [1..NPANELS]
 *
 * @returns        NO_ERROR or ERR_PANEL
 */
int  console_dump( byte streamID, byte panel )
{
    if (panel > NPANELS ) {
        return ERR_PANEL;
    }
    
    if ( panel == PANEL0 ) {
        
        my_puts( streamID, "Dump of Pixel Data\n" );
        for ( byte row = 1; row <= NROWS; row++ ) {
            sprintf( CmdOutput, "%02d:  ", row );
            for ( byte col = 1; col <= NCOLS; col++ ) {
                #if CBS_56x40_WALL
                if ( col == 6 || col == 12 || col == 18 || col == 24 || col == 30 || col == 36 ) {
                   strcat( CmdOutput, " .  " ); 
                }
                #endif
                boolean pixel = led_pixel_value(  row,  col );
                strcat( CmdOutput, pixel ? "1 " : "0 " );
            }
            strcat( CmdOutput, "\n" );
            my_puts( streamID, CmdOutput );
            if ( row == 16 || row == 38 ) {
                my_puts( streamID,  "     .........     ...........     ...........     ...........     ...........     ...........     .........\n" );
            }
        }
        my_puts( streamID, "\n" );   
    }
    else {

        sprintf( CmdOutput, "Dump of Pixel Data - panel %d\n", panel );
        my_puts( streamID, CmdOutput);
        for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
            CmdOutput[0] = '\0';
            for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
                boolean pixel = led_pixel_value(  
                                        led_row_of_panel[panel] + row - 1,
                                        led_col_of_panel[panel] + col - 1 );
                strcat( CmdOutput, pixel ? "1 " : "0 " );
            }
            strcat( CmdOutput, "\n" );
            my_puts( streamID, CmdOutput );
        }
        my_puts( streamID, "\n" ); 
    }
    return NO_ERROR;
}


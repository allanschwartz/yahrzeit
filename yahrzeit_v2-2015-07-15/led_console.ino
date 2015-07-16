/**
 * @file        led_console.c
 *
 * @brief       Console Interface code for the Yahrzeit Project
 *
 * @history     version 0.4 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in 2015
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
void console_init( int streamID )
{
    led_init();

    // splash
    my_puts( streamID, VersionString );

    // some initial pattern, on power-up (as a self-test)
    console_log("light test");
    led_all_on( 1, PANEL0 ); 
    selftest_marching_row( PANEL0 );
    led_all_on( 0, PANEL0 ); 
    console_log("ready >");
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
enum cmd_ids  {
    MISSING_ARG = -1,
    NONE_OF_THE_ABOVE = 0,
    CMD_ALL     = 1,    // turn on/off all LEDs
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
};
#endif


const struct cmd_s {
    enum cmd_ids  cmd_identifier;            // see enum cmd_ids above
    const uint8_t  cmd_required_args;
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
static enum cmd_ids  match_cmd_verb( char **argv )
{
    for ( const struct cmd_s *p = &Commands[0]; p->cmd_verb != NULL; p++ ) {
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
static void prompt( int streamID )
{
    char temp[40];
    snprintf( temp, sizeof temp, "%s >", display_time( millis() ));
    my_puts ( streamID, temp );
}


/**
 * console_thread ... implements the console I/O gets(), puts() loop.
 *    in each call to console_thread, we read one line, and execute one command.
 *
 * @returns
 *      returns after every command line is executed
 */
void  console_thread( void )
{
    // the command from the host
#define MAXCMDLINE 64
    static char     cmd_input[ MAXCMDLINE ] = { 0 };
    static int      cmd_pos = 0;               // 0..MAXCMDLINE

    if ( console_gets( cmd_input, sizeof cmd_input, &cmd_pos ) ) {
        char *t_rendered = display_time(millis());
  
        if ( strlen( cmd_input ) > 0 ) {
            const char *result = shell_execute( SOCKET, cmd_input );
            if ( strlen(result) ) {
                snprintf( CmdOutput, sizeof CmdOutput, "%s | %s  ---- %s ---- \n",
                        t_rendered, cmd_input, result );
            }
            else {
                snprintf( CmdOutput, sizeof CmdOutput, "%s | %s\n",
                        t_rendered, cmd_input );
            }
            my_puts( SOCKET, CmdOutput );
        }
        else {
            snprintf( CmdOutput, sizeof CmdOutput, "%s |\n", t_rendered );
            my_puts( SOCKET, CmdOutput );
        }
        cmd_pos = 0;            // reset to the beginning of the cmd_input buffer
        memset( cmd_input, 0, sizeof cmd_input );
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
 *    returns immediately, that is returns each character is read.
 *    The boolean result code indicates whether a full line was read
 *
 * @param str       the resulting null-terminated string
 * @param size      max size we can store, including a NULL
 * @param iLen      pointer to a count of the number of bytes read into str
 *
 * @returns boolean, true: a full line was read; false: if not
 */
boolean console_gets(char *str, const int size, int *piIndex)
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
        if (*piIndex >= (size - 1)) {
            return true;
        }

        // normal character
        Serial.write(c);               // echoplex
        str[*piIndex] = c;
        *piIndex = *piIndex + 1;
        str[*piIndex] = '\0';
    }
    return false;
}


/**
 * shell_execute ... perform one command
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param str         the unparsed command to execute
 *
 * @returns        a string representing the command output
 */
const char * shell_execute(int streamID, char *str) 
{    
    #define MAXARGS 5
    char *   arg_string[ MAXARGS ];        // this is an auto (on the stack)
    int      arg_value[ MAXARGS ];

    if ( str[0] == 0 || str[0] == '#' ) {
        return "";
    }

#define OPTIONAL_ARG(i)       (arg_string[i] != NULL)

    memset( arg_string, 0, sizeof arg_string );
    parse_command(str, arg_string);         // tokenize the string str into arg_string[]

    // as an code-size optimization, lets convert argv[1] ... argv[n] from ascii to an int
    for (byte i = 0; i < MAXARGS; i++ ) {
        if ( OPTIONAL_ARG(i) ) {
            arg_value[i] = atoi( arg_string[i] );
        }
        else {
            arg_value[i] = 0;
        }
    } 
                                    
    enum cmd_ids  id = match_cmd_verb(arg_string);
    switch ( id ) {

        case CMD_ALL:
        {
            // all on|off [<panel>]
            int rc = led_all_on(
                onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                arg_value[2]                  /* <panel> */
            );
            return error_strings[rc];
            break;
        }

        case CMD_BRIGHT:
        {
            int rc = led_set_intensity( arg_value[1] );
            LedMatrix.refresh();
            return error_strings[rc];
        }

        case CMD_DATA:
        {
            //  "DAta <row> <col> <binary data>" },
            int rc = console_data_cmd( 
                              arg_value[1],     /* row */
                              arg_value[2],     /* col */
                              arg_string[3] );  /* binary data */
            return error_strings[rc];
        }

        case CMD_DUMP:
            console_dump( streamID );
            return OK;

        case CMD_HELP:
            // help or ? or anything unrecognized
            return 
                    "All  on|off [<panel>]\n"
                    "BRightness <n> (1:low, 10:high)\n"
                    "DAta <row> <col> <binary data>\n"
                    "DUmp\n"
                    "HElp\n"
                    "LOad\n"
                    "PIxel on|off <row> <col> [<panel>]\n"
                    "REfresh\n"
                    "SAve\n"
                    "TEst <testnumber> [<panel>]";

        case CMD_PIXEL:
        {
            int rc;
            // pixel 0|1 <row> <col> [<panel>]
            if ( OPTIONAL_ARG(4) ) {
                rc = led_store_in_panel(
                        onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                        arg_value[2],               /* row */
                        arg_value[3],               /* column */
                        arg_value[4] );             /* panel */
            }
            else {
                rc = led_store1(
                        onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                        arg_value[2],               /* row */
                        arg_value[3] );             /* column */
            }
            return error_strings[rc];
        }
        case CMD_REFRESH:
            LedMatrix.refresh();
            return OK;

        case CMD_LOAD:
            led_loaddata();
            return OK;

        case CMD_SAVE:
            led_savedata();
            return OK;

        case CMD_TEST:
        {
            int rc;
            // test <testnumber> [<panel>]
            if ( !OPTIONAL_ARG(1) ) {
                return
                    "TEst 1 [<panel>]     --   4 corners ON\n"
                    "TEst 2 [<panel>]     --   all pixels ON\n"
                    "TEst 3 [<panel>]     --   all pixels OFF\n"
                    "TEst 4 [<panel>] [k] --   all ON / all OFF\n"
                    "TEst 5 [<panel>] [k] --   marching row pattern\n"
                    "TEst 6 [<panel>] [k] --   marching column pattern\n"
                    "TEst 7 [<panel>] [k] --   Cylon pattern";
            }
            rc = selftest(
                        streamID,
                        arg_value[1],             /* test number */
                        arg_value[2],             /* optional <panel> */
                        OPTIONAL_ARG(3) ? arg_value[3] : 3 );    /* optional repeat count */
 
            return error_strings[rc];
        }
        case CMD_NOP:
            return "";
            
        case MISSING_ARG:
            return "MISSING ARG";
            
        case 0:
        default:
            return "Eh?";
    }
}


/**
 * parse_cmd_bufer ... function to parse the args in the command input
 *
 * @param str       the unparsed command
 * @param argv      the resulting argument vector
 */
void  parse_command(char *str, char **argv)
{
    char *p = str;

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
 * @param str       the unparsed string
 *
 * @returns true or false
 */
boolean    onoff_bool( char *str )
{
    if ( str == NULL )
        return false;
    if ( STRMATCH2( str, "true" ) )
        return true;
    if ( STRMATCH2( str, "on" ) )
        return true;
    if ( STRMATCH2( str, "false" ) )
        return false;
    if ( STRMATCH2( str, "off" ) )
        return false;
    if ( isdigit( *str ) )
        return ( atoi( str ) );

    return false;
}


/**
 * console_data_cmd ... performs the action of the "DATA" command.
 *    The given data is decoded (dehexified, stored into led data XRAM data,
 *    then the display is refreshed.
 *
 * @param row       starting row number
 * @param col       the col number
 * @param bindata   binary-encoded data. i.e., 0s and 1s in ascii chars
 *
 * @returns        NO_ERROR or an error code
 */
int console_data_cmd( byte row, int col, char *bindata )
{
    if ( row > NROWS ) {
        return ERR_ROW;
    }
    if ( col > NCOLS ) {
        return ERR_COL;
    }

    boolean bitval;
    for ( char *p = bindata; *p; p++, row++ ) {
        if ( *p == '0' ) {
            bitval = 0;
        }
        else if (*p == '1' ) {
            bitval = 1;
        }
        else {
            continue;
        }
        if ( row > NROWS ) {
            return ERR_ROW;
        }
        led_store1( bitval, row, col);
    }
    return NO_ERROR;
}


/**
 * hexdump -- a general purpose routine to dump a section of memory in hex
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param p           pointer to memory to dump
 * @param len         number of bytes
 *
 * @returns         n/a
 */
void  hexdump( int streamID, void *addr, unsigned int len )
{
    char buf[40];
    byte *p = (byte *)addr;
    for ( unsigned int offset = 0; len; offset++, len-- ) {
        if ( (offset & 31 )  == 0 )  {
            sprintf( buf, "\r\n%04x: ", offset );
            my_puts( streamID, buf );
        }
        sprintf ( buf, "%02x ", p[offset] );
        my_puts( streamID, buf );
    }
    my_puts( streamID, "\n" );
}


/**
 * console_dump ... debug routine to dump the LED data area
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 */
void  console_dump( int streamID )
{
    char buf[32];
    my_puts( streamID, "DEBUG data .. column data:\n" );

    for ( byte col = 1; col <= NCOLS; col++ ) {

        sprintf( buf, "col %04d: ", col ); 
        my_puts( streamID, buf );
        for ( byte row = 1; row <= NROWS; row ++ ) {
            sprintf(buf, "%d ", led_data1(  row,  col ));
            my_puts( streamID, buf );
        }
        my_puts( streamID, "\n" );
    }
    my_puts( streamID, "\n" );
}


/**
 * @file  	led_console.c
 *
 * @brief	Console Interface code for the Yahrzeit Project
 *
 * @history
 *              version 0.4 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 */

#include "yahrzeit_r2.h"


// ----------------------------------------------------------------------------
//            G L O B A L   S T O R A G E
// ----------------------------------------------------------------------------

static const char VersionString[] =
    "LED Controller, V2.0, 2015-06-26\n"
    "Copywrite (c) 2008-15, AMS Consulting\n"
    "----------";


// ----------------------------------------------------------------------------
//            C O N S O L E
// ----------------------------------------------------------------------------


/**
 * console_init ... initializes global data structures,
 *     display the initial pattern on the LED Matrix
 */
void console_init( void ) {
    led_init();

    // splash
    Serial.println( VersionString );

    // some initial pattern, on power-up (as a self-test)
    selftest_all_on( 0, /* panel */ 0);         // all off
    selftest_marching_row( /* panel */ 0 ); 
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

enum cmd_ids  {
    NONE_OF_THE_ABOVE = 0,
    CMD_ALL     = 1,
    CMD_BRIGHT,
    CMD_DATA,
    CMD_DUMP,
    CMD_HELP,
    CMD_LOAD,
    CMD_PIXEL,
    CMD_REFRESH,
    CMD_SAVE,
    CMD_TEST,
    CMD_NOP,    // this was required on the slower 8051 inplementation
};


const struct cmd_s {
    enum cmd_ids  cmd_identifier;            // see enum cmd_ids above
    const uint8_t  cmd_required_args;
    const char * cmd_verb;
    const char * cmd_help;
} Commands[] = {
    { CMD_ALL,    1, "all",    "All  on|off [<panel>]" },
    { CMD_BRIGHT, 1, "bright", "BRightness <n> (1:low, 10:high)" },
    { CMD_DATA,   3, "data",   "DAta <row> <col> <binary data>" },
    { CMD_DUMP,   0, "dump",   "DUmp" },
    { CMD_HELP,   0, "help",   "HElp" },
    { CMD_HELP,   0, "?",       0 }, 
    { CMD_LOAD,   0, "load",   "LOad" },
    { CMD_PIXEL,  3, "pixel",  "PIxel on|off <row> <col> [<panel>]" },
    { CMD_REFRESH, 0, "refresh", "REfresh" },
    { CMD_SAVE,   0, "save",   "SAve" },
    { CMD_TEST,   0, "test",   "TEst <testnumber> [<panel>]\n" },
    { CMD_NOP,    0, "nop",    0 },
    { NONE_OF_THE_ABOVE, 0, 0 }
};


/**
 * match_cmd_verb, matches argv[0] in the Command table to a command
 *
 * @param argv    argument array, required arguments are checked
 *
 * @returns command identifier
 */
static enum cmd_ids  match_cmd_verb(char **argv)
{    
    for ( const struct cmd_s *p = &Commands[0]; *p->cmd_verb; p++ ) {
        if ( STRMATCH2( argv[0], p->cmd_verb ) ) {
            if ( argv[p->cmd_required_args] != NULL ) {
                return p->cmd_identifier;
            }
            else {
                Serial.println( "missing arg" );
                return NONE_OF_THE_ABOVE;
            }
        }
    }
    return NONE_OF_THE_ABOVE;
}


/**
 * cmd_help ... function to print the help text
 *    uses the global command
 *
 * @returns n/a
 */
static void  cmd_help(void)
{
    Serial.println( VersionString );

    for ( const struct cmd_s *p = &Commands[0]; *p->cmd_verb; p++ ) {
        if ( p->cmd_help ) {
            Serial.println( p->cmd_help );
        }
    }
}


/**
 * console_thread .. implement the console i/o gets(), puts() loop.
 *    each call to console_thread, we read one line, and execute one command.
 *
 * @returns
 *      returns after every command line is executed
 */
void  console_thread( void )
{
    // the command from the host
#define MAXCMDLINE 64
    static char     cmd_buffer[ MAXCMDLINE ];
    static int      cmd_pos;               // 0..MAXCMDLINE
    
    static bool firsttime = true;
    
    // print the first prompt, and prepare to read input
    if (firsttime) {
        Serial.print("> ");
        memset( cmd_buffer, 0, sizeof cmd_buffer );
        cmd_pos = 0;
        firsttime = false;
    }
    
    if ( console_gets( cmd_buffer, sizeof cmd_buffer, &cmd_pos ) ) {
        console_execute( cmd_buffer );
        firsttime = true;
    }
    else {
        sleep_ms(false, 2);
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
boolean console_gets(char *str, const int size, int *piIndex) {
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
 * console_execute ... perform one command
 *
 * @param str       the unparsed command to execute
 */
void console_execute(char *str) {

    #define MAXARGS 5
    char *   Arguments[ MAXARGS ];        // this is an auto (on the stack)

#define REQUIRED_ARG(i)       (Arguments[i] != NULL)
#define OPTIONAL_ARG(i)       (Arguments[i] != NULL)

    memset( Arguments, 0, sizeof Arguments );
    parse_command(str, Arguments);         // tokenize the string str into Arguments[]

    // as an code-size optimization, lets convert argv[1] & argv[2] from ascii to an int
    int arg1 = 0, arg2 = 0, arg3 = 0;
    if ( OPTIONAL_ARG(1) ) {
        arg1 = atoi( Arguments[1] );
    }
    if ( OPTIONAL_ARG(2) ) {
        arg2 = atoi( Arguments[2] );
    }
    if ( OPTIONAL_ARG(3) ) {
        arg3 = atoi( Arguments[3] );
    }

    if ( Arguments[0] == NULL ) {
        Serial.println("eh?");
        return;
    }

    enum cmd_ids  id = match_cmd_verb(Arguments);
    switch ( id ) {

        case CMD_ALL:
            // all on|off [<panel>]
            selftest_all_on(
                onoff_bool( Arguments[1] ),  /* "on"|"off" or "0"|"1" */
                OPTIONAL_ARG(2) ? arg2 : 0  /* optional <panel> */
            );
            break;

        case CMD_BRIGHT:
            led_set_intensity(arg1);
            LedMatrix.refresh();
            break;

        case CMD_DATA:
            //  "DAta <row> <col> <binary data>" },
            console_data_cmd( arg1,             /* row */
                              arg2,             /* col */
                              Arguments[3] );   /* bindary data */
            break;

        case CMD_DUMP:
            led_debug();
            break;

        case CMD_HELP:
            // help or ? or anything unrecognized
            cmd_help();
            break;

        case CMD_PIXEL:
            // pixel 0|1 <row> <col> [<panel>]
            if ( OPTIONAL_ARG(4) ) {
                led_store_in_panel(
                    onoff_bool( Arguments[1] ),  /* "on"|"off" or "0"|"1" */
                    atoi( Arguments[4] ),   /* panel */
                    arg2,                   /* row */
                    arg3 );                 /* column */
            }
            else {
                led_store1(
                    onoff_bool( Arguments[1] ),  /* "on"|"off" or "0"|"1" */
                    arg2,                   /* row */
                    arg3 );                 /* column */
            }
            break;

        case CMD_REFRESH:
            LedMatrix.refresh();
            break;

        case CMD_LOAD:
            led_loaddata();
            Serial.println( "OK" );
            break;

        case CMD_SAVE:
            led_savedata();
            Serial.println( "OK" );
            break;

        case CMD_TEST:
            // test <testnumber> [<panel>]
            if ( REQUIRED_ARG(1) ) {
                selftest(
                    arg1,                           /* test number */
                    OPTIONAL_ARG(2) ? arg2 : 0 ,    /* optional <panel> */
                    OPTIONAL_ARG(3) ? arg3 : 3 );   /* optional repeat */
            }
            else {
                Serial.println(
                    "TEst 1 [<panel>]     --   4 corners ON\n"
                    "TEst 2 [<panel>]     --   all pixels ON\n"
                    "TEst 3 [<panel>]     --   all pixels OFF\n"
                    "TEst 4 [<panel>] [k] --   all ON / all OFF\n"
                    "TEst 5 [<panel>] [k] --   marching row pattern\n"
                    "TEst 6 [<panel>] [k] --   marching column pattern\n"
                    "TEst 7 [<panel>] [k] --   Cylon pattern\n"
                    "TEst 8 [<panel>] [k] --   lots of tests" );
            }
            break;

        case CMD_NOP:
            break;

        case 0:
        default:
            Serial.println("eh?");
            break;
    }
}


/**
 * parse_cmd_bufer ... function to break apart the DATA.Arguments
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
 * to a bool
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
 */
void  console_data_cmd( byte row, int col, char *bindata )
{
    if ( row > NROWS ) {
        Serial.println( "ERR row" );
        return;
    }
    if ( col > NCOLS ) {
        Serial.println( "ERR col" );
        return;
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
            Serial.println( "ERR row" );
            return;
        }
        led_store1( bitval, row, col);
    }
    Serial.println( "OK" );
}


#if 0
/**
 * hexdump -- a general purpose routine to dump a section of memory in hex
 *
 * @param p   pointer to memory to dump
 * @param len    number of bytes
 * @returns         n/a
 */
void  hexdump( unsigned char *p, unsigned int len )
{
    for ( unsigned int offset = 0; len; offset++, len-- ) {
        if ( (offset & 31 )  == 0 )  {
            printf( "%04x: ", offset );
        }
        printf ( "%02x ", p[offset] );
    }
    printf( "\n" );
}
#endif


/**
 * led_debug ... debug routine to display the LED data area
 */
void  led_debug()
{
    char buf[32];
    Serial.println( "DEBUG data .. column data:" );

    for ( byte col = 1; col <= NCOLS; col++ ) {

        sprintf( buf, "col %04d: ", col ); Serial.print(buf);
        for ( byte row = 1; row <= NROWS; row ++ ) {
            sprintf(buf, "%d ", led_data1(  row,  col ));
            Serial.print(buf);
        }
        Serial.println("");
    }
    Serial.println( "" );
    Serial.println( "OK" );
}


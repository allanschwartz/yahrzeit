/**
 * @file        CmdProc.cpp
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

#include <Arduino.h>
#include "yahrzeit_v3.h"
#include "CmdProc.h"


CmdProc::CmdProc(LedWall& wall)
    : ledWall_(wall)
{
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

#if 0       // this is now declared in the .h file
enum commandIds : byte  {
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

struct Command {
    commandIds  id;            // see enum cmd_ids above
    byte  requiredArgs;
    const char * verb;
};

static constexpr struct Command commands[] = {
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
    { NONE_OF_THE_ABOVE, 0, 0 },
};

/**
 * match_cmd_verb ...  matches argv[0] as a command verb in the Commands table
 *
 * @param argv    argument array, required arguments are checked
 *
 * @returns an enumerator representing the command identifier
 */
commandIds CmdProc::match_cmd_verb( char **argv )
{
    for ( const Command *p = &commands[0]; p->verb != NULL; p++ ) {
        if ( STRMATCH2( argv[0], p->verb ) ) {
            if ( argv[p->requiredArgs] != NULL ) {
                return p->id;
            }
            else {
                return MISSING_ARG;
            }
        }
    }
    return NONE_OF_THE_ABOVE;
}

/**
 * shell_execute ... perform one command
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param command     the unparsed command to interpret and execute
 *
 * @returns        a string representing the command output
 */
const char *CmdProc::execute( byte streamID, char *command )
{
    static constexpr byte kMaxArgs = 5;
    char *   arg_string[ kMaxArgs ];
    int      arg_value[ kMaxArgs ];

    if ( command[0] == 0 || command[0] == '#' || command[0] == '\r' || command[0] == '\n') {
        return "";
    }

#define OPTIONAL_ARG(i)       (arg_string[i] != NULL)

    memset( arg_string, 0, sizeof arg_string );
    parse_command(command, arg_string);         // tokenize the command into arg_string[]

    // as an code-size optimization, lets convert argv[1] ... argv[n] from ascii to an int
    for (byte i = 0; i < kMaxArgs; ++i ) {
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
            rc = ledWall_.allOn(
                     onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                     arg_value[2] );               /* <panel> */

            break;

        case CMD_BRIGHT:
            // bright <intensity>
            rc = ledWall_.setBrightness( arg_value[1] );
            ledWall_.refresh();
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
                rc = ledWall_.storeInPanel(
                         onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                         arg_value[2],              /* row */
                         arg_value[3],              /* column */
                         arg_value[4] );            /* panel */
            }
            else {
                rc = ledWall_.storeInArray(
                         onoff_bool( arg_string[1] ),  /* "on"|"off" or "0"|"1" */
                         arg_value[2],              /* row */
                         arg_value[3] );            /* column */
            }
            break;

        case CMD_REFRESH:
            ledWall_.refresh();
            rc = NO_ERROR;
            break;

        case CMD_LOAD:
            ledWall_.loadPixels();
            rc = NO_ERROR;
            break;

        case CMD_SAVE:
            ledWall_.savePixels();
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
void  CmdProc::parse_command( char *command, char **argv )
{
    char *p = command;

    for ( byte i = 0; i < kMaxArgs; ++i ) {
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
boolean CmdProc::onoff_bool( char *token )
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
int CmdProc::console_data_cmd( byte row, byte col, char *bindata )
{
    if ( row > NROWS ) {
        return ERR_ROW;
    }
    if ( col > NCOLS ) {
        return ERR_COL;
    }

    boolean bitvalue;
    for ( char *p = bindata; *p; ++p, ++row ) {
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
        ledWall_.storeInArray(bitvalue, row, col);
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
void hexdump( byte streamID, void *addr, const unsigned int len )
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
int CmdProc::console_dump( byte streamID, byte panel )
{
    if (panel > NPANELS ) {
        return ERR_PANEL;
    }
    
    if ( panel == PANEL0 ) {
        
        my_puts( streamID, "Dump of Pixel Data\n" );
        for ( byte row = 1; row <= NROWS; row++ ) {
            sprintf( CmdOutput, "%02d:  ", row );
            for ( byte col = 1; col <= NCOLS; ++col ) {
                #if CBS_56x40_WALL
                if ( col == 6 || col == 12 || col == 18 || col == 24 || col == 30 || col == 36 ) {
                   strcat( CmdOutput, " .  " ); 
                }
                #endif
                boolean pixel = ledWall_.pixelValue( row,  col );
                strcat( CmdOutput, pixel ? "1 " : "0 " );
            }
            strcat( CmdOutput, "\n" );
            my_puts( streamID, CmdOutput );
            #if CBS_56x40_WALL
            if ( row == 16 || row == 38 ) {
                my_puts( streamID,  "     .........     ...........     ...........     ...........     ...........     ...........     .........\n" );
            }
            #else
            if ( row == 8 || row == 16 || row == 24) {
                my_puts( streamID, "      - - - \n");
            }
            #endif
        }
        my_puts( streamID, "\n" );   
    }
    else {

        sprintf( CmdOutput, "Dump of Pixel Data - panel %d\n", panel );
        my_puts( streamID, CmdOutput);
        for ( byte row = 1; row <= ledWall_.rowsInPanel(panel); ++row ) {
            CmdOutput[0] = '\0';
            for ( byte col = 1; col <= ledWall_.colsInPanel(panel); ++col ) {
                boolean pixel = ledWall_.pixelValueInPanel(row, col, panel);
                strcat( CmdOutput, pixel ? "1 " : "0 " );
            }
            strcat( CmdOutput, "\n" );
            my_puts( streamID, CmdOutput );
        }
        my_puts( streamID, "\n" ); 
    }
    return NO_ERROR;
}


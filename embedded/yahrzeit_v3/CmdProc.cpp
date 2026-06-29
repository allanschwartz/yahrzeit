/**
 * @file        CmdProc.cpp
 *
 * @brief       Command processor for console and socket interfaces.
 *
 *              Parses ASCII command lines, maps command verbs to internal
 *              command identifiers, validates arguments, and dispatches
 *              operations to LedWall and related subsystems.
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
//            C O N S T R U C T O R
// ----------------------------------------------------------------------------

/**
 * @brief   Construct the command processor.
 *
 * @param wall      logical LED wall abstraction
 */
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
 *      Each command may be abbreviated to 2 characters.
 */

#if 0       // this is now declared in the .h file
enum CommandIds : byte  {
    NONE_OF_THE_ABOVE = 0,  // must be zero
    CMD_ALL = 1,            // turn on/off all LEDs
    CMD_BRIGHT,             // set the brightness
    CMD_CLEAR,              // clear one panel
    CMD_DATA,               // set a specific data bit pattern
    CMD_DUMP,               // dump the pixel memory
    CMD_HELP,               // display command help
    CMD_LOAD,               // load the pixel memory from EEPROM
    CMD_PIXEL,              // set one pixel on/off
    CMD_REFRESH,            // refresh the LED display from pixel memory
    CMD_SAVE,               // store the pixel memory into EEPROM
    CMD_STATUS,             // dump the current status/settings
    CMD_TEST,               // run one of several LED test patterns
    CMD_TIMING,             // toggle or turn on/off the timing instrumentation
    CMD_VERSION,            // print the version string
    CMD_NOP,                // was required on the slower 8051 implementation
    MISSING_ARG = 255
};
#endif
namespace {     // private to this file

/**
 * @brief Command descriptor.
 *
 * Associates a textual command verb with an internal command identifier
 * and required argument count.
 */
struct Command {
    CommandIds  id;             // see enum cmd_ids above
    byte  requiredArgs;         // 0, 1, 2 or 3 required arguments
    const char *verb;          // the last entry is nullptr
};

/**
 * @brief Table of supported commands.
 *
 * The table is terminated by a null verb pointer.
 */
static constexpr Command commands[] = {
    { CMD_ALL,    1, "all"    },
    { CMD_BRIGHT, 1, "bright" },
    { CMD_CLEAR,  1, "clear"  },
    { CMD_DATA,   3, "data"   },
    { CMD_DUMP,   0, "dump"   },
    { CMD_HELP,   0, "help"   },
    { CMD_HELP,   0, "?"      },
    { CMD_LOAD,   0, "load"   },
    { CMD_PIXEL,  3, "pixel"  },
    { CMD_REFRESH, 0, "refresh" },
    { CMD_SAVE,   0, "save"   },
    { CMD_STATUS, 0, "status" },
    { CMD_TIMING, 0, "timing" },
    { CMD_TEST,   0, "test"   },
    { CMD_VERSION, 0, "version" },
    { CMD_NOP,    0, "nop"    },
};

// Each command returns a ResultId,
// defined in LedWall.h.  Here are the corresponding strings:
const char* const resultStrings[8] = {
    "OK", "Eh?", "ERR Missing Arg", "ERR Row", 
    "ERR Col", "ERR Panel", "ERR Brightness", "ERR Test Number"
};

// displayed with the "help" command, or any syntax error
const char HelpText[]  = 
    "\n"
    "\tAll  on|off [<panel>]\n"
    "\tBRightness <n> (1:bright, 254:dim)\n"
    "\tCLear <panel>\n"
    "\tDAta <row> <col> <binary data>\n"
    "\tDUmp [<panel>]\n"
    "\tHElp\n"
    "\tLOad\n"
    "\tPIxel on|off <row> <col> [<panel>]\n"
    "\tREfresh\n"
    "\tSAve\n"
    "\tSTatus\n"
    "\tTEst <testnumber> [<panel>]\n"
    "\tVErsion\n";
    
// displayed with the "test" command
const char TestMenu[]  = 
    "\n"
    "\tTEst 1 [<panel>]  --   4 corners ON\n"
    "\tTEst 2 [<panel>]  --   all pixels ON\n"
    "\tTEst 3 [<panel>]  --   all pixels OFF\n"
    "\tTEst 4 [<panel>]  --   checkerboard test\n"
    "\tTEst 5 [<panel>]  --   marching row pattern\n"
    "\tTEst 6 [<panel>]  --   marching column pattern\n";

}       // end anonymous namespace


/**
 * @brief       matches 2 characters of a string
 *
 * @returns     true for match
 */
inline bool strMatch2(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, 2) == 0;
}

/**
 * @brief       matches command verb in the Commands table
 *
 * @param argv  argument array, required arguments are checked
 *
 * @returns     a CommandIds value representing the command identifier
 */
CommandIds CmdProc::matchCommandVerb(char * const argv[]) const
{
    for ( const Command &cmd: commands) {
        if ( strMatch2( argv[0], cmd.verb ) ) {
            // Verify that all required arguments are present.
            if ( argv[cmd.requiredArgs] != nullptr ) {
                return cmd.id;
            }
            else {
                return MISSING_ARG;
            }
        }
    }
    return NONE_OF_THE_ABOVE;
}

// ----------------------------------------------------------------------------
//            P U B L I C   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * @brief           Execute one parsed command line.
 *
 * @param streamID  output stream (SOCKET or CONSOLE)
 * @param inputBuf  command line buffer (modified during parsing)
 *
 * @return          pointer to result string (static), or nullptr
 *                  which is the command output
 */
const char *CmdProc::execute( const byte streamID, char *command )
{
    char *argString[MAX_ARGS] {};
    int argValue[MAX_ARGS] {};

    // the producer of commands is a process on Unix, which generates comments and blank lines
    if ( command[0] == 0 || command[0] == '#' || command[0] == '\r' || command[0] == '\n') {
        return "";
    }


    parseCommand(command, argString);         // tokenize the command into argString[]

    // as an code-size optimization, lets convert argv[1] ... argv[n] from ascii to an int
    // we are not putting NULs into argString[], therefore any text argument must be last
    for (byte i = 0; i < MAX_ARGS; ++i ) {
        if ( argString[i] != nullptr ) {
            argValue[i] = atoi( argString[i] );
        }
    }

    const CommandIds cmdId = matchCommandVerb( argString );
    ResultIds rc;
    switch ( cmdId ) {

        case CMD_ALL:
            // all on|off [<panel>]
            rc = ledWall_.allOn(
                     parseOnOff( argString[1] ),  /* "on"|"off" or "0"|"1" */
                     argValue[2] );               /* <panel> */

            break;

        case CMD_BRIGHT:
            // bright <brightness>
            ledWall_.setBrightness( argValue[1] );
            rc = NO_ERROR;
            break;

        case CMD_CLEAR:
            // clear <panel>
            rc = ledWall_.allOn(
                     0,                            /* off */
                     argValue[1] );               /* <panel> */
            break;

        case CMD_DATA:
            //  data <row> <col> <binary data>
            rc = dataCommand(
                     argValue[1],                 /* row */
                     argValue[2],                 /* col */
                     argString[3] );              /* binary data */
            break;

        case CMD_DUMP:
            // dump [<panel>]
            rc = dumpPixels( streamID,
                     argValue[1] );               /* optional <panel> */ 
            break;

        case CMD_HELP:
            // help or ? 
            return HelpText;

        case CMD_PIXEL:
            // pixel 0|1 <row> <col> [<panel>]
            // if 4 arguments, the 4th is the panel number
            if ( argString[4] != nullptr ) {
                rc = ledWall_.setPixelInPanel(
                         parseOnOff( argString[1] ),  /* "on"|"off" or "0"|"1" */
                         argValue[2],              /* row */
                         argValue[3],              /* column */
                         argValue[4] );            /* panel */
            }
            else {
                rc = ledWall_.setPixel(
                         parseOnOff( argString[1] ),  /* "on"|"off" or "0"|"1" */
                         argValue[2],              /* row */
                         argValue[3] );            /* column */
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

        case CMD_STATUS:
            return statusText();

        case CMD_TEST:
        {
            // test <testnumber> [<panel>]
            if ( argString[1] == nullptr ) {
                return TestMenu;
            }
            rc = selftest(
                     streamID,
                     argValue[1],                 /* test number */
                     argValue[2]);                /* optional <panel> */
            break;
        }

        case CMD_TIMING:
            // timing [on|off|0|1]
            // With no argument, toggle timing output.
            if (argString[1] == nullptr) {
                timingOutputEnabled = !timingOutputEnabled;
            }
            else {
                timingOutputEnabled = parseOnOff(argString[1]);
            }
            snprintf(outputBuf, sizeof outputBuf, "timing instrumentation is %s",
                     timingOutputEnabled ? "ON" : "OFF");
            return outputBuf;

        case CMD_VERSION:        
            // print the version string
            return versionString;

        case CMD_NOP:
            return "\n";

        case MISSING_ARG:
            rc = ERR_MISSING;
            break;

        case 0:
        default:
            rc = ERR_SYNTAX;
    }
    return resultStrings[rc];       // convert the result code to a string
}

// ----------------------------------------------------------------------------
//            P R I V A T E   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * @brief       parse the arguments in the command line
 *
 * @param command   the unparsed command
 * @param argv      the resulting argument vector
 */
void  CmdProc::parseCommand( char *command, char **argv )
{
    char *p = command;

    for ( byte i = 0; i < MAX_ARGS; ++i ) {
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
 * @brief       convert a string representing on/off, 
 *              or true/false, or 1/0 to a bool
 *
 * @param token  the unparsed string
 *
 * @returns     true or false
 */
bool CmdProc::parseOnOff(  const char *token ) const
{
    if ( token == nullptr )
        return false;
    if ( strMatch2( token, "true" ) )
        return true;
    if ( strMatch2( token, "on" ) )
        return true;
    if ( strMatch2( token, "false" ) )
        return false;
    if ( strMatch2( token, "off" ) )
        return false;
    if ( isdigit( *token ) )
        return ( atoi( token ) );

    return false;
}

/**
 * @brief performs the action of the "DATA" command.
 *    The given data is decoded, stored into the led data-store
 *
 * @param row       starting row number
 * @param col       the col number
 * @param bindata   binary-encoded data. i.e., 0s and 1s in ascii chars
 *
 * @returns        NO_ERROR or an error code
 */
ResultIds CmdProc::dataCommand( byte row, byte col, char *bindata )
{
    if ( row < 1 || row > displayConfig.nRows ) {
        return ERR_ROW;
    }
    if ( col < 1 || col > displayConfig.nCols ) {
        return ERR_COL;
    }

    bool bitValue;
    for ( char *p = bindata; *p; ++p, ++row ) {
        if ( *p == '0' ) {
            bitValue = 0;
        }
        else if (*p == '1' ) {
            bitValue = 1;
        }
        else {
            continue;
        }
        if ( row > displayConfig.nRows ) {
            return ERR_ROW;
        }
        const ResultIds rc = ledWall_.setPixel(bitValue, row, col);
        if (rc != NO_ERROR) {
            return rc;
        }
    }
    return NO_ERROR;
}


#if 0
/**
 * @brief           hexdump -- a general purpose routine to dump a section of memory in hex
 *
 * @param streamID  display output on the SOCKET or CONSOLE
 * @param addr      pointer to memory to dump
 * @param len       number of bytes
 */
void hexdump( byte streamID, void *addr, const unsigned int len )
{
    byte *p = (byte *)addr;
    for ( unsigned int offset = 0; offset < len; ++offset ) {
        if ( (offset & 31 )  == 0 )  {
            sprintf( outputBuf, "\r\n%04x: ", offset );
            writeOutput( streamID, outputBuf );
        }
        sprintf ( outputBuf, "%02x ", p[offset] );
        writeOutput( streamID, outputBuf );
    }
    writeOutput( streamID, "\n" );
}
#endif


/**
 * @brief         debug function to dump the LED data 
 *
 * @param panel   panel number 0 or [1..displayConfig.nPanels]
 *
 * @returns       NO_ERROR or ERR_PANEL
 */
ResultIds CmdProc::dumpPixels( byte streamID, byte panel )
{    
    if (panel > displayConfig.nPanels ) {
        return ERR_PANEL;
    }
    
    if ( panel == PANEL0 ) {
        
        writeOutput( streamID, "Dump of Pixel Data\n" );
        for ( byte row = 1; row <= displayConfig.nRows; row++ ) {
            snprintf(outputBuf, sizeof outputBuf, "%02d:  ", row);
            for ( byte col = 1; col <= displayConfig.nCols; ++col ) {
                #if CBS_56x40_WALL
                if ( col == 6 || col == 12 || col == 18 || col == 24 || col == 30 || col == 36 ) {
                   strcat( outputBuf, " .  " ); 
                }
                #endif
                bool pixel = ledWall_.pixelValue( row,  col );
                strcat( outputBuf, pixel ? "1 " : "0 " );
            }
            strcat( outputBuf, "\n" );
            writeOutput( streamID, outputBuf );
            #if CBS_56x40_WALL
            if ( row == 16 || row == 38 ) {
                writeOutput( streamID,  "    .........     ...........     ...........     ...........     ...........     ...........     .........\n" );
            }
            #else
            if ( row == 8 || row == 16 || row == 24) {
                writeOutput( streamID, "     - - - \n");
            }
            #endif
        }
        writeOutput( streamID, "\n" );   
    }
    else {

        snprintf(outputBuf, sizeof outputBuf, "Dump of Pixel Data - panel %d\n", panel );
        writeOutput( streamID, outputBuf);
        outputBuf[0] = '\0';
        for ( byte row = 1; row <= ledWall_.rowsInPanel(panel); ++row ) {
            for ( byte col = 1; col <= ledWall_.colsInPanel(panel); ++col ) {
                bool pixel = ledWall_.pixelValueInPanel(row, col, panel);
                strcat( outputBuf, pixel ? "1 " : "0 " );
            }
            strcat( outputBuf, "\n" );
            writeOutput( streamID, outputBuf );
            outputBuf[0] = '\0';
        }
        writeOutput( streamID, "\n" ); 
    }
    return NO_ERROR;
}

/**
 * @brief         debug function to dump the current status/settings
 *
 * @returns       pointer to formatted status string
 */
const char * CmdProc::statusText()
{
    const EthernetHardwareStatus hardwareStatus = Ethernet.hardwareStatus();
    const EthernetLinkStatus linkStatus = Ethernet.linkStatus();

    snprintf(outputBuf, sizeof outputBuf,
             "STATUS\n"
             "\tbrightness=%u rows=%u cols=%u panels=%u\n"
             "\tserialReady=%s ethernetReady=%s socketReady=%s\n"
             "\tIP=%u.%u.%u.%u\n"
             "\tgateway=%u.%u.%u.%u\n"
             "\tsubnet=%u.%u.%u.%u\n"
             "\tdns=%u.%u.%u.%u\n"
             "\tMAC=%02X:%02X:%02X:%02X:%02X:%02X\n"
             "\tethernetHardware=%s link=%s\n"
             "\ttiming=%s\n",

             displayConfig.brightness,
             displayConfig.nRows,
             displayConfig.nCols,
             displayConfig.nPanels,

             Serial ? "true" : "false",
             ethernetIsReady() ? "true" : "false",
             (socketClient && socketClient.connected()) ? "true" : "false",

             networkConfig.ipAddr[0],
             networkConfig.ipAddr[1],
             networkConfig.ipAddr[2],
             networkConfig.ipAddr[3],

             networkConfig.gateway[0],
             networkConfig.gateway[1],
             networkConfig.gateway[2],
             networkConfig.gateway[3],

             networkConfig.subnet[0],
             networkConfig.subnet[1],
             networkConfig.subnet[2],
             networkConfig.subnet[3],

             networkConfig.dnsAddr[0],
             networkConfig.dnsAddr[1],
             networkConfig.dnsAddr[2],
             networkConfig.dnsAddr[3],

             networkConfig.mac[0],
             networkConfig.mac[1],
             networkConfig.mac[2],
             networkConfig.mac[3],
             networkConfig.mac[4],
             networkConfig.mac[5],

             (hardwareStatus == EthernetW5500
                    ? "W5500"
                    : ( hardwareStatus == EthernetNoHardware ? "none": "other")),

             (linkStatus == LinkON ? "up": "down"),
             (timingOutputEnabled ? "on": "off")

        );

    return outputBuf;
}

/**
 * @file        CmdProc.ino
 *
 * @brief       Shared command processing for serial and Ethernet input.
 *
 *              Parses the compact command language, dispatches display operations,
 *              and renders help, version, and controller/network status output.
 *              Serial and TCP transports supply complete command lines.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 2.1 revised in November 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#include "CmdProc.h"
#include "yahrzeit_v2.h"
#include <string.h>

static constexpr byte MAXARGS = 5;

const char* const ResultStrings[8] = {
    "OK", "Eh?", "ERR Missing Arg", "ERR Row",
    "ERR Col", "ERR Panel", "ERR Brightness", "ERR Test Number"
};

const char HelpText[]  =
    "\n"
    "\tAll  on|off [<panel>]\n"
    "\tBRightness <n> (0:full, 255:off)\n"
    "\tDAta <row> <col> <binary data>\n"
    "\tDUmp [<panel>]\n"
    "\tHElp\n"
    "\tLOad\n"
    "\tPIxel on|off <row> <col> [<panel>]\n"
    "\tREfresh\n"
    "\tSAve\n"
    "\tSTatus\n"
    "\tTEst <testnumber> [<panel>|*]\n"
    "\tVErsion\n";

const char TestMenu[]  =
    "\n"
    "\tTEst 1 [<panel>|*] --   4 corners ON\n"
    "\tTEst 2 [<panel>|*] --   all pixels ON\n"
    "\tTEst 3 [<panel>|*] --   all pixels OFF\n"
    "\tTEst 4 [<panel>|*] --   checkerboard test\n"
    "\tTEst 5 [<panel>|*] --   marching row pattern\n"
    "\tTEst 6 [<panel>|*] --   marching column pattern\n"
    "\t* repeats the test for panels 1 through n\n";


// ----------------------------------------------------------------------------
//            C O M M A N D   P R O C E S S I N G
// ----------------------------------------------------------------------------


/**
 * @brief   Describe the compact server-to-controller command language.
 *
 * Commands use case-insensitive, two-character prefix matching (or "?").
 * Arguments remain pointers into the original, unmodified command line.
 * Numeric conversion stops at whitespace; exact-string arguments such as
 * "*" must be last, without trailing whitespace. Boolean words use prefixes.
 */



/**
 * @brief   Associate a command verb with its identifier and argument count.
 */
struct Command {
    commandIds id;
    byte requiredArgs;
    const char *verb;
};

/**
 * @brief   List supported commands in a fixed array for range traversal.
 */
static constexpr Command commands[] = {
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
    { CMD_STATUS, 0, "status" },
    { CMD_TEST,   0, "test"   },
    { CMD_VERSION, 0, "version" },
    { CMD_NOP,    0, "nop"    },
};


/**
 * @brief   Compare the first two characters case-insensitively.
 *
 * @param s1    first null-terminated string
 * @param s2    second null-terminated string
 * @returns     true when the first two characters match
 */
inline bool strMatch2(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, 2) == 0;
}

/**
 * @brief   Match a command prefix and check that required arguments exist.
 *
 * @param argv    argument array, required arguments are checked
 *
 * @returns an enumerator representing the command identifier
 */
static enum commandIds  match_cmd_verb( char **argv )
{
    for (const auto &command : commands) {
        if ( strMatch2( argv[0], command.verb ) ) {
            if ( argv[command.requiredArgs] != NULL ) {
                return command.id;
            }
            else {
                return MISSING_ARG;
            }
        }
    }
    return NONE_OF_THE_ABOVE;
}


/**
 * @brief   Write a time-stamped prompt to the selected stream.
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 */
static void prompt( byte streamID )
{
      char *uptimeRendered = display_uptime();
      strcat( uptimeRendered, " >" );
      writeOutput( streamID, uptimeRendered );
}


/**
 * @brief   Format the compiled display configuration and current network state.
 *
 * @returns a pointer to private static storage, overwritten by the next call
 */
static const char *status_text( void )
{
    static char statusOutput[640];
    IPAddress localAddress = Ethernet.localIP();
    const EthernetHardwareStatus hardwareStatus = Ethernet.hardwareStatus();
    const EthernetLinkStatus linkStatus = Ethernet.linkStatus();
    const char *hardwareName;

    switch (hardwareStatus) {
        case EthernetNoHardware:
            hardwareName = "none";
            break;
        case EthernetW5100:
            hardwareName = "W5100";
            break;
        case EthernetW5200:
            hardwareName = "W5200";
            break;
        case EthernetW5500:
            hardwareName = "W5500";
            break;
        default:
            hardwareName = "other";
            break;
    }

    const char *linkName = (linkStatus == LinkON)
                         ? "up"
                         : (linkStatus == LinkOFF ? "down" : "unknown");
    const boolean clientConnected = socketClient && socketClient.connected();
    const unsigned long clientIdleSeconds = clientConnected
                                          ? (millis() - socketLastByteReceivedMs) / 1000UL
                                          : 0;

    snprintf(statusOutput, sizeof statusOutput,
             "STATUS\n"
             "\tfirmware=V2.1\n"
             "\tboard=%s\n"
             "\tconfiguration=%s\n"
             "\trows=%u cols=%u panels=%u\n"
             "\tpins DI=%u OE=%u CP=%u ST=%u\n"
             "\tconfiguredIP=%u.%u.%u.%u activeIP=%u.%u.%u.%u port=%u\n"
             "\tgateway=%u.%u.%u.%u subnet=%u.%u.%u.%u\n"
             "\tdns=%u.%u.%u.%u\n"
             "\tMAC=%02X:%02X:%02X:%02X:%02X:%02X\n"
             "\tethernetHardware=%s link=%s\n"
             "\tlistener=%s port=%u\n"
             "\tclient=%s idle=%lu seconds\n"
             "\trxBytes=%lu pending=%d before=%d buffered=%u lastRead=%u result=%d\n"
             "\trxZeroPartials=%lu stalls=%lu\n",
             CONTROLLER_BOARD_MODEL,
             DISPLAY_CONFIGURATION,
             NROWS, NCOLS, NPANELS,
             DI_pin, OE_pin, CP_pin, ST_pin,
             ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3],
             localAddress[0], localAddress[1],
             localAddress[2], localAddress[3],
             SOCKET_LISTEN_PORT,
             gateway[0], gateway[1], gateway[2], gateway[3],
             subnet[0], subnet[1], subnet[2], subnet[3],
             dnsaddr[0], dnsaddr[1], dnsaddr[2], dnsaddr[3],
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
             hardwareName, linkName,
             socketListenerRunning ? "running" : "stopped",
             SOCKET_LISTEN_PORT,
             clientConnected ? "connected" : "disconnected",
             clientIdleSeconds,
             socketReadDiagnostics.bytesReceived,
             clientConnected ? socketClient.available() : 0,
             socketReadDiagnostics.availableBefore,
             socketReadDiagnostics.buffered,
             socketReadDiagnostics.lastRead.charsRead,
             static_cast<int>(socketReadDiagnostics.lastRead.status),
             socketReadDiagnostics.zeroBytePartials,
             socketReadDiagnostics.readStalls);

    return statusOutput;
}


/**
 * @brief   Execute one command shared by serial and Ethernet input.
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param command     the unparsed command to interpret and execute
 *
 * @returns        a string representing the command output
 */
const char * shell_execute( byte streamID, char *command )
{
    char *   arg_string[ MAXARGS ] {};
    int      arg_value[ MAXARGS ] {};

    // The Unix command producer generates comments and blank lines.
    if ( command[0] == 0 || command[0] == '#' || command[0] == '\r' || command[0] == '\n' ) {
        return "";
    }

    parse_command(command, arg_string);         // locate arguments without modifying input
    if (arg_string[0] == NULL) {
        return "";                            // whitespace-only command
    }

    // as an code-size optimization, lets convert argv[1] ... argv[n] from ascii to an int
    // We do not insert NULs into arg_string[], so any text argument must be last.
    for (byte i = 0; i < MAXARGS; i++ ) {
        if ( arg_string[i] != nullptr ) {
            arg_value[i] = atoi( arg_string[i] );
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
            if ( arg_string[4] != nullptr ) {
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

        case CMD_STATUS:
            return status_text();

        case CMD_TEST:
            // test <testnumber> [<panel>|*]
            if ( arg_string[1] == nullptr ) {
                return TestMenu;
            }

            if ( arg_string[2] != nullptr && strcmp(arg_string[2], "*") == 0 ) {
                rc = NO_ERROR;
                for (byte panel = 1; panel <= NPANELS; panel++) {
                    rc = selftest(streamID,
                                  arg_value[1],     /* test number */
                                  panel);
                    if (rc != NO_ERROR) {
                        break;
                    }
                }
            }
            else {
                rc = selftest(
                         streamID,
                         arg_value[1],             /* test number */
                         arg_value[2]);            /* optional <panel> */
            }
            break;

        case CMD_VERSION:
            return versionText();

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
 * @brief   Locate up to MAXARGS whitespace-separated argument starts.
 *
 * @note    Deliberately does not insert NUL terminators or modify command.
 *          Each pointer includes the remaining command suffix. The caller
 *          must initialize argv to NULL before calling this function.
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
 * @brief   Convert boolean word prefixes or a numeric value to a boolean.
 *
 * @param token       the unparsed string
 *
 * @returns false for NULL, unknown text, or zero; true for on/true or nonzero digits.
 */
boolean    onoff_bool( char *token )
{
    if ( token == NULL )
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
 * @brief   Decode the DATA command's binary suffix into the pixel buffer.
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




/**
 * @brief   Write pixel data for the whole display or one panel.
 *
 * @param streamID  output stream, SOCKET or CONSOLE
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

        writeOutput( streamID, "Dump of Pixel Data\n" );
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
            writeOutput( streamID, CmdOutput );
            #if CBS_56x40_WALL
            if ( row == 16 || row == 38 ) {
                writeOutput( streamID,  "     .........     ...........     ...........     ...........     ...........     ...........     .........\n" );
            }
            #endif
        }
        writeOutput( streamID, "\n" );
    }
    else {

        sprintf( CmdOutput, "Dump of Pixel Data - panel %d\n", panel );
        writeOutput( streamID, CmdOutput);
        for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
            CmdOutput[0] = '\0';
            for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
                boolean pixel = led_pixel_value(
                                        led_row_of_panel[panel] + row - 1,
                                        led_col_of_panel[panel] + col - 1 );
                strcat( CmdOutput, pixel ? "1 " : "0 " );
            }
            strcat( CmdOutput, "\n" );
            writeOutput( streamID, CmdOutput );
        }
        writeOutput( streamID, "\n" );
    }
    return NO_ERROR;
}

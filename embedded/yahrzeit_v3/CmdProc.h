#pragma once

#include <Arduino.h>
#include "LedWall.h"

/**
 * @file        CmdProc.h
 *
 * @brief       Command processor for console and socket interfaces.
 *
 *              Parses ASCII command lines, maps command verbs to internal
 *              command identifiers, validates arguments, and dispatches
 *              operations to LedWall and related subsystems.
 */

enum CommandIds : byte {
    NONE_OF_THE_ABOVE = 0,
    CMD_ALL = 1,        // turn on/off all LEDs
    CMD_BRIGHT,         // set the brightness
    CMD_CLEAR,          // clear one panel
    CMD_DATA,           // set a specific data bit pattern
    CMD_DUMP,           // dump the pixel memory
    CMD_HELP,           // display command help
    CMD_LOAD,           // load the pixel memory from EEPROM
    CMD_PIXEL,          // set a pixel memory on/off
    CMD_REFRESH,        // refresh the LED display from pixel memory
    CMD_SAVE,           // store the pixel memory into EEPROM
    CMD_STATUS,         // dump the current status/settings
    CMD_TEST,           // do one of several LED test patterns
    CMD_VERSION,        // print the version string
    CMD_NOP,            // (was required on the slower 8051 implementation)
    MISSING_ARG = 255
};


class CmdProc {
public:
    /**
     * @brief   Construct the command processor.
     */
    explicit CmdProc(LedWall& wall);

    /**
     * @brief   Execute one parsed command line
     */
    const char *execute(byte streamID, char *command);

private:
    LedWall& ledWall_;
    static constexpr byte kMaxArgs = 5;

    /*
     * @brief   Matches command verb in the Commands table
     */
    CommandIds match_cmd_verb(char * const argv[]) const;

    /*
     * @brief   Parse the arguments in the command line
     */
    void parse_command(char *command, char **argv);

    /*
     * @brief   Convert a string representing on/off, 
     *          or true/false, or 1/0 to a bool
     */
    bool onoff_bool(const char *token) const;

    /*
     * @brief   Performs the action of the "DATA" command.
     *    The given data is decoded, stored into the led data-store
     */
    byte console_data_cmd(byte row, byte col, char *bindata);

    /*
     * @brief   Debug function to dump the LED data 
     */
    int console_dump( byte streamID, byte panel );

    /*
     * @brief   Debug function to dump the status/settings
     */
    const char * console_status();

};

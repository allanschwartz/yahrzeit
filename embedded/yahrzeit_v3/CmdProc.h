#pragma once

#include <Arduino.h>
#include "LedWall.h"

/**
 * @file        CmdProc.h
 *
 * @brief       ASCII command processor for serial and socket control paths.
 *
 *              CmdProc parses one text command line, maps the command verb
 *              to an internal command ID, validates arguments, and dispatches
 *              the requested operation to LedWall or related diagnostics.
 *
 *              This module is intentionally independent of the input source.
 *              The same command processor is used for both USB serial input
 *              and TCP socket input.
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
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
    CMD_TIMING,         // toggle or turn on/off the timing instrumentation
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
    static constexpr byte MAX_ARGS = 5;

    /*
     * @brief   Matches command verb in the Commands table
     */
    CommandIds matchCommandVerb(char * const argv[]) const;

    /*
     * @brief   Parse the arguments in the command line
     */
    void parseCommand(char *command, char **argv);

    /*
     * @brief   Convert a string representing on/off, 
     *          or true/false, or 1/0 to a bool
     */
    bool parseOnOff(const char *token) const;

    /*
     * @brief   Performs the action of the "DATA" command.
     *    The given data is decoded, stored into the led data-store
     */
    ResultIds dataCommand(byte row, byte col, char *bindata);

    /*
     * @brief   Debug function to dump the LED data 
     */
    ResultIds dumpPixels( byte streamID, byte panel );

    /*
     * @brief   Debug function to dump the status/settings
     */
    const char * statusText();

};

/**
 * @file        CmdProc.h
 *
 * @brief       ASCII command processor for serial and socket control paths.
 *
 *              CmdProc parses one text command line, maps the command verb
 *              to an internal command ID, checks required argument presence,
 *              and dispatches the requested operation to LedWall or related
 *              diagnostics.
 *
 *              This module is intentionally independent of the input source.
 *              The same command processor is used for both USB serial input
 *              and TCP socket input.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 3.0 revised in April 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#pragma once

#include <Arduino.h>
#include "LedWall.h"

enum CommandIds : byte {
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
     * @brief   Construct a command processor for the supplied logical wall.
     */
    explicit CmdProc(LedWall& wall);

    /**
     * @brief   Parse and execute one mutable command buffer.
     */
    const char *execute(byte streamID, char *command);

private:
    LedWall& ledWall_;
    static constexpr byte MAX_ARGS = 5;

    /**
     * @brief   Match a command verb and verify required argument presence.
     */
    CommandIds matchCommandVerb(char * const argv[]) const;

    /**
     * @brief   Locate whitespace-separated arguments without copying them.
     */
    void parseCommand(char *command, char **argv);

    /**
     * @brief   Convert on/off, true/false, or numeric text to bool.
     */
    bool parseOnOff(const char *token) const;

    /**
     * @brief   Decode and store one DATA command in the framebuffer.
     */
    ResultIds dataCommand(byte row, byte col, char *bindata);

    /**
     * @brief   Write formatted framebuffer contents to one output stream.
     */
    ResultIds dumpPixels( byte streamID, byte panel );

    /**
     * @brief   Format controller, display, and network status.
     */
    const char * statusText();

};

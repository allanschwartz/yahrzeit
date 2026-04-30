#pragma once

#include <Arduino.h>
#include "LedWall.h"
#include "yahrzeit_v3.h"

/**
 * @file        CmdProc.h
 *
 * @brief       Shared command processor for console and socket interfaces.
 *
 *              Console and socket code should handle transport.
 *              CmdProc owns command parsing and command execution.
 */

enum commandIds : byte {
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

class CmdProc {
public:
    /**
     * Construct the command processor.
     *
     * @param wall      logical LED wall abstraction
     */
    explicit CmdProc(LedWall& wall);

    /**
     * execute ... perform one command
     *
     * @param streamID    output stream: CONSOLE or SOCKET
     * @param command     mutable command buffer
     *
     * @returns           result string
     */
    const char *execute(byte streamID, char *command);

private:
    static constexpr byte kMaxArgs = 5;

    LedWall& ledWall_;

    commandIds match_cmd_verb( char **argv );
    void parse_command(char *command, char **argv);
    boolean onoff_bool(char *token);
    int console_data_cmd(byte row, byte col, char *bindata);
    int console_dump( byte streamID, byte panel );

};
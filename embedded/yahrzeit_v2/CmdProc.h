/**
 * @file        CmdProc.h
 *
 * @brief       Shared command parsing and dispatch for serial and Ethernet.
 *
 *              Defines the compact command language and declares the shared
 *              parser, command dispatcher, result text, and pixel data helpers.
 *              Both serial and TCP input use this same command processor;
 *              transport and client lifecycle remain in their own modules.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 2.1 interface extracted in November 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#pragma once

#include <Arduino.h>
#include "led_device.h"

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
    CMD_STATUS,         // display controller and network configuration
    CMD_TEST,           // do one of several LED test pattern
    CMD_VERSION,        // display firmware and compile-time board target
    CMD_NOP,            // (was required on the slower 8051 inplementation)
    MISSING_ARG = 255
};

/**
 * @brief   Text corresponding to the shared ResultIds values.
 */
extern const char * const ResultStrings[8];

/**
 * @brief   Execute one mutable command line from serial or Ethernet input.
 */
const char *shell_execute( byte streamID, char *command );

/**
 * @brief   Locate argument starts without modifying the command buffer.
 */
void    parse_command( char *command, char **argv );

/**
 * @brief   Convert boolean word prefixes or numeric text to a boolean.
 */
boolean onoff_bool( char *token );

/**
 * @brief   Decode a DATA command's binary suffix into the pixel buffer.
 */
int     console_data_cmd( byte row, byte col, char *bindata );

/**
 * @brief   Write pixel data for the whole display or one panel.
 */
int     console_dump( byte streamID, byte panel );

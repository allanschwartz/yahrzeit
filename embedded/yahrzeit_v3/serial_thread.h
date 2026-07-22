/**
 * @file        serial_thread.h
 *
 * @brief       USB serial console input loop and shared diagnostics.
 *
 *              The serial thread reads line-oriented commands from the
 *              Arduino USB serial console and passes complete command lines
 *              to the shared CmdProc command processor.
 *
 *              This path is useful for bench testing, controller bring-up,
 *              and field diagnostics when a USB cable is connected.
 *
 *              This module also provides small diagnostic helpers, including
 *              timestamped logging and uptime formatting, used by both the
 *              serial and socket input paths.
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


/**
 * @brief   Initialize the USB serial console at 115200 baud.
 */
void    serialInit();

/**
 * @brief   Service the nonblocking serial command loop once.
 */
void    serialThread();

/**
 * @brief   Accumulate one newline-terminated command from USB serial input.
 */
bool    serialGets(char inputBuf[], const unsigned maxsize, unsigned &index);

/**
 * @brief   Write one uptime-prefixed diagnostic line to the serial console.
 */
void    serialLog( const char *msg );


/**
 * @brief   Format time since startup as hh:mm:ss.mmm in a shared buffer.
 */
const char *displayUptime();

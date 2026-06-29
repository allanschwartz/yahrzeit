#pragma once

#include <Arduino.h>

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
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */


/**
 * @brief   Initialize the UART or Serial class
 */
void    serialInit();

/**
 * @brief   Serial input main loop thread
 */
void    serialThread();

/**
 * @brief   Read a single line from the serial UART, (similar to fgets())
 */
bool    serialGets(char inputBuf[], const unsigned maxsize, unsigned &index);

/**
 * @brief   Displays a time-stamped line, typically a diagnostic line
 *          on the serial console.  A trailing newline is also written.
 *          Used in both serial_thread.ino and socket_thread.ino
 */
void    serialLog( const char *msg );


/**
 * @brief   Displays the uptime, in the format hh:mm:ss.mmm  (with millisecond precision)
 *          Used in both serial_thread.ino and socket_thread.ino
 */
const char *displayUptime();

/**
 * @file        serial_thread.h
 *
 * @brief       USB-connected UART console input loop and shared diagnostics.
 *
 *              The serial thread reads line-oriented commands from the Mega's
 *              serial console and passes complete command lines to the shared
 *              command processor in CmdProc.ino.
 *
 *              This path is useful for bench testing, controller bring-up,
 *              and field diagnostics when a USB cable is connected.
 *
 *              This module also provides startup console behavior and
 *              timestamped logging. Uptime formatting remains in the main
 *              sketch and is declared in yahrzeit_v2.h.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 2.1 reorganized in November 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#pragma once

#include <Arduino.h>


/**
 * @brief   Initialize the Mega's serial console at 9600 baud.
 */
void    serial_init();

/**
 * @brief   Initialize pixel state, print the banner, and run startup tests.
 */
void    console_init();

/**
 * @brief   Collect serial input and execute at most one command per call.
 */
void    console_thread();

/**
 * @brief   Drain available serial input until a command is ready.
 */
boolean console_gets(char *input, unsigned int maxsize, int *piIndex);

/**
 * @brief   Write one uptime-prefixed diagnostic line to the serial console.
 */
void    console_log(const char *msg);

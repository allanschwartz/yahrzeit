#pragma once

#include <Arduino.h>

/**
 * @file        serial_thread.h
 *
 * @brief       Serial input main loop
 *
 */


/**
 * @brief   Initialize the UART or Serial class
 */
void    serial_init();

/**
 * @brief   Serial input main loop thread
 */
void    serial_thread();

/**
 * @brief   Read a single line from the serial UART, (similar to fgets())
 */
bool    serial_gets(char inputBuf[], const unsigned maxsize, unsigned &index);

/**
 * @brief   Displays a time-stamped line, typically a diagnostic line
 *        on the serial console.  A trailing newline is also written.
 */
void    serial_log( const char *msg );


/**
 * @brief   Displays the uptime, in the format hh:mm:ss.mmm  (with millisecond precision)
 */
const char *display_uptime();

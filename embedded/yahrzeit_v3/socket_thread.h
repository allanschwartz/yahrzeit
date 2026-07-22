/**
 * @file        socket_thread.h
 *
 * @brief       TCP socket command input loop.
 *
 *              The socket thread accepts line-oriented commands over Ethernet
 *              and passes complete command lines to the shared CmdProc command
 *              processor.
 *
 *              This is the normal control path used by the PHP Yahrzeit site:
 *
 *                  bin/yahrzeit
 *                      -> nc
 *                          -> Arduino TCP socket
 *                              -> socket_thread
 *                                  -> CmdProc
 *                                      -> LedWall
 *
 *              The socket code owns network readiness, connection handling,
 *              and incremental line input. It should not parse Yahrzeit
 *              command semantics or manipulate wall geometry directly.
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
 * @brief   Determine whether Ethernet hardware and link state permit socket
 *          service.
 */
bool    ethernetIsReady();

/**
 * @brief   Initialize Ethernet with the configured static network settings.
 */
void    ethernetInit();

/**
 * @brief   Begin listening on the command socket when Ethernet is ready.
 */
void    socketInit();

/**
 * @brief   Service the nonblocking TCP command loop once.
 */
void    socketThread();

// socketGets can return the following, representing connection state:
enum GetsReturns: byte {
    GETS_NOCONNECTION,
    GETS_NOCHAR,
    GETS_FULLCMD,
};

/**
 * @brief   Accumulate one newline-terminated command from the TCP client.
 */
GetsReturns  socketGets(char inputBuf[], const unsigned maxsize, unsigned &index);

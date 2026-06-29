#pragma once

#include <Arduino.h>

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
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

/**
 * @brief    Is the Ethernet ready for a socket connection
 */
bool    ethernetIsReady();

/**
 * @brief    Initialize Ethernet with the configured network settings
 */
void    ethernetInit();

/**
 * @brief    Initialize and setup the Socket-based input 
 */
void    socketInit();

/**
 * @brief    Socket-based input main loop thread
 */
void    socketThread();

// socketGets can return the following, representing connection state:
enum GetsReturns: byte {
    GETS_NOCONNECTION,
    GETS_NOCHAR,
    GETS_PARTIAL,
    GETS_FULLCMD,
};

/**
 * @brief    Read a line from socket input
 */
GetsReturns  socketGets(char inputBuf[], const unsigned maxsize, unsigned &index);

/**
 * @file        socket_thread.h
 *
 * @brief       TCP socket command input loop.
 *
 *              The socket thread accepts line-oriented commands over Ethernet
 *              and passes complete command lines to the shared command
 *              processor in CmdProc.ino.
 *
 *              This is the normal control path used by the PHP Yahrzeit site:
 *
 *                  bin/yahrzeit
 *                      -> nc
 *                          -> Arduino TCP socket
 *                              -> socket_thread
 *                                  -> shell_execute
 *                                      -> LED device functions
 *
 *              The socket code owns link polling, listener recovery, client
 *              connection handling, idle timeout, and incremental line input.
 *              It does not parse command semantics or manage wall geometry.
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
#include <Ethernet.h>

/**
 * @brief   Current TCP client, shared with output and status reporting.
 */
extern EthernetClient socketClient;

/**
 * @brief   Software flag recording whether the listener has been started.
 */
extern boolean socketListenerRunning;

/**
 * @brief   Time in milliseconds when client input was last recorded.
 */
extern unsigned long socketLastByteReceivedMs;

/**
 * @brief   Outcome of incremental TCP command collection.
 */
enum SocketGetsResult {
    GETS_NOCHAR = -1,
    GETS_PARTIAL = 0,
    GETS_FULLCMD = 1,
};

/**
 * @brief   Bytes actually consumed and the command collection outcome.
 */
struct SocketReadResult {
    unsigned int charsRead;
    SocketGetsResult status;
};

/**
 * @brief   Per-client receive diagnostics, retained until the next client.
 */
struct SocketReadDiagnostics {
    unsigned long bytesReceived;
    unsigned long zeroBytePartials;
    unsigned long readStalls;
    int availableBefore;
    unsigned int buffered;
    SocketReadResult lastRead;
};
extern SocketReadDiagnostics socketReadDiagnostics;

/**
 * @brief   Initialize Ethernet with static network settings and log detected
 *          hardware, link state, and the active IP address and command port.
 */
void    ethernet_init();

/**
 * @brief   Start the command listener when hardware is present and it has
 *          not already been started.
 */
void    socketInit();

/**
 * @brief   Service link state, listener recovery, and TCP command input once.
 */
void    socket_thread();

/**
 * @brief   Accumulate one command from the already-connected TCP client.
 */
SocketReadResult socket_gets(char *input, unsigned int maxsize, int *piIndex);

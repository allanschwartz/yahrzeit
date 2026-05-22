#pragma once

#include <Arduino.h>

/**
 * @file        socket_thread.h
 *
 * @brief       Socket-based input main loop
 *
 */

/**
 * @brief    Is the Ethernet ready for a socket connection
 */
bool    ethernet_is_ready();

/**
 * @brief    Initialize and setup the Socket-based input 
 */
void    socket_init();

/**
 * @brief    Socket-based input main loop thread
 */
void    socket_thread();

// socket_gets can return the following, representing connection state:
enum GetsReturns: byte {
    GETS_NOCONNECTION,
    GETS_NOCHAR,
    GETS_PARTIAL,
    GETS_FULLCMD,
};

/**
 * @brief    Read a line from socket input
 */
GetsReturns  socket_gets(char inputBuf[], const unsigned maxsize, unsigned &index);


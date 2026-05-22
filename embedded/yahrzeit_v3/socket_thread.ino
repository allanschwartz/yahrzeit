/**
 * @file        socket_thread.ino
 *
 * @brief       Socket main line code for the Yahrzeit Project
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 3.0 revised in April 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 *
 * @notes       see project notes in file yahrzeit_v3.h
 */

#include "yahrzeit_v3.h"

// ----------------------------------------------------------------------------
//            S O C K E T   C O N S O L E
// ----------------------------------------------------------------------------

constexpr byte Ethernet_CS_pin = 10;   // W5500 CS pin on Ethernet Shield 2

/**
 * ethernet_is_ready .. is the Ethernet ready for a socket connection?
 */
bool ethernet_is_ready()
{
    return Ethernet.hardwareStatus() != EthernetNoHardware
        && Ethernet.linkStatus() == LinkON;
}

/**
 * v .. intialize the Ethernet instance, setting the IP Address
 */
void ethernet_init()
{
    serial_log("ethernet: begin");

    Ethernet.init(Ethernet_CS_pin);

    Ethernet.begin(networkConfig.mac,
                   networkConfig.ipAddr,
                   networkConfig.dnsAddr,
                   networkConfig.gateway,
                   networkConfig.subnet);

    delay(10);  // allow W5500 Link/status to stabilize
    auto hw = Ethernet.hardwareStatus();
    if (hw == EthernetNoHardware) {
        serial_log("ethernet: no hardware");
    }
    else if (hw == EthernetW5100) {
        serial_log("ethernet: W5100");
    }
    else if (hw == EthernetW5200) {
        serial_log("ethernet: W5200");
    }
    else if (hw == EthernetW5500) {
        serial_log("ethernet: W5500");
    }

    if (Ethernet.linkStatus() == LinkOFF) {
        serial_log("ethernet: link down");
        return;
    }

    IPAddress localIP = Ethernet.localIP();
    snprintf(outputBuf, sizeof outputBuf,
             "ethernet: IP %d.%d.%d.%d",
             localIP[0], localIP[1], localIP[2], localIP[3]);
    serial_log(outputBuf);
}

/**
 * socket_init ... begin listening on a specific socket
 */
void socket_init()
{
    if (!ethernet_is_ready()) {
        serial_log("socket: not started, ethernet unavailable");
        return;
    }

    socket.begin();

    serial_log("socket: listening on 2001");
}


/**
 * socket_thread .. implement the socket stream I/O gets(), puts() loop.
 *    in each call to socket_thread, we read one line, and execute one command.
 *
 * @returns
 *      returns after every command line is executed
 */
void  socket_thread()
{
    // the command from the client (the Yahrzeit Server)
    constexpr unsigned   MAXINPUTLINE = 64;
    static    char       inputBuf[ MAXINPUTLINE ] {};
    static    unsigned   inputBufPos = 0;                 // 0..MAXINPUTLINE-1

    if (!ethernet_is_ready()) {
        return;
    }

    GetsReturns rc = socket_gets( inputBuf, sizeof inputBuf, inputBufPos );

    switch ( rc ) {
        case GETS_NOCONNECTION: // no available byte or no connection
        case GETS_NOCHAR:       // no data is available
        case GETS_PARTIAL:      // recorded a partial command line
            break;
        case GETS_FULLCMD:      // complete command line, to interpret
        {
            char feedbackLine[80];
            if ( inputBuf[0] != '\0' ) {
                const unsigned long startUs = micros();
                const char *result = cmdProc.execute( SOCKET, inputBuf );
                snprintf( feedbackLine, sizeof feedbackLine, "%s | %s\n",
                      display_uptime(), inputBuf );
                my_puts( SOCKET, feedbackLine );
                if (result != nullptr) {
                    my_puts( SOCKET, result );
                    my_puts( SOCKET, "\n");
                }
                const unsigned long elapsedUs = micros() - startUs;
                snprintf( feedbackLine, sizeof feedbackLine, "%s | %lu.%03lu ms\n",
                      display_uptime(), elapsedUs/1000UL, elapsedUs % 1000UL);
                my_puts( SOCKET, feedbackLine );
                
            }
            else {
                // echo back blank lines
                snprintf( feedbackLine, sizeof feedbackLine, "%s |\n", display_uptime() );
                my_puts( SOCKET, feedbackLine );
            }
            inputBufPos = 0;
            inputBuf[0] = '\0';
            break;
        }
    }
}


/**
 * socket_gets ... this is our fgets() routine which reads a single line from
 *       the network socket
 *
 *    console_gets() reads in at most one less than size characters from the
 *    serial UART stream and and stores them into the buffer pointed to by str.
 *    Reading stops after a newline or cr or the str is filled.
 *    If a newline or cr is read, it is not stored into the buffer.
 *    A terminating null byte ('0' is stored after the last character.
 *
 *    This non-threaded version of console_gets() does not block, rather it
 *    returns immediately, that is returns after the current packet is read.
 *    The bool result code indicates whether a full line was read
 *
 * @param inputBuf    the resulting null-terminated string
 * @param maxsize  max size we can store, including a NULL
 * @param index  reference to a count of the number of bytes read into inputBuf
 *
 * @returns an int, an enumeration of the getsReturns (or the state of the gets)
 */
GetsReturns socket_gets(char inputBuf[], const unsigned maxsize, unsigned &index)
{
    if (!socketClient || !socketClient.connected()) {
        socketClient = socket.available();

        if (socketClient) {
            index = 0;
            inputBuf[0] = '\0';
            serial_log("socket_gets: socket client connected");
        }
        else {
            return GETS_NOCONNECTION;
        }
    }

    while (socketClient.available() > 0) {
        const int c = socketClient.read();        // read byte-by-byte

        if (c < 0) {
            return GETS_NOCHAR;
        }
        if (c == 0 || c == '\r') {
            // remove NUL and CR from the input stream
            continue;
        }
        // Handle LF
        if (c == '\n') {
            return GETS_FULLCMD;
        }

        // If the command is too long, terminate and process the truncated line.
        if (index >= (maxsize - 1)) {
            inputBuf[index] = '\0';
            return GETS_FULLCMD;
        }

        // normal case, normal character
        inputBuf[index] = static_cast<char>(c);
        index++;
        inputBuf[index] = '\0';
    }
    return GETS_NOCHAR;
}


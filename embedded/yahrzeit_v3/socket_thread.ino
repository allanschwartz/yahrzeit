/**
 * @file        socket_thread.ino
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

#include "yahrzeit_v3.h"

// ----------------------------------------------------------------------------
//            S O C K E T   C O N S O L E
// ----------------------------------------------------------------------------

static constexpr byte ETHERNET_CS_PIN = 10;   // W5500 CS pin on Ethernet Shield 2

/**
 * @brief   Is the Ethernet ready for a socket connection?
 */
bool ethernetIsReady()
{
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        return false;
    }

    // W5100-class hardware may not report link status as LinkON. Treat only an
    // explicit LinkOFF as unavailable; unknown link status can still work.
    return Ethernet.linkStatus() != LinkOFF;
}

/**
 * @brief   Initialize the Ethernet instance, setting the IP address.
 */
void ethernetInit()
{
    serialLog("ethernet: begin");

    Ethernet.init(ETHERNET_CS_PIN);

    Ethernet.begin(networkConfig.mac,
                   networkConfig.ipAddr,
                   networkConfig.dnsAddr,
                   networkConfig.gateway,
                   networkConfig.subnet);

    delay(10);  // allow W5500 link/status to stabilize
    auto hw = Ethernet.hardwareStatus();
    if (hw == EthernetNoHardware) {
        serialLog("ethernet: no hardware");
    }
    else if (hw == EthernetW5100) {
        serialLog("ethernet: W5100");
    }
    else if (hw == EthernetW5200) {
        serialLog("ethernet: W5200");
    }
    else if (hw == EthernetW5500) {
        serialLog("ethernet: W5500");
    }

    if (Ethernet.linkStatus() == LinkOFF) {
        serialLog("ethernet: link down");
        return;
    }

    IPAddress localIP = Ethernet.localIP();
    snprintf(outputBuf, sizeof outputBuf,
             "ethernet: IP %d.%d.%d.%d",
             localIP[0], localIP[1], localIP[2], localIP[3]);
    serialLog(outputBuf);
}

/**
 * @brief   Begin listening on the command socket.
 */
void socketInit()
{
    if (!ethernetIsReady()) {
        serialLog("socket: not started, ethernet unavailable");
        return;
    }

    socket.begin();
    snprintf(outputBuf, sizeof outputBuf,
             "socket: listening on %u",
             SOCKET_LISTEN_PORT);
    serialLog(outputBuf);
}

/**
 * @brief   Implement the socket stream I/O gets(), puts() loop.
 *
 * In each call to socketThread, read at most one line and execute one command.
 */
void socketThread()
{
    constexpr unsigned MAX_INPUT_LINE = 64;
    static char inputBuf[MAX_INPUT_LINE] {};
    static unsigned inputBufPos = 0;       // 0..MAX_INPUT_LINE-1

    if (!ethernetIsReady()) {
        return;
    }

    GetsReturns rc = socketGets(inputBuf, sizeof inputBuf, inputBufPos);

    switch (rc) {
        case GETS_NOCONNECTION: // no available byte or no connection
        case GETS_NOCHAR:       // no data is available
        case GETS_PARTIAL:      // recorded a partial command line
            break;
        case GETS_FULLCMD:      // complete command line, to interpret
        {
            char feedbackLine[80];
            if (inputBuf[0] != '\0') {
                const unsigned long startUs = micros();
                const char *result = cmdProc.execute(SOCKET, inputBuf);
                snprintf(feedbackLine, sizeof feedbackLine, "%s | %s\n",
                         displayUptime(), inputBuf);
                writeOutput(SOCKET, feedbackLine);
                if (result != nullptr) {
                    writeOutput(SOCKET, result);
                    writeOutput(SOCKET, "\n");
                }
                if (timingOutputEnabled) {
                    const unsigned long elapsedUs = micros() - startUs;
                    snprintf(feedbackLine, sizeof feedbackLine, "%s | %lu.%03lu ms\n",
                             displayUptime(), elapsedUs / 1000UL, elapsedUs % 1000UL);
                    writeOutput(SOCKET, feedbackLine);
                }
            }
            else {
                // Echo back blank lines.
                snprintf(feedbackLine, sizeof feedbackLine, "%s |\n", displayUptime());
                writeOutput(SOCKET, feedbackLine);
            }
            inputBufPos = 0;
            inputBuf[0] = '\0';
            break;
        }
    }
}

/**
 * @brief   Read a single line from the network socket.
 *
 * This non-threaded version of console_gets() does not block. It returns
 * after the current packet is read, or immediately if no data is available.
 *
 * @param inputBuf   The resulting null-terminated string.
 * @param maxsize    Max size to store, including the NULL terminator.
 * @param index      Reference to the number of bytes read into inputBuf.
 *
 * @returns          An enumeration representing the gets state.
 */
GetsReturns socketGets(char inputBuf[], const unsigned maxsize, unsigned &index)
{
    if (!socketClient || !socketClient.connected()) {
        socketClient = socket.available();

        if (socketClient) {
            index = 0;
            inputBuf[0] = '\0';
            serialLog("socketGets: socket client connected");
        }
        else {
            return GETS_NOCONNECTION;
        }
    }

    while (socketClient.available() > 0) {
        const int c = socketClient.read();  // read byte-by-byte

        if (c < 0) {
            return GETS_NOCHAR;
        }
        if (c == 0 || c == '\r') {
            // Remove NUL and CR from the input stream.
            continue;
        }
        if (c == '\n') {
            return GETS_FULLCMD;
        }

        if (index >= (maxsize - 1)) {
            inputBuf[index] = '\0';
            return GETS_FULLCMD;
        }

        inputBuf[index] = static_cast<char>(c);
        ++index;
        inputBuf[index] = '\0';
    }
    return GETS_NOCHAR;
}

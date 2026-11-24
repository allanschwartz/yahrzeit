/**
 * @file    socket_thread.ino
 *
 * @brief   V2 Ethernet command transport.
 */

#include "yahrzeit_v2.h"

constexpr byte ETHERNET_CS_PIN = 10; // Arduino Ethernet shield chip select

boolean socketListenerRunning = false;
unsigned long socketLastByteReceivedMs = 0;
SocketReadDiagnostics socketReadDiagnostics {0, 0, 0, 0, 0, {0, GETS_NOCHAR}};

static constexpr unsigned long MINUTE_MS = 60UL * 1000UL;
static constexpr unsigned long SOCKET_IDLE_TIMEOUT_MS = 10UL * MINUTE_MS;
static constexpr unsigned long LINK_CHECK_INTERVAL_MS = 50UL;


/**
 * @brief   Initialize Ethernet with static network settings and report
 *          detected hardware, link state, and active IP address.
 */
void ethernet_init()
{
    console_log("ethernet: begin");
    Ethernet.init(ETHERNET_CS_PIN);
    Ethernet.begin(mac, ipaddr, dnsaddr, gateway, subnet);

    delay(10);  // brief pause before sampling hardware and link state
    switch (Ethernet.hardwareStatus()) {
        case EthernetNoHardware:
            console_log("ethernet: no hardware");
            return;
        case EthernetW5100:
            console_log("ethernet: W5100");
            break;
        case EthernetW5200:
            console_log("ethernet: W5200");
            break;
        case EthernetW5500:
            console_log("ethernet: W5500");
            break;
        default:
            console_log("ethernet: other hardware");
            break;
    }

    const EthernetLinkStatus linkStatus = Ethernet.linkStatus();
    console_log(linkStatus == LinkON ? "ethernet: link up"
                : linkStatus == LinkOFF ? "ethernet: link down"
                : "ethernet: link unknown");

    const IPAddress localIP = Ethernet.localIP();
    char message[48];
    snprintf(message, sizeof message, "ethernet: IP %u.%u.%u.%u port=%u",
             localIP[0], localIP[1], localIP[2], localIP[3],
             SOCKET_LISTEN_PORT);
    console_log(message);
}


/**
 * @brief   Start the command listener when hardware is present and it has
 *          not already been started.
 *
 * @note    Called by socket_thread() after its link-state check, including
 *          after link recovery. The flag records software startup state,
 *          not an independent check of Ethernet controller health.
 */
void socketInit()
{
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        socketListenerRunning = false;
        return;
    }
    if (socketListenerRunning) {
        return;
    }

    socket.begin();
    socketListenerRunning = true;
    console_log("ethernet: listener started");
}


/**
 * socket_thread .. implement the socket stream I/O gets(), puts() loop.
 *    in each call to socket_thread, we read one line, and execute one command.
 *
 * @returns
 *      returns after every command line is executed
 */
void  socket_thread( void )
{
    // the command from the client (the Yahrzeit Server)
    constexpr unsigned int MAXINPUTLINE = 64;
    // These variables remain static so partial commands survive loop calls.
    static char          inputBuf[ MAXINPUTLINE ] = { 0 };
    static int           inputBufPos = 0;                 // 0..MAXCMDLINE-1
    static unsigned long lastLinkCheckMs = 0;
    static EthernetLinkStatus cachedLinkStatus = Unknown;

    const unsigned long nowMs = millis();
    if (lastLinkCheckMs == 0 ||
        nowMs - lastLinkCheckMs >= LINK_CHECK_INTERVAL_MS) {
        lastLinkCheckMs = nowMs;
        cachedLinkStatus = Ethernet.linkStatus();
    }

    if (Ethernet.hardwareStatus() == EthernetNoHardware ||
        cachedLinkStatus == LinkOFF) {
        if (socketClient) {
            socketClient.stop();
        }
        inputBufPos = 0;
        inputBuf[0] = '\0';
        socketReadDiagnostics.buffered = 0;
        socketListenerRunning = false;
        return;
    }

    socketInit();

    // Own the TCP client lifecycle here. socket_gets() only reads an already
    // connected client.
    if (!socketClient || !socketClient.connected()) {
        if (socketClient) {
            socketClient.stop();
        }

        socketClient = socket.available();
        inputBufPos = 0;
        inputBuf[0] = '\0';
        socketReadDiagnostics.buffered = 0;

        if (!socketClient) {
            return;
        }

        socketLastByteReceivedMs = millis();
        socketReadDiagnostics = {0, 0, 0, 0, 0, {0, GETS_NOCHAR}};
        console_log( "socket: client connected" );
    }

    const int availableBefore = socketClient.available();
    socketReadDiagnostics.availableBefore = availableBefore;
    if (availableBefore == 0 &&
        millis() - socketLastByteReceivedMs >= SOCKET_IDLE_TIMEOUT_MS) {
        socketClient.stop();
        inputBufPos = 0;
        inputBuf[0] = '\0';
        socketReadDiagnostics.buffered = 0;
        console_log( "socket: idle client closed" );
        return;
    }

    const SocketReadResult readResult = socket_gets( inputBuf, sizeof inputBuf, &inputBufPos );
    socketReadDiagnostics.lastRead = readResult;
    socketReadDiagnostics.buffered = inputBufPos;
    socketReadDiagnostics.bytesReceived += readResult.charsRead;
    if (availableBefore > 0 && readResult.charsRead == 0) {
        ++socketReadDiagnostics.readStalls;
    }
    if (readResult.status == GETS_PARTIAL && readResult.charsRead == 0) {
        ++socketReadDiagnostics.zeroBytePartials;
    }

    // Only bytes actually read renew the timer, not a reader result by itself.
    if (readResult.charsRead > 0) {
        socketLastByteReceivedMs = millis();
    }

    switch ( readResult.status ) {
        case GETS_NOCHAR:       // no data is available
            break;
        case GETS_PARTIAL:      // collected input, but no complete command yet
            break;
        case GETS_FULLCMD:      // complete command line, to interpret
        {
            char *uptimeRendered = display_uptime();
            if ( strlen( inputBuf ) > 0 ) {
                const char *result = shell_execute( SOCKET, inputBuf );
                snprintf( CmdOutput, sizeof CmdOutput, "%s | %s\n",
                          uptimeRendered, inputBuf );
                writeOutput( SOCKET, CmdOutput );
                if ( strlen(result) ) {
                    writeOutput( SOCKET, result );
                    if (result[strlen(result) - 1] != '\n') {
                        writeOutput( SOCKET, "\n" );
                    }
                }
            }
            else {
                snprintf( CmdOutput, sizeof CmdOutput, "%s |\n", uptimeRendered );
                writeOutput( SOCKET, CmdOutput );
            }

            inputBufPos = 0;
            memset( inputBuf, 0, sizeof inputBuf );
            socketReadDiagnostics.buffered = 0;
            break;
        }
    }
}


/**
 * @brief   Read one line from the already-connected TCP client
 *
 *    socket_gets() collects at most maxsize - 1 characters from the
 *    TCP stream into input, preserving a partial line between calls.
 *    NUL and carriage return are ignored. Reading stops after a newline or
 *    when the buffer is filled. The newline is not stored.
 *    A terminating null byte ('\0') is stored after the last character.
 *
 *    This function owns no listener, connection, timeout, or reconnection
 *    policy. socket_thread() establishes the client before calling it.
 *
 * @param input    the resulting null-terminated string
 * @param maxsize  max size we can store, including a NULL
 * @param piIndex  pointer to a count of the number of bytes read into input
 *
 * @returns Actual bytes consumed and GETS_NOCHAR, GETS_PARTIAL, or GETS_FULLCMD.
 *          The count includes ignored CR/NUL bytes, backspaces, and newline.
 */
SocketReadResult socket_gets(char *input, const unsigned int maxsize, int *piIndex)
{
    unsigned int charsRead {};
    if (socketClient.available() == 0) {
        return {charsRead, GETS_NOCHAR};
    }

    while (socketClient.available() > 0) {
        int c = socketClient.read();
        if (c < 0) {
            return {charsRead, GETS_NOCHAR};
        }

        ++charsRead;

        // Handle backspace
        if (c == '\b') {
            if (*piIndex > 0) {
                --(*piIndex);
                input[*piIndex] = '\0';
            }
            continue;
        }

        // Ignore NUL and CR; LF completes one command. This avoids a
        // second blank command when a client sends CRLF.
        if (c == 0 || c == '\r') {
            continue;
        }
        if (c == '\n') {
            return {charsRead, GETS_FULLCMD};
        }

        // Handling potential overflow of buffer, by just returning
        if (*piIndex >= (maxsize - 1)) {
            return {charsRead, GETS_FULLCMD};
        }

        // normal character
        input[*piIndex] = c;
        *piIndex = *piIndex + 1;
        input[*piIndex] = '\0';
    }
    return {charsRead, GETS_PARTIAL};
}

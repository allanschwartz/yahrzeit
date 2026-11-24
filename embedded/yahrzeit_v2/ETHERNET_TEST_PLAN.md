# V2 Ethernet Reliability Test Plan

Prepared November 22, 2026, for Allan and Zach.

## Purpose

Check that the Arduino Mega controller and Ethernet Shield 2 recover from
connection interruptions without removing the shield or power-cycling the
controller. The installed controller previously suffered Ethernet lockups;
operators restored operation by unplugging and reseating the Ethernet shield.

These tests focus on Ethernet. No pixel boards need to be connected.

## Bench configuration

| Item | Setting |
|---|---|
| Controller | Arduino Mega 2560, V2.1 yahrzeit controller firmware <br> Arduino Ethernet Shield 2, (<i>aka</i> W5500 |
| Shield MAC | `A8:61:0A:AE:E4:E2` |
| Controller IP | at CBS: `192.168.13.9`<br> at Allan's: `192.168.86.143` (port `2001`) |
| Gateway | `192.168.86.1` |
| Subnet mask | `255.255.255.0` |
| Serial Monitor | 9600 baud; newline or carriage-return line ending |
| Display configuration | at CBS: `CBS_56x40_WALL`: 56 rows, 40 columns, in 21 panels <br> at Allan's: `TEST_24x6_FIXTURE`: 24 rows, 6 columns, 2 panels|


Verify these settings using serial `status` before starting. The firmware's
network settings must match the actual test network.

Incoming bytes restart the idle timer, including bytes forming an incomplete
command. An otherwise healthy idle connection is deliberately closed after
10 minutes without input.

## Status LED patterns

The external green status LED on D45 indicates controller activity, independently
of the Ethernet jack's link/activity LEDs. It breathes while the main loop runs,
flashes rapidly during panic, and remains off during startup tests, as detailed
below.

| Pattern | Meaning | Timing / expected behavior |
|---|---|---|
| Smooth breathing | The cooperative main loop is running. | Rises from off to a peak brighness (a PWM value of 128), then fades to off; one complete cycle takes 2.56 seconds. |
| Rapid, regular flashing | An assertion failed and the controller entered `panic()`. | Full on for 100 ms, off for 100 ms: **5 flashes per second (5 Hz)**. Continues for five minutes, then the watchdog resets the Mega approximately one second later. |
| Off during startup | Normal while setup and the startup lamp/marching-row tests execute. | Breathing begins when the controller reaches its main loop. The startup tests still take time even with no pixel boards connected. |
| Frozen brightness after startup | The main loop is not currently updating the breathing pattern. | May occur during a blocking command or indicate a hang. The LED can remain bright, dim, or off; check the serial console and record what was running. |

**Breathing confirms main-loop activity, not Ethernet health.** It should
continue when the cable is unplugged or no TCP client is connected. Confirm
network recovery separately with serial `status` and a fresh netcat command.

For a deliberate panic-indicator test, enter `panic` in **Serial Monitor**.
This temporary, undocumented command executes `ASSERT(false)` and does not
return to normal command processing. It is not available over Ethernet.
Expect the rapid flashing and assertion details on serial, followed by the
five-minute wait and watchdog restart. Record this intentional reset separately
from Ethernet recovery failures.

## Commands and observations

Keep Arduino Serial Monitor open throughout testing. Opening it may reset the
Mega, so open it before measuring uptime or idle intervals.

Connect from macOS Terminal:

```sh
nc -v -G 3 192.168.86.143 2001
```

`-G 3` limits connection establishment to three seconds. It does not impose a
three-second idle timeout. Do not add `-w` during the firmware idle-timeout test,
because netcat might then disconnect before the controller does.

Once connected, enter any of these commands and press Return:

```text
status
version
help
```

Use Control-C to exit netcat. For a quick connectivity check:

```sh
ping -c 4 192.168.86.143
nc -vz -G 3 192.168.86.143 2001
```

The `-z` check opens and closes a connection without sending a command; use the
interactive command above to verify actual command processing. A failed ping
alone does not prove that an address is vacant or that TCP is unavailable.

During each test, observe:

- Normal breathing of the D45 status LED.
- Serial link, listener, connection, and idle-close messages.
- Serial `status`: `link`, `listener`, `client`, and active IP address.
- Uptime, to distinguish recovery from an unexpected controller reset.
- Whether a fresh netcat connection can execute commands.

When Ethernet is connected and ready, expect `ethernetHardware=W5500`,
`link=up`, and `listener=running port=2001`. A brief `link down` message during
startup is acceptable if the link subsequently comes up and the listener starts.

## Test cases

For each case, record pass/fail and observations in the results table below.

### 1. Normal connection and disconnection

1. Connect with netcat and issue `status`, `version`, and `help`.
2. Exit with Control-C.
3. Reconnect and issue `status`.

**Expected:** Commands return complete responses. Reconnection works without
resetting the Mega. Uptime continues increasing.

### 2. Ten-minute idle timeout

1. Connect and issue `status`.
2. Record the time, then send nothing for slightly more than 10 minutes.
3. Check Serial Monitor and reconnect with netcat.

**Expected:** The controller logs `socket: idle client closed`. The old session
closes, and a fresh connection executes commands. The Mega does not reset.

### 3. Input renews the idle timer

1. Connect and issue `status`.
2. Wait nine minutes, then issue `status` again.
3. Wait another nine minutes and issue another command.
4. Finally, leave it untouched for more than 10 minutes.

**Expected:** The connection survives the nine-minute intervals, then closes
after 10 minutes without input, measured from the most recent received bytes.

### 4. Ethernet cable loss without an active client

1. With no netcat session active, unplug the controller's Ethernet cable.
2. Leave it unplugged for 10 seconds. Check serial `status`.
3. Reconnect the cable, allow the physical link to negotiate, and check status.
4. Connect with netcat and issue commands.

**Expected:** While disconnected, status reports `link=down` and the listener
is stopped. After reconnection, the link returns and the firmware starts the
listener automatically. The breathing LED and serial console remain responsive.

### 5. Ethernet cable loss with an active client

1. Connect with netcat and issue a command.
2. Unplug the controller's Ethernet cable for 10 seconds.
3. Reconnect the cable. Exit the old netcat session if necessary.
4. Start a fresh netcat session and issue `status`.

**Expected:** The old session may close or become stale. The firmware cleans up
the old client and restarts listening after link recovery. A new connection works;
the old TCP session is not expected to resume automatically.

### 6. Boot with Ethernet disconnected

1. Unplug the Ethernet cable and reset the Mega.
2. Allow serial startup to finish. Confirm the serial console works.
3. Connect the Ethernet cable and wait for link negotiation.
4. Connect with netcat and issue commands.

**Expected:** Startup completes without Ethernet. Connecting the cable later
brings up the listener without another reset.

### 7. Repeated cable interruptions

Repeat tests 4 and 5 a total of 10 times, including both short and longer
disconnections. Verify a command response after every reconnection.

**Expected:** No accumulating failures, socket exhaustion, unexpected resets,
or need to power-cycle or reseat the shield.

### 8. Force-kill the netcat process

1. Connect with netcat and issue `status`.
2. In a second Terminal window, identify that netcat process:

   ```sh
   pgrep -fl 'nc.*192\.168\.86\.143.*2001'
   ```

3. Identify the PID belonging to the test session, then run the following,
   replacing `<PID>` with that numeric process ID:

   ```sh
   kill -KILL <PID>
   ```

4. Start another netcat session and issue commands.

**Expected:** The controller accepts a new connection without resetting.

**Interpretation:** SIGKILL abruptly terminates the application, but macOS
normally closes its TCP socket afterward. This does not reliably simulate a
client disappearing without TCP closure. Use test 9 for that condition.

### 9. Client network disappears while controller link stays up

1. Connect from the Mac and issue `status`.
2. Disconnect the Mac's network connection while leaving the controller's
   Ethernet connected. If using Wi-Fi, temporarily turn off Wi-Fi; account for
   any other active network interface that might still provide connectivity.
3. Keep the Mac disconnected for more than 10 minutes.
4. Observe the Mega through USB Serial Monitor.
5. Restore the Mac's network connection, exit any stale netcat session, and
   start a fresh connection.

**Expected:** The controller's physical link remains up. Its input-idle timeout
cleans up the client even if TCP has not already detected the loss. A fresh
connection works after the Mac rejoins the network.

This temporarily interrupts the Mac's network access, including remote sessions.
It does not require taking the house network down.

### 10. Incomplete command and connection cleanup

1. Connect and type `sta` without pressing Return.
2. Wait more than 10 minutes, or disconnect the client.
3. Reconnect and enter `status` followed by Return.

**Expected:** The new command works normally; characters from the previous
client do not remain in its input buffer.

Some terminal configurations buffer typed text until Return. In that case,
netcat may not actually transmit the partial line. To send three bytes without
a newline reliably, use this command instead of step 1:

```sh
{ printf 'sta'; sleep 630; } | nc -v -G 3 192.168.86.143 2001
```

The sleep keeps netcat's input open for 10.5 minutes. The expected closure is
the controller's 10-minute timeout, not the end of that sleep.

### 11. Controller reset during a TCP session

1. Connect and issue `status`.
2. Reset the Mega while netcat remains open.
3. Allow startup to finish, exit the old session, and reconnect.

**Expected:** The old connection closes or becomes stale. Uptime restarts
because this reset is intentional. A fresh connection works after startup.

### 12. Extended run

Leave the controller powered overnight or longer:

- First, allow a client to remain idle and confirm timeout/reconnection.
- Then send commands periodically at intervals shorter than 10 minutes.
- Include a cable interruption late in the run and verify recovery.

**Expected:** Stable command responses, normal breathing LED, and no unexplained
resets or progressive loss of connectivity. A successful short bench run alone
does not establish long-term reliability.


## If a test fails

Before resetting or unplugging the shield, capture:

1. Test number, time, and exact sequence of actions.
2. LED behavior: breathing, frozen, panic blinking, or off.
3. Whether the USB serial console still answers `status`.
4. Complete serial status and recent diagnostic output, including uptime.
5. Ping and netcat results after the failure.
6. What ultimately restored operation: reconnecting the cable, waiting,
   reconnecting netcat, resetting the Mega, cycling power, or reseating the shield.

## Results

Checkboxes below identify tests completed on the final bench build. Tests 7
and 9 retain earlier reported passes but were not repeated; test 12 was not run.
Allan accepted this scope before packing the controller for the CBS visit.
Detailed responses and timing are preserved in `test-results/`.

The earlier incomplete-command failure no longer reproduced after idle renewal
was based on actual bytes read. This does not establish the cause of the
original onsite lockup.

Firmware build/date: November 22, 2026, 16:57:44 (final bench regression build).

Tester and date: Allan, with automated TCP checks, November 22, 2026.

Switch/cable details: home network; switch model and cable details not recorded.

| Done | Test | Pass/fail | Notes / recovery required |
|---|---|---|---|
| [x] | 1. Normal connection/disconnection | PASS | Retested 2026-11-22, build 16:57:44: status/version/help complete; SIGINT and reconnect successful; uptime 00:01:26 to 00:01:33; receive diagnostic counters normal. |
| [x] | 2. Ten-minute idle timeout | PASS | Retested 2026-11-22, build 16:57:44: idle closure after 600.479 s; reconnect succeeded without reset. |
| [x] | 3. Input renews timeout | PASS | Retested 2026-11-22, build 16:57:44: Two nine-minute renewals survived; idle closure after 600.374 s; reconnect succeeded without reset. |
| [x] | 4. Cable loss without client | PASS | Retested 2026-11-22, build 16:57:44: unplugged serial showed link down/listener stopped; LED kept breathing; after reconnection fresh TCP status succeeded with continuous uptime. |
| [x] | 5. Cable loss with client | PASS | Retested 2026-11-22, build 16:57:44: active client cleaned up on link loss; serial responsive and LED breathing; fresh TCP status succeeded after reconnect, without reset. |
| [x] | 6. Boot without Ethernet | PASS | Retested 2026-11-22, build 16:57:44: serial confirmed link down at boot, ready at 00:00:08.808, listener started at 00:01:14.511; LED breathing; fresh TCP status succeeded. |
| [ ] | 7. Repeated interruptions | Earlier PASS | Reported by Allan, 2026-11-22; not repeated on final build. |
| [x] | 8. Force-kill netcat | PASS | Retested 2026-11-22, build 16:57:44: SIGKILL terminated nc; fresh status succeeded without reset. |
| [ ] | 9. Client network disappears | Earlier PASS | Reported by Allan, 2026-11-22; not repeated on final build. |
| [x] | 10. Incomplete command cleanup | PASS | Retested 2026-11-22, build 16:57:44: unterminated sta closed after 600.373 s; immediate reconnect/status clean; uptime continued from 00:04:34 to 00:14:35. Earlier build failed at 630 s. |
| [x] | 11. Controller reset | PASS | 2026-11-22: Allan pressed RESET with TCP connected; old session closed; fresh status succeeded with restarted uptime. |
| [ ] | 12. Extended run | Not run | Allan elected to stop testing after final-build regression. |

**Primary pass criterion:** Network interruptions recover without removing the
Ethernet shield or power-cycling the controller. Tests that intentionally reset
the controller are recorded separately from tests of recovery without a reset.

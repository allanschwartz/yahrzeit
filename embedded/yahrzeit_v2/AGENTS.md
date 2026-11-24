# V2 Mega firmware

Applies to this directory in addition to ../AGENTS.md.

## Scope and style

- V2 targets the Arduino Mega 2560 with an Arduino Ethernet shield (W5100 or
  W5500/Shield 2), using the modern Ethernet library. Do not change V3 as part
  of V2 work unless explicitly requested.
- Allan reviews V2 and V3 side by side. Prefer comparable organization while
  preserving V2 behavior; do not copy V3 regressions or introduce a class
  conversion just to match its structure.
- Use multiline /** ... */ documentation with explicit @brief. Header
  declarations carry only concise briefs so the interface is easy to survey;
  keep detailed @param, @returns and @note descriptions in implementation files.
  Keep full top-of-file Doxygen headers, including module overview, history,
  author and copyright, even in .h files. Keep comments aligned with behavior.
- Allan edits concurrently in an editor and may save late. Reread files before
  editing and preserve his changes. Never discard unrelated workspace changes.

## Module boundaries

- yahrzeit_v2.ino: wiring/network configuration, socket objects, setup/loop,
  version output, writeOutput, sleep_ms, uptime, panic and status LED.
- socket_thread.h/.ino: Ethernet initialization, listener recovery, client
  lifecycle, 10-minute input-idle timeout, and incremental TCP input.
- serial_thread.h/.ino: 9600-baud UART input via USB bridge, echo/backspace,
  startup console tests, and timestamped logging.
- CmdProc.ino: shared command dispatch, text tables, status output, and dumps.
- led_device.h/.ino: logical addressing, panel tables, result IDs and EEPROM
  persistence. CmdProc.h owns command IDs, result text and command declarations.
- led_selftest.ino: tests 1–6; test 7/Cylon was deliberately removed.
- yyz_pixel.cpp/.h: low-level latched display driver; preserve signal timing.
- watchdog_startup.cpp: AVR .init3 watchdog shutdown. The bootloader must also
  support watchdog resets; compile success does not prove restart behavior.

## Intentional conventions

- Command verbs and boolean word prefixes compare two characters without case
  sensitivity through inline strMatch2(). Preserve the remaining command protocol
  and parser limitations. parse_command records
  argument starts without inserting NULs. atoi stops at whitespace; arguments
  compared as exact strings must be last without trailing spaces. Boolean words
  and command verbs use prefix matching. Do not add strict numeric/coordinate
  validation or tokenize in place without explicit approval. Whitespace-only
  commands are guarded before command matching.
- Dimension/origin tables include index PANEL0 (zero): full display dimensions
  and origin row 1/column 1. Coordinates are one-based at the LED device layer.
- Brightness is active-low OE PWM: 0 is full brightness, 255 is off.
- YYZ boards latch outputs: sleep_ms optionally refreshes once, then delays.
  Do not restore the removed TEST_16x64_LED_MATRIX or LEDMatrix driver.
- Full-buffer readers currently process a truncated prefix and leave remaining
  input queued; do not silently redesign this protocol behavior.
- Serial CR and LF each terminate a command; TCP ignores CR and terminates on LF.
- socket_gets returns {charsRead, status}; renew the idle timer only for
  actual consumed bytes. Receive diagnostics are per client and exposed by status.
- Link Unknown is distinct from LinkOFF and must remain usable for W5100.
- socketInit is called from socket_thread after link checking so link loss can
  clear the listener flag and later retry startup. The flag is software state,
  not proof of hardware/socket health. Missing hardware must not start a listener.

## Hardware and validation

- Current shield label MAC: A8:61:0A:AE:E4:E2. Do not reuse this MAC on a second
  simultaneously connected device. Network/IP changes must be deliberate.
- Display DI/OE/CP/ST: 42/44/46/48. Ethernet CS: 10; unused SD CS: 4 HIGH.
  External ALIVE LED uses D45 through 1 kohm to its anode, cathode to GND.
  Normal animation is a 256-entry PROGMEM sine wave, peak PWM 128, period 2560 ms.
- PANIC_MINUTES remains near panic code; panic blinks for five minutes then
  uses AVR watchdog reset. It is not an always-on watchdog for arbitrary hangs.
- TEST_FIXTURE is currently selected: two V3-compatible 24-row by 3-column
  boards, 24x6 overall, with panel columns 1 and 4. TEST_2x24_HARNESS retains
  the older 24x2 geometry. Select exactly one geometry in yahrzeit_v2.h.
- CBS_56x40_WALL selects site networking (192.168.13.9, gateway .13.8); bench
  geometries select lab networking (192.168.86.243, gateway .86.1). Reserve
  .243 for this shield before use; .240, .241 and .242 responded to ARP.
  Home network inventory: /Users/allan/Desktop/HOUSE/home_network_inventory.md;
  MAC addresses are more reliable identifiers than its historical IP addresses.
- Build with arduino-cli compile --fqbn arduino:avr:mega; on this Mac the CLI is
  bundled under /Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/.
- Use /private/tmp for build artifacts and focused host checks. Compile affected
  configurations and test meaningful behavioral changes; comment-only edits
  need only verification that executable code is unchanged.
- Never upload firmware or send physical controller commands without explicit
  authorization. Report hardware behavior as unverified until actually tested.

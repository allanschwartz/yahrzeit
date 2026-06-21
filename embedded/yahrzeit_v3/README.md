# Yahrzeit Embedded Controller (v3)

This folder contains the firmware for the third-generation Yahrzeit wall controller. The code runs on an Arduino-compatible controller, receives commands over USB serial or Ethernet, and drives the LED wall through the YYZ pixel interface.

## Purpose

The embedded side is responsible for:

- accepting command lines from the appliance or a console,
- updating the logical wall framebuffer,
- refreshing the LED array,
- saving/loading display state,
- and supporting self-test patterns.

## Main components

- `yahrzeit_v3.ino` — setup, main loop, startup sequence, and shared utility functions.
- `yahrzeit_v3.h` — project-wide constants, configuration macros, and shared declarations.
- `CmdProc.*` — parses command text and dispatches operations.
- `LedWall.*` — maps logical wall coordinates and panel geometry to the framebuffer.
- `YyzPixel.*` — low-level driver for the shift-register / LED chain.
- `serial_thread.*` and `socket_thread.*` — collect command input from the serial console and TCP socket.
- `selftest.h` and `selftest.ino` — test patterns used during bring-up.

## Hardware model

There are two supported layouts:

- `TEST_FIXTURE` — used for bench work and the 9-column test board.
- `CBS_56x40_WALL` — the actual 56-row by 40-column synagogue wall geometry.

Only one geometry should be enabled at a time.

## Related hardware

- The physical wall hardware lives under [Hardware](../../Hardware).
- The test fixture board is represented by the `yahrzeit_pixel9` work under [Hardware/yahrzeit_pixel9](../../Hardware/yahrzeit_pixel9).
- The cabling/interface board is the `pixel_interface_v3` design under [Hardware/pixel_interface_v3](../../Hardware/pixel_interface_v3).
- The older shield-era design is the `YYZ_SHIELD_v2` work under [Hardware/YYZ_SHIELD_v2](../../Hardware/YYZ_SHIELD_v2).

## Wiring / pin assumptions

The controller currently uses the following pin assignments for the pixel chain:

- `DI` = pin 4
- `OE` = pin 5
- `CP` = pin 6
- `ST` = pin 7

These assignments are defined in the top-level sketch and are a key part of the firmware configuration.

## Command protocol

The controller accepts ASCII commands over serial or socket, including:

- `ALL on|off [panel]`
- `BRIGHTNESS <n>`
- `CLEAR <panel>`
- `DATA <row> <col> <binary-data>`
- `DUMP [panel]`
- `HELP`
- `LOAD`
- `PIXEL on|off <row> <col> [panel]`
- `REFRESH`
- `SAVE`
- `STATUS`
- `TEST <testnumber> [panel]`
- `VERSION`

## Notes for contributors

- Keep the command protocol stable unless there is a documented reason to change it.
- Keep hardware geometry logic in the wall abstraction layer rather than scattering it through the command code.
- Treat the test fixture configuration as a development tool, and keep the full-wall geometry clearly separated.
- When editing firmware, update this document if the wiring assumptions, command set, or hardware layout change.

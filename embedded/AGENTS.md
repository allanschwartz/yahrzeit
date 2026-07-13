# Yahrzeit Embedded Project Notes

This directory contains the embedded controller history and current Arduino
controller work for the Yahrzeit Wall.

Primary goals:

- Preserve controller behavior unless explicitly asked to change it.
- Keep diffs small and easy to inspect.
- Do not change the controller command protocol unless explicitly asked.
- Prefer simple Arduino/C++ code over clever abstraction.
- Avoid dynamic allocation unless there is a clear reason.
- Be careful with RAM, flash, and timing-sensitive display refresh code.

Style preferences:

- Follow Arduino style loosely for `.ino` files.
- Use lowerCamelCase for ordinary functions and variables.
- Use UpperCamelCase for classes.
- Use uppercase-with-underscores for `constexpr` constants that replace old
  `#define` names.
- Do not add a leading `k` to constants just to indicate const-ness.
- Keep Arduino pin definitions in the main `.ino` file when they describe the
  actual application wiring.
- Pin names may preserve silkscreen names, such as `DI_PIN`, `OE_PIN`,
  `CP_PIN`, and `ST_PIN`.

Current V3 boundaries:

- `yahrzeit_v3.ino` owns board wiring, network defaults, and top-level object
  instantiation.
- `socket_thread.ino` owns Ethernet/socket setup and command transport.
- `serial_thread.ino` owns serial command input.
- `CmdProc` owns controller command parsing and dispatch.
- `LedWall` owns wall-level LED behavior.
- `YyzPixel` owns low-level pixel/frame-buffer behavior.

Validation:

- Prefer compiling with the Arduino toolchain or IDE after meaningful changes.
- For host-side checks, use syntax/search review where Arduino build tools are
  not available.
- Do not run commands that transmit to physical hardware unless explicitly
  asked.

Deployment notes:

- The controller TCP port is expected to match the PHP wrapper default.
- Static IP address defaults live in the embedded V3 application code.
- Site-specific network changes should be made deliberately for installation
  and committed/tagged when they are intended to become the installed version.


## Embedded controller command vocabulary

The V3 Arduino controller implements a CLI command language documented in
`yahrzeit_v3.h`.

The PHP appliance historically emits only a small subset:

- `all off`
- `pixel on <row> <col> <panel>`
- `pixel off <row> <col> <panel>`
- `refresh`
- `save`

The modern appliance/watcher should continue to emit this simple command stream.
The compiled `yahr_conduit` helper is obsolete; the controller is reachable by
`nc` or telnet over TCP/IP.

# Yahrzeit PHP Cleanup Notes

Working directory: `yahrzeit_site-v3`

This directory contains the legacy PHP "Yahrzeit Appliance" code for the
Congregation Beth Sholom Yahrzeit Wall system.

## Current cleanup goal

Preserve and understand the legacy PHP implementation before rewriting or
removing anything. The immediate goal is archaeology, not modernization.

## Architecture vocabulary

- **Yahrzeit Appliance**: the Mac/PHP/web system that stores the yahrzeit data,
  presents the GUI, evaluates synagogue minhag/policy, and emits commands.

- **Yahrzeit Embedded Controller**: the Arduino-based controller behind the
  wall. It receives a simple command stream over TCP/IP and drives the LEDs.

- **Yahrzeit Pixel / YPX / YyzPixel boards**: the LED boards mounted behind the
  wall.

The appliance decides *what* should be lit. The embedded controller decides
*how* to drive the LEDs.

## Runtime command model

The legacy PHP daemon (`yahrzeitd.php`) is not a persistent daemon in normal
use. It is a one-shot command generator.

The shell wrapper runs PHP, captures the generated command stream, and pipes it
to the embedded controller using `nc`.

The PHP appliance historically emits this small command subset:

- `all off`
- `pixel on <row> <col> <panel>`
- `pixel off <row> <col> <panel>`
- `refresh`
- `save`

The `save` command is required by this application after sending display state.
It causes the Arduino controller to persist the current LED/display state.

The `load` command is not normally sent by the appliance. The Arduino controller
loads saved state autonomously on power-up.

Current ordinary-yahrzeit strategy:

1. Emit `all off`.
2. Compute active yahrzeit names.
3. Emit `pixel on ...` for selected names.
4. Emit `refresh`.
5. Emit `save`.

## Domain term: minhag

`minhag` is Hebrew for custom or practice, but in this system it means more
than a generic software setting.

In this project, minhag is synagogue-specific rabbinic/custom policy. Examples:

- whether yahrzeits are observed by Hebrew or English date
- whether lights are controlled by fixed clock time or relative to sunset
- whether to include the Shabbat before a yahrzeit
- whether to include a full week around the yahrzeit
- which Yizkor dates are observed
- whether Yizkor is on the 1st or 2nd day of Shavuot
- when Yizkor lighting begins and ends

In code, `minhag` should be treated as site policy/configuration, not merely
UI preferences.

## Known cleanup issues

- Many paths are hard-coded to `/Users/allan/Sites/yahrzeit`.
- PHP boolean handling is fragile. The legacy `myBool()` treats any non-empty
  string, including `"NO"`, as true.
- `split()` is used in old PHP code and must become `explode()` for modern PHP.
- `7minhag.php` appears to use `yizkorPesachDay` when rendering the Shavuot
  day selector; this is likely a copy/paste bug.
- `yahr_conduit` is obsolete transport code. The modern controller can be
  driven with `nc`, possibly through a small `slowcat` wrapper.
- The current GUI write paths should not be trusted against live data until
  backup/diff testing is performed.


Historical comma-prefixed scratch files were preserved under attic/notes.
They show that, as of February 2007, the GUI screens were partially complete,
`yahrzeitd` and `yizkord` were mostly done, and `yahrzeit_watcher` was already
identified as needing a rewrite.
EOF

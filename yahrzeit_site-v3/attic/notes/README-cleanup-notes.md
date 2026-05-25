
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


---

## Obsolete transport: yahr_conduit

The `yahr_conduit` program was an older C transport helper used during the
serial/8051 and early network-controller eras. It supported transport experiments
before the command-stream-over-TCP model settled.

Later wrappers use `nc` directly. The V3 Arduino controller is reachable by
telnet or `nc`, so `yahr_conduit` is preserved only as historical transport code.

Old source, binaries, and conduit-era wrappers are preserved under:

- `attic/transport/yahr_conduit_src/`
- `attic/transport/yahr_conduit_bin/`

---

## Future watcher requirement

The legacy system used multiple cron entries to run yahrzeit and Yizkor actions.
This is fragile because Yizkor dates and times depend on synagogue minhag and
may change year to year.

Future design:

- Keep cron or launchd simple.
- Schedule only one periodic task: `yz_watcher`.
- `yz_watcher` reads the current date/time, `minhag.ini`, and the name/panel data.
- It decides whether the current state should be:
  - ordinary yahrzeit lighting
  - Yizkor lighting
  - Yom HaShoah lighting
  - Yom HaZikaron lighting
  - Erev Shabbat / weekly extension lighting
  - all off / no active observance
- It emits the appropriate command stream only when the desired state changes.
- It should log what it decided and why.

The first implementation may still use cron, but cron should call only the watcher.

## TODO: Share yahrzeit business logic with GUI screens

Some of the important yahrzeit decision logic currently lives inside
`yahrzeitd.php`, including English-date matching, Hebrew-date matching,
Friday/Shabbat observance behavior, manual/reserved handling, and the final
“should this person be lit?” decision.

This logic should eventually be factored into a shared include file, so that:

- `yahrzeitd.php` uses it to generate controller commands
- `3singlepanel.php` can use it to show `ledon.gif` / `ledoff.gif`
- future GUI/status pages can report why a name is expected to be lit

The goal is to avoid duplicating yahrzeit selection logic in the GUI.
The GUI should show the same expected lighting state that the regular
`bin/yahrzeit` path would produce.

## TODO: Write simplified GUI help files

The old GUI includes Page Help / Main Help links, but the help content should
be rewritten after the screens are simplified.

Needed help topics:
- Yahrzeit overview screen
- Panels and manual lighting operations
- Names search/browser
- Single-name record view/edit policy
- Reports and audit output
- Minhag settings and rabbinic/custom policy

The help should be short, operational, and written for a CBS volunteer or
office/rabbinic staff member, not for a software developer.o

help/
  index.htm
  yahrzeit.htm
  panels.htm
  names.htm
  reports.htm
  minhag.htm

  Yahrzeit
  What this appliance does
  What today’s scheduled lighting means
  Where manual controls are located

Panels
  How to use the photo/table to inspect a panel
  What manual lighting operations do
  When to use all-off / all-on / Yizkor

Names
  How to search by name, date, option, or location
  What Location means: panel-row-column
  Click a name to view the record

Reports
  Audit report
  Upcoming yahrzeits
  Command preview / notransmit mode

Minhag
  What “minhag” means here
  Which settings affect normal yahrzeit lighting
  Which settings affect Yizkor
  Warning: change only with rabbinic/office approval

  Manual lighting operations immediately send commands to the controller.

Use “Turn all lights off” only when you intend to clear the wall.
Use “Turn all lights on” or “Run Yizkor pattern” only for maintenance,
testing, or a scheduled Yizkor observance when automation is unavailable.











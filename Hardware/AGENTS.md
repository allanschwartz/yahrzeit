# Yahrzeit Hardware Project Notes

This directory contains mechanical, PCB, and fabrication-related design files
for the Yahrzeit Wall project.

Primary goals:

- Preserve measured dimensions unless explicitly asked to change them.
- Keep OpenSCAD designs parametric and easy to tune from caliper measurements.
- Prefer simple printable geometry over clever mechanisms.
- Keep connector cutouts table-driven where possible.
- Make small, reviewable changes; mechanical work is iterative.

OpenSCAD style:

- Use millimeters.
- Keep important dimensions as named variables near the top of the file.
- Prefer straightforward modules with descriptive names.
- Use `hull()` for rounded rectangles/slots when it keeps the model simple.
- Use `difference()` for cutouts and holes.
- Avoid decorative complexity unless it improves printing, strength, or use.

Enclosure guidance:

- Board standoffs and cover posts should be sized around the actual threaded
  inserts and screws in use.
- Connector panels and case openings should allow practical FDM clearance.
- Keep sacrificial/test-print parts separate when they speed calibration.
- Do not assume connector cutouts are correct until they have been checked
  against printed plastic and the real board stack.

KiCad guidance:

- Preserve existing schematic, PCB organization, and footprints unless asked.
- Do not make electrical design changes casually.
- During board review, explicitly check diode/LED polarity against the physical
  intent, not only KiCad DRC. KiCad LED footprints commonly use pin 1 as
  cathode and pin 2 as anode, while field wiring/connectors often expect pin 1
  to be `+` and the highest-numbered pin to be `GND`.
- For LEDs or external two-wire indicators, verify that the PCB geometry and
  silkscreen make polarity obvious without relying on English-only notes.
- Treat fabrication outputs as generated artifacts unless the user asks to
  refresh them.
- Prefer inspection and explanation over automated KiCad edits unless the task
  is explicit.

Validation:

- Render changed `.scad` files with OpenSCAD when possible.
- If available, use the command-line OpenSCAD renderer for a quick syntax/CGAL
  check before handing the model back.
- Do not overwrite generated STL files unless the user asks for refreshed
  output.

Repository hygiene:

- Keep site-local installation files, such as `/etc/netplan/*.yaml`, out of
  git. Add examples only when they are clearly marked as examples.

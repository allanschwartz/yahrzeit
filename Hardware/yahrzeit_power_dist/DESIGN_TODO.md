# Yahrzeit Power Distribution Board — Design TODO

**Status:** Preliminary design; review these items before fabrication or BOM
release.

## Power-present indicator

- Add a conspicuous power-present LED near J1, with a board legend such as
  `POWER ON — 12V`.
- Connect the indicator to the protected/rectified 12 V rail, after F1 and D1,
  rather than only to the regulated 5 V output. This ensures that an energized
  board remains visibly identified even if U1 has failed or is inhibited.
- Tentative circuit: green SMD LED plus an SMD series resistor. Select the LED,
  current, resistor value, power rating, footprints, and MPNs during the BOM
  review. Approximately 2–4 mA should provide a clearly visible indicator
  without meaningful loading.
- Position the LED so it remains visible with the board installed and does not
  sit beneath a cable or the buck module.
- The indicator is advisory only. Documentation must still require power to be
  disconnected and verified with a meter before servicing.

## Component and footprint audit

### F1 — resolve part/footprint mismatch

- The present MPN, Littelfuse `0215004.MXP`, is a 4 A, 5 x 20 mm cartridge
  fuse.
- The assigned footprint, `Fuseholder_Blade_Mini_Keystone_3568`, is for a MINI
  automotive blade fuse and is not compatible with that MPN.
- Choose one implementation and make the symbol value, MPN, datasheet, holder,
  and PCB footprint agree.
- Confirm the required time-current characteristic, interrupting rating,
  voltage rating, inrush behavior, and field-replacement procedure.

### D1 — select and verify the final reverse-polarity diode

- Confirm a currently manufactured and orderable single Schottky rectifier.
- The selected part must match the two-lead TO-220 footprint and pin numbering.
- Add the manufacturer datasheet to the schematic field.
- Calculate forward loss and temperature rise at measured maximum input
  current. Reconsider an ideal-diode MOSFET solution if the Schottky loss is
  excessive.

### F2–F7 — reconcile rating and labeling

- The current MPN, Littelfuse `1812L150/16DR`, is a resettable PTC with a 1.5 A
  hold current and 2.8 A trip current at 20 degrees C; it is not a 1.25 A
  slow-blow fuse.
- Either relabel the schematic and board for the actual 1.5 A-hold PTC or
  select a genuine 1.25 A-hold device.
- Evaluate temperature derating, normal branch current, fault current, trip
  time, resistance/voltage drop, and reset behavior.
- After selection, copy the final MPN and manufacturer datasheet into F2–F7.

### Connector ordering codes

- Replace abbreviated JST values such as `B2P-VH` and `B3P-VH` with complete,
  orderable manufacturer MPNs, including the required plating/packaging
  suffixes.
- Confirm mating housings, crimp contacts, wire gauge, polarization, and cable
  drawings for J1 and J3–J8.

## Final pre-fabrication checks

- Measure actual column current and voltage at the wall under worst-case LED
  load before finalizing fuse and conductor ratings.
- Complete the high-current routing, front-side power copper, back-side GND
  plane, and ground-stitching-via design.
- Review U1 input/output capacitor loops and the local `SENSE` and `ADJ`
  routing.
- Check creepage/clearance, copper bottlenecks, thermal reliefs, connector
  access, hinge clearance, cable clearance, and mounting-hole clearance.
- Run KiCad ERC and DRC and resolve or explicitly document every exception.
- Generate and review the fabrication outputs, BOM, placement data, assembly
  drawing, and cable drawings before ordering.

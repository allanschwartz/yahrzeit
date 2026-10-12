# Yahrzeit 5 V Power Distribution Board — Design TODO

## Site visit — Tuesday, October 6, 2026

- Choose the power-distribution board location, as 3 inches about the power supply 4x8
- Verified that the six existing 18 AWG output cables reach the new terminals
  with enough slack for ferrule termination and servicing.
- Measured the routed length needed for CBL-PD-01, as 24 inches
- (next site visit) Measure and record column current in normal operation and full Yizkor mode,
  noting which column and operating settings were measured.

**Status — September 29, 2026:** Direct-5 V distribution layout is ready for
PCBWay quotation. Fabrication choice: 140 × 50 mm, two-layer FR-4, 2.0 mm
thickness, 2 oz finished copper, green mask, white silkscreen, lead-free HASL.

## Electrical validation

- Measure the actual current of one fully illuminated panel column and verify
  adequate margin in the selected 5 V supply.
- Measure voltage at the supply, J1, and the most distant YYZ_PIXEL board under
  maximum load. Select the supply setpoint and feed-wire gauge from these
  measurements; do not exceed the pixel electronics' permitted supply voltage.
- Check supply startup, shutdown, and load-step behavior. Decide from those
  measurements whether an input TVS is useful.
- Confirm that the 7.5 A F1 fuse tolerates normal startup current while opening
  safely for a feed or board fault.
- Validate F2–F7 normal current, voltage drop, temperature derating, fault trip
  behavior, and reset behavior.
- Confirm LED1 is clearly visible at the final 5 V setpoint.

## Component and mechanical verification

- Print the current PCB STEP model and fit physical examples of J1, J2–J7, the
  Keystone 3568 holder, and its MINI blade fuse.
- Using the mockup, verify connector fit and orientation, cable bend room,
  fuse-removal access, LED visibility, mounting holes, and hinged-panel
  clearance.

## Pricing and documentation

- Obtain PCBWay assembly/parts pricing for ten boards; retain the
  ten-board, 2 oz fabrication quote as the current planning basis.
- Verify the Amazon power-supply price and select quantities for installed
  columns and spares. Seven columns need 42 output adapters and seven input
  cables; ten complete assemblies need 60 and ten respectively.
- Update the budget for local cable fabrication and confirm whether two
  PCBWay shipments are still required. Separate quoted charges from
  allowances, shipping, taxes/import charges, and labor.
- Prepare the intended customer quote at $100 per installed column
  ($700 for seven), with the included scope stated explicitly.
- Verify the saved PCB/3D views and schematic PDF/PNG match the release files.
  Export sources and drawings from the same final design revision.
- Verify schematic-to-PCB agreement, particularly TP3's move to output 6.

## Prototype and production release

- Check main-feed, fuse-pad, and ground-return temperature and voltage drop
  at maximum measured load using the selected 2 oz copper.
- Review thermal reliefs, ground-plane continuity, stitching vias, connector
  access, and copper and solder-mask clearances.
- Run final ERC and DRC and document any intentionally accepted warnings.
- Generate and review Gerbers, drill files, BOM, placement data, assembly
  drawings, and cable drawings before ordering the electrical prototype.
- Assemble and bench-test one complete prototype before releasing the full
  installation quantity.
- Arrange for a qualified electrician to design and install the mains-side
  distribution and power-supply connections.

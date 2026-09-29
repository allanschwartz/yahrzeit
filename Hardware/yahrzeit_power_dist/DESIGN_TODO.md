# Yahrzeit 5 V Power Distribution Board — Design TODO

**Status — September 29, 2026:** Direct-5 V distribution layout is ready for
PCBWay quotation. Fabrication choice: 140 × 50 mm, two-layer FR-4, 2.0 mm
thickness, 2 oz finished copper, green mask, white silkscreen, lead-free HASL.
The earlier 200 × 50 mm board was quoted at $82.63 for ten bare boards;
request a new quote for this revision. Assembly and cables remain unquoted.

Completed layout work includes 20 mm output spacing, a broad fused +5 V bus,
nominally 3 mm main-input routing, 2 mm branch routing, and mounting holes
7.5 mm from each adjacent edge. TP3 measures output 6; `POWERED` indicates
the bus after F1. Text sizes and label placement have been reviewed.

The resized layout has no unconnected items and passes targeted 0.2 mm
trace-to-trace/pad clearance checks. Run editor DRC after the final edits
and before exporting fabrication files. Schematic ERC subsequently passed
with zero violations after adding a GND power flag; connector pin types and
physical circuit connections were preserved. The remaining work is below.

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
- Measure the installed J1 supply-cable route and confirm its final length.
- Confirm the 50 mm J2–J7 output-cable length in the installation, especially
  after the board shrank and output spacing changed to 20 mm.
- First experiment with local cable assembly using the crimpers and wire
  already received. Make and inspect sample JST VH, Molex Mini-Fit Jr., and
  ferrule terminations; check conductor retention, mating fit, continuity,
  polarity, and strain relief before producing the complete set.
- Record local cable material costs and assembly time. Use those results to
  decide whether PCBWay cable fabrication is still needed.

## Pricing and documentation

- Obtain PCBWay assembly/parts pricing for five and ten boards; retain the
  ten-board, 2 oz fabrication quote as the current planning basis.
- Verify the Amazon power-supply price and select quantities for installed
  columns and spares. Seven columns need 42 output adapters and seven input
  cables; ten complete assemblies need 60 and ten respectively.
- Update the budget for local cable fabrication and confirm whether two
  PCBWay shipments are still required. Separate quoted charges from
  allowances, shipping, taxes/import charges, and labor.
- Prepare the intended customer quote at $200 per installed column
  ($1,400 for seven), with the included scope stated explicitly.
- Verify the saved PCB/3D views and schematic PDF/PNG match the release files.
  Export sources and drawings from the same final design revision.
- Verify schematic-to-PCB agreement, particularly TP3's move to output 6.
- Check the F1 supplier link: the current README URL contains `0297002`,
  while the specified fuse is `0297007.WXNV` (7.5 A). Correct it before
  releasing the purchasing BOM.

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

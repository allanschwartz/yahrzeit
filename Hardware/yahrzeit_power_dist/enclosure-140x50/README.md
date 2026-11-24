# Power-distribution enclosure and alignment jig

These OpenSCAD models are a first printable mechanical design for the
140 × 50 mm Yahrzeit power-distribution PCB.

## Enclosure

`power_dist_enclosure.scad` contains five selectable views:

- `tray`: wall-mounted back tray with four external mounting ears, four PCB
  standoffs, and four cover posts.
- `cover`: removable protective cover with top and bottom cable openings,
  a complete lightening pattern whose lower-right slot also reveals the power
  LED, an internal clip for one spare MINI fuse, and four recessed M3 screw
  holes.
- `assembly`: translucent-cover preview with simplified component envelopes.
- `pcb_mockup`: complete printable clearance model with the 140 × 50 × 2 mm
  PCB, mounting holes, all component envelopes, the inserted-fuse envelope,
  and an 0805-sized LED1 surrogate for checking the cover window.
- `insert_test_block`: calibration print with three numbered heat-set-insert
  pockets. Pocket 1 is the original straight 3.93 mm pilot; pocket 2 adds a
  5.2 mm × 0.5 mm tapered entrance; pocket 3 adds the tapered entrance and a
  3.4 mm × 1.0 mm relief beneath the insert.

The external mounting ears keep the wall screws accessible after the PCB is
installed. Cable openings extend to the wall-facing edge of the cover, so the
cover lifts away without disconnecting input or output wires. Removing four
front screws exposes every terminal screw and the blade fuse. A snap clip on
the inside-right margin retains one genuine Littelfuse MINI spare by its
10.84 × 3.69 mm plastic body, leaving its blades exposed for removal.

Two 1.2 mm-wide tray ribs support the PCB beneath J1/F1 and J2-J7 while
terminal screws are tightened. They avoid the through-hole pin rows and stop 0.15 mm
below the normally seated PCB, so the four mounting standoffs remain the
primary seating surfaces.

The model uses Zwmssl `M3×6×5` heat-set inserts: M3 thread, 6 mm length,
5 mm outside diameter, and the manufacturer's specified 3.93 mm printed-hole
diameter. Printed testing selected insert-pocket design 3: a 3.93 mm pilot,
5.2 mm × 0.5 mm tapered entrance, and a 3.4 mm × 1.0 mm relief beneath the
insert for displaced plastic and the screw tip. Actual fit still depends on
filament, wall count, printer calibration, iron temperature, heating time,
and insertion technique. The board uses four M3×6 pan-head screws through its
4.5 mm mounting holes.

The successful PLA trial used a 200 °C iron setting. Hold the insert upright
with tweezers, place the tip in the insert, heat for a slow count of five, then
press straight for a slow count of three. Treat those timings as a repeatable
starting point rather than a universal specification; allow the plastic to
soften instead of forcing the insert.

With a 0.4 mm nozzle, slice the tray with at least five wall loops so the
material immediately surrounding each insert pocket is effectively solid.
This is more useful than increasing sparse infill across the entire tray.

The cover screw recess is 6.4 mm diameter and 1.3 mm deep. A typical M3 pan
head is approximately 6 mm diameter and 2.4 mm high, so it will stand about
1.1 mm proud. With 1.1 mm of cover material beneath the recess, the 6 mm screw
engages approximately 4.9 mm of the insert.

Recommended first prints:

1. Print `insert_test_block`, test pockets 1–3, and tune the selected insert
   geometry.
2. Print `pcb_mockup`, then print `tray` and check the complete component
   clearance model before the physical PCB arrives.
3. Check wire bend radius, fuse removal, screwdriver reach, LED alignment,
   panel-hinge clearance, and the maximum component height.
4. Adjust `inside_depth`, cable slots, and cover fit before printing `cover`.

The provisional height allowance is 17 mm above the PCB for the Keystone 3568
holder with an inserted MINI fuse, plus 3 mm cover clearance. Phoenix Contact
specifies a 10 mm installed height for the `5442206`, and C1 is 12.5 mm high.
Measure the physical F1/fuse stack when the sample parts arrive and update
`max_component_h` if necessary.

## Alignment jigs

`power_dist_alignment_jig.scad` duplicates the enclosure tray's wall-facing
outline and four mounting ears, without its standoffs. A durable solid panel,
4 inches wide and `enclosure_gap` high, extends below it. Visually center that
panel on the 4 × 8 inch electrical box and align its lower edge with the box's
top edge, then transfer the four 3 mm mounting-hole centers. The default is
25.4 mm for a 1-inch enclosure gap; use 50.8 mm for 2 inches or 76.2 mm for
3 inches. `power_dist_alignment_jig-1in.stl` is the ready-to-print 1-inch
version; print it flat with the raised label facing up.

`power_supply_alignment_jig.scad` fits inside the nominal 4 × 8 inch box with
0.1 inch of clearance at each edge. With the jig vertical and its lower-left
corner as the origin, its centered shallow recess locates the 3.236-inch-wide
by 3.91-inch-high BTF-LIGHTING AM50-B05-HD power supply. Two 1/8-inch guide
holes transfer the measured supply mounting-hole locations: 0.24 × 0.18 inch
from the recess's lower-left corner and 0.24 × 0.05 inch from its upper-right
corner. Verify the physical box, supply outline, and hole locations before
drilling the metal box.

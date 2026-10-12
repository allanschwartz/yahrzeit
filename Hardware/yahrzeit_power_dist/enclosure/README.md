# Power-distribution enclosure and alignment jig

These OpenSCAD models are a first printable mechanical design for the
140 × 50 mm Yahrzeit power-distribution PCB.

## Enclosure

`power_dist_enclosure.scad` contains five selectable views:

- `tray`: wall-mounted back tray with four external mounting ears, four PCB
  standoffs, and four cover posts.
- `cover`: removable protective cover with top and bottom cable openings,
  an extensive lightening pattern, a power-LED window, and four recessed M3
  screw holes.
- `assembly`: translucent-cover preview with simplified component envelopes.
- `pcb_mockup`: complete printable clearance model with the 140 × 50 × 2 mm
  PCB, mounting holes, all component envelopes, the inserted-fuse envelope,
  and a raised 0805-sized LED1 surrogate for checking the cover window.
- `fit_coupon`: small calibration print for the heat-set insert pilot and
  cover clearance.

The external mounting ears keep the wall screws accessible after the PCB is
installed. Cable openings extend to the wall-facing edge of the cover, so the
cover lifts away without disconnecting input or output wires. Removing four
front screws exposes every terminal screw and the blade fuse.

Two 1.2 mm-wide tray ribs support the PCB beneath J1 and J2-J7 while terminal
screws are tightened. They avoid the through-hole pin rows and stop 0.15 mm
below the normally seated PCB, so the four mounting standoffs remain the
primary seating surfaces.

The model uses Zwmssl `M3×6×5` heat-set inserts: M3 thread, 6 mm length,
5 mm outside diameter, and the manufacturer's specified 3.93 mm printed-hole
diameter. The 6 mm PCB standoffs remain unchanged because each insert reaches
the tray surface without penetrating the 2.8 mm tray. Print the fit coupon
before the complete tray; actual printed-hole fit still depends on filament
and printer calibration. The board uses four M3×6 pan-head screws through its
4.5 mm mounting holes.

The cover screw recess is 6.4 mm diameter and 1.3 mm deep. A typical M3 pan
head is approximately 6 mm diameter and 2.4 mm high, so it will stand about
1.1 mm proud. With 1.1 mm of cover material beneath the recess, the 6 mm screw
engages approximately 4.9 mm of the insert.

Recommended first prints:

1. Print `fit_coupon` and tune `m3_insert_pilot_d` and `cover_fit`.
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

## Alignment jig

`power_dist_alignment_jig.scad` registers against the top and both outside
edges of a nominal 4 × 8 inch electrical box. It centers the enclosure and
transfers the four mounting-ear holes. `enclosure_gap` controls the distance
from the top of the electrical box to the bottom of the enclosure; use 50.8 mm
for 2 inches or 76.2 mm for 3 inches.

Before printing, measure the actual box width and set `box_width`. Print the
jig flat. At the site, seat both side fingers against the box, place a small
level in the crossbar slot, and mark the four 3 mm locator holes. Remove the
jig before drilling the final pilot holes appropriate for the plywood and
mounting screws.

// Simplified visualization of Phoenix Contact 5442206 BC-508X10-2 GN.
// Envelope and pin locations: Phoenix specifications and BC-508X10 drawing.
// https://www.phoenixcontact.com/en-us/products/printed-circuit-board-terminal-bc-508x10-2-gn-5442206
// Body 10.16 x 8.10 x 10 mm; pin pitch 5.08; pin length 3.5 mm.
// Pin row centered in depth. Pin 1 at origin, pin 2 at +X.
// Wire entry is -Y in model coordinates (+Y in KiCad footprint coordinates).
// Port, screw, and clamp details are illustrative, not manufacturing geometry.
part = "all";
$fn = 40;
pitch = 5.08;
body_width = 10.16;
body_depth = 8.1;
body_height = 10;
module housing() {
    difference() {
        translate([-pitch/2, -body_depth/2, 0])
            cube([body_width, body_depth, body_height]);
        for (x = [0, pitch]) {
            translate([x-1.7, -body_depth/2-0.01, 1.6]) cube([3.4, 4.1, 3.4]);
            translate([x, 0, 7.5]) cylinder(h=2.6, r=1.75);
        }
    }
}
module metal() {
    for (x = [0, pitch]) {
        translate([x-0.5, -0.25, -3.5]) cube([1, 0.5, 3.6]);
        difference() {
            translate([x, 0, 7.5]) cylinder(h=2, r=1.5);
            translate([x-1.6, -0.3, 9]) cube([3.2, 0.6, 0.6]);
        }
        translate([x-1.5, -0.1, 1.8]) cube([3, 0.3, 2.8]);
    }
}
if (part == "all" || part == "housing") color([0.28, 0.58, 0.35]) housing();
if (part == "all" || part == "metal") color([0.7, 0.7, 0.72]) metal();

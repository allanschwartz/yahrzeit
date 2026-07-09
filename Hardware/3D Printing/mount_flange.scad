// Yahrzeit Embedded Controller
// Plywood mounting adapter for 65 mm wide enclosure
// Four-hole "+" mounting pattern
// Units: mm

$fn = 64;

// ---------- Enclosure is 65mm x 74mm, with 4 mounting threads ----------
box_w = 65;
screw_d = 3.0;          // clearance for M2.5 screw
screw_head_d = 6.0;
screw_head_depth = 1.5;
// Four enclosure mounting points
hole_spacing = 20;              // measured center-to-center
holes = [
    [-hole_spacing/2, 0],
    [ hole_spacing/2, 0],
    [0, -hole_spacing/2],
    [0,  hole_spacing/2]
];

// ---------- Adapter plate ----------
plate_w = box_w + 2 * 20;               // 20 mm flange each side
plate_h = 45;
plate_t = 3;

// ---------- Wood screw holes ----------
wood_hole_d = 4.0;
wood_head_d = 8.5;
wood_head_depth = 2.5;
wood_x = box_w/2 + 10;                // left/right screw spacing from center
wood_y = 0;

// ---------- Build ----------
adapter_plate();

module adapter_plate() {
    difference() {
        // main plate
        rounded_box([plate_w, plate_h, plate_t], 4);

        // M2.5 holes for enclosure
        // Enclosure mounting holes
        for (hole = holes) {

            // 3 mm clearance hole
            translate([hole[0], hole[1], -1])
                cylinder(d = screw_d, h = plate_t + 2);

            // Counterbore for screw head
            translate([hole[0], hole[1], plate_t - screw_head_depth])
                cylinder(d = screw_head_d, h = screw_head_depth + 1);
        }

        // wood screw holes in side flanges
        for (x = [-wood_x, wood_x]) {
            translate([x, wood_y, -1])
                cylinder(d = wood_hole_d, h = plate_t + 2);
        }
    }
}

module rounded_box(size, r) {
    x = size[0];
    y = size[1];
    z = size[2];

    hull() {
        for (ix = [-1, 1])
            for (iy = [-1, 1])
                translate([ix*(x/2-r), iy*(y/2-r), 0])
                    cylinder(r = r, h = z);
    }
}
// Centering and hole-transfer jig for the BTF-LIGHTING AM50-B05-HD supply
// in a nominal 4 x 8 inch metal box.
// Units: millimeters

inch = 25.4;

/* [Electrical box] */
// Origin is the arrowed lower-left corner when the jig is held vertically.
// X runs across the 4-inch dimension; Y runs up the 8-inch dimension.
box_nominal_w = 4 * inch;
box_nominal_h = 8 * inch;
edge_clearance = 0.1 * inch; // clearance on each of all four sides

/* [Power supply] */
// Use the supply exactly as held in the printed jig: no coordinate rotation.
supply_w = 3.236 * inch;
supply_h = 3.91 * inch;

// Hole locations measured from opposite corners of the supply outline.
lower_left_hole_offset = [0.24 * inch, 0.18 * inch];
upper_right_hole_offset = [0.24 * inch, 0.05 * inch];
drill_guide_d = 0.125 * inch;

/* [Jig] */
jig_t = 3.2;
outline_recess_depth = 0.8;
corner_r = 3.0;
label_depth = 0.4;

$fn = 48;

jig_w = box_nominal_w - 2 * edge_clearance;
jig_h = box_nominal_h - 2 * edge_clearance;
supply_x = (jig_w - supply_w)/2;
supply_y = (jig_h - supply_h)/2;

drill_points = [
    [supply_x + lower_left_hole_offset[0],
     supply_y + lower_left_hole_offset[1]],
    [supply_x + supply_w - upper_right_hole_offset[0],
     supply_y + supply_h - upper_right_hole_offset[1]]
];

module rounded_box(size, r) {
    hull()
        for (x = [r, size[0]-r])
            for (y = [r, size[1]-r])
                translate([x,y,0]) cylinder(h=size[2], r=r);
}

module power_supply_alignment_jig() {
    difference() {
        rounded_box([jig_w, jig_h, jig_t], corner_r);

        // The shallow pocket centers and outlines the supply without making
        // the full jig fragile. The supply's 3.91-inch dimension is vertical.
        translate([supply_x, supply_y, jig_t - outline_recess_depth])
            cube([supply_w, supply_h, outline_recess_depth + 0.1]);

        for (p = drill_points)
            translate([p[0], p[1], -0.1])
                cylinder(h=jig_t + 0.2, d=drill_guide_d);
    }

    translate([jig_w/2, 5, jig_t - 0.1])
        linear_extrude(label_depth)
            text("AM50-B05-HD  CENTERING JIG", size=4,
                 halign="center", valign="center");
}

power_supply_alignment_jig();

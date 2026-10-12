// Alignment and hole-transfer jig for the Yahrzeit power-distribution enclosure
// Units: millimeters
//
// Register the lower crossbar on the top edge of the 4 x 8 electrical box.
// The two downward fingers touch its left and right sides, centering the jig.
// Level the crossbar, then mark or drill the four enclosure mounting-ear holes.

/* [Installation] */
box_width = 203.2;          // nominal 8 inches; replace with measured width
enclosure_gap = 50.8;      // bottom of enclosure above box: 50.8 or 76.2
enclosure_outer_w = 156.5;
enclosure_outer_h = 66.5;
ear_w = 13;
ear_center_y = 12;
mount_hole_dx = enclosure_outer_w + ear_w;
mount_hole_dy = enclosure_outer_h - 2 * ear_center_y;

/* [Jig] */
jig_t = 3.0;
crossbar_h = 16;
side_stop_w = 8;
side_stop_drop = 18;
spine_w = 24;
locator_pad_d = 13;
mark_hole_d = 3.0;
level_vial_slot_w = 45;
level_vial_slot_h = 8;
label_depth = 0.5;

$fn = 48;

// y=0 is the top edge of the electrical box and the lower edge of the
// crossbar. Therefore enclosure_gap is the true box-to-enclosure clearance.
enclosure_center_y = enclosure_gap + enclosure_outer_h/2;
hole_x = [box_width/2 - mount_hole_dx/2,
          box_width/2 + mount_hole_dx/2];
hole_y = [enclosure_center_y - mount_hole_dy/2,
          enclosure_center_y + mount_hole_dy/2];

module rounded_rect_2d(size, r=2) {
    hull()
        for (x = [r, size[0]-r])
            for (y = [r, size[1]-r])
                translate([x,y]) circle(r=r);
}

module jig_body_2d() {
    union() {
        rounded_rect_2d([box_width, crossbar_h], 2);

        translate([box_width/2-spine_w/2, 0])
            rounded_rect_2d([spine_w, enclosure_center_y + mount_hole_dy/2], 2);

        for (x = hole_x)
            for (y = hole_y) {
                hull() {
                    translate([box_width/2, y]) circle(d=spine_w);
                    translate([x, y]) circle(d=locator_pad_d);
                }
            }

        // Fingers register against the outside edges of the box.
        translate([-side_stop_w, -side_stop_drop])
            square([side_stop_w, side_stop_drop + crossbar_h]);
        translate([box_width, -side_stop_drop])
            square([side_stop_w, side_stop_drop + crossbar_h]);
    }
}

module alignment_jig() {
    difference() {
        linear_extrude(jig_t) jig_body_2d();

        for (x = hole_x)
            for (y = hole_y)
                translate([x, y, -0.1])
                    cylinder(h=jig_t+0.2, d=mark_hole_d);

        // Slot accepts a small spirit level or provides a visual horizontal line.
        translate([box_width/2-level_vial_slot_w/2,
                   (crossbar_h-level_vial_slot_h)/2,
                   -0.1])
            cube([level_vial_slot_w, level_vial_slot_h, jig_t+0.2]);
    }

    translate([box_width/2, crossbar_h + 8, jig_t])
        linear_extrude(label_depth)
            text(str("GAP ", enclosure_gap, " mm"), size=5,
                 halign="center", valign="center");
}

alignment_jig();

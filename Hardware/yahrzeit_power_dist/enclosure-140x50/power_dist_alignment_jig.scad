// Alignment and hole-transfer jig for the Yahrzeit power-distribution enclosure
// Units: millimeters
//
// The upper portion duplicates the enclosure tray's wall-facing outline and
// mounting-ear holes, but omits every PCB and cover standoff. The solid lower
// panel is aligned visually with the centerline and top edge of the 4 x 8 box.

/* [Installation] */
enclosure_gap = 25.4;      // 25.4 = 1 inch; 50.8 = 2 inches; 76.2 = 3 inches

/* [Enclosure tray geometry] */
board_w = 140;
board_h = 50;
board_clearance = 8.25;
tray_w = board_w + 2 * board_clearance;
tray_h = board_h + 2 * board_clearance;
tray_t = 2.8;
corner_r = 3.0;
ear_w = 13.0;
ear_h = 18.0;
ear_t = 3.2;
ear_center_y = 12.0;
ear_hole_d = 4.2;

/* [Jig] */
alignment_panel_w = 101.6; // 4 inches: half the nominal box width
mark_hole_d = 3.0;
label_depth = 0.5;

$fn = 48;

wall_mount_holes = [
    [-ear_w / 2, ear_center_y],
    [tray_w + ear_w / 2, ear_center_y],
    [-ear_w / 2, tray_h - ear_center_y],
    [tray_w + ear_w / 2, tray_h - ear_center_y]
];

// The following rounded_box() and mounting_ear() modules are copied from
// power_dist_enclosure.scad. Keep this block identical to the enclosure so
// the jig's tray-shaped portion cannot acquire subtly different geometry.
module rounded_box(size, r) {
    hull() {
        for (x = [r, size[0] - r])
            for (y = [r, size[1] - r])
                translate([x, y, 0]) cylinder(h=size[2], r=r);
    }
}

module mounting_ear(x, y) {
    // The 0.3 mm overlap with the tray avoids a merely tangent joint.
    translate([x - ear_w/2 - (x < 0 ? 0 : 0.3), y - ear_h/2, 0])
        difference() {
            rounded_box([ear_w + 0.3, ear_h, ear_t], 2.0);
            translate([ear_w/2 + (x < 0 ? 0 : 0.3), ear_h/2, -0.1])
                cylinder(h=ear_t + 0.2, d=ear_hole_d);
        }
}

module tray_mounting_surface() {
    union() {
        rounded_box([tray_w, tray_h, tray_t], corner_r);
        for (p = wall_mount_holes) {
            mounting_ear(p[0], p[1]);

            // Refill the tray's 4.2 mm clearance hole here; alignment_jig()
            // replaces it with a smaller 3 mm pilot-hole transfer guide.
            translate([p[0], p[1], 0])
                cylinder(h=ear_t, d=ear_hole_d);
        }
    }
}

module jig_body() {
    union() {
        tray_mounting_surface();

        // A durable, solid 4-inch-wide panel extends from the enclosure down
        // to the electrical-box top edge. Center it visually on the 8-inch
        // box; its upper edge and the tray's lower edge coincide.
        translate([(tray_w - alignment_panel_w)/2, -enclosure_gap])
            rounded_box([alignment_panel_w, enclosure_gap + 2, ear_t], 2.0);
    }
}

module alignment_jig() {
    difference() {
        jig_body();

        // Smaller transfer holes locate the centers of the final wall holes.
        for (p = wall_mount_holes)
            translate([p[0], p[1], -0.1])
                cylinder(h=ear_t + 0.2, d=mark_hole_d);
    }

    translate([tray_w/2, -enclosure_gap/2, ear_t - 0.1])
        linear_extrude(label_depth)
            text(str(enclosure_gap/25.4, " IN GAP"), size=5,
                 halign="center", valign="center");
}

alignment_jig();

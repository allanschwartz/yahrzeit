// Yahrzeit power-distribution board wall enclosure
// Units: millimeters
//
// Print the tray with its wall-facing surface on the bed. Print the cover
// front-face-down. The cover lifts straight away from the wall without
// disconnecting field wiring.

/* [Part] */
part = "pcb_mockup"; // [tray,cover,assembly,pcb_mockup,fit_coupon]

/* [PCB] */
board_w = 140;
board_h = 50;
board_t = 2.0;
board_hole_inset = 7.5;
board_hole_d = 4.5;

/* [Enclosure] */
wall = 2.4;
// 8.25 mm keeps the strengthened cover posts outside the PCB outline.
board_clearance = 8.25;
tray_w = board_w + 2 * board_clearance;
tray_h = board_h + 2 * board_clearance;
tray_t = 2.8;
max_component_h = 17.0;    // F1 holder plus inserted MINI fuse, provisional
component_clearance = 3.0;
inside_depth = max_component_h + component_clearance;
cover_front_t = 2.4;
cover_fit = 0.35;          // radial clearance around tray
corner_r = 3.0;

/* [PCB mounting] */
pcb_standoff_h = 6.0;
pcb_standoff_d = 8.0;
pcb_support_rib_w = 1.2;
pcb_support_gap = 0.15;    // ribs sit just below a normally seated PCB
m3_insert_od = 5.0;
m3_insert_length = 6.0;
m3_insert_pilot_d = 3.93;  // Zwmssl M3x6x5 package specification
m3_insert_pocket_depth = 6.15;
m3_clearance_d = 3.4;

/* [Cover mounting] */
cover_post_d = 8.0;
cover_post_edge = 4.0;
cover_screw_recess_d = 6.4;
cover_screw_recess_depth = 1.3;

/* [Wall mounting] */
ear_w = 13.0;
ear_h = 18.0;
ear_t = 3.2;
ear_hole_d = 4.2;
ear_center_y = 12.0;
mount_hole_dx = tray_w + ear_w;
mount_hole_dy = tray_h - 2 * ear_center_y;

/* [Cable access] */
output_slot_x0 = board_clearance + 10;
output_slot_w = board_w - 20;
output_slot_depth = 15.0;
input_slot_center_x = board_clearance + 58.5;
input_slot_w = 18.0;
input_slot_depth = 15.0;

/* [Visibility and ventilation] */
led_board_x = 121;
led_board_y_kicad = 39.2625;
// KiCad has y=0 at the board top; this model has y=0 at the bottom.
led_board_y = board_h - led_board_y_kicad;
led_window_d = 5.0;
mock_led_w = 1.25;         // rotated 0805 package footprint
mock_led_l = 2.0;
mock_led_h = 3.0;          // intentionally taller than the real LED
lightening_slot_w = 12;
lightening_slot_h = 5;
lightening_pitch_x = 16;
lightening_pitch_y = 11;

$fn = 48;

board_x = board_clearance;
board_y = board_clearance;
pcb_bottom_z = tray_t + pcb_standoff_h;
pcb_top_z = pcb_bottom_z + board_t;
cover_depth = pcb_top_z + inside_depth + cover_front_t;

// Convert a KiCad board Y coordinate to this model's bottom-up board Y.
function board_y_from_kicad(kicad_y) = board_h - kicad_y;

pcb_holes = [
    [board_x + board_hole_inset,           board_y + board_hole_inset],
    [board_x + board_w - board_hole_inset, board_y + board_hole_inset],
    [board_x + board_hole_inset,           board_y + board_h - board_hole_inset],
    [board_x + board_w - board_hole_inset, board_y + board_h - board_hole_inset]
];

cover_posts = [
    [cover_post_edge, cover_post_edge],
    [tray_w - cover_post_edge, cover_post_edge],
    [cover_post_edge, tray_h - cover_post_edge],
    [tray_w - cover_post_edge, tray_h - cover_post_edge]
];

wall_mount_holes = [
    [-ear_w / 2, ear_center_y],
    [tray_w + ear_w / 2, ear_center_y],
    [-ear_w / 2, tray_h - ear_center_y],
    [tray_w + ear_w / 2, tray_h - ear_center_y]
];

module rounded_box(size, r) {
    hull() {
        for (x = [r, size[0] - r])
            for (y = [r, size[1] - r])
                translate([x, y, 0]) cylinder(h=size[2], r=r);
    }
}

module insert_standoff(x, y, h, d=pcb_standoff_d) {
    // Sink slightly into the tray to guarantee one printable solid.
    translate([x, y, tray_t - 0.2])
        difference() {
            cylinder(h=h + 0.2, d=d);
            // The 6 mm insert reaches the tray surface in a 6 mm PCB
            // standoff. The tray itself remains as the closed bottom.
            translate([0, 0, h + 0.2 - m3_insert_pocket_depth])
                cylinder(h=m3_insert_pocket_depth + 0.2,
                         d=m3_insert_pilot_d);
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

module pcb_support_ribs() {
    rib_h = pcb_standoff_h + 0.1 - pcb_support_gap;

    // J2-J7: centered on KiCad y=11.5 and running between the upper
    // mounting standoffs. This misses the terminal pins at KiCad y=9.
    translate([board_x + board_hole_inset,
               board_y + board_y_from_kicad(11.5) - pcb_support_rib_w/2,
               tray_t - 0.1])
        cube([board_w - 2*board_hole_inset,
              pcb_support_rib_w,
              rib_h]);

    // J1: local support beneath the connector body, clear of its pin row.
    translate([board_x + 53,
               board_y + board_y_from_kicad(35.5) - pcb_support_rib_w/2,
               tray_t - 0.1])
        cube([11, pcb_support_rib_w, rib_h]);
}

module tray() {
    difference() {
        union() {
            rounded_box([tray_w, tray_h, tray_t], corner_r);

            for (p = wall_mount_holes)
                mounting_ear(p[0], p[1]);

            for (p = pcb_holes)
                insert_standoff(p[0], p[1], pcb_standoff_h);

            for (p = cover_posts)
                insert_standoff(p[0], p[1], pcb_top_z + inside_depth - tray_t,
                                 cover_post_d);

            pcb_support_ribs();
        }

        // Wall-screw access remains clear through the tray edge.
        for (p = wall_mount_holes)
            translate([p[0], p[1], -0.1])
                cylinder(h=ear_t + 0.2, d=ear_hole_d);
    }
}

module cover_shell() {
    difference() {
        rounded_box([tray_w + 2*(wall + cover_fit),
                     tray_h + 2*(wall + cover_fit),
                     cover_depth], corner_r + wall);

        // Open rear cavity; the cover fits around, rather than onto, the tray.
        translate([wall, wall, -0.1])
            rounded_box([tray_w + 2*cover_fit,
                         tray_h + 2*cover_fit,
                         cover_depth - cover_front_t + 0.1], corner_r);

        // Output wires leave at the board's upper edge. Opening reaches the
        // rear edge so the cover can be removed around connected wires.
        translate([wall + cover_fit + output_slot_x0,
                   tray_h + wall + cover_fit - 0.1,
                   -0.1])
            cube([output_slot_w, wall + 1, output_slot_depth + 0.1]);

        // Input wire opening below J1.
        translate([wall + cover_fit + input_slot_center_x - input_slot_w/2,
                   -0.1,
                   -0.1])
            cube([input_slot_w, wall + 1, input_slot_depth + 0.1]);

        // Front-facing M3 cover screws.
        for (p = cover_posts) {
            translate([wall + cover_fit + p[0],
                       wall + cover_fit + p[1],
                       cover_depth - cover_front_t - 0.1])
                cylinder(h=cover_front_t + 0.2, d=m3_clearance_d);
            translate([wall + cover_fit + p[0],
                       wall + cover_fit + p[1],
                       cover_depth - cover_screw_recess_depth])
                cylinder(h=cover_screw_recess_depth + 0.1,
                         d=cover_screw_recess_d);
        }

        // LED viewing hole.
        translate([wall + cover_fit + board_x + led_board_x,
                   wall + cover_fit + board_y + led_board_y,
                   cover_depth - cover_front_t - 0.1])
            cylinder(h=cover_front_t + 0.2, d=led_window_d);

        // Broad lightening pattern. Preserve solid material around LED1.
        for (x = [18 : lightening_pitch_x : tray_w - 18])
            for (y = [17 : lightening_pitch_y : tray_h - 15])
                if (sqrt(pow(x - (board_x + led_board_x), 2) +
                         pow(y - (board_y + led_board_y), 2)) > 11)
                    hull()
                        for (sx = [-1, 1])
                            translate([wall + cover_fit + x +
                                       sx*(lightening_slot_w-lightening_slot_h)/2,
                                       wall + cover_fit + y,
                                       cover_depth - cover_front_t - 0.1])
                                cylinder(h=cover_front_t + 0.2,
                                         d=lightening_slot_h, $fn=24);
    }
}

module printable_pcb_mockup() {
    union() {
        color("seagreen", 0.8)
            difference() {
                cube([board_w, board_h, board_t]);
                for (x = [board_hole_inset, board_w-board_hole_inset])
                    for (y = [board_hole_inset, board_h-board_hole_inset])
                        translate([x, y, -0.1])
                            cylinder(h=board_t+0.2, d=board_hole_d);
            }

        // This occupies the actual rotated 0805 package area. The extra
        // height makes LED1 easy to see through the cover window.
        color("limegreen")
            translate([led_board_x - mock_led_w/2,
                       led_board_y - mock_led_l/2,
                       board_t - 0.2])
                cube([mock_led_w, mock_led_l, mock_led_h + 0.2]);

        // J2-J7 terminal blocks. Footprint origins are at x=23..123,
        // KiCad y=9, with each footprint rotated 180 degrees.
        color("forestgreen")
            for (x = [23 : 20 : 123])
                translate([x - 7.62, 36.95, board_t - 0.2])
                    cube([10.16, 8.1, 10.2]);

        // J1 terminal block at KiCad (55.92, 38).
        color("forestgreen")
            translate([53.38, 7.95, board_t - 0.2])
                cube([10.16, 8.1, 10.2]);

        // C1: 10 mm diameter by 12.5 mm high, centered at the measured
        // KiCad coordinate (96.7, 34.7).
        color("navy")
            translate([96.7, board_y_from_kicad(34.7), board_t - 0.2])
                cylinder(h=12.7, d=10);

        // F1 holder with inserted MINI fuse. Measured KiCad bounds:
        // x=76.5..83.4, y=26.9..43.2.
        color("dimgray")
            translate([76.5, board_y_from_kicad(43.2), board_t - 0.2])
                cube([83.4-76.5, 43.2-26.9, max_component_h + 0.2]);

        // F2-F7 resettable PTCs, rotated 1812 packages.
        color("goldenrod")
            for (x = [23 : 20 : 123])
                translate([x - 1.6, 30 - 2.25, board_t - 0.2])
                    cube([3.2, 4.5, 1.4]);

        // C2, C3, and R1. Heights are conservative printable envelopes.
        color("sienna")
            translate([107 - 1.25, 15.5 - 1.6, board_t - 0.2])
                cube([2.5, 3.2, 2.7]);
        color("sienna")
            translate([114 - 0.625, 15.5 - 1.0, board_t - 0.2])
                cube([1.25, 2.0, 1.2]);
        color("sienna")
            translate([121 - 0.625, 15.2375 - 1.0, board_t - 0.2])
                cube([1.25, 2.0, 1.2]);
    }
}

module board_mockup() {
    translate([board_x, board_y, pcb_bottom_z])
        printable_pcb_mockup();
}

module fit_coupon() {
    // Small print for tuning the insert pilot and cover clearance.
    difference() {
        cube([32, 18, 9]);
        translate([8, 9, 9 - m3_insert_pocket_depth])
            cylinder(h=m3_insert_pocket_depth + 0.2,
                     d=m3_insert_pilot_d);
        translate([23, -0.1, 2]) cube([5 + 2*cover_fit, 8, 7.1]);
    }
}

if (part == "tray") tray();
if (part == "cover") cover_shell();
if (part == "pcb_mockup") printable_pcb_mockup();
if (part == "fit_coupon") fit_coupon();
if (part == "assembly") {
    color("gainsboro") tray();
    board_mockup();
    color([0.80, 0.85, 0.90, 0.35])
        translate([-wall-cover_fit, -wall-cover_fit, 0]) cover_shell();
}

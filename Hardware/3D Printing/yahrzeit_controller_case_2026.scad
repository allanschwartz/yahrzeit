// Arduino Uno R4 + Ethernet shield + small shield case
// Intended for a wall-mounted enclosure with keyholes and M3 brass inserts
//
// Coordinate convention:
// - Origin at the bottom of the Uno board (zero plane)
// - +Z is upward
// - "power side" is the side with barrel jack, USB-C power, and reset button
//
// This is a starting parametric model. Dimensions are chosen conservatively
// and can be refined after measuring the real hardware.

/* [Board stack] */
uno_length = 68.6;
uno_width = 53.4;
pcb_thickness = 1.6;

/* [General] */
case_wall_thickness = 2.2;
case_bottom_thickness = 2.2;
case_top_thickness = 2.2;
// Free space between the board stack footprint and the inside wall.
// 5 mm leaves room for M3 cover-insert posts merged into the case corners.
board_side_clearance = 5.0;
case_length = uno_length + 2 * board_side_clearance + 2 * case_wall_thickness;
case_width = uno_width + 2 * board_side_clearance + 2 * case_wall_thickness;


ethernet_pcb_bottom_above_uno_bottom = 17.0;
pixel_if_pcb_bottom_above_ethernet_bottom = 15.0;
top_clearance_above_stack = 8.0;

/* [Mounting hardware] */
standoff_od = 7.0;
insert_hole_d = 4.2;     // pilot hole for M3 heat-set insert; copied from fixture_plate.scad
standoff_height = 6.0;

/* [Wall mounting] */
flange_width = 12.0;
flange_corner_radius = 2;
flange_thickness = case_bottom_thickness + 1.0;
flange_length = case_length / 2;
keyhole_slot_width = 3.5;
keyhole_slot_length = 14.0;
keyhole_head_diam = 6.0;
keyhole_center_x = case_length / 2;

/* [front panel] */
front_panel_width = 42;
front_panel_height = 30;
front_panel_y0 = 13;
front_panel_z0 = case_bottom_thickness + standoff_height + pcb_thickness;

/* [rear panels] */
rear_panel_width = 21;
rear_panel_height = 8;
rear_panel_y0 = 20;
rear_panel_z0 = 39;

/* [Ventilation] */
vent_slot_width = 3.2;
vent_slot_height = 11.0;
vent_pitch_x = 9.0;
vent_pitch_z = 14.0;
vent_margin_x = 10.0;
vent_margin_z = 7.0;

/* [Cover attachment] */
cover_post_od = 7.0;
cover_post_wall_overlap = 2.0;
cover_screw_hole_diam = 3.4;
cover_screw_recess_diam = 6.0;
cover_screw_recess_depth = 1.2;

/* [Derived] */

uno_bottom_z = case_bottom_thickness + standoff_height;
ethernet_bottom_z = uno_bottom_z + ethernet_pcb_bottom_above_uno_bottom;
pixel_if_bottom_z = ethernet_bottom_z + pixel_if_pcb_bottom_above_ethernet_bottom;
stack_top_z = pixel_if_bottom_z + pcb_thickness;
case_height = max(stack_top_z + top_clearance_above_stack, 30);
cover_post_z0 = 0;
cover_post_height = case_height;
board_origin_x = case_wall_thickness + board_side_clearance;
board_origin_y = case_wall_thickness + board_side_clearance;
cover_post_offset = case_wall_thickness + cover_post_od / 2 - cover_post_wall_overlap;


// Arduino Uno R4 Minima mounting hole coordinates from ABX00080 mechanical drawing.
// Origin is lower-left of the board outline.
arduino_mount_holes = [
    [13.97,  2.54],
    [66.04,  7.62],
    [66.04, 35.56],
    [15.24, 50.80]
];

cover_screw_points = [
    [cover_post_offset, cover_post_offset],
    [case_length - cover_post_offset, cover_post_offset],
    [cover_post_offset, case_width - cover_post_offset],
    [case_length - cover_post_offset, case_width - cover_post_offset]
];

module rounded_rect(size, r) {
    hull() {
        translate([r, r, 0]) cylinder(h=size[2], r=r, $fn=48);
        translate([size[0]-r, r, 0]) cylinder(h=size[2], r=r, $fn=48);
        translate([r, size[1]-r, 0]) cylinder(h=size[2], r=r, $fn=48);
        translate([size[0]-r, size[1]-r, 0]) cylinder(h=size[2], r=r, $fn=48);
    }
}

module standoff(x, y, z0) {
    translate([x, y, z0]) {
        difference() {
            cylinder(h = standoff_height, d = standoff_od, $fn=48);
            translate([0,0,-0.1]) cylinder(h = standoff_height + 0.2, d = insert_hole_d, $fn=48);
        }
    }
}

module cover_post(x, y) {
    translate([x, y, cover_post_z0])
        cylinder(h = cover_post_height, d = cover_post_od, $fn=48);
}

module cover_insert_hole(x, y) {
    translate([x, y, case_height - standoff_height - 0.1])
        cylinder(h = standoff_height + 0.2, d = insert_hole_d, $fn=48);
}

module flange(y0) {
    translate([(case_length - flange_length) / 2, y0, 0])
        rounded_rect([flange_length, flange_width, flange_thickness], r=flange_corner_radius);
}

module keyhole_cutout(x, y) {
    translate([x, y, -1]) {
        hull() {
            translate([-keyhole_slot_length / 2, 0, 0])
                cylinder(h = flange_thickness + 2, d = keyhole_slot_width, $fn=48);
            translate([keyhole_slot_length / 2, 0, 0])
                cylinder(h = flange_thickness + 2, d = keyhole_slot_width, $fn=48);
        }

        cylinder(h = flange_thickness + 2, d = keyhole_head_diam, $fn=48);
    }
}

module side_vent_pattern() {
    for (x = [vent_margin_x : vent_pitch_x : case_length - vent_margin_x]) {
        for (z = [vent_margin_z + vent_slot_height / 2
                : vent_pitch_z
                : case_height - vent_margin_z - vent_slot_height / 2]) {
            // South/north wall ventilation. Each cutout is a rounded vertical
            // slot extruded through the wall in Y.
            vent_slot_y_wall(x, case_wall_thickness + 1, z);
            vent_slot_y_wall(x, case_width + 1, z);
        }
    }
}

module vent_slot_y_wall(x, y, z) {
    hull() {
        translate([x, y, z - vent_slot_height / 2 + vent_slot_width / 2])
            rotate([90, 0, 0])
                cylinder(h = case_wall_thickness + 2, d = vent_slot_width, $fn=24);
        translate([x, y, z + vent_slot_height / 2 - vent_slot_width / 2])
            rotate([90, 0, 0])
                cylinder(h = case_wall_thickness + 2, d = vent_slot_width, $fn=24);
    }
}

// The front panel is for the power and Ethernet connectors
module front_panel_opening() {
    translate([-1,
               front_panel_y0,
               front_panel_z0])
        cube([case_wall_thickness + 2,
              front_panel_width,
              front_panel_height]);
}

// the rear panel is for the data cable.
module rear_panel_opening() {
    translate([case_length - case_wall_thickness - 1,
               rear_panel_y0,
               rear_panel_z0])
        cube([case_wall_thickness + 2,
              rear_panel_width,
              rear_panel_height]);
}

module main_case() {
    difference() {
        union() {
            difference() {
                union() {
                    // Outer shell
                    rounded_rect([case_length, case_width, case_height], r=3);

                    // Opposing wall-mount flanges, on the north/south sides.
                    flange(-flange_width);
                    flange(case_width);
                }

                // Hollow interior
                translate([case_wall_thickness, case_wall_thickness, case_bottom_thickness])
                    rounded_rect([case_length - 2*case_wall_thickness, case_width - 2*case_wall_thickness, case_height], r=2);

                // panel openings
                front_panel_opening();
                rear_panel_opening();

                // Ventilation holes on the side walls
                side_vent_pattern();

                // Keyhole openings in the north/south flanges.
                for (y = [-flange_width / 2, case_width + flange_width / 2]) {
                    keyhole_cutout(keyhole_center_x, y);
                }
            }

            // Cover screw posts, merged into the upper half of the case.
            for (p = cover_screw_points) {
                cover_post(p[0], p[1]);
            }
        }

        // Drill the insert holes after the posts are merged into the case body.
        for (p = cover_screw_points) {
            cover_insert_hole(p[0], p[1]);
        }

    }

    // Standoffs inside the bottom shell, aligned to the Arduino Uno holes.
    for (p = arduino_mount_holes) {
        standoff(board_origin_x + p[0],
                 board_origin_y + p[1],
                 case_bottom_thickness);
    }
}

module top_cover() {
    difference() {
        rounded_rect([case_length, case_width, case_top_thickness], r=3);

        // Four mounting holes for M3 screws into the cover-post inserts.
        for (p = cover_screw_points) {
            translate([p[0], p[1], -1]) cylinder(h = case_top_thickness + 2, d = cover_screw_hole_diam, $fn=24);
            translate([p[0], p[1], case_top_thickness - cover_screw_recess_depth]) cylinder(h = cover_screw_recess_depth + 1, d = cover_screw_recess_diam, $fn=24);
        }
    }
}

main_case();
// Separate, flat-printable cover preview.
translate([0, case_width + flange_width + 12, 0])
    top_cover();

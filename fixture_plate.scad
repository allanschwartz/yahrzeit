// Yahrzeit Wall Test Fixture Plate
// Units: mm

// -----------------------------------------------------------------------------
// Global parameters
// -----------------------------------------------------------------------------

plate_w = 240;
plate_thickness = 4.0;

edge_gap = 20;
row_gap = 20;

platform_h = 1.0;
standoff_h = 5.0;

standoff_od = 7;
insert_hole_d = 4;     // pilot hole for M3 heat-set insert; verify for your insert
standoff_fn = 48;

legend_h = 0.6;
legend_size = 5;
legend_font = "Liberation Sans:style=Bold";
legend_gap = 4;        // gap between board/platform and label
legend_band = 10;      // vertical space reserved below each board row

// Visual colors for OpenSCAD preview.
// For actual tri-color printing, export/separate bodies or use slicer color changes.
base_color = "gray";
platform_color = "green";
standoff_color = "white";

// -----------------------------------------------------------------------------
// Board dimensions
// -----------------------------------------------------------------------------

yz_w = 100;
yz_h = 100;

raspi_w = 85.6;
raspi_h = 56.5;

arduino_w = 68.6;
arduino_h = 53.4;

// Horizontal spacing
top_pair_gap = 45;

yz_side_gap = 10;
yz_mid_gap = 20;

// Derived bottom row positions
yz1_x = yz_side_gap;
yz2_x = yz1_x + yz_w + yz_mid_gap;
yz_y  = edge_gap + legend_band;

// Derived top row positions
top_pair_w = arduino_w + top_pair_gap + raspi_w;
top_x = (plate_w - top_pair_w) / 2;

arduino_x = top_x;
raspi_x   = top_x + arduino_w + top_pair_gap;

top_row_h = max(arduino_h, raspi_h);
top_y = yz_y + yz_h + row_gap + legend_band;

// Plate height
plate_h = top_y + top_row_h + edge_gap;


// -----------------------------------------------------------------------------
// Basic parts
// -----------------------------------------------------------------------------

module plate()
{
    color(base_color)
        cube([plate_w, plate_h, plate_thickness]);
}

module platform(x, y, w, h)
{
    color(platform_color)
        translate([x, y, plate_thickness])
            cube([w, h, platform_h]);
}

module standoff_at(x, y)
{
    color(standoff_color)
        translate([x, y, plate_thickness])
            difference() {
                // Standoff extends from the base surface,
                // through the 1 mm platform, and 5 mm above it.
                cylinder(h=platform_h + standoff_h,
                         d=standoff_od,
                         $fn=standoff_fn);

                translate([0, 0, -0.1])
                    cylinder(h=platform_h + standoff_h + 0.2,
                             d=insert_hole_d,
                             $fn=standoff_fn);
            }
}

module raised_label(x, y, label)
{
    color(platform_color)
        translate([x, y, plate_thickness])
            linear_extrude(height=legend_h)
                text(label,
                     size=legend_size,
                     font=legend_font,
                     halign="center",
                     valign="center");
}

module board_platform(x, y, w, h, label, hole_xy)
{
    platform(x, y, w, h);

    for (p = hole_xy) {
        standoff_at(x + p[0], y + p[1]);
    }

    // Label centered below the board footprint.
    raised_label(x + w / 2, y - legend_gap, label);
}


// -----------------------------------------------------------------------------
// Board-specific platforms
// -----------------------------------------------------------------------------

module yz_test_fixture_platform(x, y)
{
    // Yahrzeit Wall TEst Fixture: 100 x 100 mm, holes 3 mm from each corner.
    yz_holes = [
        [3,        3],
        [yz_w - 3, 3],
        [yz_w - 3, yz_h - 3],
        [3,        yz_h - 3]
    ];

    board_platform(x, y, yz_w, yz_h, "YZ TEST FIXTURE", yz_holes);
}

module raspi_board_platform(x, y)
{
    // Raspberry Pi full-size mounting holes:
    // 85.6 x 56.5 board, holes inset 3.5 mm from left/right and bottom/top.
    // Hole rectangle is therefore 58 x 49 mm.
    raspi_holes = [
        [3.5,            3.5],
        [3.5 + 58,       3.5],
        [3.5 + 58,       3.5 + 49],
        [3.5,            3.5 + 49]
    ];

    board_platform(x, y, raspi_w, raspi_h, "RASPBERRY PI", raspi_holes);
}

module arduino_board_platform(x, y)
{
    // Arduino Uno R3-ish mounting hole coordinates.
    // Origin is lower-left of the board outline in this fixture coordinate system.
    arduino_holes = [
        [14.0,  2.5],
        [66.1,  7.6],
        [66.1, 35.5],
        [15.3, 50.8]
    ];

    board_platform(x, y, arduino_w, arduino_h, "ARDUINO", arduino_holes);
}


// -----------------------------------------------------------------------------
// Assembly
// -----------------------------------------------------------------------------

plate();

arduino_board_platform(arduino_x, top_y);
raspi_board_platform(raspi_x, top_y);

yz_test_fixture_platform(yz1_x, yz_y);
yz_test_fixture_platform(yz2_x, yz_y);
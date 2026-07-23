# Yahrzeit Wall Hardware

This directory contains the electrical and mechanical design files for the
Yahrzeit Wall controller, interface board, pixel boards, test fixtures, and
enclosures.

The current controller uses an Arduino Uno R4, an Arduino-compatible Ethernet
shield, and the Yahrzeit Controller Pixel Interface V3 board. The interface
board connects the controller stack to the installed chain of YYZ Pixel boards.

<a href="pixel_interface_v3/pixel_interface_v3-3D%20viewer.png">
  <img src="pixel_interface_v3/pixel_interface_v3-3D%20viewer.png"
       alt="Yahrzeit Controller Pixel Interface V3 board" width="500">
</a>

*KiCad rendering of the Yahrzeit Controller Pixel Interface V3 board.*

## 3D-Printed Parts

The files under [`3D Printing`](3D%20Printing/) may be browsed
online, with GitHub displaying the JPEG photographs and interactive previews of
the STL models.

### Production Mounting Flange

The production controller uses a commercial enclosure with a custom
3D-printed flange that adapts its DIN-rail mounting feature for flat wall
mounting.

- [mount\_flange.scad](3D%20Printing/mount_flange.scad) -- Parametric OpenSCAD
  source.
- [mount\_flange.stl](3D%20Printing/mount_flange.stl) -- Printable mounting
  flange.

### Alternative Controller Enclosure

This enclosure was designed for the three-board controller stack. The
close-fitting commercial enclosure documented in the embedded-controller
installation notes is the preferred production enclosure, but this printable
design is retained as a replacement option and design reference.

- [yahrzeit\_controller\_case.scad](3D%20Printing/yahrzeit_controller_case.scad)
  -- Parametric OpenSCAD source.
- [yahrzeit\_controller\_case.stl](3D%20Printing/yahrzeit_controller_case.stl)
  -- Printable enclosure body.
- [yahrzeit\_controller\_case-cover.stl](3D%20Printing/yahrzeit_controller_case-cover.stl)
  -- enclosure cover.
- [yahrzeit\_controller\_case.jpg](3D%20Printing/yahrzeit_controller_case.jpg)
  -- Photograph of the printed enclosure body.

### Test-Fixture Plate

The fixture plate supports the small YYZ Pixel test assembly used during
controller development and bench testing.

- [fixture\_plate.scad](3D%20Printing/fixture_plate.scad) -- Parametric OpenSCAD
  source.
- [fixture\_plate.stl](3D%20Printing/fixture_plate.stl) -- Printable fixture
  plate.
- [fixture\_plate.jpg](3D%20Printing/fixture_plate.jpg) -- Photograph of the
  printed fixture plate.

## Yahrzeit Controller Pixel Interface V3

The current interface board adapts the Arduino controller stack to the
five-signal YYZ Pixel ribbon cable and provides the green ALIVE status LED.

- [pixel\_interface\_v3.kicad\_pro](pixel_interface_v3/pixel_interface_v3.kicad_pro)
  -- Open this project file in KiCad.
- [pixel\_interface\_v3.kicad\_sch](pixel_interface_v3/pixel_interface_v3.kicad_sch)
  -- KiCad schematic.
- [pixel\_interface\_v3.kicad\_pcb](pixel_interface_v3/pixel_interface_v3.kicad_pcb)
  -- KiCad PCB layout.
- [pixel\_interface\_v3-3D viewer.png](pixel_interface_v3/pixel_interface_v3-3D%20viewer.png)
  -- KiCad 3D rendering.
- [pixel\_interface\_v3-schematic.png](pixel_interface_v3/pixel_interface_v3-schematic.png)
  -- Schematic preview.
- [pixel\_interface\_v3\_D1\_polarity\_note.pdf](pixel_interface_v3/pixel_interface_v3_D1_polarity_note.pdf)
  -- Status-LED polarity and assembly note.

## Nine-Section YYZ Pixel Board

<a href="yahrzeit_pixel9/yahrzeit_pixel9.png">
  <img src="yahrzeit_pixel9/yahrzeit_pixel9.png"
       alt="Nine-section YYZ Pixel board" width="500">
</a>

*KiCad rendering of the nine-section YYZ Pixel board.*

The `yahrzeit_pixel9` board combines nine complete YYZ Pixel circuits on one
PCB. It uses the same octal bus transceiver and 8-bit shift register as the
individual boards installed in the wall.

- [yahrzeit\_pixel9.kicad\_pro](yahrzeit_pixel9/yahrzeit_pixel9.kicad_pro) --
  Open this project file in KiCad.
- [yahrzeit\_pixel9.kicad\_sch](yahrzeit_pixel9/yahrzeit_pixel9.kicad_sch) --
  KiCad top-level schematic.
- [yahrzeit\_pixel9.kicad\_pcb](yahrzeit_pixel9/yahrzeit_pixel9.kicad_pcb) --
  KiCad PCB layout.
- [yahrzeit\_pixel9-schematic.pdf](yahrzeit_pixel9/yahrzeit_pixel9-schematic.pdf)
  -- multipage schematic.
- [yahrzeit\_pixel9.png](yahrzeit_pixel9/yahrzeit_pixel9.png) -- KiCad board
  rendering.

## Reference Schematics

Approximately 280 individual YYZ Pixel boards are installed behind the
Yahrzeit Wall's etched-glass memorial panels. Three variations support six,
eight, or ten memorial lights. Their common circuit is documented in:

- [Schematic\_yyz\_pixel.pdf](schematics/Schematic_yyz_pixel.pdf) -- Original
  installed YYZ Pixel board schematic.

The nine-section board is electrically nine of these circuits condensed onto
one PCB. Both designs use the same 74HC245 octal bus transceiver and
74HC595 8-bit shift register:

- [74HC\_HCT245.pdf](schematics/74HC_HCT245.pdf) -- Bus-transceiver data sheet.
- [74HC\_HCT595.pdf](schematics/74HC_HCT595.pdf) -- Shift-register data sheet.

The [`schematics`](schematics/) directory also preserves the relevant Arduino
and Ethernet reference schematics.

## See Also

- [../README.md](../README.md) -- Yahrzeit Project Introduction.
- [../embedded/yahrzeit_v3/README.md](../embedded/yahrzeit_v3/README.md) --
  Yahrzeit Embedded Controller.
- [../embedded/yahrzeit_v3/INSTALL.md](../embedded/yahrzeit_v3/INSTALL.md) --
  Yahrzeit Embedded Controller — Installation Notes.
- [../docs/yahrzeit-v3-release-notes.md](../docs/yahrzeit-v3-release-notes.md)
  -- Yahrzeit Wall V3 Release Notes.
- [../docs/INSTALLATION_FORM.md](../docs/INSTALLATION_FORM.md) -- Yahrzeit Wall
  V3 Installation Record.

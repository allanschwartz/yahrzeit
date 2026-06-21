# Yahrzeit Project

This repository contains the full Yahrzeit Wall system for Congregation Beth Sholom: the web appliance, scheduling logic, embedded controller firmware, LED hardware designs, and supporting documentation.

## What the project does

The system tracks memorial (Yahrzeit) records, determines when lights should be on, and sends commands to the physical wall so that the correct LEDs illuminate at the correct times.

## Repository layout

- `yahrzeit_site-v3/` — current PHP web application for viewing and editing memorial records, configuring synagogue policy, and running reports.
- `yahrzeit_site-v2/` — earlier web site implementation.
- `embedded/` — firmware and controller software for the embedded wall controller.
  - `yahrzeit_v1/` — original controller implementation.
  - `yahrzeit_v2/` — Arduino-based development work.
  - `yahrzeit_v3/` — newer controller and threading-based implementation.
- `Hardware/` — PCB and enclosure design files, schematics, and related hardware documentation.
- `panels/` — stored panel layouts and panel master assets used by the display system.
- `presentations/` — presentation materials and images related to the project.

## System overview

1. The web application stores memorial data and synagogue settings.
2. The scheduling/engine logic decides which names and panels should be lit.
3. The embedded controller receives commands and drives the LED wall.
4. The hardware layer defines the pixel boards, wiring, and panel geometry.

## Key concepts

- **Yahrzeit Wall** — the physical LED display.
- **Yahrzeit Appliance** — the software environment used by operators to manage the wall.
- **Embedded Controller** — the controller that talks to the LEDs.
- **Pixel Board / Light Engine** — the physical LED modules that make up the display.
- **Minhag / Lighting Policy** — synagogue-specific rules that determine when observances are active.

## Notes for contributors

- The current web application lives under [yahrzeit_site-v3](yahrzeit_site-v3).
- The embedded controller work is separate from the PHP appliance and should be treated as its own hardware/software subsystem.
- If you are updating documentation, prefer clear, project-level explanations that describe how the pieces fit together.


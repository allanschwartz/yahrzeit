# YYZ Pixel V2 Cable and Power Notes

**Status:** Deferred design work
**Recorded:** August 22, 2026

These notes preserve field observations and design alternatives to revisit
before releasing the YYZ Pixel V2 boards and cable assemblies for production.
They are not yet production requirements.

## Existing Wall Installation

The existing 10-conductor signal ribbon cables are routed flat and straight
against the rear of the continuous aluminum chassis. They are presently held
in place with ordinary blue painter's tape.

During the August 2026 service visit, one signal cable was found hanging loose
rather than adhered to the chassis. It was secured again. The loose cable may
have contributed to the observed intermittent signal or clock problems, but
this was not proven conclusively.

Keeping the ribbon cable flat against the aluminum may reduce movement, loop
area, and coupling from nearby wiring. Painter's tape is useful for temporary
field work, but its adhesive should not be treated as a long-term production
cable-retention method. Consider durable cable clips, tie mounts, or another
serviceable mechanical restraint for the Version 2 installation.

## Shielded Ribbon-Cable Options

Shielded cable compatible with 0.050-inch-pitch IDC mass termination is
commercially available. Candidate product families include:

- [3M 90204 Series](https://www.3m.com/3M/en_US/p/d/b00039655/) - flat,
  pleated-copper-foil shielded cable; 28 AWG; available with 10 conductors;
  intended for IDC mass termination.
- [3M 3659 Series](https://www.3m.com/3M/en_US/p/d/b00039391/) - round,
  shielded and jacketed flat cable with periodic flat sections for IDC
  termination; potentially useful for the approximately 2.8-meter long run.
- [3M 1785 Series](https://www.3m.com/3M/en_US/p/d/b00039714/) - shielded,
  twisted-pair flat cable; available as 10 conductors/five pairs; 28 AWG on
  0.050-inch centers.
- [3M HF017 Series](https://www.3m.com/3M/en_US/p/d/b10098196/) - shielded,
  IDC-compatible, low-smoke/zero-halogen cable that may be appropriate for a
  permanent installation in a public building.

A standard plastic 2x5 IDC socket does not terminate the cable shield.
Any shielded-cable specification must therefore define:

- how the shield or drain conductor exits the jacket;
- where it connects to chassis or circuit ground;
- whether it is connected at one end or both ends;
- strain relief at each jacket-to-ribbon transition; and
- the permitted length of unshielded ribbon at each connector.

Shielding alone may not correct the underlying signal-integrity problem.
Pairing each important signal, especially `CP`, with a ground conductor may be
more useful than retaining the legacy arrangement of duplicated adjacent
signals. Before completing CBL-05 and CBL-06, reconsider the signal pinout,
ground allocation, connector polarization, and whether the new system must
remain directly compatible with every legacy cable.

Current cable definitions are in [`cables/`](cables/):

- `CBL-05-idc-100mm.yml` - short board-to-board signal jumper.
- `CBL-06-idc-2800mm.yml` - provisional long signal interconnect.

Their rendered WireViz PDF drawings are working documents. Cable and connector
manufacturer part numbers still need to be selected before production.

## Power-Supply Margin

The installed supplies are nominally **5 V, 5 A**. At least one supply was
found to be bad during the August 2026 service work. The remaining design also
appears to operate close to the supplies' current limit when all LEDs are on.

The installed YYZ Pixel boards use **220-ohm LED current-limiting resistors**.
One supply powers approximately six columns by 56 rows, or 336 LEDs. Assuming
an LED forward voltage between 1.8 V and 2.2 V:

```text
Current per LED = (5 V - Vf) / 220 ohms

Vf = 1.8 V:  14.5 mA per LED; 336 LEDs = 4.89 A
Vf = 2.0 V:  13.6 mA per LED; 336 LEDs = 4.58 A
Vf = 2.2 V:  12.7 mA per LED; 336 LEDs = 4.28 A
```

This estimate covers LED current only. It leaves little or no allowance for:

- logic and controller current;
- voltage loss in connectors, cables, and distribution wiring;
- component tolerances;
- power-supply derating and aging; or
- transient behavior when the display changes.

Field testing showed that the affected sections behaved correctly with all
LEDs commanded on when output-enable PWM reduced average brightness/current.
That observation is consistent with inadequate power margin, although voltage
and current should be measured before selecting replacement supplies.

Before releasing the Version 2 design:

1. Measure actual current and voltage at the supply and at the farthest board
   with representative LEDs fully on.
2. Select the final Version 2 LED resistor value from visual and electrical
   testing.
3. Establish a continuous-current design target with suitable engineering
   margin rather than sizing the supply at the calculated maximum load.
4. Review connector, wire, fuse, and distribution-board ratings against that
   target.

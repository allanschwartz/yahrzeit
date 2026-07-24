# CBS Yahrzeit Wall V3 Field Installation

Packing, commissioning, acceptance, rollback, and sign-off

- Site: Congregation Beth Sholom, San Francisco
- Planned installation: Tuesday–Wednesday, July 28–29, 2026
- Server release/tag: ______________________________________________
- Controller release/tag: __________________________________________
- Field leads: Allan Schwartz and Zach Lipton

Use this runbook to ensure that all equipment arrives together, the
existing V2 system remains available until V3 is accepted, the V3 server
and controller are commissioned in a deliberate sequence, and the final
installation facts are recorded.

Do not write passwords or the Wi-Fi password in the permanent copy.
Record only the responsible account and where credentials were
transferred or stored securely.

## Packing Checklist

The equipment is organized into six carryable containers because parking
near the synagogue cannot be assumed. Check each item while packing and
again before leaving the site.

### First Five Minutes Pouch

Place this pouch at the top of the Electrical Tool Tote or Backpack; it
is not a seventh container.

- [ ] Short Ethernet cable
- [ ] Multi-bit screwdriver
- [ ] Label maker or preprinted labels and marker
- [ ] Small flashlight
- [ ] Notebook and pen
- [ ] USB flash drive containing the tagged release and key documents

### Yahrzeit Appliance Bin

- [ ] Primary embedded Yahrzeit controller appliance
- [ ] Backup embedded Yahrzeit controller appliance
- [ ] Dedicated regulated USB-C power supplies
- [ ] Ethernet cables
- [ ] Spare Pixel Interface boards
- [ ] Pixel boards and signal cables

### Yahrzeit Server Box

- [ ] Primary NUC server
- [ ] NUC power supply
- [ ] Ethernet cable
- [ ] Mini keyboard
- [ ] Mouse
- [ ] HDMI cable
- [ ] Small Ethernet switch and its power supply

### Development Bin

- [ ] Arduino Uno R4 boards
- [ ] Spare Pixel Interface board
- [ ] Development fixture
- [ ] Soldering kit
- [ ] Test cables

### Electrical Tool Tote

- [ ] Screw gun
- [ ] Spare battery
- [ ] Drill bits
- [ ] Tie wraps
- [ ] Electrical tape
- [ ] Duct tape
- [ ] Adhesive mounting squares
- [ ] General electrical hand tools

### Backpack

- [ ] Laptop and charger
- [ ] Presentation
- [ ] House Manual pages
- [ ] Installation documents
- [ ] Notebook and pens
- [ ] Phone charger

### Miscellaneous Box

- [ ] Remaining installation hardware
- [ ] Spare cables
- [ ] Miscellaneous supplies

### Before Departure

- [ ] Primary NUC boots successfully.
- [ ] Spare NUC boots successfully.
- [ ] Both controller appliances power up.
- [ ] Both controller appliances respond through Ethernet.
- [ ] The current tagged release is available locally and on GitHub.
- [ ] Production CSV and `minhag.ini` are available on removable media.
- [ ] Printed binder and this checklist are packed.
- [ ] All six containers are present.

## Field Installation Sequence

### 1. Arrival and Kickoff

- [ ] Meet with the customer at approximately 10:00.
- [ ] Confirm Zach's availability for server and network work.
- [ ] Confirm who may authorize final acceptance.
- [ ] Walk through the planned sequence and rollback point.
- [ ] Demonstrate the V3 controller appliance, Pixel Interface board,
      representative Pixel board, and NUC server.
- [ ] Confirm that V2 will remain intact until V3 passes acceptance.

### 2. Record Site and Network Information

| Item | Value |
|---|---|
| Server administrator account | |
| Server hostname | |
| Server IPv4 address | |
| Server subnet/prefix | |
| Default gateway | |
| DNS server(s) | |
| Controller hostname, if any | |
| Controller IPv4 address | |
| Controller TCP port | |
| Browser URL | |
| Wi-Fi SSID, if needed | |
| Credential custodian/location | |
| Primary NUC location | |
| Spare NUC location | |

- [ ] Determine whether the existing controller subnet is preserved by
      VLAN, routing, or a secondary server address.
- [ ] Determine how Zach accesses the server remotely.
- [ ] Confirm that the web interface is not exposed directly to the
      public Internet.
- [ ] Record the approved addressing in the permanent installation
      record.

### 3. Preserve the V2 System

- [ ] Photograph and label every existing cable before disconnecting it.
- [ ] Record the V2 server and controller network settings.
- [ ] Export or copy the current memorial CSV.
- [ ] Copy the current synagogue policy or configuration.
- [ ] Save the existing crontab.
- [ ] Preserve any useful operating logs.
- [ ] Confirm that the V2 hardware can be reconnected without
      reconstruction.
- [ ] Do not erase or repurpose V2 until V3 has been accepted.

### 4. Install the V3 Server

- [ ] Install Ubuntu on the primary NUC, with Zach as the administrative
      user if that remains the agreed plan.
- [ ] Set the permanent Pacific time zone.
- [ ] Set the approved hostname and network configuration.
- [ ] Verify local network and Internet access.

Useful checks:

```sh
hostname
hostname -I
timedatectl
ping -c 3 github.com
```

- [ ] Download and inspect the installer.

```sh
BASE=https://raw.githubusercontent.com/allanschwartz/yahrzeit/master
curl -fsSL "$BASE/yahrzeit_site-v3/bin/install-yahrzeit.sh" \
    -o /tmp/install-yahrzeit.sh
less /tmp/install-yahrzeit.sh
bash /tmp/install-yahrzeit.sh
```

- [ ] Enter or confirm the production controller hostname/address when
      prompted.
- [ ] Confirm that the installer completes without error.
- [ ] Run the installer a second time to verify idempotency.
- [ ] Confirm that Apache, PHP, SSH, and cron start automatically.
- [ ] Confirm that the browser reaches the Yahrzeit Wall home page.
- [ ] Confirm that the configured server and controller addresses shown
      on the home page are correct.

### 5. Load Production Data and Policy

- [ ] Restore the production memorial CSV through the Reports page.
- [ ] Restore or enter the approved Minhag settings.
- [ ] Save the Minhag page and confirm that the managed cron schedule is
      regenerated.
- [ ] Confirm that all expected Yizkor dates and lighting times are
      shown correctly.
- [ ] Run the database audit.

Command-line checks:

```sh
cd ~/src/yahrzeit/yahrzeit_site-v3
bin/yahrzeit --audit
bin/yahrzeit --dry-run
php tests/yahrzeit_engine_policy_test.php
```

- [ ] Review representative records in the Names and Single Name pages.
- [ ] Compare modified records with the etched glass panels.

### 6. Commission the V3 Controller

- [ ] Record the controller firmware release/tag.
- [ ] Record the Arduino board model and Ethernet hardware.
- [ ] Configure and verify the production controller address and port.
- [ ] Verify the production wall geometry.
- [ ] Calibrate normal Yahrzeit brightness.
- [ ] Calibrate full-wall Yizkor/all-on brightness.
- [ ] Verify the ALIVE LED.
- [ ] Confirm that the controller responds to `version` and `status`.
- [ ] Confirm that test commands behave correctly on the development
      fixture before connecting the sanctuary wall.

### 7. Install the Controller at the Wall

- [ ] Go to the sanctuary with Zach and the facilities contact.
- [ ] Photograph the wall access area before opening it.
- [ ] Open the Yahrzeit Wall access area.
- [ ] Label and disconnect the V2 controller, leaving it available for
      rollback.
- [ ] Mount the V3 controller enclosure using the existing mounting
      location where practical.
- [ ] Connect regulated power, Ethernet, and the Pixel Interface cable.
- [ ] Secure the enclosure and cables against vibration and accidental
      disconnection.
- [ ] Verify the ALIVE LED and Ethernet link indicators.

### 8. Verify Both Control Paths

- [ ] From a laptop on the local network, connect directly to the
      controller.

```sh
nc CONTROLLER_HOST 2001
version
status
```

- [ ] Exercise appropriate controller tests before applying wall-wide
      commands.
- [ ] From the NUC, verify controller communication.

```sh
cd ~/src/yahrzeit/yahrzeit_site-v3
bin/yahrzeit
```

- [ ] From the web interface, test Yahrzeit lighting.
- [ ] From the Yizkor page, test full-wall lighting.
- [ ] Verify all-off and all-on operations.
- [ ] Compare the software display with the physical wall.
- [ ] Check for missing, shifted, duplicated, or unexpectedly lit names.

## Acceptance Test

### Server and Web Interface

- [ ] The NUC boots without manual intervention.
- [ ] The system clock, Pacific time zone, and date are correct.
- [ ] Apache serves the Yahrzeit application after reboot.
- [ ] The home page reports the expected server and controller
      addresses.
- [ ] All navigation tabs and Page Help links work.
- [ ] The User Guide opens in its own browser window.
- [ ] The interface is usable on a desktop browser.
- [ ] The Yizkor controls are usable on a phone.

### Data and Policy

- [ ] The production memorial CSV is installed.
- [ ] Database audit reports no unexplained errors.
- [ ] Representative English-date records are correct.
- [ ] Representative Hebrew-date records are correct.
- [ ] Panel and location assignments match the glass.
- [ ] This-week and next-week reports match the configured lighting
      policy.
- [ ] Friday's report boundary is Erev Shabbat through the following
      Erev Shabbat.
- [ ] Upcoming Yizkor events match the saved Minhag.

### Controller and Wall

- [ ] `version` reports the expected firmware release.
- [ ] `status` reports the expected board, Ethernet hardware, network
      configuration, wall geometry, and brightness values.
- [ ] The ALIVE LED operates continuously.
- [ ] Normal Yahrzeit lighting uses the calibrated brightness.
- [ ] Full-wall lighting uses the calibrated all-on brightness.
- [ ] All-off clears the wall.
- [ ] Yahrzeit lighting matches the report and screen display.
- [ ] Yizkor lighting illuminates the complete wall.
- [ ] No controller errors remain unexplained.

### Automation and Persistence

- [ ] `crontab -l` retains unrelated entries.
- [ ] The managed CBS Yahrzeit block is present once.
- [ ] Daily cron refresh is present.
- [ ] The nightly Yahrzeit operation is scheduled for the expected
      sunset-relative or fixed time.
- [ ] Yizkor on/off operations are scheduled for the saved times.
- [ ] `data/automation.log` records starts, decisions, actions, and
      failures clearly.
- [ ] Cron refresh does not produce unexplained changes.
- [ ] The controller retains the intended display after refresh/save.
- [ ] A reboot does not require manual repair.

### Maintenance and Recovery

- [ ] CSV download produces a usable backup.
- [ ] CSV upload makes a backup before replacement and runs an audit.
- [ ] Zach can find the online User Guide and Page Help.
- [ ] Zach has the repository URL, release tag, and printed binder.
- [ ] The spare NUC and V2 rollback equipment are labeled and stored.
- [ ] Remote maintenance access works by the approved method.

### Acceptance Exceptions

Exception 1:

______________________________________________________________________

Disposition:

______________________________________________________________________

Exception 2:

______________________________________________________________________

Disposition:

______________________________________________________________________

## Rollback Plan

Keep the V2 server and controller intact, labeled, and available until
the V3 installation passes the acceptance test and the customer
authorizes completion.

### Roll Back If

- The V3 server cannot boot or serve the application reliably.
- The V3 server cannot reach the production controller network.
- The V3 controller cannot drive the wall correctly.
- Lighting results disagree materially with the production data or
  policy.
- Errors cannot be understood and corrected within the installation
  window.
- The customer requests restoration of V2.

### Rollback Procedure

- [ ] Stop V3 automation or remove its managed cron block.
- [ ] Power down the V3 controller.
- [ ] Disconnect V3 power, Ethernet, and Pixel Interface cables.
- [ ] Reconnect the labeled V2 controller cables.
- [ ] Restore the V2 server/network path if it was changed.
- [ ] Restore the saved V2 crontab if needed.
- [ ] Verify direct controller communication.
- [ ] Run the established V2 lighting operation.
- [ ] Confirm that the wall display is restored.
- [ ] Record the failure, evidence, and next action.
- [ ] Leave V3 equipment labeled and safely stored for diagnosis.

## Post-Installation Follow-Up

- [ ] Monitor `data/automation.log` after the first nightly Yahrzeit
      operation.
- [ ] Review the log after the first Shabbat.
- [ ] Review the log after the next Yizkor observance.
- [ ] Recheck the installation after approximately one week.
- [ ] Review log rotation after approximately three months.
- [ ] Confirm that the spare NUC still boots and is identifiable.
- [ ] Record any customer-requested changes separately from acceptance
      defects.

## Installation Sign-Off

| Item | Result |
|---|---|
| Installation date | |
| Primary NUC location | |
| Spare NUC location | |
| V2 rollback equipment location | |
| Server hostname/address | |
| Controller hostname/address/port | |
| Server release/tag | |
| Controller release/tag | |
| Normal brightness | |
| Full-wall brightness | |
| Production CSV identifier/date | |
| Minhag reviewed by | |
| Automation log reviewed by | |
| Exceptions attached | Yes / No |
| Final result | Accepted / Accepted with exceptions / Rolled back |

| Role | Name | Signature | Date |
|---|---|---|---|
| Installer | | | |
| Technical maintainer | | | |
| CBS representative | | | |

## See Also

- **Project**
  - [`./README.md`](../README.md)
- **Server**
  - [`./yahrzeit_site-v3/README.md`](../yahrzeit_site-v3/README.md)
  - [`./yahrzeit_site-v3/INSTALL.md`](../yahrzeit_site-v3/INSTALL.md)
- **Controller**
  - [`./embedded/yahrzeit_v3/README.md`](../embedded/yahrzeit_v3/README.md)
  - [`./embedded/yahrzeit_v3/INSTALL.md`](../embedded/yahrzeit_v3/INSTALL.md)
- **Hardware**
  - [`./Hardware/README.md`](../Hardware/README.md)

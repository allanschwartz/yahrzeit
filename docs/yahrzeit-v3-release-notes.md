# Congregation Beth Sholom Yahrzeit Wall V3 Release Notes

**Release:** V3  
**Date:** July 2026  
**Status:** Released

## Overview

Yahrzeit Wall V3 modernizes the system that has served Congregation Beth
Sholom for many years. It retains the existing memorial database, panel
layout, and physical wall while replacing and improving the server appliance,
operator interface, automation, and embedded controller software.

The principal goal of V3 is dependable, unattended operation. Routine
Yahrzeit and Yizkor lighting is calculated from the memorial records and the
synagogue's saved practices, scheduled automatically, sent to the wall, and
recorded in an operational log.

## Major Improvements

### Operator Interface

- A clearer home page summarizes the appliance, controller, current lighting,
  configured practices, upcoming automation, and the next Yizkor dates.
- A dedicated Yizkor page presents the upcoming Yizkor services and provides
  the wall-wide lighting controls in a form that is also practical on a
  mobile phone.
- The Panels pages show the physical wall, panel locations, and the memorial
  lights expected to be lit.
- The Names page provides flexible searching by name, date, memorial option,
  and panel location. Existing memorial records may be reviewed and corrected
  individually.
- The Reports page produces day, week, and month reports. It also provides
  database auditing, a preview of controller commands, and CSV export and
  import for backup and batch maintenance.
- The Minhag page provides one place to configure Yahrzeit observance,
  lighting time, and Yizkor dates.

### Scheduling and Automation

- Yahrzeit lighting may be observed on the Yahrzeit date alone or for the
  full Erev Shabbat-to-Erev Shabbat week.
- Lighting may be scheduled at a fixed clock time or relative to sunset.
- The four annual Yizkor services are scheduled from their Hebrew dates. An
  optional additional full-wall memorial date may also be configured.
- The appliance creates and maintains its own managed cron schedule from the
  saved Minhag settings. Sunset-based times are refreshed automatically as
  sunset changes through the year.
- Week reports and automated weekly lighting use the same date-selection
  policy.

### Operational Reliability

- Each scheduled operation records its start, decision, action, controller
  result, completion status, and duration in `data/automation.log`.
- Controller connection failures and rejected commands are reported clearly;
  successful individual commands do not overwhelm the log.
- Standard log rotation retains a useful operating history without allowing
  the automation log to grow indefinitely.
- Dry-run, command-preview, and database-audit facilities allow important
  behavior to be checked without changing the wall.
- Updates preserve the live memorial database and congregation settings.

### Controller

- The updated controller software runs on the current Arduino-based controller
  and accepts its command language through either Ethernet or USB serial.
- It maintains the wall image in memory, refreshes the physical display,
  preserves saved state, and returns a result for each command.
- Built-in status, network, timing, and self-test facilities support bench
  testing and troubleshooting.
- Production panel geometry and network settings are selected when the
  controller is prepared for installation.

### Installation and Recovery

- A repeatable installer builds or updates the Ubuntu-based Yahrzeit
  appliance, installs the required services, configures the web server and
  automation, and asks for the controller address.
- The controller address is kept in a simple site configuration file and may
  be an IP address or a locally resolvable host name.
- The installer can be run again when repairing or replacing the appliance.
  Existing memorial and Minhag data is retained during a normal update.
- The server application has been modernized for PHP 8.

### Documentation

- Every operator page has page-specific online help.
- A complete online User Guide covers routine operation, reports, memorial
  database maintenance, backup, restoration, maintenance, and
  troubleshooting.
- The User Guide is also supplied as a PDF for inclusion in the Congregation
  Beth Sholom House Manual.
- Installer notes and an internal software map support future technical
  maintenance and recovery.

## Compatibility and Upgrade Notes

- V3 continues to use the existing `yahrzeits-rev4.csv` memorial database and
  the established panel identifiers and locations.
- A database audit should be run after importing or substantially editing the
  memorial database and before relying on scheduled lighting.
- Before installation at Congregation Beth Sholom, confirm the controller
  network address and load the production wall geometry into the controller.
- The web interface is intended for use on the synagogue's protected internal
  network. Any remote access or additional authentication must be supplied by
  the synagogue's network environment.

## Verification

V3 includes automated policy tests which compare the names selected for weekly
reports with the names selected for weekly lighting. The current regression
run performs 50,796 such comparisons across the memorial database and a range
of calendar dates.

Installation, Minhag updates, managed cron replacement, logging, controller
communication, CSV preservation, and repeated installation have also been
tested on Intel NUC appliance hardware.

## Known Limitation

English-date, day-only observance has a year-boundary case requiring further
work when a December 31 death anniversary is evaluated for January 1. This
does not affect Congregation Beth Sholom's configured Hebrew-date observance.

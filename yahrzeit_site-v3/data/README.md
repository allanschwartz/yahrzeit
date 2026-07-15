# Yahrzeit live data files

This directory contains the data files used by the current PHP implementation.

Live files:

- `minhag.ini` — synagogue-specific policy/custom configuration.
- `yahrzeits-rev4.csv` — current memorial name/date/location database.
- `automation.log` — scheduled decisions, actions, transport results, and
  failures. The installer retains 13 compressed weekly rotations.

Historical, test, migration, and scratch CSV files have been moved to `attic/data/`.

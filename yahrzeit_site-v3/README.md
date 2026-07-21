# CBS Yahrzeit Wall Appliance

This directory contains the PHP web application, scheduler, command-line
engine, and installer for the Congregation Beth Sholom Yahrzeit Wall
appliance.

This README is intended for future maintainers of the appliance, including
people who did not build the original system.

The top-level repository README explains the whole Yahrzeit project: embedded
firmware, hardware, panel assets, and historical versions. This README is only
about the `yahrzeit_site-v3` appliance software.

## What This Appliance Does

The appliance is responsible for:

- displaying the current Yahrzeit wall status,
- rendering memorial/name views and reports,
- maintaining the memorial CSV database,
- applying synagogue lighting policy from `data/minhag.ini`,
- running scheduled Yahrzeit and Yizkor phases,
- generating controller command streams,
- and transmitting those commands to the Arduino-based wall controller.

## System Shape

The usual control path is:

```text
cron
  -> bin/yahrzeit_scheduler
    -> bin/yahrzeit
      -> bin/yahrzeit_engine.php
        -> include/yahrzeit_policy.inc.php
          -> controller command stream
          -> nc TCP connection
            -> Arduino controller
```

The web screens use the same underlying code where possible. Reports, audits,
and command previews are intended to be safe; live wall operations call
`bin/yahrzeit` and may transmit to the controller.

## Directory Layout

- `bin/` - command-line tools, scheduler, engine, and installer.
- `include/` - shared PHP helpers for dates, names, panels, LED mapping,
  lighting policy, and page layout.
- `data/` - live CSV data, `minhag.ini`, backups, and the automation log.
- `help/` - help pages shown by the web UI.
- `docs/` - project notes and generated/internal documentation.
- `css/`, `images/`, `js/` - legacy browser-side assets used by the screen
  templates.

## Important Files

- `data/yahrzeits-rev4.csv` - memorial database.
- `data/minhag.ini` - synagogue lighting policy and display settings.
- `bin/yahrzeit-controller.conf` - controller hostname/address and TCP port.
- `bin/yahrzeit` - shared scheduler/web execution stage and controller transport.
- `bin/yahrzeit_engine.php` - decides what should be lit or reported.
- `bin/yahrzeit_scheduler` - cron-facing scheduled phase runner.
- `bin/fix-up-crontab` - installs the policy-derived managed cron block.
- `include/yahrzeit_policy.inc.php` - shared day/week lighting decisions.
- `include/panels.inc.php` - static wall/panel geometry.
- `include/leds.inc.php` - maps panel/person locations to controller commands.

## Installation

For a fresh Ubuntu/Debian appliance, run:

```sh
curl -fsSL https://raw.githubusercontent.com/allanschwartz/yahrzeit/master/yahrzeit_site-v3/bin/install-yahrzeit.sh -o /tmp/install-yahrzeit.sh
chmod +x /tmp/install-yahrzeit.sh
/tmp/install-yahrzeit.sh
```

The installer:

- installs required packages,
- enables SSH,
- disables AppArmor,
- clones or updates only `yahrzeit_site-v3` using Git sparse checkout,
- configures Apache to serve the site,
- runs PHP and shell syntax checks,
- runs non-transmitting sanity checks,
- and installs or repairs the managed cron schedule.

The installer does not transmit commands to the physical controller during
its tests.

On an existing appliance, the checkout is treated as deployed code rather
than a development worktree. The installer preserves `data/minhag.ini` and
`data/yahrzeits-rev4.csv`, updates the remaining site files to exactly match
the selected remote branch, and then restores those two live data files.
Server-side edits to application source files are intentionally discarded.

By default, the installer places the sparse checkout under:

```text
~/src/yahrzeit/yahrzeit_site-v3
```

and links it into Apache as:

```text
/var/www/html/yahrzeit
```

## Network Configuration

There are two fixed-address concerns:

- the appliance/server address, used by operators browsing to the web UI,
- the Arduino controller address, used by `bin/yahrzeit` when transmitting
  commands.

The installer does not configure a static appliance/server IP address. On a
fresh Ubuntu appliance, configure that on-site using the local network policy,
usually in `/etc/netplan/*.yaml`.

Do not commit site-specific `/etc/netplan/*.yaml` files to this repository.

## Controller Address

The controller connection is stored as shell assignments in:

```text
bin/yahrzeit-controller.conf
```

`bin/yahrzeit` sources this file before parsing its command-line options. The
installer remembers the previously recorded `CONTROLLER_HOST` before updating
the checkout, installs the current tracked configuration, and asks whether to
keep that address or enter a replacement. The configuration file itself is not
preserved. The fixed port and transport defaults update with the software. The
Arduino controller firmware must use the matching address and port.

For a one-command diagnostic override, use environment variables:

```sh
CONTROLLER_HOST=192.168.86.240 CONTROLLER_PORT=2001 bin/yahrzeit --dry-run
```

## Safe Validation

From this directory, syntax-check the PHP and shell files with:

```sh
for f in index.php [0-9]*.php include/*.inc.php help/*.php \
         bin/yahrzeit_engine.php bin/yahrzeit_scheduler bin/fix-up-crontab \
         tests/*.php; do
    php -l "$f"
done

for f in bin/yahrzeit bin/install-yahrzeit.sh; do
    bash -n "$f"
done
```

Run data/audit checks that do not transmit:

```sh
bin/yahrzeit --audit
bin/yahrzeit --dry-run
php tests/yahrzeit_engine_policy_test.php
```

## Live Controller Operations

Only run these when the controller IP is correct and it is safe to talk to the
wall:

```sh
bin/yahrzeit
bin/yahrzeit --all-off
bin/yahrzeit --all-on
bin/yahrzeit --yizkor
```

The default command applies normal Yahrzeit lighting. Use all four commands
carefully: without `--dry-run`, each transmits to the controller and changes
the wall.

## Scheduled Operation

`bin/fix-up-crontab` reads `data/minhag.ini` and manages only the marked
Yahrzeit block in the installation account's crontab. It preserves unrelated
cron jobs. Normal Yahrzeit lighting is reapplied every evening; the saved
day-only or weekly observance determines which memorial lights are selected.
Fixed times remain stable, while sunset-based times are recalculated for that
evening automatically.

Every managed cron invocation appends to `data/automation.log`. Each run records
its phase, decision or action, controller transport summary, cron-repair
result, final status, and duration. Normal scheduled runs do not record the
complete controller command stream. The installer configures `logrotate` to
retain 13 compressed weekly logs without sending routine cron email.

```sh
tail -100 data/automation.log
ls -lh data/automation.log*
```

```sh
bin/fix-up-crontab --dry-run
sudo bin/fix-up-crontab
```

The installer records the intended cron account in
`/etc/yahrzeit-cron-user` and authorizes the Minhag page to invoke only this
repair helper. Rerun the installer, or the helper directly, after an operating
system upgrade or appliance migration if scheduled operation needs repair.

On hardened Ubuntu releases, Apache runs with the home directory mounted
read-only in its private service namespace and with sudoers configuration
hidden. The installer adds a systemd drop-in that makes only this site's
`data/` directory writable and permits sudo to read the single-command
authorization used by the cron-repair wrapper. The remaining Apache
filesystem protections stay enabled.

## Data Backup And Restore

The two appliance-specific live data files are:

```text
data/yahrzeits-rev4.csv
data/minhag.ini
```

The web Reports page can export and import this CSV. The upload path creates a
backup under `data/backups/` and runs an audit afterward.

Before replacing the appliance or doing major maintenance, copy:

```text
data/yahrzeits-rev4.csv
data/minhag.ini
data/backups/
bin/yahrzeit-controller.conf
```

The controller configuration is replaceable software configuration rather
than memorial data, but retaining it records the controller address selected
for that appliance.

## Updating The Appliance

The preferred one-person engineering workflow is:

1. Make production changes in the master repository on the development Mac.
2. Commit the changes.
3. Tag the installation version.
4. Push `master` and the tag.
5. On the installed appliance, rerun `bin/install-yahrzeit.sh`.

## What Not To Change Casually

- Do not change the CSV schema without a migration plan.
- Do not change controller protocol commands unless the embedded firmware is
  updated at the same time.
- Do not edit panel geometry without running audits and checking reports.
- Do not depend on lab IP addresses in production.
- Do not add live controller commands to install-time tests.

## Related Documentation

- `../README.md` - top-level map of the full Yahrzeit project.
- `docs/yahrzeit-installer-notes.txt` - installer notes.
- `docs/php-internal-map.html` - internal map of the PHP code.
- `docs/todo.md` - working punch list.
- `AGENTS.md` - coding guidance for AI/code-assistant work in this directory.

For embedded controller firmware and hardware design files, see the top-level
repository README and the sibling `embedded/` and `Hardware/` directories.

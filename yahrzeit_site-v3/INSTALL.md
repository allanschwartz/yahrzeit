# Yahrzeit Server Appliance Installation

These notes describe how to install the Congregation Beth Sholom Yahrzeit Wall
PHP application on a fresh Ubuntu/Debian appliance.

<p align="center">
  <a href="images/intel-nuc-cutaway.jpg">
    <img src="images/intel-nuc-cutaway.jpg"
         alt="Conceptual cutaway view of an Intel NUC" height="500">
  </a>
</p>

*A conceptual cutaway view of an Intel NUC. The installer similarly peels back
the appliance's simple exterior and assembles the operating-system, web-server,
scheduling, and application layers inside.*

## Target Platform

- Intel NUC or similar x86-64 computer
- Fresh Ubuntu Server LTS installation
- Wired Ethernet
- Apache and PHP
- Application served at:

  ```text
  http://<appliance-ip>/yahrzeit/
  ```

## Installation Model

The installer is downloaded onto the appliance and run locally. It does not
transmit commands to the physical LED controller.

During an update, it remembers the controller address and brightness values
and preserves the two live data files:

```text
data/minhag.ini
data/yahrzeits-rev4.csv
```

It replaces the deployed code with the selected remote branch, restores the
live data, and asks whether to retain or replace the remembered controller
address. Source-code changes made directly in the appliance checkout are
discarded.

## Before Installation

1. Install Ubuntu Server LTS.
2. Enable OpenSSH during Ubuntu installation.
3. Log in as the appliance administrator.
4. Confirm the hostname, assigned address, and Internet access:

   ```sh
   hostname
   hostname -I
   ping -c 3 github.com
   ```

For production, the appliance should have a stable address. Use either a router
DHCP reservation or the site's Ubuntu network configuration, typically
`/etc/netplan/*.yaml`.

Change the production password before installing the appliance at the
synagogue.

## Run the Installer

Download and inspect the installer:

```sh
curl -fsSL -o /tmp/install-yahrzeit.sh \
  https://raw.githubusercontent.com/allanschwartz/yahrzeit/master/yahrzeit_site-v3/bin/install-yahrzeit.sh

less /tmp/install-yahrzeit.sh
```

Then run it:

```sh
bash /tmp/install-yahrzeit.sh
```

The installer is safe to rerun. It updates the deployed application and
refreshes the Apache configuration, web link, validation results, and managed
cron schedule.

## What the Installer Configures

- Git, using sparse checkout for `yahrzeit_site-v3`
- Apache
- a stable local Apache `ServerName`, avoiding warning `AH00558`
- PHP CLI and Apache PHP support
- `netcat` for controller communication
- GNU `timeout` support
- OpenSSH enabled and started
- cron enabled and started
- AppArmor disabled on this dedicated appliance
- `/var/www/html/yahrzeit` linked to the checked-out application
- a root-page redirect to `/yahrzeit/`
- PHP PCRE JIT disabled to avoid executable-memory warnings in hardened
  environments
- local syntax, audit, and policy tests
- policy-derived cron entries owned by the appliance administrator
- 13 compressed weekly automation logs under `data/`
- interactive controller-address configuration
- preservation of calibrated normal and full-wall brightness values during an
  update
- preservation of `data/minhag.ini` and `data/yahrzeits-rev4.csv` during an
  update
- a narrow Apache systemd exception allowing writes to the application data
  directory and access to the cron helper's sudoers authorization

## Installation Locations

Repository:

```text
https://github.com/allanschwartz/yahrzeit
```

Default application directory:

```text
~/src/yahrzeit/yahrzeit_site-v3
```

Apache URL path:

```text
/yahrzeit/
```

## After Installation

Open the application:

```text
http://<appliance-ip>/yahrzeit/
```

Run the non-transmitting validation:

```sh
cd ~/src/yahrzeit/yahrzeit_site-v3
bin/yahrzeit --audit
bin/yahrzeit --dry-run
php tests/yahrzeit_engine_policy_test.php
```

Confirm `bin/yahrzeit-controller.conf` before any live controller test.

Review the managed cron block printed by the installer. Saving the Minhag page
also refreshes that block. To repair it manually:

```sh
sudo bin/fix-up-crontab
```

Review scheduled-operation history when needed:

```sh
tail -100 data/automation.log
ls -lh data/automation.log*
```

## See Also

- **Project**
  - [`./README.md`](../README.md)
- **Server**
  - [`./yahrzeit_site-v3/README.md`](README.md)
  - [`./yahrzeit_site-v3/INSTALL.md`](INSTALL.md)
- **Controller**
  - [`./embedded/yahrzeit_v3/README.md`](../embedded/yahrzeit_v3/README.md)
  - [`./embedded/yahrzeit_v3/INSTALL.md`](../embedded/yahrzeit_v3/INSTALL.md)
- **Hardware**
  - [`./Hardware/README.md`](../Hardware/README.md)

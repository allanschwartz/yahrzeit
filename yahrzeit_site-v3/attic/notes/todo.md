# Yahrzeit Site TODO / Punch List

This file is the working punch list for the PHP/server-side Yahrzeit Wall application.  It overlaps with the burn-down/status page, but this view is intentionally organized by file/module so it is easy to decide what to touch next.

Priority markers:

- **[A]** near-term / important before CBS deployment
- **[B]** later cleanup / refactor / nice-to-have
- **(at CBS)** requires discussion, site access, or production data

---

## Command-Line / Scheduled Programs

### `bin/yahrzeit`

- Done.
- Main operator-facing command wrapper.
- Generates/transmits controller commands and supports audit/report/preview modes through `bin/yahrzeit_engine.php`.

### `bin/yahrzeit_engine.php`

- Done for current deployment.
- Future cleanup:
  - **[B]** Split calendar/date helpers into `include/date_support.inc.php`.
  - **[B]** Move per-person helper functions into an expanded `include/names.inc.php`.
  - **[B]** Consider `include/audit_support.inc.php` for panel/name validation and duplicate-location checks.
  - Keep `yahrzeit_engine.php` as the orchestration layer: parse options, select mode, read data, and call engine/report/audit functions.

### `bin/yahrzeit_scheduler`

- Mostly done for now.
- Uses explicit cron phases:
  - `--phase yizkor-on`
  - `--phase yizkor-off`
  - `--phase yahrzeit`
- **[B] (at CBS)** Confirm CBS Yizkor service timing and decide whether the scheduler remains three-phase or simplifies to two-phase.

### Cron installation helper

- **[A]** Update `bin/add-to-private-crontab` for the new phase-based scheduler.
- **[A]** Treat crontab generation/installation as a technical installation task, not as something the web form performs automatically.
- **[A]** Keep suggested cron entries visible/copyable for the technical maintainer.

---

## GUI Screens

### `0yahrzeit.php`

- Works, but could justify the home/status screen with more useful live summary information.
- **[A]** Add a “next Yizkor observance” helper/library call.
- **[A]** Add a “names lit today” or “currently scheduled memorial names” summary count.
- **[B]** Jazz up this screen with a better status/dashboard presentation.
- Historical note: old removed status lines should be replaced only with real, reliable status information.

### `1viewpanels.php`

- OK.
- Read-only wall/panel overview plus manual wall-wide operations.

### `3singlepanel.php`

- OK.
- Read-only single-panel database assignment view.

### `4viewnames.php`

- OK.
- Read-only searchable memorial-name browser.

### `5singlename.php`

- **[A]** Decide final policy:
  - test/retain legacy add/modify behavior, or
  - make the screen read-only.
- Current direction favors read-only or narrowly scoped safe operations, but this is not fully decided.

### `6reports.php`

- **[A]** Test CSV download/upload.
- **[A]** Verify upload creates a backup and immediately runs audit.
- Reports/audit/preview functionality is otherwise in good shape.

### `7minhag.php`

- OK.
- Saves `data/minhag.ini`.
- Does not modify crontab directly.
- Should warn the maintainer that Yizkor timing changes may require manual crontab review/update.

---

## Help Pages

### `help/0yahrzeit.php`

- OK.

### `help/1viewpanels.php`

- OK.

### `help/3singlepanel.php`

- OK.

### `help/4viewnames.php`

- OK.

### `help/5singlename.php`

- OK, but may need wording update after final `5singlename.php` policy decision.

### `help/6reports.php`

- OK.

### `help/7minhag.php`

- OK.
- Updated for phase-based scheduler and crontab-advisory model.

### `help/user_guide.php`

- OK.
- Should be kept in sync with scheduler model and deployment assumptions.

---

## Include / Library Files

### `include/misc.inc.php`

- Works.
- **[B]** Split into smaller files later:
  - path/config helpers
  - layout/rendering helpers
  - Minhag config helpers
- **[A]** Refactor `emitMessagePage()` to use CSS/div layout instead of tables.
- **[A]** Refactor `emitTopOfScreen()` to use CSS/div layout instead of tables.
- **[B]** Eventually modernize the whole table-based page shell.

### `include/names.inc.php`

- OK.
- Future destination for per-person helper functions currently inside `yahrzeit_engine.php`.

### `include/panels.inc.php`

- OK.

### `include/leds.inc.php`

- OK.

---

## Platform / Installation

### Platform selection

- Done.
- Selected direction: Raspberry Pi 4 with USB SSD boot/storage.
- Avoid long-term production dependence on microSD root filesystem.
- Use a reliable power supply/power brick suitable for long unattended service.

### Porting

- **[A]** Port/test the site on the selected Raspberry Pi 4 + SSD platform.
- **[A]** Confirm PHP version, required PHP modules, cron behavior, filesystem paths, and network configuration.

### Installation

- **[A]** Document appliance installation path, probably `/home/pi/yahrzeit_site-v3` unless changed.
- **[A]** Install/update crontab entries for phase-based scheduler.
- **[A]** Confirm startup/reboot behavior.
- **[A]** Confirm logs and backups are stored in sane locations.

### Upgrade / maintenance

- **[B]** Create a simple upgrade procedure.
- **[B]** Create backup/restore notes for:
  - `data/yahrzeits-rev4.csv`
  - `data/minhag.ini`
  - crontab
  - installed application tree
- **[B]** Consider cloning a spare SSD after installation.

---

## Data / Deployment Issues

- **[A] (at CBS)** Fix known audit defect: Emile Kingsley uses unknown panel `col58`.
- **[A]** Run audit after any production CSV update.
- **[B]** Decide whether any login/security support is needed on the deployed appliance.

---

## Historical Notes

- This file descends from an older PHP/server-side bug and TODO list dating back to the original 2008 project.
- Some old references, such as direct `/tty` serial work, predate the later single-port communication server and are now historical only.
- The project stalled for years, then the PHP/server-side cleanup was pushed through rapidly in the 2026 modernization pass.

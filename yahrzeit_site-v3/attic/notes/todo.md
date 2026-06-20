# Yahrzeit Site TODO / Punch List

This is the working punch list for the PHP/server-side Yahrzeit Wall application.  It overlaps with the burn-down/status page, but this view is intentionally organized by file/module so it is easy to decide what to touch next.

Priority markers:

- **[A]** near-term / important before CBS deployment
- **[B]** later cleanup / refactor / nice-to-have
- **(at CBS)** requires discussion, site access, or production data

General rules for future work:

- Prefer small, reviewable, behavior-preserving changes.
- Keep the project procedural PHP unless there is a deliberate decision otherwise.
- Do not convert the application to classes as part of routine cleanup.
- Keep screen files mostly responsible for GET/POST dispatch and page rendering.
- Keep data parsing, wall geometry, date rules, and lighting decisions in include/engine code.
- Do not rewrite the live memorial CSV database unless the task explicitly requires it and a backup/audit path is in place.
- Do not transmit to the physical controller from tests unless explicitly intended.

---

## Cross-Cutting PHP Cleanup

These tasks apply throughout the PHP codebase.

- **[A]** Run `php -l` on every edited PHP file before commit.
- **[A]** Fix any obvious PHP warnings, undefined-index reads, or brittle assumptions encountered during normal work.
- **[B]** Clean up poorly coded PHP functions when touching the file anyway.
  - Preserve behavior.
  - Prefer clearer names, smaller helpers, and simple control flow.
  - Avoid clever abstractions, closures, or class conversions unless specifically justified.
- **[B]** Correct stale or misleading documentation at the top of PHP files.
- **[B]** Add lightweight function comments to important PHP functions.
  - Use a short Doxygen-like block when useful.
  - A simple `@brief` is usually enough.
  - Do not add noisy boilerplate comments to trivial helpers.
- [B] Review or remove ONNOW support. It was an ambitious manual override feature, but current operational expectation is that staff will not edit the CSV for one-off lighting requests. Editing support is primarily for correcting DOD / Hebrew DOD records.

---

## Command-Line / Scheduled Programs

### `bin/yahrzeit`

- Works.
- Main operator-facing command wrapper.
- Generates/transmits controller commands and supports audit/report/preview modes through `bin/yahrzeit_engine.php`.


### `bin/yahrzeit_engine.php`

- Done for current deployment.
- Future cleanup:
  - **[A]** Identify and isolate the shared “lighting decision” logic.
    - Goal: one function should decide whether a memorial record should be lit for a given timestamp/minhag context.
    - This function should support command generation, dashboard counts, and panel visualization.
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
- [A] Provide explicit command-line installer/repair tool for scheduler crontab.
- [A] Document that OS upgrades or appliance migration may require rerunning it.
- [B] 7minhag.php may display current/suggested cron entries, but should not modify crontab autom

---

## GUI Screens

### `0yahrzeit.php`

- Works.
- Home/status dashboard now shows date/time, Hebrew date, sunset/Shabbat timing, configured policy summary, controller summary, and wall image.
- **[A]** Add a “next Yizkor observance” helper/library call.
- **[A]** Add a count of memorial LEDs that should be lit now in normal yahrzeit mode.
  - Compute from database + minhag + current timestamp.
  - Do not query the controller 
  - Do not duplicate lighting policy logic in the screen file.
- **[A]** Add a summary line such as `N yahrzeit lights should be lit now` once shared lighting-policy logic exists.
- **[B]** Jazz up this screen with a better status/dashboard presentation.
- Historical note: old removed status lines should be replaced only with real, reliable status information.

### `1viewpanels.php`

- OK.
- Read-only wall/panel overview plus manual wall-wide operations.
- Manual all-on/all-off should remain protected by the `bin/yahrzeit` transmit timeout.

### `3singlepanel.php`

- OK.
- Read-only single-panel database assignment view.
- **[A]** Show `ledon.gif` or `ledoff.gif` depending on whether the memorial location should be lit in the current yahrzeit window.
  - This should use the same shared lighting-policy function used by `0yahrzeit.php` and `bin/yahrzeit_engine.php`.
  - in order to facilitate that:
    - Add include/yahrzeit_policy.inc.php to hold shared lighting-policy logic used by yahrzeit_engine.php, 0yahrzeit.php, and 3singlepanel.php.
  - Keep row/column rendering order for HTML tables: rows outside, columns inside.
- **[A]** Add a panel-level summary line such as `N yahrzeit lights lit on this panel`.

### `4viewnames.php`

- OK.
- Read-only searchable memorial-name browser.
- **[B]** After shared lighting-policy logic exists, consider adding a search/display marker for records currently scheduled to be lit.

### `5singlename.php`

- OK.
- **[B]** implement "add" or "new" name if an only if the synagogue asks for this.

### `6reports.php`

- OK.
- Reports, audit, command preview, CSV download, and CSV upload/maintenance live here.
- **[B]** Keep upload/download/audit result paths easy to review; do not let upload silently replace data without backup and audit.

### `7minhag.php`

- OK.
- Saves `data/minhag.ini`.
- Does not modify crontab directly.
- Should warn the maintainer that Yizkor timing changes may require manual crontab review/update.

---

## Help Pages

### `help/0yahrzeit.php`

- OK.
- **[B]** Update if the dashboard gains lit-count or next-Yizkor summaries.

### `help/1viewpanels.php`

- OK.

### `help/3singlepanel.php`

- OK.
- **[B]** Update after the panel view distinguishes currently-lit vs unlit memorials.

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
- Owns memorial CSV record parsing, mapping, and writing.
- **[A]** Do not mix transient runtime lighting state into the persistent memorial record data.
- **[B]** Future destination for pure per-person helper functions currently inside `yahrzeit_engine.php`.
- **[B]** If an `is_lit` or similar flag is needed for display, prefer a computed/transient result from shared lighting-policy logic rather than writing it into the CSV-backed record structure.

### `include/date_support.inc.php`

- OK.
- Owns date, Hebrew-date, sunset, Shabbat, and related helpers.
- **[B]** Add or update short function comments where they clarify non-obvious date rules.

### `include/panels.inc.php`

- OK.
- Owns static wall/panel geometry.

### `include/leds.inc.php`

- OK.
- Owns LED/panel/controller mapping.

### Future `include/lighting_policy.inc.php` or equivalent

- **[A]** Consider introducing a shared library for yahrzeit/yizkor lighting decisions.
- Should answer questions such as:
  - Should this memorial record be lit at this timestamp?
  - Is the current system in normal yahrzeit mode or Yizkor mode?
  - How many memorial records should be lit now?
  - Which records should be marked lit for panel/name views?
- Must be shared by dashboard/report/panel visualization and command-generation logic to avoid duplicate policy decisions.

---

## Platform / Installation

### Platform selection

- Direction under discussion.
- Earlier direction was Raspberry Pi 4 with USB SSD boot/storage.
- Current thinking may favor an available Intel NUC for production because it is fewer components and more PC-like to service.
- Avoid long-term production dependence on microSD root filesystem if Raspberry Pi is used.
- Use reliable power, wired Ethernet, and a serviceable storage device.

### Porting

- **[A]** Port/test the site on the selected appliance platform.
- **[A]** Confirm PHP version, required PHP modules, cron behavior, filesystem paths, and network configuration.
- **[A]** Confirm `timeout` command is available for `bin/yahrzeit` transmit protection.

### Installation

- **[A]** Document appliance installation path.
  - Possible Raspberry Pi path: `/home/pi/yahrzeit_site-v3`.
  - Possible NUC path should be chosen during appliance setup.
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

## Internal Documentation

- **[B]** Create a standalone internal developer map, probably `docs/php-internal-map.html`.
  - Include left-sidebar navigation.
  - Document top-level screens, include files, command-line tools, scheduler, data files, and architectural boundaries.
  - Use existing file headers and actual code as source of truth.
  - Do not invent behavior.

---

## Historical Notes

- This file descends from an older PHP/server-side bug and TODO list dating back to the original 2008 project.
- Some old references, such as direct `/tty` serial work, predate the later single-port communication server and are now historical only.
- The project stalled for years, then the PHP/server-side cleanup was pushed through rapidly in the 2026 modernization pass.

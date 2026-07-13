# Yahrzeit Site TODO / Punch List

This is the working punch list for the PHP/server-side Yahrzeit Wall application.  It overlaps with the burn-down/status page, but this view is intentionally organized by file/module so it is easy to decide what to touch next.

Priority markers:

- **[A]** near-term / important before CBS deployment
- **[B]** later cleanup / refactor / nice-to-have
- **(at CBS)** requires discussion, site access, or production data

---

## Command-Line / Scheduled Programs

### `bin/yahrzeit`

- Works.
- Main operator-facing command wrapper.


### `bin/yahrzeit_engine.php`

- Done for current deployment.
- **[A]** Correct the `week` and `next-week` report ranges to use the shared
  Erev Shabbat-to-Erev Shabbat lighting-week policy.
  - `report_date_range()` currently starts `week` on the selected anchor date
    and `next-week` exactly seven days later, then reports seven calendar days.
  - Reuse `yahrzeit_lighting_week_range()` or a shared companion helper rather
    than implementing a separate calendar-week calculation.
  - Preserve the sunset-based Friday boundary used by normal lighting policy.
  - Update the Reports screen/help and section 27-8 wording so the anchor-date
    behavior and reported Erev Shabbat observance period are clear.
  - Add boundary tests for Friday before sunset, Friday after sunset, and dates
    near Gregorian and Hebrew year transitions.
- Future cleanup:
  - **[B]** Consider `include/audit_support.inc.php` for panel/name validation and duplicate-location checks.
  - Keep `yahrzeit_engine.php` as the orchestration layer: parse options, select mode, read data, and call engine/report/audit functions.

### `bin/yahrzeit_scheduler`

- Mostly done for now.
- Uses explicit cron phases:
  - `--phase yizkor-on`
  - `--phase yizkor-off`
  - `--phase yahrzeit`
- **[B] (at CBS)** Confirm CBS Yizkor service timing and decide whether the scheduler remains three-phase or simplifies to two-phase.
- **[A]** Add the erev_shabbat_to_erev_shabbat weekly lighting transition.
  - When `yahrzeitLightTime = atSunset`, each successful scheduled Friday run
    calls `bin/fix-up-crontab` to calculate and install the following Friday's
    run time.
  - Keep the recurring Friday entry as a fail-safe: if rescheduling fails, the
    previous time remains and cron still attempts the next Friday run.
  - A fixed-time policy does not require weekly crontab rewriting.

### Cron installation helper

- **[A]** Replace the obsolete `bin/add-to-vixie-crontab` with an idempotent
  `bin/fix-up-crontab` for the phase-based scheduler.
  - Read the saved policy from `data/minhag.ini`; do not accept shell text
    constructed from POST values.
  - Manage only a clearly marked Yahrzeit block and preserve unrelated cron
    entries.
  - Provide a preview mode and print the resulting managed cron lines.
  - Generate the normal-Yahrzeit entry from `yahrzeitObservance`:
    - `week`: run Friday only.
    - `day`: run every day.
  - For a fixed lighting time, emit a stable weekly or daily entry.
  - When `yahrzeitLightTime = atSunset`, calculate the next occurrence's time
    and refresh the entry after each successful scheduled run.
  - Make failures explicit and do not transmit to the controller.
- **[A]** After a successful Minhag save, call `bin/fix-up-crontab`, capture its
  output, and show the installed cron lines or a clear failure on the result page.
- **[A]** Ensure the deployed web-server account updates the intended appliance
  crontab rather than accidentally creating a separate web-server-user crontab.
- **[A]** Call the same helper at the end of appliance installation so cron can
  be installed or repaired without using the web screen.
- **[A]** Document that OS upgrades or appliance migration may require rerunning it.

---

## GUI Screens

### `0yahrzeit.php`

- Works.
- Home/status dashboard now shows date/time, Hebrew date, sunset/Shabbat timing, configured policy summary, controller summary, and wall image.
- **[A]** Add a “next Yizkor observance” helper/library call.
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

- OK.
- **[B]** Implement "add" or "new" name if and only if the synagogue asks for this.

### `6reports.php`

- OK.
- Reports, audit, command preview, CSV download, and CSV upload/maintenance live here.

### `7minhag.php`

- **[A]** Complete the remaining Minhag policies through the engine, scheduler,
  help, and installed cron configuration.
  - The fixed-time/sunset controls are saved but are not currently consumed by
    the scheduler. Either implement them completely or remove them cleanly.
  - Audit every Yahrzeit and Yizkor control from the form through the policy
    engine, scheduler, help, and installed cron configuration. Do not display
    a control that merely saves an unused setting.
  - Review the simplified policy with the rabbi or Ritual Committee, favoring
    reliable automatic operation over unnecessary configurability.
- **[A]** After saving, invoke `bin/fix-up-crontab` and display its escaped output.
- **[A]** Distinguish "configuration saved" from "cron update failed" rather than
  silently leaving the old schedule installed.

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
- **[B]** Refactor `emitMessagePage()` to use CSS/div layout instead of tables.
- **[B]** Refactor `emitTopOfScreen()` to use CSS/div layout instead of tables.
- **[B]** Eventually modernize the whole table-based page shell.

### `include/names.inc.php`

- OK.
- Owns memorial CSV record parsing, mapping, and writing.

### `include/date_support.inc.php`

- OK.
- Owns date, Hebrew-date, sunset, Shabbat, and related helpers.

### `include/panels.inc.php`

- OK.
- Owns static wall/panel geometry.

### `include/leds.inc.php`

- OK.
- Owns LED/panel/controller mapping.

### `include/yahrzeit_policy.inc.php`

- Provides the active normal-yahrzeit decision and erev_shabbat-aware lighting window.

### Weekly reports: `bin/yahrzeit_engine.php`, `bin/yahrzeit`, and `6reports.php`

- Current report, panel displays, dry-run preview, and normal engine output agree for the active erev_shabbat_to_erev_shabbat window.
  - Keep report generation in the engine, wrapper argument handling in `bin/yahrzeit`, and Reports-screen form/rendering changes in `6reports.php`.

---

## Platform / Installation

### Installation

- **[A]** Install/update crontab entries for phase-based scheduler.

---

## Data / Deployment Issues

- **[A] (at CBS)** Fix known audit defect: Emile Kingsley uses unknown panel `col58`.
- **[B]** Decide whether any login/security support is needed on the deployed appliance.

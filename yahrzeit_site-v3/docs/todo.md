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
- Shared scheduler/web execution stage and controller transport.


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
- **[B] (at CBS)** Confirm CBS Yizkor service timing and decide whether the scheduler remains three-phase or simplifies to two-phase.
- **[B]** Add installer-managed rotation for `data/scheduler.log`, retaining
  approximately three months of compressed weekly logs without emailing
  routine cron output.
- **[B]** Audit end-to-end scheduled-operation logging before deployment.
  - Keep scheduler decisions, generated actions, cron-repair results, wrapper
    failures, controller address/port, and final exit status in the same log.
  - Confirm that timeout, connection-refused, broken-pipe, and other failures
    from the `slowcat | nc` transport conduit are visible and unambiguous.
  - Avoid logging the complete normal controller command stream unless a debug
    mode is deliberately enabled; record a useful summary instead.
  - Review the accumulated log after approximately three months and after the
    next Yizkor observance.

---

## GUI Screens

### `0yahrzeit.php`

- Works.
- Home/status dashboard now shows date/time, Hebrew date, server/controller
  addresses, sunset/Shabbat timing, configured policy summary, controller
  summary, and wall image.

### `1viewpanels.php`

- OK.
- Read-only wall/panel overview plus manual wall-wide operations.
- **[A]** Review the all-on, all-off, and Yizkor controls, including which
  operations require confirmation or other safeguards.

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

- **[A] (at CBS)** Review the simplified policy with the rabbi or Ritual
  Committee, favoring reliable automatic operation over unnecessary
  configurability.

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

- OK.

### `help/6reports.php`

- OK.

### `help/7minhag.php`

- OK.

### `help/user_guide.php`

- OK.

---

## Include / Library Files

### `include/misc.inc.php`

- Works.
- **[B]** Refactor `emitMessagePage()` to use CSS/div layout instead of tables.
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

---

## Data / Deployment Issues

- **[A] (at CBS)** Fix known audit defect: Emile Kingsley uses unknown panel `col58`.
- **[B]** Decide whether any login/security support is needed on the deployed appliance.

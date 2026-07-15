# Yahrzeit Site TODO / Punch List

This is the working punch list for the PHP/server-side Yahrzeit Wall application.

Priority markers:

- **[A]** near-term / important before CBS deployment
- **[B]** later cleanup / refactor / nice-to-have
- **(at CBS)** requires discussion, site access, or production data

---

## Command-Line / Scheduled Programs

### `bin/fix-up-crontab`

- OK.

### `bin/install-yahrzeit-appliance.sh`

- OK.

### `bin/yahrzeit`

- OK.

### `bin/yahrzeit_engine.php`

- Done for current deployment.
- **[A]** Correct the `week` and `next-week` report ranges to use the shared
  Erev Shabbat-to-Erev Shabbat lighting-week policy.
  - `report_date_range()` currently starts `week` on the selected anchor date
    and `next-week` exactly seven days later, then reports seven calendar days.
  - Reuse `yahrzeit_lighting_week_range()` or a shared companion helper rather
    than implementing a separate calendar-week calculation.
  - Update the Reports screen/help and section 27-8 wording so the anchor-date
    behavior and reported Erev Shabbat observance period are clear.
  - Add boundary tests for Friday before sunset, Friday after sunset, and dates
    near Gregorian and Hebrew year transitions.

### `bin/yahrzeit_scheduler`

- OK.
- **[B] (at CBS)** Review the accumulated scheduler logs after approximately
  three months and after the next Yizkor observance.

---

## GUI Screens

### `0yahrzeit.php`

- OK.

### `1viewpanels.php`

- OK.

### `3singlepanel.php`

- OK.

### `4viewnames.php`

- OK.

### `5singlename.php`

- OK.

### `6reports.php`

- OK.

### `7minhag.php`

- OK.

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

### `include/date_support.inc.php`

- OK.

### `include/leds.inc.php`

- OK.

### `include/misc.inc.php`

- OK.

### `include/names.inc.php`

- OK.

### `include/panels.inc.php`

- OK.

### `include/yahrzeit_policy.inc.php`

- OK.

---

## Data / Deployment Issues

- **[A] (at CBS)** Fix known audit defect: Emile Kingsley uses unknown panel `col58`.
- **[A] (at CBS)** Use CBS's latest yahrzeits-rev*.csv; review minhag.ini
- **[A] (at CBS)** Review the MINHAG policy with the rabbi or Ritual Committee
- **[B] (at CBS)** Confirm CBS Yizkor service timing

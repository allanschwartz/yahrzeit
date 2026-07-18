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

### `bin/install-yahrzeit.sh`

- OK.

### `bin/yahrzeit`

- OK.

### `bin/yahrzeit_engine.php`

- OK.

### `bin/yahrzeit_scheduler`

- OK.
- **[B] (at CBS)** Review the accumulated scheduler logs after approximately
  three months and after the next Yizkor observance.

---

## GUI Screens

### `0yahrzeit.php`

- OK.

### `1yizkor.php`

- OK.

### `2panels.php`

- OK.

### `3singlepanel.php`

- OK.

### `4names.php`

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

### `help/1yizkor.php`

- OK.

### `help/2panels.php`

- OK.

### `help/3singlepanel.php`

- OK.

### `help/4names.php`

- OK.

### `help/5singlename.php`

- OK.

### `help/6reports.php`

- OK.

### `help/7minhag.php`

- OK.

### `help/8userguide.php`

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

## CBS On-Site Questions and Data

- **[A] (at CBS)** Install the latest CBS data/yahrzeits-rev4.csv
- **[A] (at CBS)** Fix the known audit defect: Emile Kingsley uses unknown panel `col58`.
- **[A] (at CBS)** Obtain and install CBS's latest `yahrzeits-rev*.csv` file.
- **[A] (at CBS)** Review the saved Minhag policy with the rabbi or Ritual Committee.
- **[B] (at CBS)** Ask whether all Yizkor and memorial observances can share one lighting schedule or need separate times for each observance (for example, morning Yizkor services versus an evening Yom HaShoah event).

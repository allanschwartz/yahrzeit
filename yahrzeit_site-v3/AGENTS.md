# Yahrzeit Wall PHP Project Notes

This is a legacy PHP 8 modernization of the Congregation Beth Sholom Yahrzeit Wall control application.

Primary goals:
- Preserve behavior unless explicitly asked to change it.
- Prefer small, reviewable diffs.
- Keep procedural PHP style; do not convert to classes unless asked.
- Keep old table-based page layout unless asked.
- Do not change CSV schema unless asked.
- Do not change controller protocol unless asked.
- Use constants for page metadata where helpful.
- Prefer simple helper functions over clever abstraction.
- Avoid closures unless clearly useful.
- Do not put business logic into screen files if it belongs in include files.

Important boundaries:
- include/names.inc.php handles memorial CSV records.
- include/panels.inc.php handles static panel geometry.
- include/date_support.inc.php handles date/sunset/Hebrew-date helpers.
- include/leds.inc.php handles LED/panel mapping.
- bin/yahrzeit_engine.php decides what should be lit.
- screen PHP files should mostly render pages and dispatch GET/POST actions.

File size / maintainability guidelines:
- Prefer PHP files under roughly 500 lines.
- Prefer shell/Python scripts under roughly 200 lines.
- If a file grows beyond that, do not automatically split it, but flag it as a candidate for refactor.
- Prefer extracting coherent procedural helper files over adding classes.
- Do not split files just to satisfy a line-count rule; split only when there is a clear boundary.


Validation commands:
php -l 0yahrzeit.php
php -l 1viewpanels.php
php -l 3singlepanel.php
php -l 4viewnames.php
php -l 5singlename.php
php -l 6reports.php
php -l 7minhag.php
php -l include/misc.inc.php
php -l include/date_support.inc.php
php -l include/names.inc.php
php -l include/panels.inc.php
php -l include/leds.inc.php
bash -n bin/yahrzeit

Do not run commands that transmit to the physical controller unless explicitly asked.

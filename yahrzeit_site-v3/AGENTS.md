# Yahrzeit Wall PHP Project Notes

This is a legacy PHP 8 modernization of the Congregation Beth Sholom Yahrzeit Wall control application.

Primary goals:

- Preserve behavior unless explicitly asked to change it.
- Prefer small, reviewable diffs.
- Keep procedural PHP style; do not convert to classes unless asked.
- Keep old table-based page layout unless asked.
- Do not change CSV schema unless asked.
- Do not rewrite the live memorial CSV database unless explicitly asked and a backup/audit path is in place.
- Do not change controller protocol unless asked.
- Use constants for page metadata where helpful.
- Prefer simple helper functions over clever abstraction.
- Avoid closures unless clearly useful.
- Do not put business logic into screen files if it belongs in include files.
- When touching a file, fix obvious PHP warnings, undefined-index reads, brittle assumptions, and stale or misleading file-header documentation within the task's scope.
- Improve unclear or poorly structured functions opportunistically when doing so remains a small, behavior-preserving change; prefer clear names, small helpers, and simple control flow.
- Perform documentation cleanup only in files already being visited for the current task; do not start repository-wide comment churn.
- Add PHPDoc to shared functions and functions with non-obvious contracts, array shapes, side effects, persistence, controller communication, or calendar logic.
- Do not add boilerplate docblocks to trivial helpers. Prefer native type declarations and clear names when they adequately express the contract, and use ordinary comments to explain non-obvious reasoning.
- In PHPDoc, use the opening summary sentence instead of a Doxygen-style `@brief` tag.

Important boundaries:

- include/names.inc.php handles memorial CSV records.
- include/panels.inc.php handles static panel geometry.
- include/date_support.inc.php handles date/sunset/Hebrew-date helpers.
- include/leds.inc.php handles LED/panel mapping.
- include/yahrzeit_policy.inc.php decides which memorial records are active.
- bin/yahrzeit_engine.php applies policy and emits reports, audits, or command
  text.
- screen PHP files should mostly render pages and dispatch GET/POST actions.

File size / maintainability guidelines:

- Prefer PHP files under roughly 500 lines.
- Prefer shell/Python scripts under roughly 200 lines.
- If a file grows beyond that, do not automatically split it, but flag it as a candidate for refactor.
- Prefer extracting coherent procedural helper files over adding classes.
- Do not split files just to satisfy a line-count rule; split only when there is a clear boundary.


Validation commands:

```sh
for f in index.php [0-9]*.php include/*.inc.php help/*.php \
         bin/yahrzeit_engine.php bin/yahrzeit_scheduler bin/fix-up-crontab \
         tests/*.php; do
    php -l "$f"
done

for f in bin/yahrzeit bin/install-yahrzeit.sh; do
    bash -n "$f"
done

php tests/yahrzeit_engine_policy_test.php
```

Run `tests/yahrzeit_engine_policy_test.php` after changes to date handling,
memorial-record mapping, policy, reports, or the lighting engine. The test must
exit successfully.

Do not run commands that transmit to the physical controller unless explicitly asked.

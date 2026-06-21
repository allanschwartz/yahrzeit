# CBS Yahrzeit Wall Appliance

This repository contains the PHP web application, scheduler, and installer for the Congregation Beth Sholom Yahrzeit Wall appliance.

## What this project does

The appliance is responsible for:

- displaying the current Yahrzeit wall status,
- rendering memorial/name views and reports,
- applying synagogue lighting policy,
- running scheduled yahrzeit and Yizkor phases,
- and preparing controller command streams for the physical wall.

## Repository layout

- `bin/` — command-line tools and scheduler/engine entry points
- `include/` — shared PHP helpers for dates, names, panels, lighting policy, and page layout
- `data/` — CSV data and configuration such as `minhag.ini`
- `help/` — help pages for the web UI
- `docs/` — notes and project documentation
- `js/` — legacy browser-side helper scripts used by the screen templates

## Installation

For a fresh Ubuntu/Debian appliance, run the installer script from the project root or download it directly:

```bash
curl -fsSL https://raw.githubusercontent.com/allanschwartz/yahrzeit/master/yahrzeit_site-v3/bin/install-yahrzeit-appliance.sh -o /tmp/install-yahrzeit-appliance.sh
chmod +x /tmp/install-yahrzeit-appliance.sh
/tmp/install-yahrzeit-appliance.sh
```

The installer:

- installs required packages,
- clones or updates the repository,
- configures Apache to serve the site,
- runs PHP syntax checks and basic runtime sanity checks,
- and prints suggested cron entries.

It does not install cron automatically and does not transmit commands to the controller during tests.

## Notes

- The scheduler logic is intentionally separate from screen rendering.
- The shared lighting policy logic is used by both the engine and the GUI views.
- For production deployment, review the cron suggestions shown by the installer before enabling scheduled runs.

## License and maintenance

This project is maintained for the CBS Yahrzeit Wall appliance workflow. For deployment questions, review the installer script and the documentation in `docs/`.

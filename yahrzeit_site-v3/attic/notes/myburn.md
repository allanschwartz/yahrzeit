# Yahrzeit Controller — Task Status

Status file updated from the original February 2007 development checklist.

## Legend

| Mark | Meaning |
|---|---|
| ✅ | Complete / working for current V3 application |
| ⚠️ | Works, but known follow-up or policy decision remains |
| ⬜ | Not started / future work |
| 🗄️ | Removed or attic only; not part of current application |

`%Y%` in the old file appears to have meant “yes / complete,” but it was not self-documenting, so this file now uses Markdown check marks.

---

## PHP Web Application

| Area | File | Status | Notes |
|---|---|---:|---|
| Home / status | `0yahrzeit.php` | ✅ | Home/status page; current date, Hebrew date, sunset context, wall/database summary. |
| Panels overview | `1viewpanels.php` | ✅ | Read-only wall geometry view plus limited manual wall-wide controls. |
| Add/modify panel | `2addmodifypanel.php` | 🗄️ | Obsolete. Static panel geometry replaces editable panel workflow. |
| Single panel | `3singlepanel.php` | ✅ | Read-only database assignment view for one physical panel. |
| Names browser | `4viewnames.php` | ✅ | Read-only searchable memorial-record browser. |
| Single record | `5singlename.php` | ✅ / ⚠️ | Works; still contains legacy edit/save behavior. Future decision: make fully read-only or replace with narrow safe operations. |
| Reports / audit | `6reports.php` | ✅ | Reports, database audit, command preview, CSV download/upload. |
| Minhag settings | `7minhag.php` | ✅ | Synagogue-wide Yahrzeit/Yizkor policy; does not edit crontab. |
| Redirect | `index.php` | ✅ | Redirects to `0yahrzeit.php`. |

**PHP file status:** all current PHP application files are complete for the present V3 cleanup pass.
---

## Page Help Files

| Page | Help file | Status | Notes |
|---|---|---:|---|
| Home / status | `help/0yahrzeit.php` | ✅ | Styled standalone help page. |
| Panels overview | `help/1viewpanels.php` | ✅ | Styled standalone help page. |
| Single panel | `help/3singlepanel.php` | ✅ | Styled standalone help page. |
| Names browser | `help/4viewnames.php` | ✅ | Styled standalone help page. |
| Single record | `help/5singlename.php` | ✅ | Styled standalone help page. |
| Reports / audit | `help/6reports.php` | ✅ | Styled standalone help page. |
| Minhag settings | `help/7minhag.php` | ✅ | Styled standalone help page, including cron-maintainer guidance. |
| User guide | `help/user_guide.php` | ✅ | Overall operator/user guide. |

---

## Server-Side / Scheduling

| Area | File | Status | Notes |
|---|---|---:|---|
| Command wrapper | `bin/yahrzeit` | ✅ | Main operator/web/scheduler command path; handles transmission and direct wall operations. |
| Engine | `bin/yahrzeit_engine.php` | ✅ / ⚠️ | Works; large file. Future refactor should move date helpers and per-person helpers into smaller include files. |
| Scheduler | `bin/yahrzeit_scheduler` | ✅ | Simplified phase model: `yizkor-on`, `yizkor-off`, `yahrzeit`. |
| Crontab helper | `bin/add-to-private-crontab` | ⚠️ | Update/install process should reflect the current phase-based schedule. |

---

## Include Files

| File | Status | Notes |
|---|---:|---|
| `include/misc.inc.php` | ✅ | Shared site helpers, path helpers, layout helpers, Minhag config helpers. |
| `include/leds.inc.php` | ✅ | Low-level controller command emitters. |
| `include/names.inc.php` | ✅ | Memorial CSV mapping and record access. |
| `include/panels.inc.php` | ✅ | Static panel geometry. |

---

## Embedded Controller

| Area | Status | Notes |
|---|---:|---|
| Header comments | ✅ | `.h` files documented with module purpose and terse Doxygen-style function briefs. |
| Top-level architecture | ✅ | `yahrzeit_v3.h` documents current architecture, CLI, build geometry, and implementation history. |
| Compile status | ✅ | Clean compile after current documentation pass. |
| WiFi variant | ⬜ | Back-burner idea: runtime Ethernet/WiFi transport selection if a UNO R4 WiFi is later tested. |

---

## Platform

| Step | Status | Notes |
|---|---:|---|
| Selection | ✅ | Target appliance platform: Raspberry Pi 4 with USB SSD boot/storage, wired Ethernet, and long-life power supply. |
| Porting | ⬜ | Install current `yahrzeit_site-v3` on selected Pi 4 platform and verify PHP, cron, filesystem paths, and network access. |
| Installation | ⬜ | Install at CBS, configure fixed IP/networking, confirm controller reachability, configure crontab, and verify logs. |
| Upgrade / Maintenance | ⬜ | Prepare cloned spare SSD, document recovery procedure, and define backup/update process. |

---

## Open Follow-Up Items

- Fix known audit defect: Emile Kingsley uses unknown panel `col58`.
- Confirm CBS Yizkor service timing and decide whether the scheduler remains three-phase or simplifies to two-phase.
- Decide whether `5singlename.php` should become fully read-only.
- Decide whether Yizkor time fields in `7minhag.php` remain visible, become advisory, or are replaced by suggested cron-entry output.
- Update `bin/add-to-private-crontab` to match the final approved scheduler phase model.
- Consider later layout modernization: replace the old table-based page shell with a CSS/div layout.
- Consider future `yahrzeit_engine.php` refactor:
  - `include/date_support.inc.php`
  - expanded `include/names.inc.php`
  - possible `include/audit_support.inc.php`


## Project Management Note

This file began as a personal Scrum/status board for the Yahrzeit Wall project.
Because the project spans the web GUI, backend PHP engine, scheduler, embedded
controller, installation platform, and field deployment, this checklist helped
track many small deliverables across several self-managed sprints.

In the original 2008/2015 work, Allan acted as product owner, Scrum master,
frontend developer, backend developer, embedded developer, tester, and installer.
The file is still useful as a lightweight project burn-down/status document.

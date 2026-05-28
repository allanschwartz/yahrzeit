<?php
/*
 * NAME
 *      help/user_guide.php
 *
 * DESCRIPTION
 *      Overall operator guide for the CBS Yahrzeit Wall web application.
 *
 *      This page explains the complete system at a practical level: what the
 *      application does, how normal operation works, which screen to use for
 *      common tasks, and which actions require special care.
 */
?>

<?php
require_once "../include/misc.inc.php";

$title = "Yahrzeit Wall User Guide";
$description = "Operator guide for the CBS Yahrzeit Wall application.";
$tab = 0;         // Home
$helpfile = "";  // no nested page-help link on the guide page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle">Yahrzeit Wall User Guide</div>

    <div class="helpBody">

<p>
This guide explains the CBS Yahrzeit Wall web application at an operator
level. It is intended for office staff, ritual committee members, and
technical volunteers who maintain the memorial database, review reports,
check wall assignments, or perform occasional manual wall operations.
</p>

<h3>What This Application Does</h3>

<p>
The Yahrzeit Wall application maintains the memorial-name database, applies
Congregation Beth Sholom lighting policy, generates reports, audits memorial
locations, and sends command streams to the embedded wall controller.
</p>

<p>
In normal operation, no one needs to manually light the wall each day. The
scheduled automation decides when lighting should change and uses the same
command path available from the web application and command line.
</p>

<h3>System Overview</h3>

<p>
The system has three main parts:
</p>

<ul>
    <li><strong>Yahrzeit Wall</strong> &mdash; the physical memorial wall, panels, names, and LEDs.</li>
    <li><strong>Yahrzeit web application</strong> &mdash; the PHP application that maintains data, reports, settings, and operator screens.</li>
    <li><strong>Embedded controller</strong> &mdash; the Arduino-based controller that receives commands and drives the LED wall.</li>
</ul>

<p>
The normal scheduled-control path is:
</p>

<pre>
cron
    -> bin/yahrzeit_scheduler
        -> bin/yahrzeit
            -> bin/yahrzeit_engine.php
                -> controller command stream
            -> nc
                -> Arduino V3 controller
</pre>

<p>
In practical terms: cron decides when scheduled phases run,
<code>yahrzeit_scheduler</code> decides whether the requested phase applies
today, <code>yahrzeit</code> decides how to run and transmit the action, and
<code>yahrzeit_engine.php</code> decides what names should be lit, audited, or
reported.
</p>

<h3>Normal Operation</h3>

<p>
Normal yahrzeit and Yizkor lighting is automatic. Cron runs a small number of
named scheduler phases at fixed times. Each scheduler phase either performs
one action or does nothing.
</p>

<pre>
11:00 AM   yahrzeit_scheduler --phase yizkor-on
           If today is a configured Yizkor day, turn on Yizkor lighting.

1:00 PM    yahrzeit_scheduler --phase yizkor-off
           If today is a configured Yizkor day, restore normal yahrzeit lighting.

4:00 PM    yahrzeit_scheduler --phase yahrzeit
           Run normal yahrzeit lighting.
</pre>

<p>
The exact times may be adjusted in the appliance cron table to match
Congregation Beth Sholom practice.
</p>

<p>
The web application is mainly used for review, maintenance, reporting,
auditing, and occasional manual operation. Routine daily lighting should not
require manual web-page action.
</p>

<h3>Application Pages</h3>

<p>
Each page has its own help link. The brief summary below explains when to use
each page.
</p>

<ul>
    <li><strong>Home</strong> &mdash; shows current date, Hebrew date, sunset-related times, wall configuration, and memorial database summary information.</li>
    <li><strong>Panels</strong> &mdash; shows the physical panel layout and provides limited wall-wide manual lighting controls.</li>
    <li><strong>Single Panel</strong> &mdash; shows one physical panel as a database view of assigned memorial names and locations.</li>
    <li><strong>Names</strong> &mdash; searches and reviews memorial records.</li>
    <li><strong>Single Name</strong> &mdash; reviews one memorial record in detail.</li>
    <li><strong>Reports</strong> &mdash; runs yahrzeit reports, audits the database, previews controller commands, and maintains the CSV memorial database.</li>
    <li><strong>Minhag</strong> &mdash; edits synagogue-wide Yahrzeit and Yizkor lighting policy.</li>
</ul>

<h3>Common Tasks</h3>

<p>
To find a memorial name, use the Names page. To see where a person is assigned
on the wall, open the memorial record or view the relevant panel.
</p>

<p>
To review upcoming observances, use the Reports page. Day, week, next-week,
month, and next-month reports list names whose yahrzeits fall in the selected
period.
</p>

<p>
To check the integrity of the memorial database, use the Reports page to run
the audit. The audit checks for unknown panel IDs, invalid row or column
values, malformed records, and duplicate assignments to the same LED location.
</p>

<p>
To preview what would be sent to the controller, use the command preview on
the Reports page. Preview does not transmit commands to the wall.
</p>

<p>
To replace the memorial database, use the CSV upload tool on the Reports page.
The current CSV file is backed up before replacement, and an audit is run
after upload.
</p>

<p>
To change synagogue-wide lighting policy, use the Minhag page. These settings
affect the entire wall and should match Congregation Beth Sholom practice.
</p>

<h3>Safety Notes</h3>

<p>
Some operations affect the live wall or the live database. Use extra care with
these actions:
</p>

<ul>
    <li><strong>Manual wall-wide lighting controls</strong> send commands to the controller immediately.</li>
    <li><strong>CSV upload</strong> replaces the live memorial database.</li>
    <li><strong>Minhag changes</strong> affect scheduling and lighting policy for the whole wall.</li>
    <li><strong>Panel views</strong> show database assignments, not live electrical LED status.</li>
</ul>

<p>
After changing memorial data, run the audit before relying on scheduled
lighting. After changing Yizkor or timing settings, review reports or previews
as appropriate to confirm the intended behavior.
</p>

<h3>Manual Wall Operations</h3>

<p>
The Panels page may provide manual wall-wide operations such as all on, all
off, and Yizkor. These controls are useful for testing, maintenance, or
special operator action. They are not normally required for daily scheduled
operation.
</p>

<p>
Because these controls act immediately, use them only when intentionally
changing the current wall display.
</p>

<h3>Database and Files</h3>

<p>
The live memorial database is stored as a CSV file:
</p>

<pre>
data/yahrzeits-rev4.csv
</pre>

<p>
The synagogue lighting-policy settings are stored in:
</p>

<pre>
data/minhag.ini
</pre>

<p>
Normal web pages read these files through the application include files rather
than parsing them directly. The Reports page provides the safest web path for
CSV download, upload, audit, and preview.
</p>

<h3>Technical Overview</h3>

<p>
The web application and embedded controller are intentionally layered. On the
site side, the scheduler decides when to act, the wrapper decides how to run
and transmit, and the engine decides what names should be lit or reported.
</p>

<p>
On the embedded controller side, the network or serial input code collects
command lines, the command processor parses them, the wall abstraction applies
logical row/column/panel operations, and the low-level pixel driver updates
the hardware.
</p>

<pre>
socket_thread / serial_thread
    -> CmdProc
        -> LedWall
            -> YyzPixel
</pre>

<h3>Troubleshooting</h3>

<p>
If a report looks wrong, first confirm the selected date and then check the
Minhag settings that control date method and timing.
</p>

<p>
If a name appears in the wrong wall position, run the database audit from the
Reports page. Then correct the CSV memorial database and audit again.
</p>

<p>
If the wall display does not change after a manual operation, check whether
the controller is reachable and whether the command wrapper can connect to the
configured controller host and port.
</p>

<p>
If the controller is reachable but the visible wall does not match the
expected display, use command preview, controller status, and manual test
operations carefully to separate database, scheduling, transport, and hardware
issues.
</p>

<h3>When in Doubt</h3>

<p>
For ordinary review, use Names, Panels, and Reports. For live-wall changes,
CSV upload, or Minhag changes, proceed deliberately and run audit or preview
checks afterward.
</p>

    </div>
</div>

<?php
emitPageCopyright2();
emitFooter();
?>

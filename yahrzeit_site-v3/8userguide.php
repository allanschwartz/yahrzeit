<?php
/*
 * NAME
 *      8userguide.php
 *
 * DESCRIPTION
 *      Overall operator guide for the synagogue's Yahrzeit Wall web
 *      application.
 *
 *      This page explains the complete system at a practical level: what the
 *      application does, how normal operation works, which screen to use for
 *      common tasks, and which actions require special care.
 */
?>

<?php
require_once "include/misc.inc.php";

$minhag = read_minhag_ini();
$synagogueName = trim($minhag['synagogueName'] ?? "");
if ($synagogueName === "") {
    $synagogueName = "Synagogue";
}

$title = "Yahrzeit Wall User Guide";
$description = "Operator guide for the " . h($synagogueName) .
               " Yahrzeit Wall application.";
$tab = 8;
$helpfile = "";  // no nested page-help link on the guide page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle boldText">Yahrzeit Wall User Guide</div>

<div class="helpBody">

<p>
This guide explains the <?php echo h($synagogueName); ?> Yahrzeit Wall web
application at an operator level. It is intended for office staff, ritual
committee members, and technical volunteers who maintain the memorial
database, review reports, check wall assignments, or perform occasional
manual wall operations.
</p>

<h3>What This Application Does</h3>

<p>
The Yahrzeit Wall application maintains the memorial-name database, applies
<?php echo h($synagogueName); ?> lighting policy, generates reports, audits
memorial locations, and sends command streams to the embedded wall
controller.
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
                -> Yahrzeit controller (Arduino)
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
named scheduler phases at the times saved on the Minhag page. Each scheduler
phase either performs one action or does nothing.
</p>

<pre>
11:00 AM   yahrzeit_scheduler --phase yizkor-on
           If today is a configured Yizkor day, turn on Yizkor lighting.

1:00 PM    yahrzeit_scheduler --phase yizkor-off
           If today is a configured Yizkor day, restore normal yahrzeit lighting.

Friday near sunset (weekly observance), or daily (day-only observance):
           yahrzeit_scheduler --phase yahrzeit
</pre>

<p>
The application maintains its own marked block in the appliance crontab.
Fixed times remain stable. Sunset-based times are recalculated automatically
as sunset changes through the year.
</p>

<p>
The web application is mainly used for review, maintenance, reporting,
auditing, and occasional manual operation. Routine daily lighting should not
require manual web-page action.
</p>

<h3>Common Tasks</h3>

<p>
To find a memorial name, use the
<a href="4names.php">Names page</a>. To see where a person
is assigned on the wall, open the
<a href="5singlename.php">memorial record</a> or view the
<a href="3singlepanel.php">relevant panel</a>.
</p>

<p>
To review upcoming Yahrzeit observances, use the
<a href="6reports.php">Reports page</a>. Day, week,
next-week, month, and next-month reports list names whose yahrzeits fall in the
selected period. The next enabled Yizkor dates are shown on the
<a href="1yizkor.php">Yizkor page</a> and in the
<a href="7minhag.php">Minhag preview</a>.
</p>

<p>
To check the integrity of the memorial database, use the
<a href="6reports.php">Reports page</a> to run the audit.
The audit checks for unknown panel IDs, invalid row or column values, malformed
records, and duplicate assignments to the same LED location.
</p>

<p>
To preview what would be sent to the controller, use the command preview on
the <a href="6reports.php">Reports page</a>. Preview does
not transmit commands to the wall.
</p>

<p>
To replace the memorial database, use the CSV upload tool on the
<a href="6reports.php">Reports page</a>. The current CSV
file is backed up before replacement, and an audit is run after upload.
</p>

<p>
To change synagogue-wide lighting policy, use the
<a href="7minhag.php">Minhag page</a>. These settings affect
the entire wall and should match <?php echo h($synagogueName); ?> practice.
</p>

<h3>Page-by-Page Guide</h3>

<p>
Each page has its own help link. The descriptions below explain what each page
shows and when to use it.
</p>

<div class="guidePageSummary">
    <a href="0yahrzeit.php">
        <img
            class="guidePageThumbnail"
            src="screenshots/Screenshot 0yahrzeit.png"
            alt="Yahrzeit Home page"
        >
    </a>

    <h4><a href="0yahrzeit.php">Yahrzeit Wall Home</a></h4>

    <p>
    The Yahrzeit Home page provides an at-a-glance summary of the appliance
    and its current operating context. It shows the civil and Hebrew dates,
    the server and controller addresses, the next scheduled lighting event,
    and a summary of the saved lighting policy.
    </p>

    <p>
    It also reports the number of configured panels and memorial records, and
    how many Yahrzeit lights should currently be lit. Use this page as the
    starting point when checking the system or confirming its configuration.
    Links in the summary lead to the appropriate maintenance pages.
    </p>

    <p>
    See also
    <a href="help/0yahrzeit.php" target="pagehelp">Yahrzeit Wall Home Help</a>.
    </p>
</div>

<div class="guidePageSummary">
    <a href="1yizkor.php">
        <img
            class="guidePageThumbnail"
            src="screenshots/Screenshot 2yizkor.png"
            alt="Yizkor and Manual Lighting page"
        >
    </a>

    <h4><a href="1yizkor.php">Yizkor and Manual Lighting</a></h4>

    <p>
    The Yizkor page lists the next enabled Yizkor observances, in upcoming
    date order. The dates reflect the Yom Kippur, Shemini Atzeret, Pesach, 
    Shavuot, and optional additional observances selected on the Minhag page.
    </p>

    <p>
    This page also provides immediate control of the complete Yahrzeit Wall.
    <strong>Turn all lights off</strong> and
    <strong>Turn all lights on</strong> are intended for testing and
    maintenance. <strong>Yahrzeit lights</strong> restores the normal
    date-based display, while <strong>Yizkor lights</strong> turns on the
    full-wall memorial display.
    </p>

    <p>
    Normal Yahrzeit and Yizkor lighting is automatic, so these controls are
    not ordinarily needed. They may also be used for an intentional operator
    action during a service. Each button changes the wall immediately.
    </p>

    <p>
    See also
    <a href="help/1yizkor.php" target="pagehelp">Yizkor and Manual Lighting Help</a>.
    </p>
</div>

<div class="guidePageSummary">
    <a href="2panels.php">
        <img
            class="guidePageThumbnail"
            src="screenshots/Screenshot 1panels.png"
            alt="View Yahrzeit Panels page"
        >
    </a>

    <h4><a href="2panels.php">View Yahrzeit Panels</a></h4>

    <p>
    The Panels page shows the physical layout of the
    <?php echo h($synagogueName); ?> Yahrzeit Wall. Each glass panel in the
    photograph can be selected; click a panel in the photograph to open a
    detailed view of the memorial names assigned to it.
    </p>

    <p>
    The table below the photograph provides another way to select a panel. It
    lists each panel ID, its row-and-column geometry, its total memorial
    capacity, and the number of LEDs that should currently be lit. Click a
    panel ID to open the same detailed panel view.
    </p>

    <p>
    Use this page to locate a panel, review the overall wall configuration, or
    begin checking database assignments against the physical wall. The panel
    geometry represents the installed wall and is not edited from this page.
    </p>

    <p>
    See also
    <a href="help/2panels.php" target="pagehelp">View Yahrzeit Panels Help</a>.
    </p>
</div>

<div class="guidePageSummary">
    <a href="3singlepanel.php?panel=col1a">
        <img
            class="guidePageThumbnail"
            src="screenshots/Screenshot 3singlepanel.png"
            alt="Single Yahrzeit Panel page"
        >
    </a>

    <h4>
        <a href="3singlepanel.php?panel=col1a">Single Yahrzeit Panel</a>
    </h4>

    <p>
    The Single Panel page shows the memorial names assigned to one physical
    glass panel of the <?php echo h($synagogueName); ?> Yahrzeit Wall. Its
    grid follows the installed rows and columns, so each screen position
    corresponds to one possible memorial-light position on the panel.
    </p>

    <p>
    Occupied positions show the assigned memorial name and an LED symbol
    indicating whether that memorial light should currently be lit. Empty
    positions remain open. The summary below the grid gives the total number
    of memorial lights that should be lit on the selected panel.
    </p>

    <p>
    Use this view to compare database assignments with the names etched on the
    physical glass panel. Click any memorial name to open that individual
    record for closer review or correction. If a position appears duplicated
    or invalid, run the database audit from the Reports page.
    </p>

    <p>
    See also
    <a href="help/3singlepanel.php" target="pagehelp">Single Panel Help</a>.
    </p>
</div>

<div class="guidePageSummary">
    <a href="4names.php">
        <img
            class="guidePageThumbnail"
            src="screenshots/Screenshot 4names.png"
            alt="Yahrzeit Names page"
        >
    </a>

    <h4><a href="4names.php">Yahrzeit Names</a></h4>

    <p>
    The Names page is the main viewer for the memorial database. It lists all
    memorial records and provides a search field for finding people by name,
    English or Hebrew date, observance option, or panel location. Searching is
    not case-sensitive.
    </p>

    <p>
    More than one search term may be entered. A record is displayed only when
    all the terms are found, even if they occur in different fields. For
    example, a surname may be combined with a year, Hebrew month, or panel ID
    to narrow the results. Select <strong>Clear</strong> to display all
    memorial records again.
    </p>

    <p>
    Each result shows the memorial name, stored English and Hebrew dates,
    special observance options, and assigned panel location. The Lit symbol
    indicates whether the memorial light should currently be lit. Click a
    person's name to open the individual memorial record for review or
    correction.
    </p>

    <p>
    New memorial names have historically been added in batches, rather than
    one at a time from this page. Use the
    <a href="6reports.php">Reports page</a> to download the current CSV
    database, edit the downloaded file while preserving its CSV format, and
    upload the replacement file. Records are removed through the same
    controlled CSV-replacement process. The upload operation first backs up
    the current database and then audits the replacement.
    </p>

    <p>
    See also
    <a href="help/4names.php" target="pagehelp">View Names Help</a>.
    </p>
</div>

<div class="guidePageSummary">
    <a href="5singlename.php?row=0">
        <img
            class="guidePageThumbnail"
            src="screenshots/Screenshot 5singlename.png"
            alt="Single Yahrzeit Name page"
        >
    </a>

    <h4>
        <a href="5singlename.php?row=0">Single Yahrzeit Name</a>
    </h4>

    <p>
    The Single Name page displays one individual memorial record and permits
    careful corrections to that record. It shows the person's name, stored
    English and Hebrew dates, and the panel, column, and row assigned to the
    memorial light.
    </p>

    <p>
    The observance choices specify whether the family follows the English or
    Hebrew Yahrzeit date and whether additional memorial observances apply.
    A normal record uses automatic, calendar-driven lighting. A location
    reserved for future use may instead be marked <strong>Reserved</strong>,
    which keeps its light off.
    </p>

    <p>
    Use this page for individual corrections, such as fixing a name, date, or
    panel location. After selecting <strong>Save</strong>, the application
    backs up the current memorial database, replaces only the selected record,
    and automatically runs the database audit. Review any reported audit
    problem before making another change.
    </p>

    <p>
    See also
    <a href="help/5singlename.php" target="pagehelp">Memorial Record Help</a>.
    </p>
</div>

<div class="guidePageSummary">
    <a href="6reports.php">
        <img
            class="guidePageThumbnail"
            src="screenshots/Screenshot 6reports.png"
            alt="Yahrzeit Reports page"
        >
    </a>

    <h4><a href="6reports.php">Yahrzeit Reports</a></h4>

    <p>
    The Reports page generates lists of memorial names whose Yahrzeits fall
    on a selected day, during this or next week, or during this or next month.
    The anchor date determines which day, week, or month is selected.
    </p>

    <p>
    Week reports use the Erev Shabbat-to-Erev Shabbat observance period. The
    displayed civil dates run from Saturday through Friday. When the anchor
    date is Friday, <strong>This week</strong> begins that evening, while
    <strong>Next week</strong> begins the following Friday evening.
    </p>

    <p>
    The diagnostic tools check the relationship between the memorial database
    and the physical wall. <strong>Audit Database</strong> reports invalid
    panel IDs, row or column values outside the panel geometry, malformed
    records, and duplicate LED assignments. <strong>Preview Commands</strong>
    shows the controller commands that would be generated today without
    transmitting them to the wall.
    </p>

    <p>
    The Reports page also maintains the complete CSV memorial database.
    <strong>Download CSV</strong> saves a copy outside the appliance.
    <strong>Upload CSV</strong> installs a replacement database for batch
    maintenance. Before replacement, the current database is backed up; after
    replacement, the new database is automatically audited.
    </p>

    <div class="guideReportExample">
        <p>
        <strong>Example &mdash; Next Week:</strong>
        With Friday, July 17, 2026 as the anchor date, this report selects the
        following observance week: Saturday, July 25 through Friday, July 31.
        Each row gives the observance date, memorial name, Hebrew and English
        dates, wall location, and date option.
        </p>

        <img
            class="guideExampleScreenshot"
            src="screenshots/Screenshot 6reports, NEXT WEEK.png"
            alt="Example Next Week Yahrzeit report"
        >
    </div>

    <p>
    See also
    <a href="help/6reports.php" target="pagehelp">Reports Help</a>.
    </p>
</div>

<div class="guidePageSummary">
    <a href="7minhag.php">
        <img
            class="guidePageThumbnail"
            src="screenshots/Screenshot 7minhag.png"
            alt="Yahrzeit Minhag page"
        >
    </a>

    <h4><a href="7minhag.php">Yahrzeit Minhag</a></h4>

    <p>
    The Minhag page records synagogue-wide Yahrzeit and Yizkor practices. It
    also stores the synagogue name and affiliation displayed by the
    application. Because these settings affect the entire wall, they should
    be changed only when the intended synagogue practice is understood.
    </p>

    <p>
    The Yahrzeit settings choose whether regular observances follow the
    English or Hebrew date, whether lighting changes at a fixed time or a
    selected number of minutes before sunset, and whether each Yahrzeit is
    observed for its day only or for the full Erev Shabbat-to-Erev Shabbat
    week.
    </p>

    <p>
    The Yizkor settings enable the four annual Yizkor observances and select
    the appropriate Pesach and Shavuot days. An optional additional Hebrew
    date may be configured for another full-wall memorial observance. The
    preview lists the next civil date of every currently saved observance.
    Yizkor on/off times may use fixed clock times or a window around sunset.
    </p>

    <p>
    Selecting <strong>Save</strong> writes the revised settings and
    regenerates the appliance's managed lighting schedule. The result page
    displays the installed schedule or reports that schedule installation
    failed. After changing observance dates or lighting times, review the
    preview and resulting schedule before relying on automatic operation.
    </p>

    <p>
    See also
    <a href="help/7minhag.php" target="pagehelp">Minhag Settings Help</a>.
    </p>
</div>

<h3>Safety Notes</h3>

<p>
Some operations affect the live wall or the live database. Use extra care with
these actions:
</p>

<ul>
    <li><strong>Manual wall-wide lighting controls</strong> send commands to the controller immediately.</li>
    <li><strong>CSV upload</strong> replaces the live memorial database.</li>
    <li><strong>Minhag changes</strong> affect scheduling and lighting policy for the whole wall.</li>
</ul>

<p>
After changing memorial data, run the audit before relying on scheduled
lighting. After changing Yizkor or timing settings, review reports or previews
as appropriate to confirm the intended behavior.
</p>

<h3>Manual Wall Operations</h3>

<p>
The Yizkor page provides manual wall-wide operations: all on, all off,
the normal Yahrzeit display, and full-wall Yizkor lighting. These controls
are useful for testing, maintenance, or special operator action. They are not
normally required for daily scheduled operation.
</p>

<p>
Manual restoration may be useful after testing, maintenance, a power
interruption, or an incorrect wall display. After using
<strong>Turn all lights on</strong>, <strong>Turn all lights off</strong>, or
<strong>Yizkor lights</strong>, select <strong>Yahrzeit lights</strong> to
recalculate and restore the normal date-based lighting pattern.
</p>

<p>
Because these controls act immediately, use them only when intentionally
changing the current wall display. See the
<a href="1yizkor.php">Yizkor page</a> for these operations.
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

<h3>Modifying the Memorial Database</h3>

<p>
<strong>Correcting one memorial record:</strong>
</p>

<ol>
    <li>Open the <a href="4names.php">Names page</a> and locate the memorial using the search field.</li>
    <li>Select the person's name, make the required correction on the Single Yahrzeit Name page, and select <strong>Save</strong>.</li>
    <li>Review the audit result shown after the record is saved. The application creates a timestamped backup before replacing the record.</li>
    <li>Open the affected Single Panel page and compare the displayed name and position carefully with the actual etched-glass panel.</li>
</ol>

<p>
<strong>Adding records or making batch changes:</strong>
</p>

<ol>
    <li>On the <a href="6reports.php">Reports page</a>, select <strong>Download CSV</strong> and retain the downloaded file as a backup.</li>
    <li>Make the required changes to a working copy of the CSV file. Preserve the existing columns, field meanings, and panel-location format.</li>
    <li>Select <strong>Upload CSV</strong> and choose the corrected file.</li>
    <li>Review the audit results displayed after the upload. If errors are reported, correct the working copy and upload it again.</li>
    <li>Review every affected panel on the Single Panel page and compare it with the corresponding etched-glass panel.</li>
</ol>

<p>
The server backs up the existing live database before installing an uploaded
replacement. That local backup does not replace the external downloaded copy
described below.
</p>

<h3>Backup</h3>

<p>
Use <strong>Download CSV</strong> on the Reports page to save a copy of the
current memorial database. Make a backup after significant database changes
and before performing a batch update. Add the date to the downloaded filename
and keep the copy somewhere other than on the Yahrzeit appliance.
</p>

<p>
The application also makes a timestamped local backup before replacing the
database during an upload. These local copies provide protection against an
incorrect upload, but they do not replace an off-appliance backup that remains
available if the appliance or its storage fails.
</p>

<h3>Restore</h3>

<p>
To restore the memorial database, use <strong>Choose File</strong> on the
Reports page to select a known-good backup, and then select
<strong>Upload CSV</strong>. The application backs up the current database
before replacing it and automatically audits the restored database.
</p>

<p>
Review the audit results before relying on reports or scheduled lighting. If
the audit reports errors, correct the CSV file or restore a different backup
and upload it again.
</p>

<h3>Maintenance</h3>

<p>
The Yahrzeit Wall requires little routine physical maintenance. While the wall
is operating correctly, no internal maintenance of the LED boards,
interconnect cables, or controller is recommended.
</p>

<p>
Routine maintenance consists primarily of:
</p>

<ul>
    <li>Keeping the Yahrzeit appliance, Yahrzeit Controller, and wall power supplies continuously powered and connected to the wired network.</li>
    <li>Maintaining a recent external copy of the memorial database.</li>
    <li>Reviewing the Home page and <code>data/automation.log</code> after an appliance replacement, operating-system update, network change, or power interruption.</li>
    <li>Testing the Yizkor pattern before a Yizkor observance. Confirm that the complete wall illuminates, and then select <strong>Yahrzeit lights</strong> to restore the normal display.</li>
</ul>

<p>
The appliance calculates the Yahrzeit and Yizkor dates automatically. The
schedule does not require annual entry of calendar dates.
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
If the wall does not display the expected pattern, check the system in this
order:
</p>

<ol>
    <li>Confirm power to the Yahrzeit appliance, Yahrzeit Controller, and wall power supplies.</li>
    <li>Confirm that the appliance and controller are connected to the synagogue Ethernet network.</li>
    <li>Open the browser interface. Review the Home page, the expected lighting pattern, the memorial database, and the Minhag settings.</li>
    <li>Run the database audit from the Reports page and review <code>data/automation.log</code>.</li>
    <li>Have a technical administrator verify communication between the appliance and the Yahrzeit Controller.</li>
</ol>

<p>
If only one LED or a localized group of LEDs fails while the rest of the wall
operates normally, the likely cause is a physical LED, pixel-board, or
interconnect-cable fault. Investigate the physical wall only after power,
software, database, and controller communication have been verified.
</p>

<p>
If a report looks wrong, confirm its anchor date and check the Minhag settings
that control the date method and observance period. If a name appears in the
wrong wall position, run the database audit, correct the memorial record or
CSV database, and audit again.
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

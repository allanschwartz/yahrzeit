<?php
/*
 * NAME
 *      help/0yahrzeit.php
 *
 * DESCRIPTION
 *      Help page for the Yahrzeit Wall home/status screen.
 *
 *      This page explains the summary information displayed by 0yahrzeit.php
 *      and points maintainers to the appropriate maintenance screens.
 */
?>

<?php
require_once "../include/misc.inc.php";

// help/0yahrzeit.php
$title = "Yahrzeit Wall Home Help";
$description = "Help for the Yahrzeit Wall home and status page.";
$tab = 4;         // Reports
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle">Yahrzeit Wall Home Help</div>

    <div class="helpBody">

<p>
The Home page is the main status page for the CBS Yahrzeit Wall web
application. It summarizes the current date, scheduling context, wall
configuration, and memorial database.
</p>

<h3>Current Date Information</h3>

<p>
The page may show the current English/Gregorian date, Hebrew date, sunset
time, and upcoming Shabbat-related times. These values help explain the
current scheduling context for yahrzeit and Yizkor lighting.
</p>

<h3>Wall Summary</h3>

<p>
The page may show summary counts for configured wall panels and memorial
records. These counts come from the application’s panel geometry and memorial
database.
</p>

<h3>Navigation</h3>

<p>
Use the navigation tabs to reach the main maintenance screens.
</p>

<ul>
    <li><strong>Panels</strong> — view the physical panel layout and use limited manual wall-wide lighting controls.</li>
    <li><strong>Names</strong> — search and review memorial records.</li>
    <li><strong>Reports</strong> — run yahrzeit reports, audit the database, preview controller commands, and maintain the CSV database.</li>
    <li><strong>Minhag</strong> — review or update synagogue-wide lighting policy.</li>
</ul>

<h3>Informational Only</h3>

<p>
The Home page does not edit records, change Minhag settings, or send commands
to the wall. Use the appropriate maintenance screen for those actions.
</p>

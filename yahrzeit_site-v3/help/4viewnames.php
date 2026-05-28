<?php
/*
 * NAME
 *      help/4viewnames.php
 *
 * DESCRIPTION
 *      Help page for the memorial-name browser.
 *
 *      This page explains how to search and review memorial records on
 *      4viewnames.php.
 */
?>

<?php
require_once "../include/misc.inc.php";

// help/4viewnames.php
$title = "View Names Help";
$description = "Help for searching and reviewing memorial records.";
$tab = 3;         // Reports
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle">View Names Help</div>

    <div class="helpBody">


<p>
The View Names page is a read-only browser for the Yahrzeit Wall memorial
database.
</p>

<h3>Searching</h3>

<p>
Use the search box to find memorial records by name or other displayed text.
Leave the search box empty to show all records.
</p>

<h3>Displayed Information</h3>

<p>
Each row shows the memorialized person, the stored English/Gregorian date,
the stored Hebrew date, any special options, and the physical panel location
assigned to that person.
</p>

<h3>Editing Records</h3>

<p>
This page does not edit memorial records. Use the Reports page to download
or upload the CSV memorial database, and run an audit after any replacement
file is installed.
</p>

<h3>Location</h3>

<p>
The location column identifies the panel, row, and column for the LED assigned
to the memorial record. If a location looks wrong, run the database audit from
the Reports page.
</p>

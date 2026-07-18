<?php
/*
 * NAME
 *      help/4names.php
 *
 * DESCRIPTION
 *      Help page for the memorial-name viewer.
 *
 *      This page explains how to search and review memorial records on
 *      4names.php.
 */
?>

<?php
require_once "../include/misc.inc.php";

// help/4names.php
$title = "View Names Help";
$description = "Help for searching and reviewing memorial records.";
$tab = 4;         // Names
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle boldText">Yahrzeit Names</div>

    <div class="helpBody">


<p>
The View Names page is a read-only viewer of the Yahrzeit Wall memorial
database.
</p>

<h3>Searching</h3>

<p>
Use the search box to find memorial records by name, English or Hebrew date,
observance option, or panel location. Searching is not case-sensitive.
</p>

<p>
You may enter more than one search term. Only records containing all of the
terms are displayed; the terms may match different fields and may be entered
in any order. For example, <code>Cohen 1945</code> finds records containing
both &ldquo;Cohen&rdquo; and &ldquo;1945,&rdquo; while
<code>Tishri col2b</code> finds records containing both &ldquo;Tishri&rdquo;
and the panel location &ldquo;col2b.&rdquo;
</p>

<p>
Select <strong>Clear</strong>, or leave the search box empty, to display all
records again.
</p>

<h3>Displayed Information</h3>

<p>
Each row shows the memorialized person, the stored English/Gregorian date,
the stored Hebrew date, any special options, and the physical panel location
assigned to that person. The Lit column indicates whether the memorial light
should currently be lit according to the memorial record and synagogue
lighting policy. It is not a live electrical reading from the controller.
</p>

<h3>Editing Records</h3>

<p>
This page does not provide controls for adding or deleting memorial records.
</p>

<p>
Each name is a link to a page where you can view and edit the individual
memorial record.
</p>

<p>
Use the <a href="../6reports.php">Reports</a> page to download or upload the
CSV memorial database, and run an audit after any replacement file is
installed.
</p>

<h3>Location</h3>

<p>
The location column identifies the panel, row, and column for the LED assigned
to the memorial record. If a location looks wrong, run the database audit from
the Reports page.
</p>

   </div>
</div>

<?php
emitPageCopyright2();
emitFooter();
?>

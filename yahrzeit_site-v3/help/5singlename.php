<?php
/*
 * NAME
 *      help/5singlename.php
 *
 * DESCRIPTION
 *      Help page for the single memorial-record detail screen.
 *
 *      This page explains the fields shown by 5singlename.php and cautions
 *      maintainers about legacy edit behavior.
 */
?>

<?php
require_once "../include/misc.inc.php";

// help/5singlename.php
$title = "Memorial Record Help";
$description = "Help for reviewing one memorial record in detail.";
$tab = 4;         // Names
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle boldText">Single Yahrzeit Name</div>

    <div class="helpBody">


<p>
The Single Name page shows the details for one memorial record in the
Yahrzeit Wall database.
</p>

<h3>Displayed Information</h3>

<p>
The page may show the memorialized person’s name, English/Gregorian date,
Hebrew date, special options, and assigned wall location.
</p>

<h3>Location</h3>

<p>
The location identifies the panel, row, and column for the LED assigned to
this memorial record. If the location appears incorrect, run the database
audit from the Reports page.
</p>

<h3>Editing</h3>

<p>
Use this page to correct one memorial record and its automatic observance
settings. Individual lights are not manually controlled from a memorial
record. Routine lighting is calculated automatically from the saved dates and
synagogue policy. For batch database maintenance, use the Reports page to
download, edit, upload, and audit the CSV memorial database.
</p>

<h3>After Changes</h3>

<p>
After saving a change, the application automatically audits the memorial
database. If the audit reports a problem, use the Reports page to review the
details before making another edit.
</p>

   </div>
</div>

<?php
emitPageCopyright2();
emitFooter();
?>

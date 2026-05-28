<?php
/*
 * NAME
 *      help/3singlepanel.php
 *
 * DESCRIPTION
 *      Help page for the single-panel detail view.
 *
 *      This page explains the read-only panel grid displayed by
 *      3singlepanel.php.
 */
?>

<?php
require_once "../include/misc.inc.php";

// help/3singlepanel.php
$title = "Single Panel Help";
$description = "Help for viewing one physical panel and its assigned memorial names.";
$tab = 2;         // panels
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle">Single Panel Help</div>

    <div class="helpBody">

<p>
The Single Panel page shows one physical panel of the CBS Yahrzeit Wall.
It displays the memorial names assigned to positions on that panel.
</p>

<h3>Panel Grid</h3>

<p>
Each cell represents one possible LED location on the selected panel.
Occupied locations show the memorial name assigned to that position.
Open locations are shown as empty or open.
</p>

<h3>Assigned Names</h3>

<p>
The names shown on this page come from the memorial database. The row and
column layout comes from the fixed panel geometry.
</p>

<h3>Not a Live Controller View</h3>

<p>
This page shows database assignments. It does not query the controller and
does not prove whether an LED is currently on, off, working, or physically
wired correctly.
</p>

<h3>Finding Problems</h3>

<p>
If a name appears in the wrong position, or if two names appear to share the
same position, use the Reports page to run the database audit. Correct the
CSV memorial database and audit again before relying on scheduled lighting.
</p>

<h3>Editing</h3>

<p>
This page is read-only. Panel geometry is fixed application data, and normal
memorial database maintenance is handled through the Reports page.
</p>

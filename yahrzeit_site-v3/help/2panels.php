<?php
/*
 * NAME
 *      help/2panels.php
 *
 * DESCRIPTION
 *      Help page for the panel overview screen.
 *
 *      This page explains the wall overview and panel links available from
 *      2panels.php.
 */
?>

<?php
require_once "../include/misc.inc.php";

// help/2panels.php
$title = "View Panels Help";
$description = "Help for viewing the physical Yahrzeit Wall and its panels.";
$tab = 3;         // Panels
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle boldText">Yahrzeit Panels</div>

    <div class="helpBody">

<p>
The View Panels page shows the physical layout of the CBS Yahrzeit Wall.
The photograph below is also a way to select a panel. Click anywhere on a
glass panel in the photograph to view the memorial names assigned to that
panel.
</p>

<h3>Panel Overview</h3>

<p>
The wall overview shows the configured panels in the Yahrzeit Wall. Selecting
a panel opens a read-only view of that panel and its assigned memorial
locations.
</p>

<h3>Panel Geometry</h3>

<p>
Panel names, rows, and columns are fixed application data. They represent the
physical wall layout and are not edited from this screen.
</p>

<h3>Calculated Lit LEDs</h3>

<p>
The Lit LEDs column is calculated from the memorial database and the current
lighting policy. It is not live electrical feedback from the wall.
</p>

<h3>Auditing Locations</h3>

<p>
If a panel location appears incorrect, use the Reports page to run the
database audit. The audit checks memorial records against the configured
panel geometry.
</p>

<h3>Manual Lighting</h3>

<p>
Use the <strong>Yizkor</strong> tab for immediate wall-wide Yahrzeit, Yizkor,
all-on, or all-off lighting operations.
</p>

   </div>
</div>

<?php
emitPageCopyright2();
emitFooter();
?>

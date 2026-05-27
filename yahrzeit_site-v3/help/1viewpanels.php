<?php
/*
 * NAME
 *      help/1viewpanels.php
 *
 * DESCRIPTION
 *      Help page for the panel overview and manual wall-control screen.
 *
 *      This page explains the wall overview, panel links, and manual lighting
 *      controls available from 1viewpanels.php.
 */
?>

<h2>View Panels Help</h2>

<p>
The View Panels page shows the physical layout of the CBS Yahrzeit Wall.
Use this page to browse panels and to perform limited wall-wide manual
lighting operations.
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

<h3>Manual Lighting Controls</h3>

<p>
This page may include manual operations such as turning all LEDs on, turning
all LEDs off, or turning on Yizkor lighting.
</p>

<p>
Manual lighting operations send commands to the controller immediately. Use
them only when intentionally changing the current wall display.
</p>

<h3>Normal Scheduled Operation</h3>

<p>
Normal yahrzeit and Yizkor scheduling is handled automatically by the
scheduler. Manual controls on this page are for testing, maintenance, or
special operator action.
</p>

<h3>Auditing Locations</h3>

<p>
If a panel location appears incorrect, use the Reports page to run the
database audit. The audit checks memorial records against the configured
panel geometry.
</p>

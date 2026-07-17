<?php
/*
 * NAME
 *      help/1yizkor.php
 *
 * DESCRIPTION
 *      Help page for upcoming Yizkor dates and immediate wall-wide lighting.
 */

require_once "../include/misc.inc.php";

$title = "Yizkor and Manual Lighting Help";
$description = "Help for upcoming Yizkor dates and immediate wall-wide lighting operations.";
$tab = 2;         // Yizkor
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle boldText">Wall-Wide Lighting Operations</div>

    <div class="helpBody">

<p>
This page shows the next enabled Yizkor observances and provides exceptional
manual control of the complete Yahrzeit Wall. Each lighting button sends
commands to the controller immediately.
</p>

<h3>Next Yizkor Events</h3>

<p>
The upcoming-events table shows the next civil date of each Yizkor observance
enabled on the Minhag page. It follows the configured Pesach and Shavuot day
choices and is sorted by upcoming date.
</p>

<h3>Turn All Lights Off / On</h3>

<p>
Use <strong>Turn all lights off</strong> or <strong>Turn all lights on</strong>
when testing or maintaining the wall. These choices temporarily replace the
normal display.
</p>

<h3>Yahrzeit Lights</h3>

<p>
Use <strong>Yahrzeit lights</strong> to calculate and restore the normal
Yahrzeit display for the current date. This is useful after a temporary
all-on, all-off, or Yizkor display.
</p>

<h3>Yizkor Lights</h3>

<p>
Use <strong>Yizkor lights</strong> to turn on the full-wall Yizkor display.
This button may also be used for an intentional operator action during a
service.
</p>

<h3>Normal Scheduled Operation</h3>

<p>
Normal Yahrzeit and Yizkor lighting is automatic. This page is not normally
needed for routine operation. After a manual display is no longer wanted, use
<strong>Yahrzeit lights</strong> to restore the normal Yahrzeit display.
</p>

    </div>
</div>

<?php
emitPageCopyright2();
emitFooter();
?>

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
$minhag = read_minhag_ini();
$controllerTitle = ($minhag['synagogueName'] ?? "") . " Yahrzeit Controller";
$title = "Yahrzeit Wall Home Help";
$description = "Help for the Yahrzeit Wall home and status page.";
$tab = 0;         // Yahrzeit
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle boldText"><?php echo h($controllerTitle); ?></div>

    <div class="helpBody">

<p>
The Home page is the main status page for the CBS Yahrzeit Wall web
application. It summarizes the current date, scheduling context, wall
configuration, and memorial database.
</p>

<h3>Date / Time</h3>

<p>
The Date / Time row shows the current civil date and time followed by the
current Hebrew date.
</p>

<h3>Addresses</h3>

<p>
The Addresses row shows the server address used for the current web request
and the controller hostname or IP address stored in
<code>bin/yahrzeit-controller.conf</code>.
</p>

<h3>Scheduled Events</h3>

<p>
The Scheduled Events row shows today's sunset and explains that normal
Yahrzeit lighting is reapplied every evening at the scheduled time.
</p>

<h3>Configured Policy Summary</h3>

<p>
This row summarizes the synagogue-wide Yahrzeit and Yizkor policy currently
saved on the Minhag page, including the calendar and lighting-time choices.
</p>

<h3>Controller Summary</h3>

<p>
The Controller Summary shows the number of configured wall panels, memorial
records, and memorial lights expected to be lit under the current policy. The
lit count is calculated from the database and policy; it is not electrical
feedback from the wall.
</p>

<p>
Links in this summary lead directly to panel views, name searches, reports,
database maintenance, command previews, and manual lighting operations. The
wall photograph below the summary is a visual reference and does not show live
LED status.
</p>

<h3>Navigation</h3>

<p>
Use the navigation tabs to reach the main maintenance screens.
</p>

<ul>
    <li><strong>Yizkor</strong> &mdash; use immediate wall-wide Yahrzeit, Yizkor, all-on, or all-off lighting controls.</li>
    <li><strong>Panels</strong> &mdash; view the physical panel layout and its assigned memorial names.</li>
    <li><strong>Names</strong> &mdash; search and review memorial records.</li>
    <li><strong>Reports</strong> &mdash; run Yahrzeit reports, audit the database, preview controller commands, and maintain the CSV database.</li>
    <li><strong>Minhag</strong> &mdash; review or update synagogue-wide lighting policy.</li>
    <li><strong>User Guide</strong> &mdash; open the complete operator and maintenance guide.</li>
</ul>

<h3>Informational Only</h3>

<p>
The Home page does not edit records, change Minhag settings, or send commands
to the wall. Use the appropriate maintenance screen for those actions.
</p>
    </div>
</div>

<?php
emitPageCopyright2();
emitFooter();
?>

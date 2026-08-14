<?php
/*
 * NAME
 *      help/7minhag.php
 *
 * DESCRIPTION
 *      Help page for the Minhag configuration screen.
 *
 *      This page explains the synagogue-policy settings used by the Yahrzeit
 *      Wall. It is intended for the ritual committee, office staff, or
 *      technical volunteer responsible for maintaining the wall.
 */
?>

<?php
require_once "../include/misc.inc.php";

// help/7minhag.php
$minhag = read_minhag_ini();
$synagogueName = trim($minhag['synagogueName'] ?? "");
if ($synagogueName === "") {
    $synagogueName = "the synagogue";
}

$title = "Minhag Settings Help";
$description = "Help for synagogue-wide Yahrzeit and Yizkor lighting policy.";
$tab = 6;         // Minhag
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle boldText">Yahrzeit Minhag</div>

    <div class="helpBody">

<p>
The Minhag page controls synagogue-wide rules for Yahrzeit and Yizkor
lighting. These settings affect scheduling and lighting behavior for the
whole wall. They do not edit individual memorial names.
</p>

<h3>Yahrzeit Date Method</h3>

<p>
Choose whether regular yahrzeit observances are based on the Hebrew date or
the English/Gregorian date stored for each memorial record.
</p>

<p>
The normal Yahrzeit update may run at a fixed clock time. It may instead run
a selected number of minutes before sunset. Because sunset changes throughout
the year, the appliance automatically schedules the operation.
</p>

<h3>Yahrzeit Lighting Option</h3>

<p>
Choose whether each memorial is lit for its Yahrzeit day only or for the full
week from Erev Shabbat through the following Erev Shabbat. The weekly option
prepares the wall on Friday for Yahrzeits occurring from Shabbat through the
following Friday.
</p>

<h3>Yizkor Lighting</h3>

<p>
Yizkor is observed four times during the year: on Yom Kippur, Shemini Atzeret,
Pesach, and Shavuot. In the <strong>Yizkor dates observed</strong> table, select
the checkbox for each observance held by <?php echo h($synagogueName); ?>.
</p>

<p>
Pesach and Shavuot have day-number choices because communities differ about
which day Yizkor is observed. Use the setting that matches
<?php echo h($synagogueName); ?> practice &mdash; that is, the day on which
Yizkor services are held.
</p>

<p>
The start time and duration are entered beside the observance checkboxes.
Yom Kippur has its own schedule. Shemini Atzeret, Pesach, and Shavuot share
the festival start time and duration shown in the cells spanning those three
rows. Durations are selectable in 15-minute increments from 30 minutes through
two hours.
</p>

<p>
The <strong>Next Yizkor events</strong> preview shows the next civil date for
each observance in the settings currently saved. Save any changes before
using the preview to verify the revised dates and schedule.
</p>

<h3>Saving Changes</h3>

<p>
Press Save to write the updated settings to <code>data/minhag.ini</code>.
After the configuration is saved, the appliance regenerates the schedule in
its managed cron block and displays the installed lines.
</p>

<p>
If the configuration is saved but cron installation fails, the result page
says so explicitly. The previous schedule may remain active until a technical
maintainer reruns the appliance installer or runs
<code>sudo bin/fix-up-crontab</code>.
</p>

<h3>Managed Cron Schedule</h3>

<p>
<code>bin/fix-up-crontab</code> manages only the block between its Yahrzeit
markers and preserves unrelated jobs. The normal Yahrzeit phase runs every
evening, reapplying the intended wall image. The day-only or weekly setting
determines which memorial lights are selected; it does not change how often
the wall is refreshed. Yizkor phases are installed only for the next civil
date of each selected observance. The managed block is regenerated every
morning at 12:05 AM, which advances completed observances and refreshes any
sunset-derived Yahrzeit time.
</p>

<code>
cd /path/to/yahrzeit_site-v3<br>
bin/fix-up-crontab --dry-run<br>
sudo bin/fix-up-crontab
</code>

<p>
The installer records the intended appliance account in
<code>/etc/yahrzeit-cron-user</code>. This prevents the web server from
accidentally creating a separate <code>www-data</code> crontab.
</p>

    </div>  <!-- end of helpbody div -->
</div>  <!-- end of helpbox div -->

<?php
emitPageCopyright2();
emitFooter();
?>

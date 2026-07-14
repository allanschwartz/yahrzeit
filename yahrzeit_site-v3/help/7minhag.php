<?php
/*
 * NAME
 *      help/7minhag.php
 *
 * DESCRIPTION
 *      Help page for the Minhag configuration screen.
 *
 *      This page explains the synagogue-policy settings used by the CBS
 *      Yahrzeit Wall. It is intended for the ritual committee, office staff,
 *      or technical volunteer responsible for maintaining the wall.
 */
?>

<?php
require_once "../include/misc.inc.php";

// help/7minhag.php
$title = "Minhag Settings Help";
$description = "Help for synagogue-wide Yahrzeit and Yizkor lighting policy.";
$tab = 5;         // Minhag
$helpfile = "";  // no nested page-help link on a help page

emitHeader($title, $tab);
emitTopOfScreen($title, $description, "");
?>

<div class="helpBox">
    <div class="helpTitle">Minhag Settings Help</div>

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
a selected number of minutes before sunset. Because sunset changes through
the year, the appliance automatically refreshes that cron time.
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
The Yizkor section controls which holidays and observances are treated as
full-wall Yizkor lighting days. Common Yizkor observances include Yom Kippur,
Shemini Atzeret, Pesach, and Shavuot.
</p>

<p>
Pesach and Shavuot have day-number choices because communities differ about
which day Yizkor is observed. Use the setting that matches Congregation Beth
Sholom practice — that is, the day on which Yizkor services are held.
</p>

<p>
The Minhag page determines which holidays and observances are treated as
Yizkor days. The Yizkor on and off times become the run times of the two
Yizkor scheduler phases. On other dates those phases do nothing.
</p>

<h3>Other Yizkor Date</h3>

<p>
The optional “Other” date can be used for a special full-wall observance on
a specified Hebrew date, such as a community memorial day. Examples might
include Yom HaZikaron or Yom HaShoah, depending on local practice.
</p>

<h3>Saving Changes</h3>

<p>
Press Save to write the updated settings to <code>data/minhag.ini</code>.
After the configuration is saved, the appliance regenerates its managed cron
block and displays the installed lines.
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
markers and preserves unrelated jobs. Weekly observance installs the normal
Yahrzeit phase on Friday; day-only observance installs it every day. Sunset
policies retain those recurring patterns as a fail-safe while refreshing the
clock time automatically.
</p>

<pre>
cd /path/to/yahrzeit_site-v3
bin/fix-up-crontab --preview
sudo bin/fix-up-crontab
</pre>

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

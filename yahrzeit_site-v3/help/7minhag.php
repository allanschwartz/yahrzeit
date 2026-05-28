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
Yahrzeit lighting may begin at a fixed clock time, such as 7:00 AM or
5:00 PM. If the wall is configured for Erev Shabbat to Erev Shabbat
lighting, the start time may instead be based on candle-lighting time,
such as 18 minutes before sunset.
</p>

<h3>Shabbat and Weekly Lighting</h3>

<p>
These options control whether yahrzeit lights remain on through Shabbat or
through a longer weekly observance window.
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
Yizkor days. It does not directly edit the appliance cron table or install
scheduled jobs.
</p>

<p>
The actual scheduled run times, such as when Yizkor lighting begins or when
normal yahrzeit lighting is restored, are maintained separately in the
appliance cron configuration by a technical maintainer.
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
After changing Yizkor or timing settings, run the report or audit screen if
you want to confirm the current configuration.
</p>

<p>
After saving, the application may display suggested cron entries for the
technical maintainer to review and copy manually. These entries are advisory;
the web page does not install them automatically.
</p>

<h3>Suggested Cron Entries</h3>

<p>
The suggested cron entries show the command lines that a technical maintainer
may copy into the appliance crontab. In the simplified scheduler model, cron
runs named phases at fixed times. The scheduler decides whether the requested
phase applies on that date.
</p>

<pre>
# CBS Yahrzeit Wall scheduled lighting
# Review path and times before installing with crontab -e.

0 11 * * * cd /home/pi/yahrzeit_site-v3 && bin/yahrzeit_scheduler --phase yizkor-on  >> data/scheduler.log 2>&1
0 13 * * * cd /home/pi/yahrzeit_site-v3 && bin/yahrzeit_scheduler --phase yizkor-off >> data/scheduler.log 2>&1
0 16 * * * cd /home/pi/yahrzeit_site-v3 && bin/yahrzeit_scheduler --phase yahrzeit   >> data/scheduler.log 2>&1
</pre>

<p>
The times above are examples. Adjust them to match Congregation Beth Sholom
practice before installing them.
</p>

    </div>  <!-- end of helpbody div -->
</div>  <!-- end of helpbox div -->

<?php
emitPageCopyright2();
emitFooter();
?>

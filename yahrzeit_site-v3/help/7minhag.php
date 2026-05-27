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

<h2>Minhag Settings Help</h2>

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
If English-date yahrzeits begin at nightfall, the wall treats the observance
as beginning at the prior evening’s sunset rather than at midnight.
</p>

<h3>Shabbat and Weekly Lighting</h3>

<p>
These options control whether yahrzeit lights remain on through Shabbat or
through a longer weekly observance window.
</p>

<h3>Yizkor Lighting</h3>

<p>
The Yizkor section controls which holidays receive full-wall Yizkor lighting
and what time that lighting begins and ends.
</p>

<p>
Pesach and Shavuot have day-number choices because communities differ about
which day Yizkor is observed. Use the setting that matches Congregation Beth
Sholom practice.
</p>

<h3>Other Yizkor Date</h3>

<p>
The optional “Other” date can be used for a special full-wall observance on
a specified Hebrew date, such as a community memorial day.
</p>

<h3>Saving Changes</h3>

<p>
Press Save to write the updated settings to <code>data/minhag.ini</code>.
After changing Yizkor or timing settings, run the report or audit screen if
you want to confirm the current configuration.
</p>

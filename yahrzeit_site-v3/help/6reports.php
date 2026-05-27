<?php
/*
 * NAME
 *      help/6reports.php
 *
 * DESCRIPTION
 *      Help page for the Reports and Audit screen.
 *
 *      This page explains the reporting, audit, preview, download, and upload
 *      tools available from 6reports.php.
 */
?>

<h2>Reports and Audit Help</h2>

<p>
The Reports page provides tools for reviewing the memorial database and
checking what the Yahrzeit Wall would display.
</p>

<h3>Yahrzeit Reports</h3>

<p>
Use the day, week, next-week, month, and next-month reports to list memorial
names whose yahrzeits fall in the selected period.
</p>

<h3>Audit Database</h3>

<p>
The audit checks the memorial database against the wall geometry. It reports
problems such as unknown panel IDs, row or column values outside the valid
range, and duplicate memorial records assigned to the same LED position.
</p>

<h3>Preview Controller Commands</h3>

<p>
The preview option shows the command stream that would be sent to the
controller for the selected date. It does not transmit commands to the wall.
</p>

<h3>Download CSV</h3>

<p>
Use Download CSV to save a copy of the live memorial database file.
</p>

<h3>Upload CSV</h3>

<p>
Use Upload CSV only when replacing the memorial database with a corrected
CSV file. The current file is backed up before replacement, and an audit is
run after upload.
</p>

<p>
If the audit reports errors after an upload, correct the CSV file and upload
again before relying on scheduled lighting.
</p>


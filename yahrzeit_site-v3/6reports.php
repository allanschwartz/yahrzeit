<?php
/*
 * NAME
 *      6reports.php
 *
 * DESCRIPTION
 *      Reports, audit, preview, and CSV maintenance screen for the CBS
 *      Yahrzeit Wall.
 *
 *      This page provides administrative tools for reviewing memorial data
 *      and checking what the wall would display. It is a web front-end to
 *      bin/yahrzeit and bin/yahrzeit_engine.php.
 *
 *      Supported operations include:
 *
 *          - report yahrzeits for a selected day
 *          - report yahrzeits for this week or next week
 *          - report yahrzeits for this month or next month
 *          - audit the memorial database and panel geometry
 *          - preview the generated controller command stream
 *          - download the live CSV memorial database
 *          - upload a replacement CSV memorial database
 *
 * BLUF
 *      This page is for review, audit, preview, and CSV maintenance.
 *
 *      It should not contain yahrzeit date logic, panel geometry rules, or
 *      controller command-generation logic. Those belong in
 *      bin/yahrzeit_engine.php and the include files.
 *
 * NOTES
 *      CSV upload replaces data/yahrzeits-rev4.csv after first making a
 *      timestamped backup under data/backups/.
 *
 *      After upload, the page runs an audit so the maintainer can immediately
 *      see whether the replacement CSV is usable.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized for PHP 8 and the Arduino V3 controller in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

    require_once "include/misc.inc.php";

    /*
    * Page metadata used by emitTopOfScreen().
    */

    $title = "Yahrzeit Reports";
    $description = "Generate Yahrzeit reports, preview controller commands, and export or import the Yahrzeit database.";
    $tab = 4;         // Reports
    $helpfile = "help/6reports.php";

function reports_site_root()
{
    if (function_exists('site_root')) {
        return site_root();
    }

    return __DIR__;
}

function reports_data_path($filename)
{
    if (function_exists('data_path')) {
        return data_path($filename);
    }

    return reports_site_root() . "/data/" . $filename;
}

function reports_yahrzeit_script()
{
    return reports_site_root() . "/bin/yahrzeit";
}

function reports_today()
{
    return date('Y-m-d');
}

function reports_valid_date($date)
{
    if (!preg_match('/^\d{4}-\d{2}-\d{2}$/', $date)) {
        return false;
    }

    list($year, $month, $day) = explode('-', $date);
    return checkdate((int)$month, (int)$day, (int)$year);
}

function reports_valid_kind($kind)
{
    $valid = array(
        'day'        => true,
        'week'       => true,
        'next-week'  => true,
        'month'      => true,
        'next-month' => true,
    );

    return isset($valid[$kind]);
}

function reports_run_yahrzeit($args)
{
    $script = reports_yahrzeit_script();

    if (!is_file($script)) {
        return array(1, "bin/yahrzeit was not found: " . $script . "\n");
    }

    $cmd_parts = array(escapeshellarg($script));
    foreach ($args as $arg) {
        $cmd_parts[] = escapeshellarg($arg);
    }

    $cmd = implode(' ', $cmd_parts) . " 2>&1";
    $output = array();
    $status = 0;

    exec($cmd, $output, $status);

    return array($status, implode("\n", $output) . "\n");
}

function reports_render_output_page($title, $tab, $heading, $helpfile, $message, $output = "")
{
    emitHeader($title, $tab);
    emitTopOfScreen($title, $heading, $helpfile);
?>

<table cellspacing="0" cellpadding="4" width="90%" border="0" class="botBorder">
    <tr>
        <td class="header2Bg" align="left" height="25">
            <span class="boldText"><?php echo h($message); ?></span>
        </td>
    </tr>
    <tr>
        <td class="text">
            <?php if ($output !== "") { ?>
                <pre><?php echo h($output); ?></pre>
            <?php } ?>
            <br>
            <a href="6reports.php">Return to Reports</a>
        </td>
    </tr>
</table>

<?php
    emitFooter();
}

function reports_download_csv()
{
    $csv = reports_data_path("yahrzeits-rev4.csv");

    if (!is_file($csv) || !is_readable($csv)) {
        http_response_code(404);
        echo "Yahrzeit database file not found.\n";
        exit;
    }

    header('Content-Type: text/csv');
    header('Content-Disposition: attachment; filename="yahrzeits-rev4.csv"');
    header('Content-Length: ' . filesize($csv));
    readfile($csv);
    exit;
}

function reports_upload_csv()
{
    if (!isset($_FILES['importFile'])) {
        return array(false, "No upload file was received.\n");
    }

    $file = $_FILES['importFile'];

    if ($file['error'] != UPLOAD_ERR_OK) {
        return array(false, "Upload failed with PHP upload error code " . $file['error'] . ".\n");
    }

    $original_name = isset($file['name']) ? $file['name'] : "";
    if ($original_name == "" || !preg_match('/\.csv$/i', $original_name)) {
        return array(false, "Upload rejected: file name must end in .csv.\n");
    }

    $data_dir = reports_site_root() . "/data";
    $backup_dir = $data_dir . "/backups";
    $csv = reports_data_path("yahrzeits-rev4.csv");

    if (!is_dir($backup_dir) && !mkdir($backup_dir, 0755, true)) {
        return array(false, "Could not create backup directory: " . $backup_dir . "\n");
    }

    if (is_file($csv)) {
        $backup = $backup_dir . "/yahrzeits-rev4-" . date('Ymd-His') . ".csv";
        if (!copy($csv, $backup)) {
            return array(false, "Could not create backup before import: " . $backup . "\n");
        }
    }

    if (!move_uploaded_file($file['tmp_name'], $csv)) {
        return array(false, "Could not move uploaded file into place: " . $csv . "\n");
    }

    list($status, $audit_output) = reports_run_yahrzeit(array('--audit'));

    $message = "Uploaded " . $original_name . " as data/yahrzeits-rev4.csv.\n\n";
    $message .= "Audit after import exited with status " . $status . ".\n\n";
    $message .= $audit_output;

    return array(true, $message);
}

// CSV download must happen before any page output.
if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['action']) && $_POST['action'] == 'download') {
    reports_download_csv();
}

if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $action = isset($_POST['action']) ? $_POST['action'] : "";

    if ($action == 'report') {
        $kind = isset($_POST['report_kind']) ? $_POST['report_kind'] : "";
        $date = isset($_POST['report_date']) ? $_POST['report_date'] : reports_today();

        if (!reports_valid_kind($kind)) {
            reports_render_output_page($title, $tab, $description, $helpfile,
                    "Invalid report kind", "Unknown report kind: " . $kind . "\n");
            exit;
        }

        if (!reports_valid_date($date)) {
            reports_render_output_page($title, $tab, $description, $helpfile,
                    "Invalid report date", "Date must be YYYY-MM-DD.\n");
            exit;
        }

        list($status, $output) = reports_run_yahrzeit(array('--report', $kind, '--date', $date));
        reports_render_output_page($title, $tab, $description, $helpfile,
                    "Report: " . $kind . " for " . $date . " (exit " . $status . ")", $output);
        exit;
    }

    if ($action == 'audit') {
        list($status, $output) = reports_run_yahrzeit(array('--audit'));
        reports_render_output_page($title, $tab, $description, $helpfile,
                    "Database audit (exit " . $status . ")", $output);
        exit;
    }

    if ($action == 'preview') {
        list($status, $output) = reports_run_yahrzeit(array('--notransmit'));
        reports_render_output_page($title, $tab, $description, $helpfile,
                    "Controller command preview (exit " . $status . ")", $output);
        exit;
    }

    if ($action == 'upload') {
        list($ok, $output) = reports_upload_csv();
        reports_render_output_page($title, $tab, $description, $ok ? "Upload complete" : "Upload failed", $output);
        exit;
    }

    reports_render_output_page($title, $tab, $description, $helpfile,
                "Unknown request", "Unknown reports action.\n");
    exit;
}

if ($_SERVER['REQUEST_METHOD'] != 'GET') {
    die("This script only works with GET and POST requests.");
}

$today = reports_today();

emitHeader($title, $tab);
emitTopOfScreen($title, $description, $helpfile);
?>

<form name="reports" action="<?php echo h($_SERVER['PHP_SELF']); ?>" method="POST" enctype="multipart/form-data">

<table cellspacing="0" cellpadding="4" width="90%" border="0" class="botBorder">
    <tr>
        <td width="35%"></td>
        <td width="40%"></td>
        <td width="25%"></td>
    </tr>

    <tr>
        <td colspan="3" class="header2Bg" align="left" height="25">
            <span class="boldText">Yahrzeit Reports</span>
        </td>
    </tr>

    <tr>
        <td height="25" align="left" valign="top">
            <span class="text">Upcoming or observed Yahrzeits</span>
            <br>
            <span class="textsmall">
                Select a report and an anchor date.
            </span>
        </td>
        <td colspan="2" class="text">
            <input type="radio" name="report_kind" value="day" checked> Selected day<br>
            <input type="radio" name="report_kind" value="week"> This week<br>
            <input type="radio" name="report_kind" value="next-week"> Next week<br>
            <input type="radio" name="report_kind" value="month"> This month<br>
            <input type="radio" name="report_kind" value="next-month"> Next month<br>
            <br>
            Anchor date:
            <input type="date" name="report_date" value="<?php echo h($today); ?>" class="formStyle">
            <br>
            <span class="textsmall">
                The week and month reports are computed by bin/yahrzeit and yahrzeit_engine.php.
            </span>
        </td>
    </tr>

    <tr>
        <td colspan="3" align="center">
            <button type="submit" name="action" value="report" class="button">RUN REPORT</button>
        </td>
    </tr>

    <tr>
        <td colspan="3" height="10"></td>
    </tr>

    <tr>
        <td colspan="3" class="header2Bg" align="left" height="25">
            <span class="boldText">Diagnostics</span>
        </td>
    </tr>

    <tr>
        <td height="25" align="left" valign="top">
            <span class="text">Audit database</span>
            <br>
            <span class="textsmall">
                Check panel IDs, row/column bounds, and duplicate LED locations.
            </span>
        </td>
        <td>
            <button type="submit" name="action" value="audit" class="button">AUDIT DATABASE</button>
        </td>
        <td id="notused">&nbsp;</td>
    </tr>

    <tr>
        <td height="25" align="left" valign="top">
            <span class="text">Preview controller commands</span>
            <br>
            <span class="textsmall">
                Show the commands that would be sent today, without transmitting.
            </span>
        </td>
        <td>
            <button type="submit" name="action" value="preview" class="button">PREVIEW COMMANDS</button>
        </td>
        <td id="notused">&nbsp;</td>
    </tr>

    <tr>
        <td colspan="3" height="10"></td>
    </tr>

    <tr>
        <td colspan="3" class="header2Bg" align="left" height="25">
            <span class="boldText">Export or Import Yahrzeit Database</span>
        </td>
    </tr>

    <tr>
        <td height="25" align="left">
            <span class="text">Export Yahrzeit database</span>
            <br>
            <span class="textsmall">
                Download the current data/yahrzeits-rev4.csv file.
            </span>
        </td>
        <td>
            <button type="submit" name="action" value="download" class="button">DOWNLOAD CSV</button>
        </td>
        <td id="notused">&nbsp;</td>
    </tr>

    <tr>
        <td height="25" align="left" valign="top">
            <span class="text">Import Yahrzeit database</span>
            <br>
            <span class="textsmall">
                Upload a replacement .csv file. The existing file is backed up first,
                and an audit is run after import.
            </span>
        </td>
        <td>
            <input type="file" name="importFile" maxlength="64" size="25" class="formStyle" accept=".csv,text/csv">
            <br><br>
            <button type="submit" name="action" value="upload" class="button">UPLOAD CSV</button>
        </td>
        <td id="notused">&nbsp;</td>
    </tr>

    <tr>
        <td colspan="3" height="10"></td>
    </tr>

    <tr>
        <td colspan="3" class="textsmall">
            Import is intended for batch updates, such as adding many memorial records after a memorial sale.
            Keep a downloaded copy of the current CSV before uploading a replacement.
        </td>
    </tr>
</table>

</form>

<?php
emitFooter();
?>

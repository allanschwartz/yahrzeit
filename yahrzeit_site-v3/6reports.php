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

const REPORTS_TITLE       = "Yahrzeit Reports";
const REPORTS_DESCRIPTION = "Generate Yahrzeit reports, preview controller commands, and export or import the Yahrzeit database.";
const REPORTS_TAB         = 4;
const REPORTS_HELPFILE    = "help/6reports.php";
const REPORTS_CSV_FILE    = "yahrzeits-rev4.csv";

const REPORTS_VALID_KINDS = [
    'day'        => true,
    'week'       => true,
    'next-week'  => true,
    'month'      => true,
    'next-month' => true,
];

// -----------------------------------------------------------------------------
// Path and command helpers
// -----------------------------------------------------------------------------

function reports_site_root()
{
    return function_exists('site_root') ? site_root() : __DIR__;
}

function reports_data_path($filename)
{
    return function_exists('data_path')
         ? data_path($filename)
         : reports_site_root() . "/data/" . $filename;
}

function reports_yahrzeit_script()
{
    return reports_site_root() . "/bin/yahrzeit";
}

function reports_today()
{
    return date('Y-m-d');
}

function reports_run_yahrzeit($args)
{
    $script = reports_yahrzeit_script();

    if (!is_file($script)) {
        return [1, "bin/yahrzeit was not found: $script\n"];
    }

    $cmd = implode(' ', array_map('escapeshellarg', array_merge([$script], $args))) . " 2>&1";
    $output = [];
    $status = 0;

    exec($cmd, $output, $status);

    return [$status, implode("\n", $output) . "\n"];
}

// -----------------------------------------------------------------------------
// Validation helpers
// -----------------------------------------------------------------------------

function reports_valid_date($date)
{
    if (!preg_match('/^\d{4}-\d{2}-\d{2}$/', $date)) {
        return false;
    }

    [$year, $month, $day] = explode('-', $date);
    return checkdate((int)$month, (int)$day, (int)$year);
}

function reports_valid_kind($kind)
{
    return isset(REPORTS_VALID_KINDS[$kind]);
}

function reports_uploaded_csv_looks_valid($filename)
{
    if (($fp = fopen($filename, "r")) === false) {
        return false;
    }

    $rows = 0;
    $valid = 0;

    while (($row = fgetcsv($fp, 4096, ",", "\"", "")) !== false) {
        $rows++;

        if ($rows > 5000) {
            fclose($fp);
            return false;
        }

        $name = is_array($row) ? trim($row[0] ?? "") : "";
        if (count($row) >= 8 && $name != "" && strcasecmp($name, "DECEASED") != 0) {
            $valid++;
        }
    }

    fclose($fp);

    return $valid > 100;
}

// -----------------------------------------------------------------------------
// Action handlers
// -----------------------------------------------------------------------------

function reports_handle_report()
{
    $kind = $_POST['report_kind'] ?? "";
    $date = $_POST['report_date'] ?? reports_today();

    if (!reports_valid_kind($kind)) {
        return [false, "Invalid report kind", "Unknown report kind: $kind\n"];
    }

    if (!reports_valid_date($date)) {
        return [false, "Invalid report date", "Date must be YYYY-MM-DD.\n"];
    }

    [$status, $output] = reports_run_yahrzeit(['--report', $kind, '--date', $date]);

    return [$status == 0, "Report: $kind for $date (exit $status)", $output];
}

function reports_handle_audit()
{
    [$status, $output] = reports_run_yahrzeit(['--audit']);

    return [$status == 0, "Database audit (exit $status)", $output];
}

function reports_handle_preview()
{
    [$status, $output] = reports_run_yahrzeit(['--notransmit']);

    return [$status == 0, "Controller command preview (exit $status)", $output];
}

function reports_handle_upload()
{
    $live = reports_data_path(REPORTS_CSV_FILE);

    if (!isset($_FILES['csvfile']) || $_FILES['csvfile']['error'] != UPLOAD_ERR_OK) {
        return [false, "No CSV file was uploaded successfully.", ""];
    }

    $tmp  = $_FILES['csvfile']['tmp_name'];
    $name = $_FILES['csvfile']['name'] ?? "uploaded file";

    if (!is_uploaded_file($tmp)) {
        return [false, "Upload did not come from PHP's upload mechanism.", ""];
    }

    if (filesize($tmp) <= 0) {
        return [false, "Uploaded file is empty.", ""];
    }

    if (!reports_uploaded_csv_looks_valid($tmp)) {
        return [false, "Uploaded file does not look like a valid Yahrzeit CSV database.", ""];
    }

    $stamp  = date("Ymd-His");
    $backup = reports_data_path("backups/yahrzeits-rev4.$stamp.csv");
    $new    = reports_data_path("yahrzeits-rev4.upload.$stamp.csv");

    if (!is_dir(dirname($backup)) && !mkdir(dirname($backup), 0775, true)) {
        return [false, "Could not create backup directory.", ""];
    }

    if (!copy($live, $backup)) {
        return [false, "Could not create backup copy of the current CSV file.", ""];
    }

    if (!move_uploaded_file($tmp, $new)) {
        return [false, "Could not move uploaded CSV into the data directory.", ""];
    }

    if (!rename($new, $live)) {
        return [false, "Could not replace the live CSV file. Backup was preserved.", ""];
    }

    [$audit_status, $audit_output] = reports_run_yahrzeit(['--audit']);

    $message = "Uploaded $name. Backup saved as " . basename($backup) .
               ". Audit exit status: $audit_status.";

    // The upload itself succeeded even if the audit reports data problems.
    return [true, $message, $audit_output];
}

function reports_download_csv()
{
    $csv = reports_data_path(REPORTS_CSV_FILE);

    if (!is_file($csv) || !is_readable($csv)) {
        http_response_code(404);
        echo "Yahrzeit database file not found.\n";
        exit;
    }

    header('Content-Type: text/csv');
    header('Content-Disposition: attachment; filename="' . REPORTS_CSV_FILE . '"');
    header('Content-Length: ' . filesize($csv));
    readfile($csv);
    exit;
}

function reports_dispatch_post()
{
    $action = $_POST['action'] ?? "";

    return match ($action) {
        'report'   => reports_handle_report(),
        'audit'    => reports_handle_audit(),
        'preview'  => reports_handle_preview(),
        'upload'   => reports_handle_upload(),
        default    => [false, "Unknown request", "Unknown reports action.\n"],
    };
}

// -----------------------------------------------------------------------------
// Rendering helpers
// -----------------------------------------------------------------------------

function reports_render_result_page($message, $output = "")
{
    emitHeader(REPORTS_TITLE, REPORTS_TAB);
    emitTopOfScreen(REPORTS_TITLE, REPORTS_DESCRIPTION, REPORTS_HELPFILE);
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
<?php
        emitCopyright();
?>
</table>
<br>&nbsp;<br>

<?php
    emitFooter();
}

function reports_render_main_page()
{
    $today = reports_today();

    emitHeader(REPORTS_TITLE, REPORTS_TAB);
    emitTopOfScreen(REPORTS_TITLE, REPORTS_DESCRIPTION, REPORTS_HELPFILE);
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
            <input type="file" name="csvfile" maxlength="1284" size="25" accept=".csv,text/csv">
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
<?php
        emitCopyright();
?>
</table>
</form>
<br>&nbsp;<br>

<?php
    emitFooter();
}

// -----------------------------------------------------------------------------
// Program entry point
// -----------------------------------------------------------------------------

function reports_main()
{
    $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
    $action = $_POST['action'] ?? "";

    // CSV download must happen before any page output.
    if ($method == 'POST' && $action == 'download') {
        reports_download_csv();
    }

    if ($method == 'POST') {
        [$ok, $message, $output] = reports_dispatch_post();
        reports_render_result_page($message, $output);
        return;
    }

    if ($method == 'GET') {
        reports_render_main_page();
        return;
    }

    die("This script only works with GET and POST requests.");
}

reports_main();

<?php
/*
 * NAME
 *      0yahrzeit.php
 *
 * DESCRIPTION
 *      Home and status screen for the CBS Yahrzeit Wall.
 *
 *      This page is the main landing page for the web application. It shows
 *      basic current-date information, including the Gregorian date, Hebrew
 *      date, sunset-related times, and summary counts for the configured wall
 *      panels and memorial records.
 *
 *      This page is informational. It does not control the wall, edit memorial
 *      records, update Minhag settings, or generate controller commands.
 *
 *      Most maintenance tasks are reached through the navigation tabs:
 *
 *          - Panels: view panel geometry
 *          - Yizkor: perform manual wall-wide lighting operations
 *          - Names: browse memorial records
 *          - Reports: run reports, audit the database, preview commands,
 *            and maintain the CSV database
 *          - Minhag: edit synagogue lighting policy
 *
 * BLUF
 *      This is the application home/status page.
 *
 *      It should summarize the current Yahrzeit Wall environment, not perform
 *      maintenance actions or contain core lighting logic.
 *
 * NOTES
 *      Sunset and Hebrew-date information is displayed to help maintainers
 *      understand the current scheduling context.
 *
 *      The engine applies include/yahrzeit_policy.inc.php to decide which
 *      memorials are active. bin/fix-up-crontab derives scheduled times;
 *      bin/yahrzeit_scheduler decides whether a scheduled phase applies.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized for PHP 8 and the Yahrzeit V3 release in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

require_once "include/misc.inc.php";
require_once "include/panels.inc.php";
require_once "include/names.inc.php";
require_once "include/date_support.inc.php";
require_once "include/yahrzeit_policy.inc.php";

global $minhag;
$minhag = read_minhag_ini();

date_default_timezone_set("America/Los_Angeles");

const YAHRZEIT_TITLE    = "Yahrzeit Wall";
const YAHRZEIT_TAB      = 1;
const YAHRZEIT_HELPFILE = "help/0yahrzeit.php";

// -----------------------------------------------------------------------------
// Page data helpers
// -----------------------------------------------------------------------------

function yahrzeit_page_description()
{
    global $minhag;

    return "Home and status page for the " .
           h($minhag['synagogueName'] ?? "") . " Yahrzeit Wall.";
}

/** Return the server address used for this web request. */
function yahrzeit_server_address()
{
    return $_SERVER['SERVER_ADDR'] ?? "unknown";
}

/**
 * Return trusted-HTML summary lines for configured wall data and operations.
 *
 * The light count is a policy calculation from the CSV, not a live query of
 * controller state.
 *
 * @return array<int, string>
 */
function controller_summary_lines($timestamp = null)
{
    if ($timestamp === null) {
        $timestamp = time();
    }

    $panelCount = panel_readDB();
    $nameCount  = yahrzeit_readDB();
    $litCount   = yahrzeit_lit_person_count($timestamp);

    return [
        h($panelCount) . ' panels defined (click on <a href="2panels.php">Panels</a>)',
        h($nameCount) . ' names defined (click on <a href="4names.php">Names</a>)',
        h($litCount) . ' memorial lights are lit now.',
        'Reports, audits, command previews, and CSV maintenance are available on the <a href="6reports.php">Reports</a> screen',
        'Manual lighting operations are available from the <a href="1yizkor.php">Yizkor</a> screen'
    ];
}

function yahrzeit_label($value, $labels)
{
    return $labels[$value] ?? $value;
}

/** Return trusted-HTML summary lines for the saved lighting policy. */
function yahrzeit_minhag_summary_lines()
{
    global $minhag;

    $dateType = yahrzeit_label($minhag['yahrzeitEngOrHeb'] ?? "", [
        'eng' => 'English',
        'heb' => 'Hebrew',
    ]);

    $yahrzeitTiming = yahrzeit_label($minhag['yahrzeitLightTime'] ?? "", [
        'setTime'  => 'a fixed clock time',
        'atSunset' => 'a sunset-relative window',
    ]);

    return [
        'Yahrzeits are normally observed by ' . h($dateType) . ' date',
        'Yahrzeit lighting uses ' . h($yahrzeitTiming),
        'Yizkor lighting has separate festival and Yom Kippur schedules',
        'Minhag settings are edited on the <a href="7minhag.php">Minhag</a> screen'
    ];
}

/** Return the home-page statement for the daily normal-lighting refresh. */
function yahrzeit_daily_lighting_line()
{
    return "Normal Yahrzeit lighting is refreshed every evening " .
           "at the scheduled time.";
}

// -----------------------------------------------------------------------------
// Rendering helpers
// -----------------------------------------------------------------------------

function yahrzeit_render_scheduled_events($timestamp = null)
{
    if ($timestamp === null) {
        $timestamp = time();
    }

    $todaySunsetText = cbs_sunset_time_string($timestamp);

    echo "Today's sunset in San Francisco is at " . h($todaySunsetText) . ".<br>\n";
    echo yahrzeit_daily_lighting_line() . "<br>\n";
}

function yahrzeit_render_main_page()
{
    $render_timestamp = time();
    $minhag = read_minhag_ini();

    emitHeader(YAHRZEIT_TITLE, YAHRZEIT_TAB);
    emitTopOfScreen(YAHRZEIT_TITLE, yahrzeit_page_description(), YAHRZEIT_HELPFILE);
?>

    <table cellSpacing=0 cellPadding=4 width=100% border=0 class="botBorder homeSummary">
        <tr>
            <td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText">
                    <?php echo h($minhag['synagogueName'] ?? ""); ?> Yahrzeit Controller
                </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Date / Time
            </td>
            <td class="text">
                <?php echo h(date("l F j, Y, g:i a")); ?><br>
                <?php echo h(current_hebrew_date_string()); ?>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Addresses
            </td>
            <td class="text">
                Yahrzeit server: <?php echo h(yahrzeit_server_address()); ?><br>
                Yahrzeit controller: <?php echo h(yahrzeit_controller_host()); ?>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Scheduled Events
            </td>
            <td colspan="2" class="text">
<?php
                yahrzeit_render_scheduled_events($render_timestamp);
?>
                <br>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Configured Policy Summary
            </td>
            <td colspan="2" class="text">
<?php
                foreach (yahrzeit_minhag_summary_lines() as $line) {
                    echo $line . "<br>\n";
                }
?>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Controller Summary
            </td>
            <td colspan="2" class="text">
<?php
                foreach (controller_summary_lines($render_timestamp) as $line) {
                    echo $line . "<br>\n";
                }
?>
            </td>
        </tr>

        <tr>
            <td colspan=3 align=left>
                <img src="images/image-21panels.jpg" width=700>
            </td>
        </tr>
<?php
        emitCopyright();
?>

    </table>

<?php
    emitFooter();
}

// -----------------------------------------------------------------------------
// Program entry point
// -----------------------------------------------------------------------------

function yahrzeit_main()
{
    $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

    if ($method == 'GET') {
        yahrzeit_render_main_page();
        return;
    }

    die("This script only works with GET requests.");
}

yahrzeit_main();

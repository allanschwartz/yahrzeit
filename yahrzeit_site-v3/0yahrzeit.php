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
 *          - Panels: view panel geometry and manual wall-wide operations
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
 *      The actual lighting decisions are made by bin/yahrzeit_engine.php.
 *      Scheduled timing decisions are made by bin/yahrzeit_scheduler.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized as the home/status page for the PHP 8 / Arduino V3 version
 *      in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

require_once "include/misc.inc.php";
require_once "include/panels.inc.php";
require_once "include/names.inc.php";
require_once "include/date_support.inc.php";

date_default_timezone_set("America/Los_Angeles");

const YAHRZEIT_TITLE    = "Yahrzeit Controller";
const YAHRZEIT_TAB      = 1;
const YAHRZEIT_HELPFILE = "help/0yahrzeit.php";

// -----------------------------------------------------------------------------
// Page data helpers
// -----------------------------------------------------------------------------

function yahrzeit_page_description()
{
    $minhag = read_minhag_ini();

    return "Home and status page for the Yahrzeit panels at " .
           h($minhag['synagogueName'] ?? "") . ".";
}

function controller_summary_lines()
{
    $panelCount = panel_readDB();
    $nameCount  = yahrzeit_readDB();

    return [
        h($panelCount) . ' panels defined (click on <a href="1viewpanels.php">Panels</a>)',
        h($nameCount) . ' names defined (click on <a href="4viewnames.php">Names</a>)',
        'Manual lighting operations are available from the Panels screen'
    ];
}

function yahrzeit_next_shabbat_lighting_line()
{
    $nextFriday = next_friday_timestamp();
    $fridaySunsetTimestamp = cbs_sunset_timestamp($nextFriday);

    if ($fridaySunsetTimestamp === false) {
        return "This week's Shabbat sunset time is unknown.";
    }

    $fridaySunsetText = date("l F j, Y, g:i a", $fridaySunsetTimestamp);

    return "On " . h($fridaySunsetText) . " this week's yahrzeits will be lit.";
}

// -----------------------------------------------------------------------------
// Rendering helpers
// -----------------------------------------------------------------------------

function yahrzeit_render_scheduled_events()
{
    $todaySunsetText = cbs_sunset_time_string(time());

    echo "Today's sunset in San Francisco is at " . h($todaySunsetText) . ".<br>\n";
    echo yahrzeit_next_shabbat_lighting_line() . "<br>\n";
}

function yahrzeit_render_controller_summary()
{
    foreach (controller_summary_lines() as $line) {
        echo $line . "<br>\n";
    }
}

function yahrzeit_render_main_page()
{
    $minhag = read_minhag_ini();

    emitHeader(YAHRZEIT_TITLE, YAHRZEIT_TAB);
    emitTopOfScreen(YAHRZEIT_TITLE, yahrzeit_page_description(), YAHRZEIT_HELPFILE);
?>

    <table cellSpacing=0 cellPadding=4 width=90% border=0 class="botBorder">
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
                Scheduled Events
            </td>
            <td class="text">
<?php
                yahrzeit_render_scheduled_events();
?>
                <br>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Controller Summary
            </td>
            <td class="text">
<?php
                yahrzeit_render_controller_summary();
?>
            </td>
            <td id="notused">&nbsp;</td>
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
<br>&nbsp;<br>

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

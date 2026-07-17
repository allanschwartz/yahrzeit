<?php
/*
 * NAME
 *      2panels.php
 *
 * DESCRIPTION
 *      Panel overview screen for the CBS Yahrzeit Wall.
 *
 *      This page displays the static panel geometry for the physical wall and
 *      provides a clickable wall overview for browsing individual panels.
 *
 *      Panel geometry is read from include/panels.inc.php.
 *
 * BLUF
 *      This page is for viewing the physical wall layout.
 *
 *      It should not define panel geometry, calculate yahrzeit dates, or
 *      generate controller command streams directly.
 *
 * NOTES
 *      The physical wall geometry is static application data. The old
 *      add/modify/delete panel workflow has intentionally been removed.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized as a read-only panel overview screen in 2026.
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

const PANELS_TITLE = "View Yahrzeit Panels";
const PANELS_TAB = 3;
const PANELS_HELPFILE = "help/2panels.php";

const PANEL_IMAGE_AREAS = [
    "col1a" => "25,0,140,130",
    "col1b" => "25,131,140,232",
    "col1c" => "25,233,140,334",

    "col2a" => "141,0,240,132",
    "col2b" => "141,133,240,232",
    "col2c" => "141,233,240,334",

    "col3a" => "241,0,337,134",
    "col3b" => "241,135,337,232",
    "col3c" => "241,233,337,332",

    "col4a" => "338,0,429,135",
    "col4b" => "338,136,429,230",
    "col4c" => "338,231,429,326",

    "col5a" => "430,0,517,137",
    "col5b" => "430,138,517,228",
    "col5c" => "430,229,517,321",

    "col6a" => "518,0,599,137",
    "col6b" => "518,138,599,228",
    "col6c" => "518,229,599,318",

    "col7a" => "600,0,686,138",
    "col7b" => "600,139,686,226",
    "col7c" => "600,227,686,317",
];

// -----------------------------------------------------------------------------
// Page metadata
// -----------------------------------------------------------------------------

function panels_description()
{
    return "View the physical Yahrzeit Wall and open any panel to see its assigned memorial names.";
}

// -----------------------------------------------------------------------------
// Rendering helpers
// -----------------------------------------------------------------------------

function panels_render_image_map()
{
    echo "                <map name=\"panelmap\">\n";

    foreach (PANEL_IMAGE_AREAS as $panelId => $coords) {
        echo "                    <area href=\"3singlepanel.php?panel=" . h($panelId) .
             "\" coords=\"" . h($coords) . "\">\n";
    }

    echo "                </map>\n";
    echo "                <img src=\"images/image-21panels.jpg\" usemap=\"#panelmap\" width=\"700\">\n";
}

function panels_person_is_reserved($person)
{
    if (!empty($person['reserved'])) {
        return true;
    }

    $name = trim(($person['firstName'] ?? "") . " " . ($person['lastName'] ?? ""));
    return str_contains(strtoupper($name), "RESERVED");
}

// Count the same valid, non-reserved panel positions shown by the panel-detail
// screen.  A later CSV record at a duplicate position wins, as it does there.
function panels_lit_counts($timestamp)
{
    $cells = [];
    $counts = [];
    $n = yahrzeit_readDB();

    for ($i = 0; $i < $n; $i++) {
        $person = yahrzeit_getObj($i);
        $panelId = $person['panelId'] ?? "";
        $panel = panel_getObj_byId($panelId);
        $row = (int)($person['row'] ?? 0);
        $column = (int)($person['column'] ?? 0);

        if ($panel == null || $row < 1 || $column < 1 ||
            $row > $panel['nRows'] || $column > $panel['nCols']) {
            continue;
        }

        $cells[$panelId]["$row-$column"] = $person;
    }

    foreach ($cells as $panelId => $panelCells) {
        foreach ($panelCells as $person) {
            if (!panels_person_is_reserved($person) &&
                yahrzeit_person_should_light_now($person, $timestamp)) {
                $counts[$panelId] = ($counts[$panelId] ?? 0) + 1;
            }
        }
    }

    return $counts;
}

function panels_render_geometry_table($timestamp)
{
    $litCounts = panels_lit_counts($timestamp);
?>
                <table border="2">
                    <tr class="text">
                        <th>Panel ID</th>
                        <th>Geometry</th>
                        <th>Capacity</th>
                        <th>Lit LEDs</th>
                    </tr>

<?php
    for ($i = 0; $i < panel_numrows(); $i++) {
        $panel = panel_getObj($i);
        $panelId = $panel['panelId'];
?>
                    <tr class="text">
                        <td>
                            <a href="3singlepanel.php?panel=<?php echo h($panelId); ?>">
                                <?php echo h($panelId); ?>
                            </a>
                        </td>
                        <td>
                            <?php echo h($panel['nRows']); ?> &times;
                            <?php echo h($panel['nCols']); ?>
                        </td>
                        <td>
                            <?php echo h($panel['nNames']); ?> places
                        </td>
                        <td>
                            <?php echo h($litCounts[$panelId] ?? 0); ?>
                        </td>
                    </tr>
<?php
    }
?>
                </table>
<?php
}

function panels_render_main_page()
{
    $timestamp = time();

    emitHeader(PANELS_TITLE, PANELS_TAB);
    emitTopOfScreen(PANELS_TITLE, panels_description(), PANELS_HELPFILE);
?>

    <table cellspacing="0" cellpadding="4" width="100%" border="0" class="botBorder">
        <tr>
            <td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText">Yahrzeit Panels</span>
            </td>
        </tr>

        <tr>
            <td colspan="3" align="center">
<?php
    panels_render_image_map();
?>
            </td>
        </tr>

        <tr>
            <td colspan="3" align="center">
<?php
    panels_render_geometry_table($timestamp);
?>
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

function panels_main()
{
    $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

    if ($method == 'GET') {
        panels_render_main_page();
        return;
    }

    die("This script only works with GET requests.");
}

panels_main();

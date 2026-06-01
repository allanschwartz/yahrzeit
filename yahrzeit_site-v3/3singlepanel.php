<?php
/*
 * NAME
 *      3singlepanel.php
 *
 * DESCRIPTION
 *      Single-panel detail view for the CBS Yahrzeit Wall.
 *
 *      This page displays one physical Yahrzeit Wall panel as a grid of
 *      assigned memorial locations. Each occupied position shows the memorial
 *      name assigned to that LED location. Empty positions are shown as open.
 *
 *      Panel geometry is read from include/panels.inc.php. Memorial locations
 *      are read from include/names.inc.php.
 *
 *      This page is a visual browser for the wall layout. It does not edit
 *      panel geometry or memorial records.
 *
 * BLUF
 *      This page shows WHO is assigned to WHERE on one physical panel.
 *
 *      It should not define panel geometry, edit memorial records, calculate
 *      yahrzeit schedules, or generate controller commands.
 *
 * NOTES
 *      The displayed grid is based on the memorial database and static panel
 *      geometry. It is not a live electrical query of the controller.
 *
 *      If a name appears in the wrong position, use the Reports page to run
 *      the database audit and then correct the CSV memorial database.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized as a read-only panel-detail view in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

require_once "include/misc.inc.php";
require_once "include/panels.inc.php";
require_once "include/names.inc.php";
require_once "include/date_support.inc.php";

const SINGLE_PANEL_TITLE = "Single Yahrzeit Panel";
const SINGLE_PANEL_DESCRIPTION =
    "View the memorial names assigned to one physical Yahrzeit Wall panel. " .
    "Click a name to view the memorial record for that individual.";
const SINGLE_PANEL_TAB = 2;
const SINGLE_PANEL_HELPFILE = "help/3singlepanel.php";


// -----------------------------------------------------------------------------
// Data helpers
// -----------------------------------------------------------------------------

function single_panel_person_display_name($person)
{
    $name = trim(($person['firstName'] ?? "") . " " . ($person['lastName'] ?? ""));
    return $name != "" ? $name : "(unnamed)";
}

function single_panel_person_is_reserved($person)
{
    if (!empty($person['reserved'])) {
        return true;
    }

    return str_contains(strtoupper(single_panel_person_display_name($person)), "RESERVED");
}

function single_panel_build_complete_panel($panelid)
{
    $completePanel = [];
    $n = yahrzeit_readDB();

    for ($i = 0; $i < $n; $i++) {
        $person = yahrzeit_getObj($i);

        if (($person['panelId'] ?? "") != $panelid) {
            continue;
        }

        $row = (int)($person['row'] ?? 0);
        $col = (int)($person['column'] ?? 0);

        if ($row <= 0 || $col <= 0) {
            continue;
        }

        $completePanel[$row][$col] = $person;
    }

    return $completePanel;
}

function single_panel_load_panel($panelid)
{
    return $panelid == "" ? null : panel_getObj_byId($panelid);
}


// -----------------------------------------------------------------------------
// Rendering helpers
// -----------------------------------------------------------------------------

function single_panel_render_message_page($message)
{
    emitHeader(SINGLE_PANEL_TITLE, SINGLE_PANEL_TAB);
    emitMessagePage($message, "click here to return to the Panels page", "1viewpanels.php");
    emitFooter();
}

function single_panel_render_cell($person, $cellWidth)
{
    echo '<td width="' . h($cellWidth) . '%">';
    echo '<table><tr class="text">';
    echo '<td><img src="images/ledoff.gif"></td>';
    echo '<td valign="center">';

    if ($person == null) {
        echo '<span class="textSmall"><i>(open)</i></span>';
    } elseif (single_panel_person_is_reserved($person)) {
        echo '<span class="textSmall"><i>reserved</i></span>';
    } else {
        $name = single_panel_person_display_name($person);
        $key = $person['index'] ?? "";
        echo '<a href="5singlename.php?row=' . h($key) . '">' . h($name) . '</a>';
    }

    echo '</td>';
    echo '</tr></table>';
    echo '</td>';
}

function single_panel_render_panel_table($panel, $completePanel)
{
    $nRows = (int)$panel['nRows'];
    $nCols = (int)$panel['nCols'];
    $cellWidth = ($nCols > 0) ? floor(100 / $nCols) : 16;

    echo '<table border="2">';

    for ($row = 1; $row <= $nRows; $row++) {
        echo '<tr class="text">';

        for ($col = 1; $col <= $nCols; $col++) {
            single_panel_render_cell($completePanel[$row][$col] ?? null, $cellWidth);
        }

        echo '</tr>';
    }

    echo '</table>';
}

function single_panel_render_main_page($panelid, $panel, $completePanel)
{
    $title = SINGLE_PANEL_TITLE . " -- \"" . $panelid . "\"";

    emitHeader($title, SINGLE_PANEL_TAB);
    emitTopOfScreen($title, SINGLE_PANEL_DESCRIPTION, SINGLE_PANEL_HELPFILE);
?>

    <table cellspacing="0" cellpadding="4" width="90%" border="0" class="botBorder">
        <tr>
            <td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText"><?php echo h($panelid); ?></span>
            </td>
        </tr>

        <tr>
            <td colspan="3">
<?php
                single_panel_render_panel_table($panel, $completePanel);
?>
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

function single_panel_main()
{
    $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

    if ($method != 'GET') {
        die("This script only works with GET requests.");
    }

    $panelid = $_GET['panel'] ?? "";

    if ($panelid == "") {
        single_panel_render_message_page("Missing panel ID.");
        return;
    }

    $panel = single_panel_load_panel($panelid);

    if ($panel == null) {
        single_panel_render_message_page("Unknown panel ID: " . h($panelid));
        return;
    }

    single_panel_render_main_page(
        $panelid,
        $panel,
        single_panel_build_complete_panel($panelid)
    );
}

single_panel_main();

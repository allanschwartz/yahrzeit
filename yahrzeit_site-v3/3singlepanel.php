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

    $minhag = read_minhag_ini();

    /*
     * Page metadata used by emitTopOfScreen().
     */
    $panelid = isset($_GET['panel']) ? $_GET['panel'] : "";

    $title = "Single Yahrzeit Panel";
    $description = "View the memorial names assigned to one physical Yahrzeit Wall panel. " .
                "Click a name to view the memorial record for that individual. ";
    $tab = 2;         // Panels
    $helpfile = "help/3singlepanel.php";


    function panel_person_display_name($person)
    {
        $name = trim($person['firstName'] . " " . $person['lastName']);

        if ($name == "") {
            return "(unnamed)";
        }

        return $name;
    }

    function panel_person_is_reserved($person)
    {
        if (isset($person['reserved']) && $person['reserved']) {
            return true;
        }

        $name = strtoupper(panel_person_display_name($person));
        return ($name == "RESERVED" || strpos($name, "RESERVED") !== false);
    }

    function build_complete_panel($panelid)
    {
        $completePanel = array();

        $n = yahrzeit_readDB();

        for ($i = 0; $i < $n; $i++) {
            $person = yahrzeit_getObj($i);

            if (!isset($person['panelId']) || $person['panelId'] != $panelid) {
                continue;
            }

            if (!isset($person['row']) || !isset($person['column'])) {
                continue;
            }

            $row = (int)$person['row'];
            $col = (int)$person['column'];

            if ($row <= 0 || $col <= 0) {
                continue;
            }

            $completePanel[$row][$col] = $person;
        }

        return $completePanel;
    }

    function emit_panel_cell($person, $cellWidth)
    {
        echo '<td width="' . h($cellWidth) . '%">';
        echo '<table><tr class="text">';
        echo '<td><img src="images/ledoff.gif"></td>';
        echo '<td valign="center">';

        if ($person == null) {
            echo '<span class="textSmall"><i>(open)</i></span>';
        }
        elseif (panel_person_is_reserved($person)) {
            echo '<span class="textSmall"><i>reserved</i></span>';
        }
        else {
            $name = panel_person_display_name($person);
            $key = isset($person['index']) ? $person['index'] : "";
            echo '<a href="5singlename.php?row=' . h($key) . '">' . h($name) . '</a>';
        }

        echo '</td>';
        echo '</tr></table>';
        echo '</td>';
    }

    function emit_single_panel_table($panel, $completePanel)
    {
        $nRows = (int)$panel['nRows'];
        $nCols = (int)$panel['nCols'];
        $cellWidth = ($nCols > 0) ? floor(100 / $nCols) : 16;

        echo '<table border="2">';

        for ($row = 1; $row <= $nRows; $row++) {
            echo '<tr class="text">';

            for ($col = 1; $col <= $nCols; $col++) {
                $person = isset($completePanel[$row][$col]) ? $completePanel[$row][$col] : null;
                emit_panel_cell($person, $cellWidth);
            }

            echo '</tr>';
        }

        echo '</table>';
    }

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        if ($panelid == "") {
            emitHeader($title, $tab);
            emitMessagePage(
                "Missing panel ID.",
                "click here to return to the Panels page",
                "1viewpanels.php"
            );
            emitFooter();
            exit;
        }

        panel_readDB();
        $panel = panel_getObj_byId($panelid);

        if ($panel == null) {
            emitHeader($title, $tab);
            emitMessagePage(
                "Unknown panel ID: " . h($panelid),
                "click here to return to the Panels page",
                "1viewpanels.php"
            );
            emitFooter();
            exit;
        }

        $title = "Single Yahrzeit Panel -- \"" . $panelid . "\"";
        $completePanel = build_complete_panel($panelid);

        emitHeader($title, $tab);
        emitTopOfScreen($title, $description, $helpfile);
?>

    <table cellSpacing=0 cellPadding=4 width=90% border=0 class="botBorder">
        <tr><td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25" >
                <span class=boldText><?php echo h($panelid); ?></span>
            </td>
        </tr>

        <tr>
            <td colspan=3>
<?php
                emit_single_panel_table($panel, $completePanel);
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
    } else {
        die("This script only works with GET requests.");
    }

?>

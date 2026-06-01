<?php
/*
 * NAME
 *      1viewpanels.php
 *
 * DESCRIPTION
 *      Panel overview and manual wall-control screen for the CBS Yahrzeit Wall.
 *
 *      This page displays the static panel geometry for the physical wall and
 *      provides a clickable wall overview for browsing individual panels.
 *
 *      It also provides limited manual lighting operations, such as:
 *
 *          - turn all LEDs off
 *          - turn all LEDs on
 *          - turn on Yizkor lighting
 *
 *      Panel geometry is read from include/panels.inc.php. Manual lighting
 *      operations are performed by calling bin/yahrzeit so that the web page
 *      uses the same controller path as the scheduler and command-line tools.
 *
 * BLUF
 *      This page is for viewing the physical wall layout and performing
 *      limited manual wall-wide lighting operations.
 *
 *      It should not define panel geometry, calculate yahrzeit dates, or
 *      generate controller command streams directly.
 *
 * NOTES
 *      Manual lighting operations send commands to the controller immediately.
 *
 *      The physical wall geometry is static application data. The old
 *      add/modify/delete panel workflow has intentionally been removed.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized as a read-only panel overview and manual-control screen
 *      in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

require_once "include/misc.inc.php";
require_once "include/panels.inc.php";

const PANELS_TITLE = "View Yahrzeit Panels";
const PANELS_TAB = 2;
const PANELS_HELPFILE = "help/1viewpanels.php";

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
    $minhag = read_minhag_ini();
    $synagogue = h($minhag['synagogueName'] ?? "Congregation Beth Sholom");

    return "View the physical Yahrzeit panel geometry installed at $synagogue. " .
           "Click a panel in the photo, or click a panel ID in the table, " .
           "to view the names assigned to that panel. " .
           "<p>Below are manual wall-wide lighting operations: all on, all off, and Yizkor. " .
           "These operations send commands to the wall immediately.";
}

// -----------------------------------------------------------------------------
// Manual wall operations
// -----------------------------------------------------------------------------

function run_yahrzeit_operation($operation)
{
    $allowed = [
        "all-off" => "--all-off",
        "all-on"  => "--all-on",
        "yizkor"  => "--yizkor",
    ];

    if (!isset($allowed[$operation])) {
        return [false, "Unsupported lighting operation: $operation"];
    }

    $script = __DIR__ . "/bin/yahrzeit";

    if (!is_executable($script)) {
        return [false, "Yahrzeit command script is not executable: $script"];
    }

    // Local lab override.  Remove this for CBS deployment so bin/yahrzeit
    // uses its default production controller host.
    $allanHost = "192.168.86.240";

    $cmd = escapeshellarg($script) .
           " --host " . escapeshellarg($allanHost) .
           " " . escapeshellarg($allowed[$operation]) . " 2>&1";

    exec($cmd, $output, $rc);

    return [$rc == 0, implode("\n", $output)];
}

function panels_handle_post()
{
    $operation = $_POST['lighting_operation'] ?? "";
    [$ok, $message] = run_yahrzeit_operation($operation);

    panels_render_operation_result_page($operation, $ok, $message);
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

function panels_render_geometry_table()
{
?>
                <table border="2">
                    <tr class="text">
                        <th>Panel ID</th>
                        <th>Geometry</th>
                        <th>Capacity</th>
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
                    </tr>
<?php
    }
?>
                </table>
<?php
}

function panels_render_main_page()
{
    emitHeader(PANELS_TITLE, PANELS_TAB);
    emitTopOfScreen(PANELS_TITLE, panels_description(), PANELS_HELPFILE);
?>

    <form name="viewpanels" action="<?php echo h($_SERVER['PHP_SELF']); ?>" method="POST">

    <table cellspacing="0" cellpadding="4" width="90%" border="0" class="botBorder">
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
    panels_render_geometry_table();
?>
            </td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText">Manual / Special Wall-Wide Lighting Operations</span><br>
                <span class="textSmall">These controls send commands to the wall immediately. Use them only for testing, maintenance, or special operator action.</span>
            </td>
        </tr>

        <tr>
            <td width="35%" height="25" align="left" valign="top" class="text">
                Manual lighting controls
            </td>
            <td width="40%" colspan="2" class="text">
                <button type="submit" name="lighting_operation" value="all-off" class="button">
                    Turn all lights off
                </button>

                <button type="submit" name="lighting_operation" value="all-on" class="button">
                    Turn all lights on
                </button>

                <button type="submit" name="lighting_operation" value="yizkor" class="button">
                    Run Yizkor pattern
                </button>
            </td>
        </tr>

<?php
        emitCopyright();
?>
    </table>
    <br>&nbsp;<br>
    </form>
<?php
    emitFooter();
}

function panels_render_operation_result_page($operation, $ok, $message)
{
    $titleText = $ok ? "Lighting operation completed" : "Lighting operation failed";

    emitHeader(PANELS_TITLE, PANELS_TAB);
    emitTopOfScreen($titleText,
                    "Result from manual lighting operation: " . $operation,
                    PANELS_HELPFILE);
?>

    <table cellspacing="0" cellpadding="4" width="90%" border="0" class="botBorder">
        <tr>
            <td class="text">
                <pre><?php echo h($titleText . "\n\n" . $message); ?></pre>
            </td>
        </tr>
        <tr>
            <td class="text">
                <a href="1viewpanels.php">Return to the Panels page</a>
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

function panels_main()
{
    $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

    if ($method == 'POST') {
        panels_handle_post();
        return;
    }

    if ($method == 'GET') {
        panels_render_main_page();
        return;
    }

    die("This script only works with GET and POST requests.");
}

panels_main();

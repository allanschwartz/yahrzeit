<?php
    require_once "misc.inc.php";
    require_once "panels.inc.php";

    $minhag = read_minhag_ini();

    $title = "View Yahrzeit Panels";
    $description =  "View the fixed Yahrzeit panel geometry installed at " .
                    $minhag['synagogueName'] . ". " .
                    "Click a panel in the photo, or click a panel ID in the table, " .
                    "to view the names etched on that panel. " .
                    "Below are manual lighting operations for Yizkor.";
    $tab = 2;         // Panels

    function run_yahrzeit_operation($operation)
    {
        $script = __DIR__ . "/bin/yahrzeit";

        if (!is_executable($script)) {
            return array(false, "Yahrzeit command script is not executable: $script");
        }

        $allowed = array(
            "all-off" => "--all-off",
            "all-on"  => "--all-on",
            "yizkor"  => "--yizkor",
        );

        if (!isset($allowed[$operation])) {
            return array(false, "Unsupported lighting operation: $operation");
        }

        // Local lab override.  Remove this for CBS deployment so bin/yahrzeit
        // uses its default production controller host.
        $allanHost = "192.168.86.240";

        $cmd = escapeshellcmd($script) .
               " --host " . escapeshellarg($allanHost) .
               " " . $allowed[$operation] . " 2>&1";

        $output = array();
        $rc = 0;
        exec($cmd, $output, $rc);

        return array($rc == 0, implode("\n", $output));
    }


    function emit_panel_image_map()
    {
        $areas = array(
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
        );

        echo "                <map name=\"panelmap\">\n";
        foreach ($areas as $panelId => $coords) {
            echo "                    <area href=\"3singlepanel.php?panel=" . h($panelId) .
                 "\" coords=\"" . h($coords) . "\">\n";
        }
        echo "                </map>\n";
        echo "                <img src=\"images/image-21panels.jpg\" usemap=\"#panelmap\" width=\"700\">\n";
    }


    function emit_panel_geometry_table()
    {
        $n = panel_readDB();
?>

                <table border="2">
                    <tr class="text">
                        <th>Panel ID</th>
                        <th>Geometry</th>
                        <th>Capacity</th>
                    </tr>

<?php
        for ($i = 0; $i < $n; $i++) {
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


    function emit_panels_page($title, $description, $tab)
    {
        emitHeader($title, $tab);
        emitTopOfScreen($title, $description);
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
        emit_panel_image_map();
?>
            </td>
        </tr>

        <tr>
            <td colspan="3" align="center">
<?php
        emit_panel_geometry_table();
?>
            </td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText">Manual / Special Lighting Operations for Yizkor</span><br>
                <span class="textSmall">Use these controls if automatic scheduling is unavailable or incorrect.</span>
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


    function emit_operation_result_page($title, $tab, $operation, $ok, $message)
    {
        $titleText = $ok ? "Lighting operation completed" : "Lighting operation failed";

        emitHeader($title, $tab);
        emitTopOfScreen($titleText, "Result from manual lighting operation: " . $operation);
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

<?php
        emitFooter();
    }


    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        emit_panels_page($title, $description, $tab);
    }
    elseif ($_SERVER['REQUEST_METHOD'] == 'POST') {
        $operation = isset($_POST['lighting_operation']) ? $_POST['lighting_operation'] : "";

        list($ok, $message) = run_yahrzeit_operation($operation);

        emit_operation_result_page($title, $tab, $operation, $ok, $message);
    }
    else {
        die("This script only works with GET and POST requests.");
    }
?>

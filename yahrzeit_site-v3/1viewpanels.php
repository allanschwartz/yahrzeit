<?php 
    require_once "misc.inc.php";
    require_once "panels.inc.php";
    $minhag = read_minhag_ini();

    $title = "View Yahrzeit Panels";
    $description =  "View the fixed Yahrzeit panel geometry installed at " .
                    $minhag['synagogueName'] . ". " .
                    "Click a panel in the photo, or click a panel ID in the table, " .
                    "to view the names etched on that panel. " .
                    "Below are special lighting operations for Yizkor. ";
    $tab = 2;         // Panels

    function run_yahrzeit_operation($operation)
    {
        $script = __DIR__ . "/bin/yahrzeit";

        if (!is_executable($script)) {
            return array(false, "Yahrzeit command script is not executable: $script");
        }

        $allowed = array(
            "all-on"  => "--all-on",
            "all-off" => "--all-off",
            "yizkor"  => "--yizkor",
        );

        if (!isset($allowed[$operation])) {
            return array(false, "Unsupported lighting operation: $operation");
        }
        
        // Local lab override.  Remove this for CBS deployment so bin/yahrzeit
        // uses its default production controller host.
$allanHostArg = "--host 192.168.86.240";
$cmd = escapeshellcmd($script) . " " . $allanHostArg . " " . $allowed[$operation] . " 2>&1";

        $output = array();
        $rc = 0;
        exec($cmd, $output, $rc);
    
        return array($rc == 0, implode("\n", $output));
    }

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        // handle the GET request
        emitHeader( $title, $tab );
?>

<body class="bgNone">
<form name="viewpanels" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="POST" >

<?php
    emitTopOfScreen( $title, $description );
?>

    <table cellSpacing=0 cellPadding=4 width=90% border=0 class="botBorder">
        <tr><td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class=boldText> Yahrzeit Panels </span>
            </td>
        </tr>

        <tr>
            <td colspan=3 align="center">
                <MAP NAME=panelmap>
                <AREA HREF="3singlepanel.php?panel=col1a" COORDS=25,0,140,130>
                <AREA HREF="3singlepanel.php?panel=col1b" COORDS=25,131,140,232>
                <AREA HREF="3singlepanel.php?panel=col1c" COORDS=25,233,140,334>
                <AREA HREF="3singlepanel.php?panel=col2a" COORDS=141,0,240,132>
                <AREA HREF="3singlepanel.php?panel=col2b" COORDS=141,133,240,232>
                <AREA HREF="3singlepanel.php?panel=col2c" COORDS=141,233,240,334>
                <AREA HREF="3singlepanel.php?panel=col3a" COORDS=241,0,337,134>
                <AREA HREF="3singlepanel.php?panel=col3b" COORDS=241,135,337,232>
                <AREA HREF="3singlepanel.php?panel=col3c" COORDS=241,233,337,332>
               <AREA HREF="3singlepanel.php?panel=col4a" COORDS=338,0,429,135>
                <AREA HREF="3singlepanel.php?panel=col4b" COORDS=338,136,429,230>
                <AREA HREF="3singlepanel.php?panel=col4c" COORDS=338,231,429,326>
                <AREA HREF="3singlepanel.php?panel=col5a" COORDS=430,0,517,137>
                <AREA HREF="3singlepanel.php?panel=col5b" COORDS=430,138,517,228>
                <AREA HREF="3singlepanel.php?panel=col5c" COORDS=430,229,517,321>
                <AREA HREF="3singlepanel.php?panel=col6a" COORDS=518,0,599,137>
                <AREA HREF="3singlepanel.php?panel=col6b" COORDS=518,138,599,228>
                <AREA HREF="3singlepanel.php?panel=col6c" COORDS=518,229,599,318>
                <AREA HREF="3singlepanel.php?panel=col7a" COORDS=600,0,686,138>
                <AREA HREF="3singlepanel.php?panel=col7b" COORDS=600,139,686,226>
                <AREA HREF="3singlepanel.php?panel=col7c" COORDS=600,227,686,317>
                </MAP>
                <img src="images/image-21panels.jpg" USEMAP="#panelmap" WIDTH=700>            
            </td>
        </tr>
        <tr>
       
            <td colspan=3 align="center">

<?php
                $n = panel_readDB( );
?>

                <table border=2>
                    <tr class="text">
                        <th>Panel ID</th>
                        <th>Geometry</th>
                        <th>Capacity</th>
                    </tr>

<?php
                    for ( $i = 0; $i < $n; $i++ ) {
                        $panel = panel_getObj( $i ); 
?>

                        <tr class="text">
                            <td>
                                <a href="3singlepanel.php?panel=<?php echo $panel['panelId'] ?>"><?php echo $panel['panelId'] ?></a>
                            </td>
                            <td>
                                <?php echo $panel['nRows'] ?> x
                                <?php echo $panel['nCols'] ?> 
                            </td>
                            <td>
                                <?php echo $panel['nNames'] ?> places
                            </td>
                        </tr>
<?php
                    }
?>
                </table>
            
            </td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class=boldText> Manual / Special Lighting Operations for Yizkor </span><br>
                <span class="textSmall">Use these controls if automatic scheduling is unavailable or incorrect</span><br>
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
        emitCopywrite(); 
?>

    </table>
<br>&nbsp;<br>
</form>
</body>

<?php 
        emitFooter();
    }
    elseif ($_SERVER['REQUEST_METHOD'] == 'POST') {
        emitHeader($title, $tab);

        $operation = "";

        if (isset($_POST['lighting_operation'])) {
            $operation = $_POST['lighting_operation'];
        }

        list($ok, $message) = run_yahrzeit_operation($operation);

        $title_text = $ok ? "Lighting operation completed" : "Lighting operation failed";

        echo "<pre>";
        echo htmlspecialchars($title_text) . "\n\n";
        echo htmlspecialchars($message);
        echo "</pre>";

        emitMessagePage(
            $title_text,
            "click here to return to the Panels page",
            "1viewpanels.php"
        );

    emitFooter();

    } else {
        die ("This script only works with GET and POST requests.");
    }

?>

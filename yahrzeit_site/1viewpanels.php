<?php 
    require_once "misc.inc.php";
    require_once "panels.inc.php";
    $minhag = read_minhag_ini();

    $title = "View Yahrzeit Panels";
    $description =  "View the summary of all Yahrzeit panels installed at " .
                    "${minhag['synagogueName']}. " .
                    "Click on a panel ID to view the names on that panel. " .
                    "Below are special lighting operations for Yizkor. ";
    $tab = 2;         // Panels

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
                        <th>Select</th>
                        <th>Panel ID</th>
                        <th>rows/cols</th>
                        <th>names</th>
                    </tr>

<?php
                    for ( $i = 1; $i < $n; $i++ ) {
                        $panel = panel_getObj( $i ); 
?>

                        <tr class="text">
                            <td>
                                <input type="radio" name="panelrow" 
                                        value="<?php echo $i ?>"
                                        onclick="rurl='2addmodifypanel.php?row=<?php echo $i ?>';" >
                            </td>
                            <td>
                                <a href="3singlepanel.php?panel=<?php echo $panel['panelId'] ?>"><?php echo $panel['panelId'] ?></a>
                            </td>
                            <td>
                                <?php echo $panel['nRows'] ?> rows, 
                                <?php echo $panel['nCols'] ?> columns
                            </td>
                            <td>
                                <?php echo $panel['nNames'] ?> names, 
                                2 lit
                            </td>
                        </tr>
<?php
                    }
?>
                </table>
            
            </td>
        </tr>

        <tr>
            <td colspan="3" align="center">
                <!-- no ADD, no DEL, no MODIFY 
                <input type=submit name=submit value="ADD" class="button"
                    onclick='window.location="2addmodifypanel.php?add";return false;'>
                <input type=submit name=submit value="DELETE" class="button">
                <input type=submit name=submit value="MODIFY" class="button"
                    onclick='window.location=rurl;return false;'>
                    -->
            </td>
        </tr>

        <tr>
            <td colspan="3" height="64"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class=boldText> Special Lighting Operations for Yizkor </span>
            </td>
        </tr>

        <tr>
            <td width="35%" height="25" align="left" valign="top" class="text">
                Turn all lights on or off.
            </td>
            <td width="40%" colspan=2>
            <!--
                <input type="radio" name="alllights" value="1">
                <span class="text">All lights </span> <br>
                <input type="radio" name="alllights" value="2">
                <span class="text">All lights, unless reserved </span> <br>
                &nbsp; &nbsp; &nbsp; 
                <span class="textSmall">All lights unless the space is marked with the placeholder "reserved"</span><br>
                <input type="radio" name="alllights" value="3">
                <span class="text">All lights on selected panel </span> <br>
                &nbsp; &nbsp; &nbsp; 
                <span class="textSmall"> click on a <b>Select</b> button in the above table </span><br>
                --!>
                <span class="text"> All LEDs  </span>
                        <select name="onoff" >
                            <option>On
                            <option>Off
                        </select>
                <span class="text"> panel  </span>
                <input type="text" name="panelno" maxlength="3" size="2" value="" onchange="validateNumber(this, 'testmodeErr', false);" > 
                <input type=submit name=onoff value="TURN ON/OFF" class="button">
            </td>
        </tr>


        <tr>
            <td width="35%" height="25" align="left" valign="top" class="text">
                Test Modes
            </td>
            <td width="40%" class="text">
                panel <input type="text" name="panelno" maxlength="3" size="2" value="" onchange="validateNumber(this, 'testmodeErr', false);" > 
                test <input type="text" name="testno" maxlength="3" size="2" value="" onchange="validateNumber(this, 'testmodeErr', false);" > 
                <input type=submit name=submit value="START TEST" class="button">
            </td>
            <td id="testmodeErr">&nbsp;</td>
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
        // handle POST
        //echo "<pre>"; print_r($_POST); echo "</pre>";

        if ($_POST['submit'] == 'DELETE') {
            $n = panel_readDB( );

            // write panel record
            panel_delObj( $_POST['panelrow'] );
            panel_writeDB();

            // now write a Message Page
            emitHeader( $title, $tab );
            emitMessagePage( "Selected element deleted",
                            "click here to continue",
                            "1viewpanels.php" );
            emitFooter();
        }

    } else {
        die ("This script only works with GET and POST requests.");
    }

?>

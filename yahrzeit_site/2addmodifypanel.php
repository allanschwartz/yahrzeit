<?php 
    require_once "misc.inc.php";
    require_once "panels.inc.php";
    $minhag = read_minhag_ini();

    $title = "Add/Modify Yahrzeit Panels";
    $description =  "Add or Modify a Yahrzeit panel within the table of panels.";
    $tab = 2;         // Panels

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        // handle the GET request
        emitHeader( $title, $tab );

?>

<body class="bgNone">
<form name="modifyPanelForm" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="POST" >

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


<?php
        $n = panel_readDB();

        if ($_GET['row'] ) {
            $rowIndex = $_GET['row'];
        } else {
            $rowIndex = $n;
            $n++;
        }
?>

        <tr>
            <td colspan=3 width="40%" align="center" class="text">
                <table border=2>
                    <tr class="text">
                        <th>Select</th>
                        <th>Panel ID</th>
                        <th>rows/cols</th>
                        <th>names</th>
                        <th>addressing</th>
                    </tr>

<?php
                    for ( $i = 1; $i < $n; $i++ ) {
                        $panel = panel_getObj( $i ); 
                        if ( $i != $rowIndex ) {
?>

                        <tr class="text">
                            <td>
                                <input type="radio" name="panelselect" value="<?php echo $i ?>">
                            </td>
                            <td>
                                <b>
                                <?php echo $panel['panelId'] ?>
                                </b>
                            </td>
                            <td>
                                <?php echo $panel['nRows'] ?> rows
                                <br>
                                <?php echo $panel['nCols'] ?> columns
                                <br>
                                <?php echo $panel['nExtra'] ?> extra
                            </td>
                            <td>
                                <?php echo $panel['nNames'] ?> names
                            </td>
                            <td>
                                <?php echo $panel['connectedTo'] ?>
                                <br>
                                <?php echo $panel['addressingMode'] ?>
                                <br>
                                LED-ID <?php echo $panel['firstLedId'] ?>
                            </td>
                        </tr>

<?php
                        } else {
?>

                        <input type="hidden" name="rowIndex" value="<?php echo $i ?>">
                        <tr class="text" bgcolor="#ffffcc">
                            <td>
                                <input type="radio" name="panelselect" value="<?php echo $i ?>" checked>
                            </td>
                            <td>
                                <input type="text" name="panelId" maxlength="20" size="20" class="formStyle" style="width:100"
                                    value="<?php echo $panel['panelId'] ?>" >
                            </td>
                            <td>
                                <input type="text" name="nRows" maxlength="3" size="2" class="formStyle" style="width:30"
                                    value="<?php echo $panel['nRows'] ?>" > rows
                                <br>
                                <input type="text" name="nCols" maxlength="3" size="2" class="formStyle" style="width:30"
                                    value="<?php echo $panel['nCols'] ?>" > columns
                                <br>
                                <input type="text" name="nExtra" maxlength="3" size="2" class="formStyle" style="width:30"
                                    value="<?php echo $panel['nExtra'] ?>" > extra
                            </td>
                            <td>
                                <input type="text" name="nNames" maxlength="3" size="3" class="formStyle" style="width:40"
                                    value="<?php echo $panel['nNames'] ?>"> names
                            </td>
                            <td>
                                connected to 
                                <select name="connectedTo" class="formStyleSmall">
                                <?php print_option1( $panel['connectedTo'], 
                                    array ('/dev/tty1','/dev/tty2','/dev/tty3') ); ?>
                                </select> 
                                <br>
                                addressed 
                                <select name="addressingMode" class="formStyleSmall">
                                <?php print_option1( $panel['addressingMode'],
                                    array ('row-major','column-major') ); ?>
                                </select> 
                                <br>
                                starting with LED-ID 
                                <input type="text" name="firstLedId" maxlength="4" size="4" class="formStyle" style="width:50"
                                    value="<?php echo $panel['firstLedId'] ?>"> 
                            </td>
                        </tr>
<?php
                        }
                    }
?>

                </table>

            </td>
        </tr>

        <tr>
            <td colspan="3" align="center">
                <input type=submit name=submit value="SAVE" class="button">
                <input type="button" name="CANCEL" value="Cancel" class="button"
                    onclick="javascript:history.back(1);">
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
        // handle POST

        $n = panel_readDB();

        // new panel form data
        $panel = array (
            'panelId'          =>   $_POST['panelId'],
            'nRows'            =>   $_POST['nRows'],
            'nCols'            =>   $_POST['nCols'],
            'nExtra'           =>   $_POST['nExtra'],
            'nNames'           =>   $_POST['nNames'],
            'connectedTo'      =>   $_POST['connectedTo'],
            'firstLedId'       =>   $_POST['firstLedId'],
            'addressingMode'   =>   $_POST['addressingMode']
            );

        // write panel record
        panel_putObj( $_POST['rowIndex'], $panel );
        panel_writeDB();

        // now write a Message Page
        emitHeader( $title, $tab );
        emitMessagePage( "Panel definition saved",
                        "click here to continue",
                        "1viewpanels.php" );
        emitFooter();

    } else {
        die ("This script only works with GET and POST requests.");
    }

?>

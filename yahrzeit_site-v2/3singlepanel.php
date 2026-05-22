<?php 
    require_once "misc.inc.php";
    require_once "panels.inc.php";
    require_once "names.inc.php";
    $minhag = read_minhag_ini();

    $panelid = $_GET['panel'];
    $title = "Single Yahrzeit Panel -- \"$panelid\"";
    $description = "Click on a name to view/modify the Yahrzeit observances for that individual.";
    $tab = 2;         // Panels

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        // handle the GET request
        emitHeader( $title, $tab );
?>

<body class="bgNone" onload="initGS(this.document.forms[0])">

<?php
    emitTopOfScreen( $title, $description );
?>

    <table cellSpacing=0 cellPadding=4 width=90% border=0 class="botBorder">
        <tr><td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25" >
                <span class=boldText> <?php echo $panelid ?> </span>
            </td>
        </tr>

<?php
        $n1 = yahrzeit_readDB();

        for( $i = 0 ; $i < $n1; $i++ ) {
            $person = yahrzeit_getObj( $i );
            if ( $panelid == $person['panelId'] ) {
                $completePanel[ $person['row'] ][ $person['column'] ] = $person;
            }
        }

        //echo "<pre>"; print_r($completePanel); echo "</pre>";
        
        $n2 = panel_readDB();
        $panel = panel_getObj_byId( $panelid );

        //echo "<pre>"; print_r($panel); echo "</pre>";

?>



        <tr>
            <td colspan=3>

                <table border=2>

<?php
                  for ($row = 1; $row <= $panel['nRows']; $row++ ) 
                  {
?>
                    <tr class="text">
<?php
                      for ($col = 1; $col <= $panel['nCols']; $col++ ) 
                      {
                        $person = $completePanel[ $row ][ $col ];
                        if ( $person == null ) {
                            $name = "<i>(open)</i>";
                            $key = "add";
                        } else {
                            $name = $person['firstName']." ".$person['lastName'];
                            $key = $person['index'];
                        }
?>
                        <td width="16%">
<table><tr class="text"><td><img src="ledoff.gif"></td><td valign="center"><a href="5singlename.php?row=<?php echo $key ?>"> <?php echo $name ?></a></td></tr></table>
                        </td>
<?php
                      }
?>
                    </tr>
<?php
                  }
?>
                </table>
                    
            </td>

        </tr>

        <tr>
            <td colspan="3" align="center">
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
        // now write a Message Page
        emitHeader( $title, $tab );
        emitMessagePage( "unknown request",
                        "click here to continue",
                        "1viewpanels.php" );
        emitFooter();

    } else {
        die ("This script only works with GET and POST requests.");
    }

?>

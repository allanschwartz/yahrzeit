<?php 
    require_once "misc.inc.php";
    require_once "panels.inc.php";
    require_once "names.inc.php";
    $minhag = read_minhag_ini();

    $title = "Yahrzeit Names";
    $description = "List all observed Yahrzeits, click on name to view or modify that individual.";
    $tab = 3;         // Names

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        // handle the GET request
        emitHeader( $title, $tab );
?>

<body class="bgNone" onload="initGS(this.document.forms[0])">
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
                <span class=boldText> Yahrzeit Names </span>
            </td>
        </tr>

        <tr>
            
            <td colspan=3 align="center">
<?php
                $n = yahrzeit_readDB();
?>
                <table border=2>
                    <tr class="text">
                        <th>Select</th>
                        <th>Name</th>
                        <th>Yahrzeit Date</th>
                        <th>Hebrew Yahrzeit Date</th>
                        <th>options</th>
                        <th>location</th>
                    </tr>



<?php
                    for ( $i = 1; $i < $n; $i++ ) {
                        $remembered = yahrzeit_getObj( $i );
                        $rec = yahrzeit_map_external( $remembered );
?>

                        <tr class="text">
<?php
                        if ($remembered['firstName'] == "" && $remembered['lastName'] == "" ) {
?>
                            <td colspan=6>
                                <hr>
                            </td>
<?php
                        } else {
?>
                            <td> <!-- Select -->
                                
                                <input type="radio" name="nameselect"
                                        value="<?php echo $i ?>"
                                        onclick="rurl='5singlename.php?row=<?php echo $i ?>';" >
                            </td>
                            <td>  <!-- Name -->
                                <a href="5singlename.php?row=<?php echo $i ?>">
                                     <?php echo $rec[0] ?></a>
                            </td> 
                            <td> <!-- English Yahrzeit Date -->
                                <?php echo $rec[1] ?> 
                            </td>
                            <td> <!-- Hebrew Yahrzeit Date -->
                                <?php echo $rec[2] ?> 
                            </td>
                            <td> <!-- Options -->
                                <?php echo $rec[3] ?> 
                            </td>
                            <td> <!-- Location -->
                                <?php echo $rec[4] ?> 
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
                <input type=submit name=submit value="ADD" class="button"
                    onclick='window.location="5singlename.php?add";return false;'>
                <input type=submit name=submit value="DELETE" class="button">
                <input type=submit name=submit value="MODIFY" class="button"
                    onclick='window.location=rurl;return false;'>
            </td>
        </tr>

        <tr>
            <td colspan="3" height="10"></td>
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

        if ($_POST['submit'] == 'DELETE') {
            $n = yahrzeit_readDB();

            // write panel record
            $n = $_POST['nameselect'];
            yahrzeit_delObj( $n );
            yahrzeit_writeDB();

            // now write a Message Page
            emitHeader( $title, $tab );
            emitMessagePage( "Selected name deleted",
                            "click here to continue",
                            "4viewnames.php" );
            emitFooter();
        }

    } else {
        die ("This script only works with GET and POST requests.");
    }

?>

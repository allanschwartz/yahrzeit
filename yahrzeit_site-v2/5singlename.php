<?php 
    require_once "misc.inc.php";
    require_once "names.inc.php";
    $minhag = read_minhag_ini();

    $title="Single Yahrzeit Name";
    $description = "View or modify this individual's Yahrzeit observance.";
    $tab = 3;         // Names

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        // handle the GET request
        emitHeader( $title, $tab );
?>

<body class="bgNone">
<form name="sngleNameForm" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="POST" >

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
                <span class=boldText> Single Yahrzeit Name </span>
            </td>
        </tr>

<?php
        if ( $_GET['row'] ) {
            $n = yahrzeit_readDB();
            $row = $_GET['row'];
            $person = yahrzeit_getObj( $row );
?>
            <input type="hidden" name="rowIndex" value="<?php echo $row ?>">
<?php
        } else {
            $person = array ("");
?>
            <input type="hidden" name="add" >
<?php
        }
?>
        
        <tr>
            <td height="25" align="left" valign="top" class="text">
                Name
            </td>
            <td class="text" >
                First (and Middle)
                <br>
                <input type="text" name="firstName" maxlength="64" size="25" 
                        value="<?php echo $person['firstName'] ?>"
                        class="formStyle">
            </td>
            <td class="text" >
                Last name
                <br>
                <input type="text" name="lastName" maxlength="64" size="25" 
                        value="<?php echo $person['lastName'] ?>"
                        class="formStyle">
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Yahrzeit Date
            </td>
            <td class="text">
                English
                    <select name="engYzMonth" style="width:50" class="formStyleSmall">
                        <?php print_option1( $person['engYzMonth'], $english_month_names ); ?>
                    </select>
                    <select name="engYzDD" style="width:40" class="formStyleSmall">
                        <?php print_option_n1n2( $person['engYzDD'], 1, 31, "%02d" ); ?>
                    </select>
                    <input type="text" name="engYzYYYY" maxlength="4" size="4" class="formStyle" style="width:40"
                        value="<?php echo $person['engYzYYYY'] ?>" 
                        onchange="validateNumber(this, 'dateErr', false);" >
                <br>
                Hebrew
                    <select name="hebYzDD" style="width:40" class="formStyleSmall">
                        <?php print_option_n1n2( $person['hebYzDD'], 1, 30, "%02d" ); ?>
                    </select>
                    <select name="hebYzMonth" style="width:80" class="formStyleSmall">
                        <?php print_option1( $person['hebYzMonth'], $hebrew_month_names ); ?>
                    </select>
                    <input type="text" name="hebYzYYYY" maxlength="4" size="4" class="formStyle" style="width:40"
                        value="<?php echo $person['hebYzYYYY'] ?>" 
                        onchange="validateNumber(this, 'dateErr', false);" >
            </td>
            <td id="dateErr">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Yahrzeit Location
            </td>
            <td class="text">
                Panel Name &mdash; Row &mdash; Column
                <br>
                <input type="text" name="panelId" maxlength="20" size="20" class="formStyle" style="width:100"
                    value="<?php echo $person['panelId'] ?>" >

                &mdash;
                <input type="text" name="row" maxlength="3" size="2" class="formStyle" style="width:30"
                    value="<?php echo $person['row'] ?>" >
                &mdash;
                <input type="text" name="column" maxlength="3" size="2" class="formStyle" style="width:30"           
                    value="<?php echo $person['column'] ?>"
            </td>
            <td id="locationErr">&nbsp;</td>

        <tr>
            <td colspan="3" height="10"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" valign="top" height="25" >
                <span class=boldText> When is this yahrzeit light lit? </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Observe these dates:
            </td>
            <td class="text">
                The family observes the <br>
                &nbsp; &nbsp; <input type="radio" name="engOrHeb" value="eng"
                    <?php echo ($person['useEng'] ? "checked" : "") ?> >
                    English yahrzeit date <br>
                &nbsp; &nbsp; <input type="radio" name="engOrHeb" value="heb" 
                    <?php echo ($person['useHeb'] ? "checked" : "") ?> >
                    Hebrew yahrzeit date <br>
                &nbsp;
                <br>
                In addition to the yahrzeit, and the synagogue-observed yizkor dates <br>
                &nbsp; &nbsp; <input type="checkbox" name="yomhashoah" value="TRUE"
                    <?php echo ($person['yomhashoah'] ? "checked" : "") ?> >
                    observe Yom HaShoah  <br>
                &nbsp; &nbsp; <input type="checkbox" name="yomhazikaron" value="TRUE" 
                    <?php echo ($person['yomhazikaron'] ? "checked" : "") ?> >
                    observe Yom Hazikaron 
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Light the yahrzeit light based on:
            </td>
            <td class="text" valign="top">
                <input type="radio" name="yzmode" value="auto" 
                    <?php echo (!$person['manual'] ? "checked" : "") ?> >
                    <b>Automatic</b> or calendar driven.<br>
                    &nbsp; &nbsp; &nbsp; &nbsp; Per the observances above.  <br>
                &nbsp; <br>
                <input type="radio" name="yzmode" value="manual"
                    <?php echo ($person['manual'] ? "checked" : "") ?> >
                    <b>Manually</b> turn this light on/off.  <br>
                    &nbsp; &nbsp; &nbsp; &nbsp; turn light 
                    <input type="radio" name="onoff" value="on" checked>on
                    <input type="radio" name="onoff" value="off">off
                <br>
                &nbsp; <br>
                <input type="radio" name="yzmode" value="reserved"
                    <?php echo ($person['reserved'] ? "checked" : "") ?> >
                    <b>Reserved</b> <br>
                    &nbsp; &nbsp; &nbsp; &nbsp; Light is always off. 
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="10" colspan="3"></td>
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

        $n = yahrzeit_readDB();

        // new (or modified) person
        $person = array (
            'lastName'          =>   $_POST['lastName'],
            'firstName'         =>   $_POST['firstName'],
            'engYzMonth'        =>   $_POST['engYzMonth'],
            'engYzDD'           =>   $_POST['engYzDD'],
            'engYzYYYY'         =>   $_POST['engYzYYYY'],
            'hebYzDD'           =>   $_POST['hebYzDD'],
            'hebYzMonth'        =>   $_POST['hebYzMonth'],
            'hebYzYYYY'         =>   $_POST['hebYzYYYY'],
            'useHeb'            =>   ( $_POST['engOrHeb'] == "heb" ),
            'useEng'            =>   ( $_POST['engOrHeb'] != "heb" ),
            'yomhashoah'        =>   ( $_POST['yomhashoah'] == "TRUE" ),
            'yomhazikaron'      =>   ( $_POST['yomhazikaron'] == "TRUE" ),
            'onnow'             =>   ( $_POST['onoff'] == "on" ),
            'reserved'          =>   ( $_POST['yzmode'] == "reserved" ),
            'manual'            =>   ( $_POST['yzmode'] == "manual" ),
            'panelId'           =>   $_POST['panelId'],
            'row'               =>   $_POST['row'],
            'column'            =>   $_POST['column']
            );

        // write person record
        if ( $_POST['add'] ) {
            // add a new record
            $n = yahrzeit_numrows() + 1;
        } else {
            // modify an existing record
            $n = $_POST['rowIndex'];
        }
        yahrzeit_putObj( $n, $person );
        yahrzeit_writeDB();

        // now write a Message Page
        emitHeader( $title, $tab );
        emitMessagePage( "Name definition saved",
                "click here to continue",
                "6viewnames.php" );

        emitFooter();

    } else {
        die ("This script only works with GET and POST requests.");
    }
?>

<?php
/*
 * NAME
 *      5singlename.php
 *
 * DESCRIPTION
 *      Single memorial-record detail screen for the CBS Yahrzeit Wall.
 *
 *      This page displays one memorial record from the live Yahrzeit CSV
 *      database, including name, stored English/Gregorian date, stored Hebrew
 *      date, options, and physical LED location.
 *
 *      The live memorial database is:
 *
 *          data/yahrzeits-rev4.csv
 *
 *      Memorial records are read through include/names.inc.php so that this
 *      page does not depend directly on the CSV column order.
 *
 * BLUF
 *      This page is for reviewing one memorial record in detail.
 *
 *      Editing from this page is legacy behavior and should be treated with
 *      caution. Normal CSV maintenance belongs on the Reports screen.
 *
 * NOTES
 *      This screen currently preserves some older edit/save behavior. Any
 *      future cleanup should either make this page read-only or replace broad
 *      editing with narrow, safe operations.
 *
 *      This page should not calculate scheduled lighting, generate controller
 *      commands, or define panel geometry.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Reviewed during the PHP 8 / Arduino V3 modernization in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

    require_once "include/misc.inc.php";
    require_once "include/names.inc.php";
    require_once "include/date_support.inc.php";

    $minhag = read_minhag_ini();

    /*
     * Page metadata used by emitTopOfScreen().
     */
    $title = "Single Yahrzeit Name";
    $description = "View or modify this individual's Yahrzeit observance.";
    $tab = 3;         // Names
    $helpfile = "help/5singlename.php";

   

    function post_value($name, $default = "")
    {
        return isset($_POST[$name]) ? $_POST[$name] : $default;
    }

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        emitHeader($title, $tab);

        $row = isset($_GET['row']) ? $_GET['row'] : "";
        $is_add = isset($_GET['add']) || $row == "" || $row == "add";

        if ($is_add) {
            $person = yahrzeit_blank_person();
        } else {
            yahrzeit_readDB();

            if (!ctype_digit((string)$row)) {
                die("Invalid name row: " . h($row));
            }

            $person = yahrzeit_getObj((int)$row);

            if ($person == null) {
                die("Unknown name row: " . h($row));
            }
        }
?>

<body class="bgNone">
<form name="singleNameForm" action="<?php echo h($_SERVER['PHP_SELF']) ?>" method="POST">

<?php
    emitTopOfScreen($title, $description, $helpfile);
?>

    <table cellSpacing=0 cellPadding=4 width=90% border=0 class="botBorder">
        <tr><td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class=boldText> Single Yahrzeit Name </span>
            </td>
        </tr>

<?php
        if ($is_add) {
?>
            <input type="hidden" name="add" value="1">
<?php
        } else {
?>
            <input type="hidden" name="rowIndex" value="<?php echo h($row) ?>">
<?php
        }
?>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Name
            </td>
            <td class="text">
                First (and Middle)
                <br>
                <input type="text" name="firstName" maxlength="64" size="25" 
                        value="<?php echo h($person['firstName']) ?>"
                        class="formStyle">
            </td>
            <td class="text">
                Last name
                <br>
                <input type="text" name="lastName" maxlength="64" size="25" 
                        value="<?php echo h($person['lastName']) ?>"
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
                        <?php print_option1($person['engYzMonth'], ENGLISH_MONTH_NAMES); ?>
                    </select>
                    <select name="engYzDD" style="width:40" class="formStyleSmall">
                        <?php print_option_n1n2($person['engYzDD'], 1, 31, "%02d"); ?>
                    </select>
                    <input type="text" name="engYzYYYY" maxlength="4" size="4" class="formStyle" style="width:40"
                        value="<?php echo h($person['engYzYYYY']) ?>" 
                        onchange="validateNumber(this, 'dateErr', false);">
                <br>
                Hebrew
                    <select name="hebYzDD" style="width:40" class="formStyleSmall">
                        <?php print_option_n1n2($person['hebYzDD'], 1, 30, "%02d"); ?>
                    </select>
                    <select name="hebYzMonth" style="width:80" class="formStyleSmall">
                        <?php print_option1($person['hebYzMonth'], HEBREW_MONTH_NAMES); ?>
                    </select>
                    <input type="text" name="hebYzYYYY" maxlength="4" size="4" class="formStyle" style="width:40"
                        value="<?php echo h($person['hebYzYYYY']) ?>" 
                        onchange="validateNumber(this, 'dateErr', false);">
            </td>
            <td id="dateErr">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Yahrzeit Location
            </td>
            <td class="text">
                Panel Name &mdash; Column &mdash; Row
                <br>
                <input type="text" name="panelId" maxlength="20" size="20" class="formStyle" style="width:100"
                    value="<?php echo h($person['panelId']) ?>">
                &mdash;
                <input type="text" name="column" maxlength="3" size="2" class="formStyle" style="width:30"
                    value="<?php echo h($person['column']) ?>">
                &mdash;
                <input type="text" name="row" maxlength="3" size="2" class="formStyle" style="width:30"
                    value="<?php echo h($person['row']) ?>">
            </td>
            <td id="locationErr">&nbsp;</td>
        </tr>

        <tr>
            <td colspan="3" height="10"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" valign="top" height="25">
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
                    <?php echo (!empty($person['useEng']) ? "checked" : "") ?> >
                    English yahrzeit date <br>
                &nbsp; &nbsp; <input type="radio" name="engOrHeb" value="heb" 
                    <?php echo (!empty($person['useHeb']) ? "checked" : "") ?> >
                    Hebrew yahrzeit date <br>
                &nbsp;
                <br>
                In addition to the yahrzeit, and the synagogue-observed Yizkor dates <br>
                &nbsp; &nbsp; <input type="checkbox" name="yomhashoah" value="TRUE"
                    <?php echo (!empty($person['yomhashoah']) ? "checked" : "") ?> >
                    observe Yom HaShoah  <br>
                &nbsp; &nbsp; <input type="checkbox" name="yomhazikaron" value="TRUE" 
                    <?php echo (!empty($person['yomhazikaron']) ? "checked" : "") ?> >
                    observe Yom HaZikaron 
            </td>
            <td>&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Light the yahrzeit light based on:
            </td>
            <td class="text" valign="top">
                <input type="radio" name="yzmode" value="auto" 
                    <?php echo (empty($person['manual']) && empty($person['reserved']) ? "checked" : "") ?> >
                    <b>Automatic</b> or calendar driven.<br>
                    &nbsp; &nbsp; &nbsp; &nbsp; Per the observances above.  <br>
                &nbsp; <br>
                <input type="radio" name="yzmode" value="manual"
                    <?php echo (!empty($person['manual']) ? "checked" : "") ?> >
                    <b>Manually</b> turn this light on/off.  <br>
                    &nbsp; &nbsp; &nbsp; &nbsp; turn light 
                    <input type="radio" name="onoff" value="on"
                        <?php echo (!empty($person['onnow']) ? "checked" : "") ?> >on
                    <input type="radio" name="onoff" value="off"
                        <?php echo (empty($person['onnow']) ? "checked" : "") ?> >off
                <br>
                &nbsp; <br>
                <input type="radio" name="yzmode" value="reserved"
                    <?php echo (!empty($person['reserved']) ? "checked" : "") ?> >
                    <b>Reserved</b> <br>
                    &nbsp; &nbsp; &nbsp; &nbsp; Light is always off. 
            </td>
            <td>&nbsp;</td>
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
        emitCopyright();
?>
    </table>
<br>&nbsp;<br>
</form>

<?php 
        emitFooter();
    } 
    elseif ($_SERVER['REQUEST_METHOD'] == 'POST') {
        yahrzeit_readDB();

        $engOrHeb = post_value('engOrHeb', 'heb');
        $yzmode   = post_value('yzmode', 'auto');

        $person = array(
            'lastName'          => post_value('lastName'),
            'firstName'         => post_value('firstName'),

            'engYzMonth'        => post_value('engYzMonth'),
            'engYzDD'           => post_value('engYzDD'),
            'engYzYYYY'         => post_value('engYzYYYY'),

            'hebYzDD'           => post_value('hebYzDD'),
            'hebYzMonth'        => post_value('hebYzMonth'),
            'hebYzYYYY'         => post_value('hebYzYYYY'),

            'useHeb'            => ($engOrHeb == "heb"),
            'useEng'            => ($engOrHeb != "heb"),
            'yomhashoah'        => (post_value('yomhashoah') == "TRUE"),
            'yomhazikaron'      => (post_value('yomhazikaron') == "TRUE"),

            'onnow'             => (post_value('onoff') == "on"),
            'reserved'          => ($yzmode == "reserved"),
            'manual'            => ($yzmode == "manual"),

            'panelId'           => post_value('panelId'),
            'row'               => post_value('row'),
            'column'            => post_value('column')
        );

        if (isset($_POST['add'])) {
            $row = yahrzeit_record_cound() + 1;
        } else {
            $row = post_value('rowIndex');

            if (!ctype_digit((string)$row)) {
                die("Invalid name row: " . h($row));
            }
        }

        yahrzeit_putObj((int)$row, $person);
        yahrzeit_writeDB();

        emitHeader($title, $tab);
        emitMessagePage(
            "Name definition saved",
            "click here to continue",
            "4viewnames.php"
        );

        emitFooter();

    } else {
        die("This script only works with GET and POST requests.");
    }
?>

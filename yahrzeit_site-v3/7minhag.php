<?php
    require_once "misc.inc.php";
    $minhag = read_minhag_ini();

    //echo "<pre>"; print_r($minhag); echo "</pre>";

    $title = "Yahrzeit Minhag";
    $description = "Special <i>minhag</i> or customs used for Yahrzeit panels " .
                    "specific to {minhag['synagogueName']}. " .
                    "<br>These observances apply uniformly to all individuals. ";
    $tab = 5;         // Minhag

    if ($_SERVER['REQUEST_METHOD'] == 'GET') {
        // handle the GET request
        emitHeader( $title, $tab );

?>

<body class="bgNone">
<form name="minhagForm" action="<?php echo(  $_SERVER['PHP_SELF']  ); ?>" method="POST" >

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
                <span class=boldText> Yahrzeit Application Customization for your Synagogue </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" class="text">
                Synagogue Name
            </td>
            <td >
                <input type="text" name="synagogueName" maxlength="64" size="25" 
                        value="<?php echo(  $minhag['synagogueName']  ); ?>"
                        onchange="validateAlphaNumeric(this, 'synagogueNameErr', false);" class="formStyle">
            </td>
            <td id="synagogueNameErr">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" class="text">
                Synagogue Affiliation
            </td>
            <td class="text">
                <select name="affiliation" class="formStyleSmall">
                    <?php print_option1( $minhag['affiliation'], 
                                array ('Orthodox','Conservative','Reform') ); ?>
                </select>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td colspan="3" height="10"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" valign="top" height="25" >
                <span class=boldText> Synagogue Yahrzeit Minhag </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top">
                <span class="text">
                Yahrzeit Calendar Driven Automation
                </span>
                <br>
                <span class=textSmall>
                Can be overridden for individuals.
                </span
            </td>
            <td colspan=2 class="text">
                <input type="radio" name="yahrzeitEngOrHeb" value="eng"
                        <?php echo ($minhag['yahrzeitEngOrHeb'] == "eng" ? "checked" : ""); ?> >
                        observe the English date <br>
                <input type="radio" name="yahrzeitEngOrHeb" value="heb" 
                        <?php echo ($minhag['yahrzeitEngOrHeb'] == "heb" ? "checked" : ""); ?> >
                        observe the Hebrew date <br>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Yahrzeit Light On/Off Times:
            </td>
            <td colspan=2 class="text">
                <input type="radio" name="yahrzeitLightTime" value="setTime" 
                        <?php echo ($minhag['yahrzeitLightTime'] == 'setTime' ? "checked" : ""); ?> >
                        Set time<br>
                &nbsp; &nbsp; &nbsp; &nbsp; <b>On at</b>
                <select name="yahrzeitLightOnHH" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2( $minhag['yahrzeitLightOnHH'], 1, 12, "%02d" ); ?>
                </select>
                <select name="yahrzeitLightOnMM" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2( $minhag['yahrzeitLightOnMM'], 0, 59, "%02d" ); ?>
                </select>
                <select name="yahrzeitLightOnAmPm" style="width:40" class="formStyleSmall">
                    <?php print_option1( $minhag['yahrzeitLightOnAmPm'], array ('am','pm') ); ?>
                </select>
                <br>

                &nbsp; &nbsp; &nbsp; &nbsp; <b>Off at</b>

                <select name="yahrzeitLightOffHH" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2( $minhag['yahrzeitLightOffHH'], 1, 12, "%02d" ); ?>
                </select>
                <select name="yahrzeitLightOffMM" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2( $minhag['yahrzeitLightOffMM'], 0, 59, "%02d" ); ?>
                </select>
                <select name="yahrzeitLightOffAmPm" style="width:40" class="formStyleSmall">
                    <?php print_option1( $minhag['yahrzeitLightOffAmPm'], array ('am','pm') ); ?>
                </select>
                <br>
                <input type="radio" name="yahrzeitLightTime" value="atSunset"
                        <?php echo ($minhag['yahrzeitLightTime'] == 'atSunset' ? "checked" : ""); ?> >
                    <input type="text" name="yahrzeitMinBefore" maxlength="3" size="3" class="formStyle" style="width:25"
                        value="<?php echo(  $minhag['yahrzeitMinBefore']  ); ?>"
                        onchange="validateNumber(this, 'dateErr', false);" >
                        min. before Sunset to 
                    <input type="text" name="yahrzeitMinAfter" maxlength="3" size="3" class="formStyle" style="width:25"
                        value="<?php echo(  $minhag['yahrzeitMinAfter']  ); ?>"
                        onchange="validateNumber(this, 'dateErr', false);" >
                        min. after sunset
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top">
                <span class="text">
                Extra Lighting Options
                </span>
                <br>
                <span class=textSmall>
                You may choose to extend the yahrzeit lights for more than 24 hours.
                </span>
            </td>
            <td colspan=2 class="text">
                <input type="checkbox" name="yahrzeitPlusShabbat" value="YES" 
                        <?php echo ($minhag['yahrzeitPlusShabbat'] == "YES" ? "checked" : ""); ?> >
                        Include the Shabbat before <br>
                <input type="checkbox" name="yahrzeitFullWeek" value="YES" 
                        <?php echo ($minhag['yahrzeitFullWeek'] == "YES" ? "checked" : ""); ?> >
                        Include the full week from Erev Shabbat before to Erev Shabbat after
            </td>
        </tr>

        <tr>
            <td colspan="3" height="10"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" valign="top" height="25" >
                <span class=boldText> Synagogue Yizkor Minhag </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top">
                <span class=text>Yizkor dates observed </span><br>
                <span class=textSmall>Up to five memorial dates may be selected.
                <br>These extra observances apply to all individuals.</span>
            </td>
            <td class="text">
                <input type="checkbox" name="yizkorYomKippur" value="YES" 
                        <?php echo ($minhag['yizkorYomKippur'] == "YES" ? "checked" : ""); ?> >
                        Yom Kippur <br>
                <input type="checkbox" name="yizkorShmini" value="YES" 
                        <?php echo ($minhag['yizkorShmini'] == "YES" ? "checked" : ""); ?> >
                        Shmini Atzeret <br>
                <input type="checkbox" name="yizkorPesach" value="YES" 
                        <?php echo ($minhag['yizkorPesach'] == "YES" ? "checked" : ""); ?> >
                    <select name="yizkorPesachDay" style="width:50" class="formStyleSmall">
                        <?php print_option2( $minhag['yizkorPesachDay'], 
                            array ('7'=>'7th','8'=>'8th') ); ?>
                    </select>
                    day of Passover <br>
                <input type="checkbox" name="yizkorShavuot" value="YES" 
                        <?php echo ($minhag['yizkorShavuot'] == "YES" ? "checked" : ""); ?> >
                    <select name="yizkorShavuotDay" style="width:50" class="formStyleSmall">
                        <?php print_option2( $minhag['yizkorPesachDay'], 
                            array ('1'=>'1st','2'=>'2nd') ); ?>
                    </select>
                    day of Shavuot <br>
                <input type="checkbox" name="yizkorOther" value="YES"
                        <?php echo ($minhag['yizkorOther'] == "YES" ? "checked" : ""); ?> >
                   Other date: <br>

                &nbsp;&nbsp;&nbsp;
                <input type="radio" name="otherEngOrHeb" value="eng"
                        <?php echo ($minhag['otherEngOrHeb'] == "eng" ? "checked" : ""); ?> >
                        English
                    <select name="otherEngMM" style="width:50" class="formStyleSmall">
                        <?php print_option1( $minhag['otherEngMM'], $english_month_names ); ?>
                    </select>
                    <select name="otherEngDD" style="width:40" class="formStyleSmall">
                        <?php print_option_n1n2( $minhag['otherEngDD'], 1, 31, "%02d" ); ?>
                    </select>
                    <br>
                &nbsp;&nbsp;&nbsp;
                <input type="radio" name="otherEngOrHeb" value="heb"
                        <?php echo ($minhag['otherEngOrHeb'] == "heb" ? "checked" : ""); ?> >
                        Hebrew
                    <select name="otherHebDD" style="width:40" class="formStyleSmall">
                        <?php print_option_n1n2( $minhag['otherHebDD'], 1, 30, "%02d" ); ?>
                    </select>
                    <select name="otherHebMM" style="width:80" class="formStyleSmall">
                        <?php print_option1( $minhag['otherHebMM'], $hebrew_month_names ); ?>
                    </select>
            </td>
            <td id="dateErr">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Yizkor Light On/Off Times:
            </td>
            <td colspan=2 class="text">
                <input type="radio" name="yizkorLightTime" value="setTime" 
                        <?php echo ($minhag['yizkorLightTime'] == 'setTime' ? "checked" : ""); ?> >
                        Set time<br>
                &nbsp; &nbsp; &nbsp; &nbsp; <b>On at</b>
                <select name="yizkorLightOnHH" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2( $minhag['yizkorLightOnHH'], 1, 12, "%02d" ); ?>
                </select>
                <select name="yizkorLightOnMM" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2( $minhag['yizkorLightOnMM'], 0, 59, "%02d" ); ?>
                </select>
                <select name="yizkorLightOnAmPm" style="width:40" class="formStyleSmall">
                    <?php print_option1( $minhag['yizkorLightOnAmPm'], array ('am','pm') ); ?>
                </select>
                <br>

                &nbsp; &nbsp; &nbsp; &nbsp; <b>Off at</b>

                <select name="yizkorLightOffHH" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2( $minhag['yizkorLightOffHH'], 1, 12, "%02d" ); ?>
                </select>
                <select name="yizkorLightOffMM" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2( $minhag['yizkorLightOffMM'], 0, 59, "%02d" ); ?>
                </select>
                <select name="yizkorLightOffAmPm" style="width:40" class="formStyleSmall">
                    <?php print_option1( $minhag['yizkorLightOffAmPm'], array ('am','pm') ); ?>
                </select>
                <br>
                <input type="radio" name="yizkorLightTime" value="atSunset"
                        <?php echo ($minhag['yizkorLightTime'] == 'atSunset' ? "checked" : ""); ?> >
                    <input type="text" name="yizkorMinBefore" maxlength="3" size="3" class="formStyle" style="width:25"
                        value="<?php echo(  $minhag['yizkorMinBefore']  ); ?>"
                        onchange="validateNumber(this, 'dateErr', false);" >
                        min. before Sunset to 
                    <input type="text" name="yizkorMinAfter" maxlength="3" size="3" class="formStyle" style="width:25"
                        value="<?php echo(  $minhag['yizkorMinAfter']  ); ?>"
                        onchange="validateNumber(this, 'dateErr', false);" >
                        min. after sunset
            </td>
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
        //echo "<pre>"; print_r($_POST); echo "</pre>";
        // handle POST

        // $path = "/Users/allan/Sites/yahrzeit/";
        $filename = "data/minhag.ini";
        ini_set("include_path", ".:../:./include:../include:/usr/lib/php");

        // massage Minhag screen configuration data ... only the booleans need to be mapped

        $_POST['yahrzeitPlusShabbat'] = myBool( $_POST['yahrzeitPlusShabbat'] );
        $_POST['yahrzeitFullWeek'] = myBool( $_POST['yahrzeitFullWeek'] );
        $_POST['yizkorYomKippur'] = myBool( $_POST['yizkorYomKippur'] );
        $_POST['yizkorShmini'] = myBool( $_POST['yizkorShmini'] );
        $_POST['yizkorPesach'] = myBool( $_POST['yizkorPesach'] );
        $_POST['yizkorShavuot'] = myBool( $_POST['yizkorShavuot'] );
        $_POST['yizkorOther'] = myBool( $_POST['yizkorOther'] );
        unset($_POST['submit']);

        //echo "<pre>"; print_r($_POST); echo "</pre>";


        if ( write_minhag_ini( $_POST ) < 0 ) {
            die ( "Config write failure \"minhag.ini\".");
        } else {
            emitHeader( $title, $tab );
            emitMessagePage( "Configuration Saved",
                            "click here to continue", 
                            "0yahrzeit.php" );
            emitFooter();
        }

    } else {
        die ("This script only works with GET and POST requests.");
    }
?>

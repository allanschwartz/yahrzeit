<?php
/*
 * NAME
 *      7minhag.php
 *
 * DESCRIPTION
 *      Minhag configuration screen for the CBS Yahrzeit Wall.
 *
 *      This page lets an authorized maintainer review and update the local
 *      synagogue customs that affect Yahrzeit and Yizkor lighting.
 *
 *      The settings include:
 *
 *          - whether yahrzeits follow the English or Hebrew date
 *          - whether English-date yahrzeits begin at nightfall
 *          - whether lights remain on through Shabbat or through the week
 *          - Yizkor start/end times
 *          - which Yizkor holidays are observed
 *          - whether Pesach and Shavuot Yizkor are observed on day 1/2/7/8
 *          - optional Yizkor-style lighting for another Hebrew date
 *
 *      Settings are stored in:
 *
 *          data/minhag.ini
 *
 *      These settings are read by bin/yahrzeit_engine.php when deciding what
 *      should be lit, and by bin/yahrzeit_scheduler when deciding when
 *      scheduled lighting actions are due.
 *
 * BLUF
 *      This page edits synagogue policy, not individual memorial records.
 *
 *      yahrzeit_engine.php decides WHAT should be lit using these settings.
 *      yahrzeit_scheduler decides WHEN scheduled lighting actions are due
 *      using these settings.
 *
 * NOTES
 *      This page writes only known configuration keys. Unknown POST fields
 *      are ignored intentionally.
 *
 *      Checkbox values are saved explicitly as YES or NO so that unchecked
 *      boxes do not leave stale enabled settings behind.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized for PHP 8 and the Arduino V3 controller in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

require_once "include/misc.inc.php";
require_once "include/date_support.inc.php";

const MINHAG_TITLE = "Yahrzeit Minhag";
const MINHAG_TAB = 5;
const MINHAG_HELPFILE = "help/7minhag.php";

function minhag_page_description($minhag)
{
    return "Special <i>minhag</i> or customs used for Yahrzeit Wall " .
           "specific to " . h($minhag['synagogueName'] ?? "") . ". " .
           "<br>These observances apply uniformly to all individuals.";
}

function minhag_post_value($key, $default = "")
{
    return $_POST[$key] ?? $default;
}

function minhag_build_from_post()
{
    return [
            'synagogueName'        => minhag_post_value('synagogueName'),
            'affiliation'          => minhag_post_value('affiliation'),

            'yahrzeitEngOrHeb'     => minhag_post_value('yahrzeitEngOrHeb'),
            'yahrzeitLightOnHH'    => minhag_post_value('yahrzeitLightOnHH'),
            'yahrzeitLightOnMM'    => minhag_post_value('yahrzeitLightOnMM'),
            'yahrzeitLightOnAmPm'  => minhag_post_value('yahrzeitLightOnAmPm'),
            'yahrzeitLightOffHH'   => minhag_post_value('yahrzeitLightOffHH'),
            'yahrzeitLightOffMM'   => minhag_post_value('yahrzeitLightOffMM'),
            'yahrzeitLightOffAmPm' => minhag_post_value('yahrzeitLightOffAmPm'),
            'yahrzeitLightTime'    => minhag_post_value('yahrzeitLightTime'),
            'yahrzeitMinBefore'    => minhag_post_value('yahrzeitMinBefore'),
            'yahrzeitMinAfter'     => minhag_post_value('yahrzeitMinAfter'),
            'yahrzeitPlusShabbat'  => myBool(minhag_post_value('yahrzeitPlusShabbat')),
            'yahrzeitFullWeek'     => myBool(minhag_post_value('yahrzeitFullWeek')),

            'yizkorYomKippur'      => myBool(minhag_post_value('yizkorYomKippur')),
            'yizkorShmini'         => myBool(minhag_post_value('yizkorShmini')),
            'yizkorPesach'         => myBool(minhag_post_value('yizkorPesach')),
            'yizkorPesachDay'      => minhag_post_value('yizkorPesachDay'),
            'yizkorShavuot'        => myBool(minhag_post_value('yizkorShavuot')),
            'yizkorShavuotDay'     => minhag_post_value('yizkorShavuotDay'),
            'yizkorOther'          => myBool(minhag_post_value('yizkorOther')),
            'otherEngOrHeb'        => minhag_post_value('otherEngOrHeb'),
            'otherEngMM'           => minhag_post_value('otherEngMM'),
            'otherEngDD'           => minhag_post_value('otherEngDD'),
            'otherHebDD'           => minhag_post_value('otherHebDD'),
            'otherHebMM'           => minhag_post_value('otherHebMM'),

            'yizkorLightTime'      => minhag_post_value('yizkorLightTime'),
            'yizkorLightOnHH'      => minhag_post_value('yizkorLightOnHH'),
            'yizkorLightOnMM'      => minhag_post_value('yizkorLightOnMM'),
            'yizkorLightOnAmPm'    => minhag_post_value('yizkorLightOnAmPm'),
            'yizkorLightOffHH'     => minhag_post_value('yizkorLightOffHH'),
            'yizkorLightOffMM'     => minhag_post_value('yizkorLightOffMM'),
            'yizkorLightOffAmPm'   => minhag_post_value('yizkorLightOffAmPm'),
            'yizkorMinBefore'      => minhag_post_value('yizkorMinBefore'),
            'yizkorMinAfter'       => minhag_post_value('yizkorMinAfter'),
    ];
}

function minhag_render_form($minhag)
{
    emitTopOfScreen(MINHAG_TITLE, minhag_page_description($minhag), MINHAG_HELPFILE);
?>

<form name="minhagForm" action="7minhag.php" method="POST">
    <table cellSpacing="0" cellPadding="4" width="90%" border="0" class="botBorder">
        <tr><td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText"> Yahrzeit Application Customization for your Synagogue </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" class="text">
                Synagogue Name
            </td>
            <td>
                <input type="text" name="synagogueName" maxlength="64" size="25"
                       value="<?php echo h($minhag['synagogueName']); ?>"
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
                    <?php print_option1($minhag['affiliation'], array('Orthodox', 'Conservative', 'Reform')); ?>
                </select>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr><td colspan="3" height="10"></td></tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" valign="top" height="25">
                <span class="boldText"> Synagogue Yahrzeit Minhag </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top">
                <span class="text">Yahrzeit Calendar Driven Automation</span>
                <br>
                <span class="textSmall">Can be overridden for individuals.</span>
            </td>
            <td colspan="2" class="text">
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
            <td colspan="2" class="text">
                <input type="radio" name="yahrzeitLightTime" value="setTime"
                       <?php echo ($minhag['yahrzeitLightTime'] == 'setTime' ? "checked" : ""); ?> >
                       Set time<br>
                &nbsp; &nbsp; &nbsp; &nbsp; <b>On at</b>
                <select name="yahrzeitLightOnHH" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['yahrzeitLightOnHH'], 1, 12, "%02d"); ?>
                </select>
                <select name="yahrzeitLightOnMM" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['yahrzeitLightOnMM'], 0, 59, "%02d"); ?>
                </select>
                <select name="yahrzeitLightOnAmPm" style="width:40" class="formStyleSmall">
                    <?php print_option1($minhag['yahrzeitLightOnAmPm'], array('am', 'pm')); ?>
                </select>
                <br>

                &nbsp; &nbsp; &nbsp; &nbsp; <b>Off at</b>
                <select name="yahrzeitLightOffHH" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['yahrzeitLightOffHH'], 1, 12, "%02d"); ?>
                </select>
                <select name="yahrzeitLightOffMM" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['yahrzeitLightOffMM'], 0, 59, "%02d"); ?>
                </select>
                <select name="yahrzeitLightOffAmPm" style="width:40" class="formStyleSmall">
                    <?php print_option1($minhag['yahrzeitLightOffAmPm'], array('am', 'pm')); ?>
                </select>
                <br>

                <input type="radio" name="yahrzeitLightTime" value="atSunset"
                       <?php echo ($minhag['yahrzeitLightTime'] == 'atSunset' ? "checked" : ""); ?> >
                <input type="text" name="yahrzeitMinBefore" maxlength="3" size="3" class="formStyle" style="width:25"
                       value="<?php echo h($minhag['yahrzeitMinBefore']); ?>"
                       onchange="validateNumber(this, 'dateErr', false);" >
                       min. before Sunset to
                <input type="text" name="yahrzeitMinAfter" maxlength="3" size="3" class="formStyle" style="width:25"
                       value="<?php echo h($minhag['yahrzeitMinAfter']); ?>"
                       onchange="validateNumber(this, 'dateErr', false);" >
                       min. after sunset
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top">
                <span class="text">Extra Lighting Options</span>
                <br>
                <span class="textSmall">You may choose to extend the yahrzeit lights for more than 24 hours.</span>
            </td>
            <td colspan="2" class="text">
                <input type="checkbox" name="yahrzeitPlusShabbat" value="YES"
                       <?php echo ($minhag['yahrzeitPlusShabbat'] == "YES" ? "checked" : ""); ?> >
                       Include the Shabbat before <br>
                <input type="checkbox" name="yahrzeitFullWeek" value="YES"
                       <?php echo ($minhag['yahrzeitFullWeek'] == "YES" ? "checked" : ""); ?> >
                       Include the full week from Erev Shabbat before to Erev Shabbat after
            </td>
        </tr>

        <tr><td colspan="3" height="10"></td></tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" valign="top" height="25">
                <span class="boldText"> Synagogue Yizkor Minhag </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top">
                <span class="text">Yizkor dates observed </span><br>
                <span class="textSmall">Up to five memorial dates may be selected.
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
                    <?php print_option2($minhag['yizkorPesachDay'], array('7' => '7th', '8' => '8th')); ?>
                </select>
                day of Passover <br>

                <input type="checkbox" name="yizkorShavuot" value="YES"
                       <?php echo ($minhag['yizkorShavuot'] == "YES" ? "checked" : ""); ?> >
                <select name="yizkorShavuotDay" style="width:50" class="formStyleSmall">
                    <?php print_option2($minhag['yizkorShavuotDay'], array('1' => '1st', '2' => '2nd')); ?>
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
                    <?php print_option1($minhag['otherEngMM'], ENGLISH_MONTH_NAMES); ?>
                </select>
                <select name="otherEngDD" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['otherEngDD'], 1, 31, "%02d"); ?>
                </select>
                <br>

                &nbsp;&nbsp;&nbsp;
                <input type="radio" name="otherEngOrHeb" value="heb"
                       <?php echo ($minhag['otherEngOrHeb'] == "heb" ? "checked" : ""); ?> >
                       Hebrew
                <select name="otherHebDD" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['otherHebDD'], 1, 30, "%02d"); ?>
                </select>
                <select name="otherHebMM" style="width:80" class="formStyleSmall">
                    <?php print_option1($minhag['otherHebMM'], HEBREW_MONTH_NAMES); ?>
                </select>
            </td>
            <td id="dateErr">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Yizkor Light On/Off Times:
            </td>
            <td colspan="2" class="text">
                <input type="radio" name="yizkorLightTime" value="setTime"
                       <?php echo ($minhag['yizkorLightTime'] == 'setTime' ? "checked" : ""); ?> >
                       Set time<br>
                &nbsp; &nbsp; &nbsp; &nbsp; <b>On at</b>
                <select name="yizkorLightOnHH" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['yizkorLightOnHH'], 1, 12, "%02d"); ?>
                </select>
                <select name="yizkorLightOnMM" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['yizkorLightOnMM'], 0, 59, "%02d"); ?>
                </select>
                <select name="yizkorLightOnAmPm" style="width:40" class="formStyleSmall">
                    <?php print_option1($minhag['yizkorLightOnAmPm'], array('am', 'pm')); ?>
                </select>
                <br>

                &nbsp; &nbsp; &nbsp; &nbsp; <b>Off at</b>
                <select name="yizkorLightOffHH" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['yizkorLightOffHH'], 1, 12, "%02d"); ?>
                </select>
                <select name="yizkorLightOffMM" style="width:40" class="formStyleSmall">
                    <?php print_option_n1n2($minhag['yizkorLightOffMM'], 0, 59, "%02d"); ?>
                </select>
                <select name="yizkorLightOffAmPm" style="width:40" class="formStyleSmall">
                    <?php print_option1($minhag['yizkorLightOffAmPm'], array('am', 'pm')); ?>
                </select>
                <br>

                <input type="radio" name="yizkorLightTime" value="atSunset"
                       <?php echo ($minhag['yizkorLightTime'] == 'atSunset' ? "checked" : ""); ?> >
                <input type="text" name="yizkorMinBefore" maxlength="3" size="3" class="formStyle" style="width:25"
                       value="<?php echo h($minhag['yizkorMinBefore']); ?>"
                       onchange="validateNumber(this, 'dateErr', false);" >
                       min. before Sunset to
                <input type="text" name="yizkorMinAfter" maxlength="3" size="3" class="formStyle" style="width:25"
                       value="<?php echo h($minhag['yizkorMinAfter']); ?>"
                       onchange="validateNumber(this, 'dateErr', false);" >
                       min. after sunset
            </td>
        </tr>

        <tr><td height="10" colspan="3"></td></tr>

        <tr>
            <td colspan="3" align="center">
                <input type="submit" name="submit" value="SAVE" class="button">
            </td>
        </tr>

<?php
        emitCopyright();
?>
    </table>
<br>&nbsp;<br>
</form>

<?php
    }


function minhag_render_main_page()
{
    $minhag = read_minhag_ini();

    emitHeader(MINHAG_TITLE, MINHAG_TAB);
    minhag_render_form($minhag);
    emitFooter();
}

function minhag_render_save_result($ok)
{
    emitHeader(MINHAG_TITLE, MINHAG_TAB);

    if (!$ok) {
        emitMessagePage(
            "Config write failure: minhag.ini",
            "click here to return to Minhag",
            "7minhag.php"
        );
    } else {
        emitMessagePage(
            "Configuration Saved<br><br>" .
            "If you changed Yizkor service times or other scheduled-lighting times, " .
            "review the Minhag page help for the corresponding crontab entries. " .
            "This page saves <code>data/minhag.ini</code>, but it does not install " .
            "or modify cron jobs.",
            "click here to continue",
            "0yahrzeit.php"
        );
    }

    emitFooter();
}

function minhag_handle_post()
{
    $new_minhag = minhag_build_from_post();
    minhag_render_save_result(write_minhag_ini($new_minhag) >= 0);
}

// -----------------------------------------------------------------------------
// Program entry point
// -----------------------------------------------------------------------------

function minhag_main()
{
    $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

    if ($method == 'POST') {
        minhag_handle_post();
        return;
    }

    if ($method == 'GET') {
        minhag_render_main_page();
        return;
    }

    die("This script only works with GET and POST requests.");
}

minhag_main();

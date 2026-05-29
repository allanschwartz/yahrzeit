<?php

/*
 * NAME
 *      misc.inc.php
 *
 * DESCRIPTION
 *      Shared utility functions for the CBS Yahrzeit Wall web application.
 *
 *      This file provides common site helpers used by the screen PHP files
 *      and command-line tools, including:
 *
 *          - site-root and data-file path helpers
 *          - Minhag configuration file read/write support
 *          - HTML escaping and page layout helpers
 *          - top navigation/header/footer rendering
 *          - small form and option-list helpers used by legacy screens
 *
 *      The screen files should include this file before other application
 *      include files.  Command-line programs under bin/ should include it
 *      using dirname(__DIR__) so that cron and shell invocations do not
 *      depend on the current working directory.
 *
 * NOTES
 *      This file intentionally contains general shared support code only.
 *      Date calculations, report generation, panel geometry, name-database
 *      parsing, and LED command generation belong in their more specific
 *      include files or command-line programs.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized for the Arduino V3 controller and PHP 8 in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */


global $tab;



// Absolute filesystem path to yahrzeit_site-v3.
// Because this file lives in include/, dirname(__DIR__) is the site root.
function site_root()
{
    return dirname(__DIR__);
}

function site_url_prefix()
{
    $script = $_SERVER['SCRIPT_NAME'] ?? "";
    return (strpos($script, "/help/") !== false) ? "../" : "";
}

function site_url($path)
{
    return site_url_prefix() . ltrim($path, "/");
}

// Read data/minhag.ini and merge it with defaults.
// The defaults make the application tolerant of older or partially edited
// configuration files. Boolean-like values are normalized to "YES" or "NO".
function read_minhag_ini()
{
    $filename = site_root() . "/data/minhag.ini";
    $minhagDefault = array (
        'synagogueName' => "your synagogue",
        'affiliation' => "Conservative",
        'yahrzeitEngOrHeb' => "heb",
        'yahrzeitLightOnHH' => "06",
        'yahrzeitLightOnMM' => "00",
        'yahrzeitLightOnAmPm' => "pm",
        'yahrzeitLightOffHH' => "06",
        'yahrzeitLightOffMM' => "00",
        'yahrzeitLightOffAmPm' => "pm",
        'yahrzeitLightTime' => "atSunset",
        'yahrzeitMinBefore' => "18",
        'yahrzeitMinAfter' => "72",
        'yahrzeitPlusShabbat' => "YES",
        'yahrzeitFullWeek' => "YES",
        'yizkorYomKippur' => "YES",
        'yizkorShmini' => "YES",
        'yizkorPesach' => "YES",
        'yizkorPesachDay' => "8",
        'yizkorShavuot' => "YES",
        'yizkorShavuotDay' => "2",
        'yizkorOther' => "NO",
        'otherEngOrHeb' => "eng",
        'otherEngMM' => "May",
        'otherEngDD' => "22",
        'otherHebDD' => "01",
        'otherHebMM' => "Tishri",
        'yizkorLightTime' => "setTime",
        'yizkorLightOnHH' => "10",
        'yizkorLightOnMM' => "00",
        'yizkorLightOnAmPm' => "am",
        'yizkorLightOffHH' => "01",
        'yizkorLightOffMM' => "00",
        'yizkorLightOffAmPm' => "pm",
        'yizkorMinBefore' => "18",
        'yizkorMinAfter' => "72",
    );
    if ( file_exists ( $filename ) ) {
        $minhag = parse_ini_file( $filename );
    } else {
        $minhag = $minhagDefault;
    }

    // deal with missing lines in the minhag.ini file
    foreach ( $minhagDefault as $key => $value ) {
        if ( !isset($minhag[$key]) ) {
            $minhag[$key] = $minhagDefault[$key];
        }
    }

    // translate booleans to "myBools"
    $minhag['yahrzeitPlusShabbat'] = myBool( $minhag['yahrzeitPlusShabbat'] );
    $minhag['yahrzeitFullWeek'] = myBool( $minhag['yahrzeitFullWeek'] );
    $minhag['yizkorYomKippur'] = myBool( $minhag['yizkorYomKippur'] );
    $minhag['yizkorShmini'] = myBool( $minhag['yizkorShmini'] );
    $minhag['yizkorPesach'] = myBool( $minhag['yizkorPesach'] );
    $minhag['yizkorShavuot'] = myBool( $minhag['yizkorShavuot'] );
    $minhag['yizkorOther'] = myBool( $minhag['yizkorOther'] );

    return $minhag;
}

function write_minhag_ini( $assoc_arr) 
{

    $filename = site_root() . "/data/minhag.ini";

    $content = "";

    foreach ($assoc_arr as $key=>$elem) {
        if (is_array($elem)) {
            if ($key != '') {
                $content .= "[".$key."]\r\n";                   
            }
           
            foreach ($elem as $key2=>$elem2) {
                $content .= $key2." = ".$elem2."\r\n";
            }
        }
        else {
            $content .= $key." = ".$elem."\r\n";
        }
    }

    if (!$handle = fopen($filename, 'w')) {
        die ("fopen failure");
    }
    if (!fwrite($handle, $content)) {
        die ("fwrite failure");
    }
    fclose($handle);
    return 1;
}

//     this is for numbers 1..30
function print_option_n1n2($selected, $n1, $n2, $fmt) 
{

    if ( $selected == "" ) {
        printf ("                <option selected> </option>\n" );
    }
    for ( $i = $n1; $i <= $n2; $i++ ) {
        printf ("                <option %s> $fmt </option>\n", 
                $i == $selected ? "selected":"", $i);
    }
} 


// $options is a simple array  ('apple', 'orange', 'pear')
function print_option1($selected, $options) 
{
    if ( $selected == "" ) {
        printf ("                <option selected> </option>\n" );
    }
    foreach ($options as $value) {
        printf ("                <option %s> %s </option>\n", 
                $value == $selected ? "selected":"", $value);
    }
} 


// $options is a hash array  ('macintosh' => 'apple', 'navel' => 'orange', 'bartlett' => 'pear')
function print_option2($selected, $options) 
{
    foreach ($options as $value => $text) {
        printf ("<option value=\"%s\" %s> %s </option>\n", 
                $value, $value == $selected ? "selected":"", $text);
    }
} 

// normalize PHP bools and texts representing bools into YES or NO
function myBool($v)
{
    if (is_bool($v)) {
        return $v ? "YES" : "NO";
    }

    $v = strtoupper(trim((string)$v));

    if ($v == "YES" || $v == "Y" || $v == "TRUE" || $v == "1" || $v == "ON") {
        return "YES";
    }

    return "NO";
}

// -----------------------------------------------------------------------------
// GUI rendering helpers
// -----------------------------------------------------------------------------

// Escape dynamic text before inserting it into HTML.
function h($s)
{
    return htmlspecialchars((string)$s, ENT_QUOTES, 'UTF-8');
}


function emitMessagePage( $message, $click_here_msg, $click_here_url ) 
{

$transGif = h(site_url("images/trans.gif"));
$infoIcon = h(site_url("images/info_icon.gif"));
$clickUrl = h(site_url($click_here_url));

$text = <<< ENDOFTEXT

    <table width="400" border="0" align="center" cellpadding="6" cellspacing="0" class="botBorder">
      <tbody> 
        <tr> 
          <td width="370">
            <img src="$transGif" width="1" height="1">
          </td>
        </tr>
        <tr> 
          <td>
            <table width="388" border="0" cellpadding="0" cellspacing="0" class="NobotBorder">
              <tbody>
                <tr> 
                  <td width="53" align="left" valign="top">
                    <img src="$infoIcon" width="43" height="43">
                  </td>
                  <td width="335" align="left" valign="middle">
                    <span class="boldtext">Description</span>
                    <br> 
                    <span class="text">
                
                    $message
                
                    </span> 
                    <br> <br> 
               
                    <span class="text">
                        <a href="$clickUrl"> $click_here_msg </a>
                    </span>
                  </td>
                </tr>
              </tbody>
            </table>
          </td>
        </tr>
      </tbody>
    </table>
ENDOFTEXT;

    echo $text;
}


function emitTopOfScreen($title, $description, $helpfile = "")
{
    $transGif = h(site_url("images/trans.gif"));

    $pageHelpLink = "<br>";
    if ($helpfile !== "") {
        $helpUrl = h(site_url($helpfile));
        $pageHelpLink = <<<ENDOFTEXT
                <a href="$helpUrl"
                   target="WWHFrame" class="textSmallUnderBlue">Page Help</a>
ENDOFTEXT;
    }

    $text = <<<ENDOFTEXT

    <!-- Top of Screen Page Title / Description / Page Help -->
    <table border="0" cellpadding="0" cellspacing="0" width="95%">
        <tr>
            <td width="80%" height="30" valign=bottom class="header1">
                $title
            </td>
            <td width="20%"></td>
        </tr>
        <tr>
            <td colspan="2" height=1 class="hline">
                <img src="$transGif" width=1 height=1>
            </td>
        </tr>
        <tr>
            <td width="80%" class="text">
                $description
            </td>
            <td width="20%"></td>
        </tr>
        <tr>
            <td></td>
            <td align="right">
$pageHelpLink
            </td>
        </tr>
    </table>
ENDOFTEXT;

    echo $text;
}


function toptab ( $selected, $fileref, $tabname ) 
{
    $fileUrl = h(site_url($fileref));

   echo '<td width="14" height="23" class=' . 
    ($selected ? '"tabSelectedBeg"' : '"tabUnselectedBeg"' )."> &nbsp; </td>\n";
   echo '<td class=' . 
    ($selected ? '"tabSelectedBg"' : '"tabUnselectedBg"' ).'>';
   echo '<a href="' . $fileUrl . '" class=' . 
            ($selected ? '"tabTextSel"' : '"tabTextUnsel"' ).'> ' . $tabname . "</a></td>\n";
   echo '<td width="14" class=' . 
    ($selected ? '"tabSelectedEnd"' : '"tabUnselectedEnd"' )."> &nbsp; </td>\n";
}

// Emit the common page shell: HTML head, CBS logo, top tabs, left spacer,
// and the opening table structure used by the legacy screens.
// Each screen is responsible for its own main content and then calls
// emitFooter() to close the structure.
function emitHeader( $title, $tab )
{

$mainHelpUrl = h(site_url("help/user_guide.php"));
$steelBlue = h(site_url("css/SteelBlue.css"));

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<TITLE>Yahrzeit Controller -- <?php echo $title; ?> </TITLE>

<LINK REL="SHORTCUT ICON" HREF="/favicon.ico">
<link href="<?php echo $steelBlue ?>" rel="stylesheet" type="text/css">

</head>
<body class="bgNone">

<!-- BEG entire screen table -->
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr> 
    <td valign="top" class="tabsBg">

<!-- BEG top pane table -->
<table width="99%" border="0" cellspacing="0" class="tabsBg" cellpadding="0">
  <tr height="48">
    <td valign="middle" width="380">
        <img src="<?php echo h(site_url('images/CBS+Primary+Logo+2023-Gold.webp')); ?>"
             width="320"
             style="display:block; margin-left:12px;">
    </td>

    <td valign="middle" align="center">
        <!-- BEG top tabs table -->
        <table border="0" cellspacing="0" cellpadding="0" align="center">
          <tr>
            <?php  toptab ($tab == 1, "0yahrzeit.php", "Yahrzeit" ); ?>
            <?php  toptab ($tab == 2, "1viewpanels.php", "Panels" ); ?>
            <?php  toptab ($tab == 3, "4viewnames.php", "Names" ); ?>
            <?php  toptab ($tab == 4, "6reports.php", "Reports" ); ?>
            <?php  toptab ($tab == 5, "7minhag.php", "Minhag" ); ?>
          </tr>
        </table>
        <!-- END top tabs table -->
    </td>

    <td align="right" valign="middle" width="180">
    <a href="<?php echo $mainHelpUrl ?>" target="PageHelp" class="textSmallUnderBlue"
           target="WWHFrame" class="textSmallUnderBlue">User Guide</a>
    </td>
  </tr>
</table>

    </td>

  </tr>
  <tr>
    <td class="topStrip" height="5"></td>
  </tr>
  <tr>
    <td>
        <!-- BEG outer table around left nav pane and primary pane -->
        <table width="100%" border="0" cellspacing="0" cellpadding="0">
          <tr>
            <td valign="top" width="180">

                <!-- BEG inner table around left nav pane -->
                <table width="100%" border="0" cellspacing="0" cellpadding="0">
                  <tr align="left" valign="top"> 
                    <td height="5" align="right">

                    </td>
                  </tr>

                  <tr>
                    <td valign="top" colspan="2" height="220">

                        <table width="98%" border="0" align="center" class=botBorder cellpadding="1" cellspacing="1">
                          <tr>
                            <td>
                        <!-- left nav bar goes in here-->

                    </td>
                  </tr>
                </table>
                <!-- END inner table around left nav pane -->

            </td>
          </tr>
        </table>
        <!-- END outer table around left nav pane -->

    </td>
    <td width="2"><img src="<?php echo h(site_url('images/trans.gif')); ?>" width=1 height=1></td>

    <td width="4" valign="top" align="left">

    </td>

    <td valign="top">

        <!-- BEG inner table around left nav pane -->
        <table width="100%" cellpadding="0" cellspacing="0" border="0">
          <tr>
            <td>

<head>

<SCRIPT LANGUAGE="JavaScript1.2" SRC="js/CommonValidation.js"></SCRIPT>
<SCRIPT LANGUAGE="JavaScript1.2" SRC="js/CommonMisc.js"></SCRIPT>
<SCRIPT LANGUAGE="JavaScript1.2" SRC="js/GlobalSettings.js"></SCRIPT>

</head>

<?php
}


function emitFooter()
{
?>

                  </td>
            </tr>
         </table>
       </td>    
    </tr>
       </table>

    </td>
  </tr>
</table>

</body>
</html>

<?php
}

// Footer row used inside the main screen content tables.
function emitCopyright()
{
?>

        <tr>
            <td colspan="3" height="30"></td>
        </tr>

        <tr>
            <td colspan="3" height="10">&nbsp;</td>
        </tr>

        <tr>
            <td colspan="3" height="10" >
            <span class="textSmall">Yahrzeit Controller V3</span> <br>
            <span class="textSmall">copyright &copy; 2007, 2015, 2026 AMS Consulting</span> <br>
        </tr>

<?php
}

function emitPageCopyright2()
{
?>

<div class="pageCopyright">
    <span class="textSmall">Yahrzeit Controller V3</span><br>
    <span class="textSmall">copyright &copy; 2007, 2015, 2026 AMS Consulting</span>
</div>

<?php
}


?>

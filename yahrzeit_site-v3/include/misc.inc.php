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
 *      Modernized for PHP 8 and the Yahrzeit V3 release in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */


global $tab;



// Absolute filesystem path to yahrzeit_site-v3. This file lives in include/.
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

/**
 * Read data/minhag.ini and supply defaults for missing configuration keys.
 *
 * Boolean-like Yizkor settings are normalized to "YES" or "NO". An invalid
 * yahrzeitObservance value is replaced with the default.
 *
 * @return array<string, string>
 */
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
        'yahrzeitLightTime' => "atSunset",
        'yahrzeitMinBefore' => "18",
        'yahrzeitObservance' => "week",
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

    // Keep partially edited configuration files usable.
    foreach ( $minhagDefault as $key => $value ) {
        if ( !isset($minhag[$key]) ) {
            $minhag[$key] = $minhagDefault[$key];
        }
    }

    if (!in_array($minhag['yahrzeitObservance'], ['day', 'week'], true)) {
        $minhag['yahrzeitObservance'] = $minhagDefault['yahrzeitObservance'];
    }

    // Normalize the remaining checkbox-style settings.
    $minhag['yizkorYomKippur'] = myBool( $minhag['yizkorYomKippur'] );
    $minhag['yizkorShmini'] = myBool( $minhag['yizkorShmini'] );
    $minhag['yizkorPesach'] = myBool( $minhag['yizkorPesach'] );
    $minhag['yizkorShavuot'] = myBool( $minhag['yizkorShavuot'] );
    $minhag['yizkorOther'] = myBool( $minhag['yizkorOther'] );

    return $minhag;
}

/**
 * Replace data/minhag.ini with the supplied configuration values.
 *
 * @param array<string, mixed> $assoc_arr
 * @return int 1 after a successful write, or -1 after a logged failure.
 */
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

    $directory = dirname($filename);
    if (!is_dir($directory) || !is_writable($directory)) {
        error_log("write_minhag_ini: data directory is not writable: $directory");
        return -1;
    }

    // Write beside the live file and rename it atomically so a partial write
    // cannot leave minhag.ini empty or truncated.
    $temporary = @tempnam($directory, '.minhag-');
    if ($temporary === false) {
        error_log("write_minhag_ini: could not create a temporary file in $directory");
        return -1;
    }

    $written = @file_put_contents($temporary, $content, LOCK_EX);
    if ($written === false || $written !== strlen($content)) {
        error_log("write_minhag_ini: incomplete write to $temporary");
        @unlink($temporary);
        return -1;
    }

    // Keep the file writable by both the installation account and www-data.
    @chmod($temporary, 0664);

    if (!@rename($temporary, $filename)) {
        error_log("write_minhag_ini: could not replace $filename");
        @unlink($temporary);
        return -1;
    }

    return 1;
}

// Emit numeric <option> elements for an inclusive range.
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


// Emit <option> elements from a list of display values.
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


// Emit <option> elements from value => display-text pairs.
function print_option2($selected, $options) 
{
    foreach ($options as $value => $text) {
        printf ("<option value=\"%s\" %s> %s </option>\n", 
                $value, $value == $selected ? "selected":"", $text);
    }
} 

// Normalize PHP booleans and common textual forms to "YES" or "NO".
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

/** Return the controller hostname or address from the shared appliance file. */
function yahrzeit_controller_host()
{
    $path = dirname(__DIR__) . "/bin/yahrzeit-controller.conf";
    if (!is_readable($path)) {
        return "unknown";
    }

    $config = parse_ini_file($path, false, INI_SCANNER_RAW);

    if ($config === false || empty($config['CONTROLLER_HOST'])) {
        return "unknown";
    }

    return trim($config['CONTROLLER_HOST']);
}

// -----------------------------------------------------------------------------
// GUI rendering helpers
// -----------------------------------------------------------------------------

// Escape dynamic text before inserting it into HTML.
function h($s)
{
    return htmlspecialchars((string)$s, ENT_QUOTES, 'UTF-8');
}


/**
 * Emit the standard result-message panel.
 *
 * $message and $click_here_msg may contain trusted application HTML. The link
 * URL is converted to a site-relative URL and escaped here.
 */
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


/**
 * Emit a screen title, trusted HTML description, and optional page-help link.
 */
function emitTopOfScreen($title, $description, $helpfile = "")
{
    $pageHelpLink = "&nbsp;";
    if ($helpfile !== "") {
        $helpUrl = h(site_url($helpfile));
        $pageHelpLink = <<<ENDOFTEXT
                <a href="$helpUrl"
                   target="pagehelp" class="textSmallUnderBlue">Page Help</a>
ENDOFTEXT;
    }

    $text = <<<ENDOFTEXT

    <!-- Top of Screen Page Title / Description / Page Help -->
    <div class="pageHeading">
        <div class="pageHeadingTitle header1">
            $title
        </div>
        <div class="pageHeadingDescription text">
            $description
        </div>
        <div class="pageHeadingHelp">
$pageHelpLink
        </div>
    </div>
ENDOFTEXT;

    echo $text;
}


/**
 * Emit an aligned summary of upcoming Yizkor observances.
 *
 * @param array<int, array{name:string, observance_date:string,
 *     next_date:string}> $observances
 */
function emit_yizkor_observance_table($observances)
{
    if (count($observances) == 0) {
        echo "No Yizkor observances are enabled.<br>\n";
        return;
    }

    echo "<table class=\"yizkorEvents\" cellspacing=\"0\" cellpadding=\"0\">\n";
    foreach ($observances as $observance) {
        echo "<tr>\n";
        echo "<td class=\"yizkorEventName\">" . h($observance['name']) . "</td>\n";
        echo "<td class=\"yizkorHebrewDate\">" . h($observance['observance_date']) . "</td>\n";
        echo "<td class=\"yizkorCivilDate\">" . h($observance['next_date']) . "</td>\n";
        echo "</tr>\n";
    }
    echo "</table>\n";
}


// Emit one navigation tab in the legacy table-based page shell.
function toptab($selected, $fileref, $tabname, $target = "")
{
    $fileUrl = h(site_url($fileref));
    $targetAttribute = $target === "" ? "" : ' target="' . h($target) . '"';

   echo '<td width="14" height="23" class=' . 
    ($selected ? '"tabSelectedBeg"' : '"tabUnselectedBeg"' )."> &nbsp; </td>\n";
   echo '<td class=' . 
    ($selected ? '"tabSelectedBg"' : '"tabUnselectedBg"' ).'>';
   echo '<a href="' . $fileUrl . '"' . $targetAttribute . ' class=' .
            ($selected ? '"tabTextSel"' : '"tabTextUnsel"' ).'> ' . $tabname . "</a></td>\n";
   echo '<td width="14" class=' . 
    ($selected ? '"tabSelectedEnd"' : '"tabUnselectedEnd"' )."> &nbsp; </td>\n";
}

/**
 * Emit the common HTML head, navigation, and opening page-shell markup.
 *
 * Each screen emits its main content and then calls emitFooter() to close the
 * table structure and document.
 */
function emitHeader( $title, $tab )
{

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

<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr> 
    <td valign="top" class="tabsBg">

<table width="100%" border="0" cellspacing="0" class="tabsBg" cellpadding="0">
  <tr height="48">
    <td valign="middle" width="300" class="siteBrandCell">
        <img src="<?php echo h(site_url('images/CBS+Primary+Logo+2023-Gold.webp')); ?>"
             width="300" class="siteLogo" alt="Congregation Beth Sholom Logo"
             style="display:block; margin-left:24px;">
    </td>

    <td valign="middle" align="center">
        <table border="0" cellspacing="0" cellpadding="0" align="center">
          <tr>
            <?php  toptab ($tab == 1, "0yahrzeit.php", "Yahrzeit" ); ?>
            <?php  toptab ($tab == 2, "1yizkor.php", "Yizkor" ); ?>
            <?php  toptab ($tab == 3, "2panels.php", "Panels" ); ?>
            <?php  toptab ($tab == 4, "4names.php", "Names" ); ?>
            <?php  toptab ($tab == 5, "6reports.php", "Reports" ); ?>
            <?php  toptab ($tab == 6, "7minhag.php", "Minhag" ); ?>
            <?php  toptab ($tab == 8, "8userguide.php", "User Guide", "userguide" ); ?>
          </tr>
        </table>
    </td>

    <td width="300" class="siteHeaderSpacer">&nbsp;</td>
  </tr>
</table>

    </td>

  </tr>
  <tr>
    <td class="topStrip" height="5"></td>
  </tr>
  <tr>
    <td>
        <table width="100%" border="0" cellspacing="0" cellpadding="0" class="pageShell">
          <tr>
            <td class="pageSideGutter">&nbsp;</td>
            <td valign="top" class="pageMainContent">

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


/** Close the page-shell markup opened by emitHeader(). */
function emitFooter()
{
?>

                  </td>
            </tr>
         </table>
       </td>    
       <td class="pageSideGutter">&nbsp;</td>
    </tr>
       </table>

    </td>
  </tr>
</table>

</body>
</html>

<?php
}

// Emit the compact footer row used inside a screen's main content table.
function emitCopyright()
{
?>

        <tr>
            <td colspan="3" class="pageCopyrightCell">
                <span class="textSmall">Yahrzeit Controller V3</span><br>
                <span class="textSmall">copyright &copy; 2007, 2015, 2026 AMS Consulting</span>
            </td>
        </tr>

<?php
}

// Emit the standalone div-based copyright block used by help pages.
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

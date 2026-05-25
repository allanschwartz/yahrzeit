<?php
    require_once "include/misc.inc.php";
    require_once "include/panels.inc.php";
    require_once "include/names.inc.php";

    date_default_timezone_set("America/Los_Angeles");

    $minhag = read_minhag_ini();

    $title = "Yahrzeit Controller";
    $description = "These screens control the Yahrzeit panels at " .
                   h($minhag['synagogueName']) . ".";
    $tab = 1;         // Yahrzeit

    function cbs_sunset_timestamp($timestamp)
    {
        // Congregation Beth Sholom, 301 14th Ave, San Francisco, CA.
        $latitude  = 37.78259;
        $longitude = -122.47324;

        $sun = date_sun_info($timestamp, $latitude, $longitude);

        return isset($sun['sunset']) ? $sun['sunset'] : false;
    }


    function cbs_sunset_string($timestamp)
    {
        $sunset = cbs_sunset_timestamp($timestamp);

        if ($sunset === false) {
            return "unknown";
        }

        return date("g:i a", $sunset);
    }


    function next_friday_timestamp()
    {
        $today = strtotime("today");

        if (date("l", $today) == "Friday") {
            return $today;
        }

        return strtotime("next Friday", $today);
    }


    function is_hebrew_leap_year($year)
    {
        return ((7 * (int)$year + 1) % 19) < 7;
    }


    function current_hebrew_date_string()
    {
        $jdDate = gregoriantojd((int)date('n'), (int)date('j'), (int)date('Y'));
        $hebrewDate = jdtojewish($jdDate);
        $hebrewMonthName = jdmonthname($jdDate, 4);

        list($hebrewMonth, $hebrewDay, $hebrewYear) = explode('/', $hebrewDate);

        // PHP's jdmonthname() may use AdarI for Adar in non-leap years.
        if ($hebrewMonthName == "AdarI" && !is_hebrew_leap_year($hebrewYear)) {
            $hebrewMonthName = "Adar";
        }

        return $hebrewDay . " " . $hebrewMonthName . " " . $hebrewYear;
    }


    function controller_summary_lines()
    {
        $panelCount = panel_readDB();
        $nameCount = yahrzeit_readDB();

        return array(
            h($panelCount) . ' panels defined (click on <a href="1viewpanels.php">Panels</a>)',
            h($nameCount) . ' names defined (click on <a href="4viewnames.php">Names</a>)',
            'Manual lighting operations are available from the Panels screen'
        );
    }

    emitHeader($title, $tab);
?>

<?php
    emitTopOfScreen($title, $description);
?>

    <table cellSpacing=0 cellPadding=4 width=90% border=0 class="botBorder">
        <tr>
            <td width="35%"></td>
            <td width="40%"></td>
            <td width="25%"></td>
        </tr>

        <tr>
            <td colspan="3" class="header2Bg" align="left" height="25">
                <span class="boldText">
                    <?php echo h($minhag['synagogueName']); ?> Yahrzeit Controller
                </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Date / Time
            </td>
            <td class="text">
                <?php echo h(date("l F j, Y, g:i a")); ?><br>
                <?php echo h(current_hebrew_date_string()); ?>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Scheduled Events
            </td>
            <td class="text">
<?php
            $todaySunsetText = cbs_sunset_string(time());
            echo "Today's sunset in San Francisco is about " . h($todaySunsetText) . ".<br>";

            $nextFriday = next_friday_timestamp();
            $fridaySunsetTimestamp = cbs_sunset_timestamp($nextFriday);

            if ($fridaySunsetTimestamp === false) {
                echo "This week's Shabbat sunset time is unknown.<br>";
            } else {
                $fridaySunsetText = date("l F j, Y, g:i a", $fridaySunsetTimestamp);
                echo "On " . h($fridaySunsetText) . " this week's yahrzeits will be lit.<br>";
            }
?>
                <br>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Controller Summary
            </td>
            <td class="text">
<?php
                foreach (controller_summary_lines() as $line) {
                    echo $line . "<br>\n";
                }
?>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td colspan=3 align=left>
                <img src="images/image-21panels.jpg" width=700>
            </td>
        </tr>

<?php
        emitCopyright();
?>

    </table>
<br>&nbsp;<br>

<?php 
    emitFooter();
?>

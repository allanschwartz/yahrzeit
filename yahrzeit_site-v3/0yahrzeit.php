<?php
    require_once "misc.inc.php";
    $minhag = read_minhag_ini();

    $title = "Yahrzeit Controller";
    $description = "These screens control all Yahrzeit panels at " . $minhag['synagogueName'] . ".";
    $tab = 1;         // Yahrzeit
    date_default_timezone_set("America/Los_Angeles");

    function cbs_sunset_timestamp($timestamp)
    {
        // Congregation Beth Sholom / San Francisco coordinates.
        // 301 14th Avenue, San Francisco area.
        //
        // date_sunset() is deprecated in modern PHP.  date_sun_info()
        // returns timestamps for sunrise, sunset, and twilight values.
        $latitude  = 37.7793;
        $longitude = -122.4942;

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

        return strtotime("next friday", $today);
    }
    
    emitHeader( $title, $tab );
?>

<body class="bgNone">

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
                <span class=boldText> 
<?php
                echo $minhag['synagogueName']; 
?>
                Yahrzeit Controller </span>
            </td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Date / Time
            </td>
            <td class="text">
<?php
            echo date("l F j, Y, g:i a");

            $gregorianMonth = date('n');
            $gregorianDay   = date('j');
            $gregorianYear  = date('Y');

            $jdDate = gregoriantojd($gregorianMonth, $gregorianDay, $gregorianYear);
            $hebrewDate = jdtojewish($jdDate);
            $hebrewMonthName = jdmonthname($jdDate, 4);

            list($hebrewMonth, $hebrewDay, $hebrewYear) = explode('/', $hebrewDate);

            if ($hebrewMonthName == "AdarI" &&
                $hebrewYear % 19 != 0 &&
                $hebrewYear % 19 != 3 &&
                $hebrewYear % 19 != 6 &&
                $hebrewYear % 19 != 8 &&
                $hebrewYear % 19 != 11 &&
                $hebrewYear % 19 != 14 &&
                $hebrewYear % 19 != 17) {
                    $hebrewMonthName = "Adar";
            }

            echo "<br>$hebrewDay $hebrewMonthName $hebrewYear";
?>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Scheduled Events
            </td>
            <td class="text">
<?php
            $todaySunset = cbs_sunset_string(time());

            $nextFriday = next_friday_timestamp();
            $fridaySunsetTimestamp = cbs_sunset_timestamp($nextFriday);

            if ($fridaySunsetTimestamp === false) {
                echo "Today's sunset time is unknown.<br>";
                echo "This week's Shabbat sunset time is unknown.<br>";
            } else {
                $todaySunsetText = cbs_sunset_string(time());
                $fridaySunsetText = date("l F j, Y, g:i a", $fridaySunsetTimestamp);

                echo "Today's sunset in San Francisco is about $todaySunsetText.<br>";
                echo "On $fridaySunsetText this week's yahrzeits will be lit.<br>";
            }
        //The next Yizkor date, $yname, will be $ymmm $ydd, $yyyyy
        // echo "The next Yizkor date, the eighth day of Pesach, will be April 10, 2007 (22 Nisan 5767)"
?>
                <br>
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td height="25" align="left" valign="top" class="text">
                Controller Parameters
            </td>
            <td class="text">
                21 panels defined (click on <a href="1viewpanels.php">Panels</a>)
                <br>
                23 names are now lit
                <br>
                1432 names defined (click on <a href="4viewnames.php">Names</a>)
                <br>
                3 reports are available
            </td>
            <td id="notused">&nbsp;</td>
        </tr>

        <tr>
            <td colspan=3 align=left>
                <img src="images/image-21panels.jpg" width=700>
            </td>
        </tr>

<?php
        emitCopywrite();
?>

    </table>
<br>&nbsp;<br>
</body>

<?php 
    emitFooter();
?>

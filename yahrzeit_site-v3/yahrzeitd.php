<?php

/*
 * NAME
 *      yahrzeitd.php
 *
 * SYNOPSIS
 *
 * DESCRIPTION
 *      This deamon runs at either sunset or sunrise or some other
 *      time, and turns on LEDs representing YAHRZEITs, or turns them
 *      off (if they are on).
 *
 * NOTES
 *      english date == gregorian calendar
 *      julian is obsolete
 *
 * HISTORY
 *      version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * COPYRIGHT NOTICE
 *      copyright (c) 2008, by Allan M. Schwartz
 *      All rights reserved.
 *
 * BUGS
 *
 *
 * TODO
 *
 *
 *
 * CONTENTS
 *
 *  line    Funtion Declarations
 *  ----    ------------------------------------
 *	  63    function yz_main()
 *	 138    function yz_lighton( $person )
 *	 153    function yz_lightoff( $person )
 *	 169    function yz_process_person( $person )
 *	 207    function engWeekMatch( $month, $day )
 *	 244    function engDayMatch( $month, $day )
 *	 306    function isYahrzeitToday( $person)
 *	 341    function isLeapYear( $year ) 
 *	 361    function haYomShabbat()

 */


require_once "misc.inc.php";
require_once "panels.inc.php";
require_once "names.inc.php";
require_once "leds.inc.php";

// Debug levels for yahrzeitd.php
//
//   -d 0    quiet; command stream only
//   -d 1    high-level execution summary
//   -d 5    database/person processing summary
//   -d 10   date-matching decisions
//   -d 20   English-date matching details
//   -d 30   Hebrew-date conversion details
//   -d 40   verbose per-person diagnostics
//
// Debug output is emitted as comment lines beginning with "#", so it remains
// harmless if accidentally included in a controller command stream.

function debug_level()
{
    global $options;

    return isset($options['d']) ? (int)$options['d'] : 0;
}


function debug_log($level, $message, $line = null)
{
    if (debug_level() >= $level) {
        $tag = ($line === null) ? "debug" : "debug" . $line;
        echo "# ***$tag*** $message\n";
    }
}

global $minhag;
$minhag = read_minhag_ini();

yz_main();



// main function for the "yahrzeitd"
//    does some date calculations
//    reads entire yahrzeit database
//    for each person in database, calls yz_process_person
function yz_main()
{
    global $today_month;
    global $today_day;
    global $today_year;

    global $hebrewDay;
    global $hebrewMonth;
    global $hebrewMonthName;
    global $hebrewYear;
    global $testframework;
    $testframework = false;
    global $options;

    $options = getopt("d:t:");
    //var_dump($options);


    // calculate (normal) gregorian month, day, year
    $today_month = date('n');
    $today_day = date('j');
    $today_year = date('Y');

    // normal path through the code...
    if (! $testframework ) {
            echo "\n# Today is ";
            if (haYomShabbat() ) echo "SHABBAT ";
            echo "$today_month/$today_day/$today_year ... ";

            // calculate jewish month, day, year
            $jdDate = gregoriantojd($today_month,$today_day,$today_year);
            $hebrewMonthName = jdmonthname($jdDate,4);
            $hebrewDate = jdtojewish($jdDate);
            #echo " hebrewDate $hebrewDate ";
            list($hebrewMonth, $hebrewDay, $hebrewYear) = explode('/',$hebrewDate);
            echo "$hebrewDay $hebrewMonthName ($hebrewMonth) $hebrewYear\n";

            led_all( "off" );

            $n1 = panel_readDB();
            $n2 = yahrzeit_readDB();

            for( $i = 0 ; $i < $n2 ; $i++ ) {
                $person = yahrzeit_getObj( $i );

                yz_process_person( $person );
            }

            led_data_refresh();
    }

    // test framework
    if ( $testframework ) {
        for ( $today_day = 1; $today_day <=31; $today_day++ ) {
            echo "\n----------------------------------------------------------------\n\n";
            echo "If today is ";
            if (haYomShabbat() ) echo "SHABBAT ";
            echo "$today_month/$today_day/$today_year ... ";

            // calculate jewish month, day, year
            $jdDate = gregoriantojd($today_month,$today_day,$today_year);
            $hebrewMonthName = jdmonthname($jdDate,4);
            $hebrewDate = jdtojewish($jdDate);
            #echo " hebrewDate $hebrewDate ";
            list($hebrewMonth, $hebrewDay, $hebrewYear) = explode('/',$hebrewDate);
            echo "$hebrewDay $hebrewMonthName ($hebrewMonth) $hebrewYear\n";

            $n1 = panel_readDB();
            $n2 = yahrzeit_readDB();
            if ($n2 > 1000) $n2 = 1000;

            for( $i = 0 ; $i < $n2 ; $i++ ) {
                $person = yahrzeit_getObj( $i );

                yz_process_person( $person );
            }
        }
    }

    led_data_refresh();
    echo "\nsave\n";
}


// main processing loop:
//   for each person in the data base, process and turn it on or off as necessary
function yz_process_person( $person )
{
    global $minhag;

    if ( $person['reserved'] == "YES" ) {
        yz_lightoff( $person );
        return;
    }
    if ( $person['manual']  == "YES" ) {
        yz_lighton( $person );
        return;
    }
    // else 'automatic' operation..
    if ( isYahrzeitToday( $person) ) {
        #echo "debug152 TOMORROW {person['lastName']} ON \n";
        yz_lighton( $person );
        return;
    }
    // check for the yahrzeit this comming week
    if ( ($minhag['yahrzeitPlusShabbat'] == "YES" ) && haYomShabbat() ) {
        if ( isYahrzeitThisWeek( $person ) ) {
            #echo "debug159 SHABBAT {person['lastName']} ON \n";
            yz_lighton( $person );
            return;
        }
    }
    // sometimes we leave the light on for a full week.
    if ( ($minhag['yahrzeitFullWeek'] == "YES" )  && isYahrzeitThisWeek( $person ) ) {
        #echo "debug166 WEEK {person['lastName']} ON \n";
        yz_lighton( $person );
        return;
    }
    yz_lightoff( $person );
    return;
}


// Boolean: does the given MONTH/DAY match between tomorrow and next Erev Shabbat?
function engWeekMatch($month, $day)
{
    global $today_month;
    global $today_day;
    global $today_year;
    global $english_month_mapping;

    $result_code = false;

    if ($month == "" || $day == "") {
        return false;
    }

    // Convert month names such as "Feb" or "May" to month numbers.
    if (!is_numeric($month)) {
        if (!isset($english_month_mapping[$month])) {
            return false;
        }
        $month = $english_month_mapping[$month];
    }

    $month = (int)$month;
    $day   = (int)$day;
    $year  = (int)$today_year;

    if ($month < 1 || $month > 12 || $day < 1 || $day > 31) {
        return false;
    }

    // Special case for Feb 29 in non-leap years.
    if ($month == 2 && $day == 29 && !isLeapYear($year)) {
        $day = 28;
    }

    $yz_date = mktime(0, 0, 0, $month,       $day,       $year);
    $today   = mktime(0, 0, 0, $today_month, $today_day, $today_year);

    // We assume this daemon is normally run in the late afternoon,
    // computing yahrzeits for tomorrow / after sunset.
    //
    // If today is Erev Shabbat, the window is today through next Friday.
    // Otherwise, compute the current Friday-to-Friday week window.
    if (haYomShabbat()) {
        $first_date  = strtotime("today", $today);
        $second_date = strtotime("next friday", $today);
    } else {
        $second_date = strtotime("next friday", $today);
        $first_date  = strtotime("-7 days", $second_date);
    }

    if ($first_date === false || $second_date === false || $yz_date === false) {
        return false;
    }

    if (($first_date <= $yz_date) && ($yz_date <= $second_date)) {
        $result_code = true;
    }

    if (debug_level() >= 20 && $result_code) {
        $printable_rc = $result_code ? "TRUE" : "FALSE";
        debug_log( 20,
            "yz_date $yz_date today $today first_date $first_date second_date $second_date",
            __LINE__
        );
        debug_log( 20,
            "engWeekMatch($month $day) returns $printable_rc",
            __LINE__
        );
    }

    return $result_code;
}


// Boolean: does the given MONTH/DAY match today or tomorrow?
//
// This function is used after yahrzeit dates have been converted to a
// Gregorian month/day for the current Hebrew/civil year.  The daemon is
// normally run late in the day, so "today or tomorrow" is intentional:
// after sunset, tomorrow's yahrzeit may already be the active observance.
function engDayMatch($month, $day)
{
    global $today_month;
    global $today_day;
    global $today_year;
    global $options;
    global $english_month_mapping;

    $result_code = false;

    if ($month == "" || $day == "") {
        return false;
    }

    // Convert month names such as "Feb" or "May" to month numbers.
    if (!is_numeric($month)) {
        if (!isset($english_month_mapping[$month])) {
            return false;
        }
        $month = $english_month_mapping[$month];
    }

    $month = (int) $month;
    $day   = (int) $day;
    $year  = (int) $today_year;

    if ($month < 1 || $month > 12 || $day < 1 || $day > 31) {
        return false;
    }

    // Special case for Feb 29 in non-leap years.
    if ($month == 2 && $day == 29 && !isLeapYear($year)) {
        $day = 28;
    }

    $yz_date  = mktime(0, 0, 0, $month,       $day,       $year);
    $today    = mktime(0, 0, 0, $today_month, $today_day, $today_year);
    $tomorrow = strtotime("tomorrow", $today);

    if ($yz_date == $tomorrow || $yz_date == $today) {
        $result_code = true;
    }

    if (debug_level() >= 20 && $result_code) {
        $printable_rc = $result_code ? "TRUE" : "FALSE";

        debug_log( 20,
            "yz_date $yz_date today $today tomorrow $tomorrow",
            __LINE__
        );

        debug_log( 20,
            "engDayMatch($month $day) returns $printable_rc",
            __LINE__
        );
    }

    return $result_code;
}


// Boolean: is this person's yahrzeit sometime this coming week?
function isYahrzeitThisWeek($person)
{
    global $minhag;
    global $hebrewMonthName;
    global $hebrewYear;
    global $hebrew_month_mapping;

    $result_code = false;
    $heb_or_eng = "";
    $yz_jd_date = "";
    $yz_gregorian_date = "";
    $m = "";
    $mm = "";
    $dd = "";
    $yy = "";

    $use_hebrew =
        isset($minhag['yahrzeitEngOrHeb']) &&
        $minhag['yahrzeitEngOrHeb'] == "heb";

    if (isset($person['useHeb']) && $person['useHeb']) {
        $use_hebrew = true;
    }

    $use_english =
        isset($minhag['yahrzeitEngOrHeb']) &&
        $minhag['yahrzeitEngOrHeb'] == "eng";

    if (isset($person['useEng']) && $person['useEng']) {
        $use_english = true;
    }

    if ($use_hebrew) {
        $heb_or_eng = "heb";

        $heb_month_name = isset($person['hebYzMonth'])
            ? closest_hebrew_month($person['hebYzMonth'])
            : "";

        if ($heb_month_name == "" || !isset($hebrew_month_mapping[$heb_month_name])) {
            return false;
        }

        $m = $hebrew_month_mapping[$heb_month_name];

        if (!isset($person['hebYzDD']) || !is_numeric($person['hebYzDD'])) {
            return false;
        }

        $heb_day = (int)$person['hebYzDD'];
        $heb_year = (int)$hebrewYear;

        if ($heb_day < 1 || $heb_day > 30 || $heb_year <= 0) {
            return false;
        }

        $yz_jd_date = jewishtojd($m, $heb_day, $heb_year);

        // Special case for dates in Tishrei:
        // if the yahrzeit is in Tishrei, and this is Elul,
        // then that yahrzeit is in the NEXT Hebrew year.
        if ($m == 1 && $hebrewMonthName == "Elul") {
            $yz_jd_date = jewishtojd($m, $heb_day, $heb_year + 1);
        }

        // Similar special case if this is the first week of Tishrei
        // and the person's yahrzeit is in Elul.  That Elul was last week,
        // not the coming Elul.
        if ($m == 13 && $hebrewMonthName == "Tishri") {
            $yz_jd_date = jewishtojd($m, $heb_day, $heb_year - 1);
        }

        if (!$yz_jd_date) {
            return false;
        }

        $yz_gregorian_date = JDToGregorian($yz_jd_date);
        $parts = explode('/', $yz_gregorian_date);

        if (count($parts) < 3) {
            return false;
        }

        list($mm, $dd, $yy) = $parts;

        $result_code = engWeekMatch($mm, $dd);
    }
    else if ($use_english) {
        $heb_or_eng = "eng";

        $eng_month = isset($person['engYzMonth']) ? $person['engYzMonth'] : "";
        $eng_day   = isset($person['engYzDD'])    ? $person['engYzDD']    : "";

        $result_code = engWeekMatch($eng_month, $eng_day);
    }

    if (debug_level() >= 30 && $result_code) {
        $printable_rc = $result_code ? "TRUE" : "FALSE";

        $heb_month = isset($person['hebYzMonth']) ? $person['hebYzMonth'] : "";
        $heb_day   = isset($person['hebYzDD'])    ? $person['hebYzDD']    : "";
        $first     = isset($person['firstName'])  ? $person['firstName']  : "";
        $last      = isset($person['lastName'])   ? $person['lastName']   : "";

        debug_log( 30,
            "isYahrzeitThisWeek($heb_or_eng): yz_jd_date=$yz_jd_date yz_gregorian_date=$yz_gregorian_date m=$m hebMon=$heb_month hebDay=$heb_day mm=$mm dd=$dd yy=$yy",
            __LINE__
        );

        debug_log( 30,
            "isYahrzeitThisWeek($heb_or_eng): $first $last returns $printable_rc",
            __LINE__
        );
    }

    return $result_code;
}


// Boolean: is this person's yahrzeit today?
function isYahrzeitToday($person)
{
    global $minhag;
    global $hebrewMonthName;
    global $hebrewYear;
    global $hebrew_month_mapping;

    $result_code = false;
    $heb_or_eng = "";
    $yz_jd_date = "";
    $yz_gregorian_date = "";
    $m = "";
    $mm = "";
    $dd = "";
    $yy = "";

    $use_hebrew =
        isset($minhag['yahrzeitEngOrHeb']) &&
        $minhag['yahrzeitEngOrHeb'] == "heb";

    if (isset($person['useHeb']) && $person['useHeb']) {
        $use_hebrew = true;
    }

    $use_english =
        isset($minhag['yahrzeitEngOrHeb']) &&
        $minhag['yahrzeitEngOrHeb'] == "eng";

    if (isset($person['useEng']) && $person['useEng']) {
        $use_english = true;
    }

    if ($use_hebrew) {
        $heb_or_eng = "heb";

        $heb_month_name = isset($person['hebYzMonth'])
            ? closest_hebrew_month($person['hebYzMonth'])
            : "";

        if ($heb_month_name == "" || !isset($hebrew_month_mapping[$heb_month_name])) {
            return false;
        }

        $m = $hebrew_month_mapping[$heb_month_name];

        if (!isset($person['hebYzDD']) || !is_numeric($person['hebYzDD'])) {
            return false;
        }

        $heb_day = (int)$person['hebYzDD'];
        $heb_year = (int)$hebrewYear;

        if ($heb_day < 1 || $heb_day > 30 || $heb_year <= 0) {
            return false;
        }

        $yz_jd_date = jewishtojd($m, $heb_day, $heb_year);

        // Special case for Tishrei:
        // if the yahrzeit is in Tishrei, and this is Elul,
        // then that yahrzeit is in the NEXT Hebrew year.
        if ($m == 1 && $hebrewMonthName == "Elul") {
            $yz_jd_date = jewishtojd($m, $heb_day, $heb_year + 1);
        }

        // Similar special case if this is the first week of Tishrei
        // and the person's yahrzeit is in Elul. That Elul was last week,
        // not the coming Elul.
        if ($m == 13 && $hebrewMonthName == "Tishri") {
            $yz_jd_date = jewishtojd($m, $heb_day, $heb_year - 1);
        }

        if (!$yz_jd_date) {
            return false;
        }

        $yz_gregorian_date = JDToGregorian($yz_jd_date);
        $parts = explode('/', $yz_gregorian_date);

        if (count($parts) < 3) {
            return false;
        }

        list($mm, $dd, $yy) = $parts;

        $result_code = engDayMatch($mm, $dd);
    }
    else if ($use_english) {
        $heb_or_eng = "eng";

        $eng_month = isset($person['engYzMonth']) ? $person['engYzMonth'] : "";
        $eng_day   = isset($person['engYzDD'])    ? $person['engYzDD']    : "";

        $result_code = engDayMatch($eng_month, $eng_day);
    }

    if (debug_level() >= 30 && $result_code) {
        $printable_rc = $result_code ? "TRUE" : "FALSE";

        $heb_month = isset($person['hebYzMonth']) ? $person['hebYzMonth'] : "";
        $heb_day   = isset($person['hebYzDD'])    ? $person['hebYzDD']    : "";
        $first     = isset($person['firstName'])  ? $person['firstName']  : "";
        $last      = isset($person['lastName'])   ? $person['lastName']   : "";

        debug_log( 30,
            "isYahrzeitToday($heb_or_eng): yz_jd_date=$yz_jd_date yz_gregorian_date=$yz_gregorian_date m=$m hebMon=$heb_month hebDay=$heb_day mm=$mm dd=$dd yy=$yy",
            __LINE__
        );

        debug_log( 30,
            "isYahrzeitToday($heb_or_eng): $first $last returns $printable_rc",
            __LINE__
        );
    }

    return $result_code;
}



/// Boolean: is this person's yahrzeit tomorrow?
function isYahrzeitTomorrow($person)
{
    global $minhag;
    global $hebrewMonthName;
    global $hebrewYear;
    global $hebrew_month_mapping;

    $result_code = false;
    $heb_or_eng = "";
    $yz_jd_date = "";
    $yz_gregorian_date = "";
    $m = "";
    $mm = "";
    $dd = "";
    $yy = "";

    $use_hebrew =
        isset($minhag['yahrzeitEngOrHeb']) &&
        $minhag['yahrzeitEngOrHeb'] == "heb";

    if (isset($person['useHeb']) && $person['useHeb']) {
        $use_hebrew = true;
    }

    $use_english =
        isset($minhag['yahrzeitEngOrHeb']) &&
        $minhag['yahrzeitEngOrHeb'] == "eng";

    if (isset($person['useEng']) && $person['useEng']) {
        $use_english = true;
    }

    if ($use_hebrew) {
        $heb_or_eng = "heb";

        $heb_month_name = isset($person['hebYzMonth'])
            ? closest_hebrew_month($person['hebYzMonth'])
            : "";

        if ($heb_month_name == "" || !isset($hebrew_month_mapping[$heb_month_name])) {
            return false;
        }

        $m = $hebrew_month_mapping[$heb_month_name];

        if (!isset($person['hebYzDD']) || !is_numeric($person['hebYzDD'])) {
            return false;
        }

        $heb_day = (int)$person['hebYzDD'];
        $heb_year = (int)$hebrewYear;

        if ($heb_day < 1 || $heb_day > 30 || $heb_year <= 0) {
            return false;
        }

        $yz_jd_date = jewishtojd($m, $heb_day, $heb_year);

        // Same Tishrei/Elul boundary handling used by isYahrzeitToday().
        if ($m == 1 && $hebrewMonthName == "Elul") {
            $yz_jd_date = jewishtojd($m, $heb_day, $heb_year + 1);
        }

        if ($m == 13 && $hebrewMonthName == "Tishri") {
            $yz_jd_date = jewishtojd($m, $heb_day, $heb_year - 1);
        }

        if (!$yz_jd_date) {
            return false;
        }

        $yz_gregorian_date = JDToGregorian($yz_jd_date);
        $parts = explode('/', $yz_gregorian_date);

        if (count($parts) < 3) {
            return false;
        }

        list($mm, $dd, $yy) = $parts;

        // Historical note:
        // engDayMatch() currently returns true for either today or tomorrow.
        // So this function is not strictly "tomorrow only"; it preserves
        // the legacy behavior.
        $result_code = engDayMatch($mm, $dd);
    }
    else if ($use_english) {
        $heb_or_eng = "eng";

        $eng_month = isset($person['engYzMonth']) ? $person['engYzMonth'] : "";
        $eng_day   = isset($person['engYzDD'])    ? $person['engYzDD']    : "";

        $result_code = engDayMatch($eng_month, $eng_day);
    }

    if (debug_level() >= 30 && $result_code) {
        $printable_rc = $result_code ? "TRUE" : "FALSE";

        $heb_month = isset($person['hebYzMonth']) ? $person['hebYzMonth'] : "";
        $heb_day   = isset($person['hebYzDD'])    ? $person['hebYzDD']    : "";
        $first     = isset($person['firstName'])  ? $person['firstName']  : "";
        $last      = isset($person['lastName'])   ? $person['lastName']   : "";

        debug_log( 30,
            "isYahrzeitTomorrow($heb_or_eng): yz_jd_date=$yz_jd_date yz_gregorian_date=$yz_gregorian_date m=$m hebMon=$heb_month hebDay=$heb_day mm=$mm dd=$dd yy=$yy",
            __LINE__
        );

        debug_log( 30,
            "isYahrzeitTomorrow($heb_or_eng): $first $last returns $printable_rc",
            __LINE__
        );
    }

    return $result_code;
}



// Boolean: is this year a leap year?
function isLeapYear($year)
{
    return (($year % 4) == 0) &&
           ((($year % 100) != 0) || (($year % 400) == 0));
}


// Boolean: is tonight the Sabbath?
//
// This returns true on Friday, because the daemon is normally run in the
// late afternoon to compute the lights for the coming evening / Shabbat.
function haYomShabbat()
{
    global $today_month;
    global $today_day;
    global $today_year;

    $timestamp = mktime(16, 0, 0, $today_month, $today_day, $today_year);
    $dateinfo = getdate($timestamp);

    return ($dateinfo['weekday'] == "Friday");
}

?>

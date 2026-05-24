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

function audit_mode()
{
    global $options;
    return isset($options['a']);
}

function audit_log($message)
{
    if (audit_mode()) {
        echo "# AUDIT: $message\n";
    }
}

function warn_log($message)
{
    echo "# WARNING: $message\n";
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
    global $options;

    $options = getopt("ad:t:");
    //var_dump($options);


    // calculate (normal) gregorian month, day, year
    $today_month = date('n');
    $today_day = date('j');
    $today_year = date('Y');

    // normal path through the code...
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
    echo "\nsave\n";
}


// Process one memorial record.
//
// Normal command-stream mode:
//   - emit "pixel on ..." only when this person's LED should be lit
//   - do not emit per-person "pixel off"; yz_main() already emits "all off"
//
// Audit mode:
//   - emit comment-only ON/OFF/SKIP decisions for every person
//   - useful for validating the database and date logic without sending a
//     huge stream of controller commands
function yz_process_person($person)
{
    global $minhag;

    $first = isset($person['firstName']) ? $person['firstName'] : "";
    $last  = isset($person['lastName'])  ? $person['lastName']  : "";
    $name  = trim($first . " " . $last);

    $panel = isset($person['panelId']) ? $person['panelId'] : "";
    $row   = isset($person['row'])     ? $person['row']     : "";
    $col   = isset($person['column'])  ? $person['column']  : "";

    $location = "$panel-$row-$col";

    // Basic database sanity checks.  These are comment-only diagnostics.
    // They should not prevent processing unless the record cannot possibly
    // map to a physical LED.
    if ($name == "") {
        warn_log("Skipping unnamed memorial record at location $location");
        return;
    }

    if ($panel == "" || $row == "" || $col == "") {
        warn_log("Skipping $name: malformed or incomplete location '$location'");
        return;
    }

    // Person-level options are booleans in the mapped $person object.
    $reserved = isset($person['reserved']) && $person['reserved'];
    $manual   = isset($person['manual'])   && $person['manual'];

    $reason = "";
    $should_light = false;

    if ($reserved) {
        $reason = "reserved";
        $should_light = false;
    }
    else if ($manual) {
        $reason = "manual";
        $should_light = true;
    }
    else if (isYahrzeitToday($person)) {
        $reason = "yahrzeit today";
        $should_light = true;
    }
    else if (
        isset($minhag['yahrzeitPlusShabbat']) &&
        $minhag['yahrzeitPlusShabbat'] == "YES" &&
        haYomShabbat() &&
        isYahrzeitThisWeek($person)
    ) {
        $reason = "plus-Shabbat weekly yahrzeit";
        $should_light = true;
    }
    else if (
        isset($minhag['yahrzeitFullWeek']) &&
        $minhag['yahrzeitFullWeek'] == "YES" &&
        isYahrzeitThisWeek($person)
    ) {
        $reason = "full-week yahrzeit";
        $should_light = true;
    }
    else {
        $reason = "not active";
        $should_light = false;
    }

    if (audit_mode()) {
        $state = $should_light ? "ON" : "OFF";
        audit_log("$state: $name, $location, reason=$reason");
    }

    if (debug_level() >= 40) {
        debug_log( 40,
            "yz_process_person: name='$name' location='$location' reserved=" .
            ($reserved ? "YES" : "NO") .
            " manual=" . ($manual ? "YES" : "NO") .
            " should_light=" . ($should_light ? "YES" : "NO") .
            " reason='$reason'",
            __LINE__
        );
    }

    // In audit mode, do not emit controller commands.
    if (audit_mode()) {
        return;
    }

    // Modern command-stream model:
    // yz_main() already emits "all off"; only active memorial LEDs need
    // explicit "pixel on" commands.
    if ($should_light) {
        yz_lighton($person);
    }

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

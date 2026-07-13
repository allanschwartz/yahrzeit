<?php

/*
 * NAME
 *      date_support.inc.php
 *
 * DESCRIPTION
 *      Shared Gregorian, Hebrew-calendar, sunset, Shabbat, and Yahrzeit
 *      date helper functions for the CBS Yahrzeit Wall application.
 *
 *      This file contains reusable date calculations used by the home/status
 *      page, reports, scheduler/engine logic, and future dashboard summaries.
 *
 * BLUF
 *      Date calculations live here.
 *      Memorial-record parsing lives in names.inc.php.
 *      Panel geometry lives in panels.inc.php.
 *      Controller command generation lives in leds.inc.php.
 *
 * NOTES
 *      "English date" means Gregorian calendar date.
 *
 *      Keep this file limited to date/calendar calculations and small
 *      formatting helpers. Page rendering and report orchestration should
 *      remain in the calling screen or engine file.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Split out as shared date-support code during the PHP 8 /
 *      Arduino V3 modernization in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

// Month names used by the legacy CSV data and form controls.
// Keep these spellings stable unless the CSV parser/writer is updated too.
const ENGLISH_MONTH_NAMES = [
    "Jan", "Feb", "Mar", "Apr", "May", "June",
    "July", "Aug", "Sep", "Oct", "Nov", "Dec"
];

const ENGLISH_MONTH_MAPPING = [
    "Jan" => 1, "Feb" => 2, "Mar" => 3, "Apr" => 4,
    "May" => 5, "June" => 6, "July" => 7, "Aug" => 8,
    "Sep" => 9, "Oct" => 10, "Nov" => 11, "Dec" => 12
];

const HEBREW_MONTH_NAMES = [
    "Tishri", "Heshvan", "Kislev", "Tevet", "Shevat",
    "Adar", "AdarI", "AdarII", "Nisan", "Iyar",
    "Sivan", "Tammuz", "Av", "Elul"
];

const HEBREW_MONTH_MAPPING = [
    "Tishri" => 1, "Heshvan" => 2, "Kislev" => 3, "Tevet" => 4,
    "Shevat" => 5, "Adar" => 6, "AdarI" => 6, "AdarII" => 7,
    "Nisan" => 8, "Iyar" => 9, "Sivan" => 10,
    "Tammuz" => 11, "Av" => 12, "Elul" => 13
];

// Congregation Beth Sholom, 301 14th Ave, San Francisco, CA.
const CBS_LATITUDE = 37.78259;
const CBS_LONGITUDE = -122.47324;

/**
 * Normalize a Hebrew month from legacy CSV text or form input.
 *
 * Common Adar I/II spellings are handled explicitly; other minor misspellings
 * use the closest known month name.
 */
function closest_hebrew_month( $str )
{
    $target = ucfirst( strtolower( $str ));

    if ( $target == "" ) {
        return "";
    }

    if ( $target == "Adar 2" || $target == "Adar2" || $target == "Adar II" || $target == "AdarII" ) {
        return "AdarII";
    }

    if ( $target == "Adar 1" || $target == "Adar1" || $target == "Adar I" || $target == "AdarI" ) {
        return "AdarI";
    }

    $closest = "";
    $shortest = -1;

    foreach (HEBREW_MONTH_NAMES as $word) {
        $lev = levenshtein($target, $word);

        if ($lev == 0) {
            $closest = $word;
            break;
        }

        if ($lev <= $shortest || $shortest < 0) {
            $closest  = $word;
            $shortest = $lev;
        }
    }

    return( $closest );
}

/** Return sunset at CBS for the supplied timestamp, or false if unavailable. */
function cbs_sunset_timestamp($timestamp)
{
    $sun = date_sun_info($timestamp, CBS_LATITUDE, CBS_LONGITUDE);

    return isset($sun['sunset']) ? $sun['sunset'] : false;
}


/** Return CBS sunset as a display time such as "7:42 pm". */
function cbs_sunset_time_string($timestamp)
{
    $sunset = cbs_sunset_timestamp($timestamp);

    if ($sunset === false) {
        return "unknown";
    }

    return date("g:i a", $sunset);
}


// Return midnight on today when today is Friday, otherwise next Friday.
function next_erev_shabbat_timestamp()
{
    $today = strtotime("today");

    if (date("l", $today) == "Friday") {
        return $today;
    }

    return strtotime("next Friday", $today);
}


/** Return whether a Hebrew year is one of the seven leap years in its cycle. */
function is_hebrew_leap_year($year)
{
    return ((7 * (int)$year + 1) % 19) < 7;
}


/** Return today's civil-date Hebrew equivalent as "day month year". */
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

/**
 * Populate the legacy Gregorian and Hebrew date-context globals.
 *
 * Policy helpers that consume those globals must call this first for the
 * timestamp being evaluated.
 */
function set_yahrzeit_date_context($timestamp)
{
    global $today_month;
    global $today_day;
    global $today_year;

    global $hebrewDay;
    global $hebrewMonth;
    global $hebrewMonthName;
    global $hebrewYear;

    $today_month = date('n', $timestamp);
    $today_day   = date('j', $timestamp);
    $today_year  = date('Y', $timestamp);

    $jdDate = gregoriantojd($today_month, $today_day, $today_year);
    $hebrewMonthName = jdmonthname($jdDate, 4);
    $hebrewDate = jdtojewish($jdDate);
    list($hebrewMonth, $hebrewDay, $hebrewYear) = explode('/', $hebrewDate);
}

/** Return the Hebrew calendar year containing the supplied civil timestamp. */
function hebrew_year_for_timestamp($timestamp)
{
    $jd = gregoriantojd(date('n', $timestamp), date('j', $timestamp), date('Y', $timestamp));
    $hebrewDate = jdtojewish($jd);
    $parts = explode('/', $hebrewDate);

    return isset($parts[2]) ? (int)$parts[2] : 0;
}

/**
 * Return whether a Gregorian month/day matches today or tomorrow.
 *
 * The scheduled engine runs late in the civil day, when tomorrow's date may
 * already be the active Hebrew observance after sunset.
 */
function english_day_matches_today_or_tomorrow($month, $day)
{
    global $today_month;
    global $today_day;
    global $today_year;
    $result_code = false;

    if ($month == "" || $day == "") {
        return false;
    }

    // Convert month names such as "Feb" or "May" to month numbers.
    if (!is_numeric($month)) {
        if (!isset(ENGLISH_MONTH_MAPPING[$month])) {
            return false;
        }
        $month = ENGLISH_MONTH_MAPPING[$month];
    }

    $month = (int) $month;
    $day   = (int) $day;
    $year  = (int) $today_year;

    if ($month < 1 || $month > 12 || $day < 1 || $day > 31) {
        return false;
    }

    // Special case for Feb 29 in non-leap years.
    if ($month == 2 && $day == 29 && !is_english_leap_year($year)) {
        $day = 28;
    }

    $yz_date  = mktime(0, 0, 0, $month,       $day,       $year);
    $today    = mktime(0, 0, 0, $today_month, $today_day, $today_year);
    $tomorrow = strtotime("tomorrow", $today);

    if ($yz_date == $tomorrow || $yz_date == $today) {
        $result_code = true;
    }

    return $result_code;
}

function english_month_number($month)
{

    if ($month == "") {
        return 0;
    }

    if (is_numeric($month)) {
        return (int)$month;
    }

    return isset(ENGLISH_MONTH_MAPPING[$month]) ? (int)ENGLISH_MONTH_MAPPING[$month] : 0;
}

function is_english_leap_year($year)
{
    return (($year % 4) == 0) &&
           ((($year % 100) != 0) || (($year % 400) == 0));
}

/**
 * Return whether the established date context is Friday.
 *
 * The historical name reflects the scheduled Friday preparation run. This
 * helper does not inspect the actual time of day or calculate sunset.
 */
function is_erev_shabbat_for_lighting()
{
    global $today_month;
    global $today_day;
    global $today_year;

    $timestamp = mktime(16, 0, 0, $today_month, $today_day, $today_year);
    $dateinfo = getdate($timestamp);

    return ($dateinfo['weekday'] == "Friday");
}

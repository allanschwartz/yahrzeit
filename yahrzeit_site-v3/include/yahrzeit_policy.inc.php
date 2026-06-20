<?php

/*
 * NAME
 *      yahrzeit_policy.inc.php
 *
 * DESCRIPTION
 *      Shared lighting-policy helpers for the CBS Yahrzeit Wall.
 *
 *      This module decides whether a memorial should be lit under the current
 *      database, Minhag, and date policy.  It does not parse CSV records,
 *      render pages, map LEDs, emit controller commands, or transmit data.
 *
 *      Callers must load the normal shared modules first: names.inc.php and
 *      date_support.inc.php.  The Minhag configuration is read from the
 *      legacy global $minhag, as it is in the existing engine.
 *
 * HISTORY
 *      Extracted from bin/yahrzeit_engine.php during the PHP 8 / Arduino V3
 *      modernization in 2026.
 */

// Return the active normal-lighting week as midnight timestamps.  The week is
// anchored to the sunset-based erev_shabbat_to_erev_shabbat cycle, so the
// boundary is evaluated at sunset rather than assuming a fixed midnight day.
function yahrzeit_lighting_week_range($timestamp = null)
{
    if ($timestamp === null) {
        $timestamp = time();
    }

    $today = strtotime(date("Y-m-d", $timestamp));
    $weekday = (int)date("N", $timestamp);

    if ($weekday == 5) {
        $sunset = cbs_sunset_timestamp($timestamp);
        $start = ($sunset !== false && $timestamp >= $sunset)
            ? strtotime("+1 day", $today)
            : strtotime("-6 days", $today);
    } else {
        // ISO weekday: Saturday is 6, Sunday is 7, and Monday is 1.
        $days_since_saturday = ($weekday + 1) % 7;
        $start = strtotime("-$days_since_saturday days", $today);
    }

    return [$start, strtotime("+6 days", $start)];
}

function yahrzeit_english_day_is_in_lighting_week($month, $day, $timestamp)
{
    $month = english_month_number($month);
    $day = (int)$day;

    if ($month < 1 || $month > 12 || $day < 1 || $day > 31) {
        return false;
    }

    [$start, $end] = yahrzeit_lighting_week_range($timestamp);

    for ($year = (int)date("Y", $start); $year <= (int)date("Y", $end); $year++) {
        $candidate_day = ($month == 2 && $day == 29 && !is_english_leap_year($year))
            ? 28
            : $day;
        $candidate = mktime(0, 0, 0, $month, $candidate_day, $year);

        if ($start <= $candidate && $candidate <= $end) {
            return true;
        }
    }

    return false;
}

// Does this person match the current yahrzeit "today" or "week" window?
// Date context globals must already have been populated with
// set_yahrzeit_date_context().
function yahrzeit_person_matches_observance_window($person, $window, $timestamp = null)
{
    global $minhag;
    global $hebrewMonthName;
    global $hebrewYear;

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
        $month_name = isset($person['hebYzMonth'])
            ? closest_hebrew_month($person['hebYzMonth'])
            : "";

        if ($month_name == "" || !isset(HEBREW_MONTH_MAPPING[$month_name])) {
            return false;
        }

        if (!isset($person['hebYzDD']) || !is_numeric($person['hebYzDD'])) {
            return false;
        }

        $month = HEBREW_MONTH_MAPPING[$month_name];
        $day = (int)$person['hebYzDD'];
        $year = (int)$hebrewYear;

        if ($day < 1 || $day > 30 || $year <= 0) {
            return false;
        }

        $julian_day = jewishtojd($month, $day, $year);

        // A Tishrei yahrzeit observed during Elul is in the next Hebrew year;
        // an Elul yahrzeit during early Tishrei was in the prior Hebrew year.
        if ($month == 1 && $hebrewMonthName == "Elul") {
            $julian_day = jewishtojd($month, $day, $year + 1);
        }
        if ($month == 13 && $hebrewMonthName == "Tishri") {
            $julian_day = jewishtojd($month, $day, $year - 1);
        }

        if (!$julian_day) {
            return false;
        }

        $parts = explode('/', JDToGregorian($julian_day));
        if (count($parts) < 3) {
            return false;
        }

        [$month, $day] = $parts;
    } else if ($use_english) {
        $month = isset($person['engYzMonth']) ? $person['engYzMonth'] : "";
        $day = isset($person['engYzDD']) ? $person['engYzDD'] : "";
    } else {
        return false;
    }

    if ($window == "today") {
        return english_day_matches_today_or_tomorrow($month, $day);
    }

    if ($window == "week") {
        return yahrzeit_english_day_is_in_lighting_week($month, $day, $timestamp);
    }

    return false;
}

function yahrzeit_person_is_observed_today($person)
{
    return yahrzeit_person_matches_observance_window($person, "today");
}

function yahrzeit_person_is_observed_this_week($person, $timestamp = null)
{
    return yahrzeit_person_matches_observance_window($person, "week", $timestamp);
}

// Return the normal yahrzeit-mode decision and its existing audit reason.
// This sets the legacy date-context globals to the supplied timestamp.
function yahrzeit_person_lighting_decision($person, $timestamp = null)
{
    global $minhag;

    if ($timestamp === null) {
        $timestamp = time();
    }

    set_yahrzeit_date_context($timestamp);

    if (!empty($person['reserved'])) {
        return ['should_light' => false, 'reason' => 'reserved'];
    }

    if (!empty($person['manual'])) {
        return ['should_light' => true, 'reason' => 'manual'];
    }

    if (yahrzeit_person_is_observed_today($person)) {
        return ['should_light' => true, 'reason' => 'yahrzeit today'];
    }

    if (
        isset($minhag['yahrzeitPlusShabbat']) &&
        $minhag['yahrzeitPlusShabbat'] == "YES" &&
        is_erev_shabbat_for_lighting() &&
        yahrzeit_person_is_observed_this_week($person, $timestamp)
    ) {
        return ['should_light' => true, 'reason' => 'plus-Shabbat weekly yahrzeit'];
    }

    if (
        isset($minhag['yahrzeitFullWeek']) &&
        $minhag['yahrzeitFullWeek'] == "YES" &&
        yahrzeit_person_is_observed_this_week($person, $timestamp)
    ) {
        return ['should_light' => true, 'reason' => 'full-week yahrzeit'];
    }

    return ['should_light' => false, 'reason' => 'not active'];
}

// Public convenience helper for screens and reports.
function yahrzeit_person_should_light_now($person, $timestamp = null)
{
    $decision = yahrzeit_person_lighting_decision($person, $timestamp);

    return $decision['should_light'];
}

// Count how many memorial records should be lit at the supplied timestamp.
// This uses the shared policy helper so screens do not duplicate date logic.
function yahrzeit_lit_person_count($timestamp = null)
{
    if ($timestamp === null) {
        $timestamp = time();
    }

    $count = 0;
    $n = yahrzeit_readDB();

    for ($i = 0; $i < $n; $i++) {
        $person = yahrzeit_getObj($i);
        if (yahrzeit_person_should_light_now($person, $timestamp)) {
            $count++;
        }
    }

    return $count;
}

?>

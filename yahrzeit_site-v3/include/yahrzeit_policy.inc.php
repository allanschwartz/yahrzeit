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

/**
 * Return the active Saturday-through-Friday lighting window.
 *
 * A Friday preparation run is treated as though Shabbat has begun, regardless
 * of its clock time, and therefore selects tomorrow through the following
 * Friday. Returned timestamps represent local midnight at both inclusive
 * endpoints.
 *
 * @return array{0: int, 1: int}
 */
function yahrzeit_lighting_week_range($timestamp = null)
{
    if ($timestamp === null) {
        $timestamp = time();
    }

    $today = strtotime(date("Y-m-d", $timestamp));
    $weekday = (int)date("N", $timestamp);

    if ($weekday == 5) {
        $start = strtotime("+1 day", $today);
    } else {
        // ISO weekday: Saturday is 6, Sunday is 7, and Monday is 1.
        $days_since_saturday = ($weekday + 1) % 7;
        $start = strtotime("-$days_since_saturday days", $today);
    }

    return [$start, strtotime("+6 days", $start)];
}

/** Return the Saturday-through-Friday lighting window following this one. */
function yahrzeit_next_lighting_week_range($timestamp = null)
{
    if ($timestamp === null) {
        $timestamp = time();
    }

    return yahrzeit_lighting_week_range(strtotime("+7 days", $timestamp));
}

/**
 * Return whether a Gregorian month/day falls within the lighting week.
 *
 * A February 29 observance falls on February 28 in a non-leap year.
 */
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

/**
 * Match one memorial against the requested "today" or "week" window.
 *
 * The date-context globals must first be populated for $timestamp. Hebrew
 * dates are converted to their civil occurrence, including the Elul/Tishri
 * year boundary, before the selected window is tested.
 *
 * @param string $window "today" or "week"
 */
function yahrzeit_person_matches_observance_window($person, $window, $timestamp = null)
{
    global $hebrewMonthName;
    global $hebrewYear;

    $use_hebrew = yahrzeit_person_uses_hebrew_date($person);

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
    } else {
        $month = isset($person['engYzMonth']) ? $person['engYzMonth'] : "";
        $day = isset($person['engYzDD']) ? $person['engYzDD'] : "";
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

/**
 * Decide whether one memorial should be lit in normal automatic operation.
 *
 * This sets the legacy date-context globals to the supplied timestamp.
 *
 * @return array{should_light: bool, reason: string}
 */
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

    $observance = $minhag['yahrzeitObservance'] ?? "week";

    if ($observance == "week" && yahrzeit_person_is_observed_this_week($person, $timestamp)) {
        return ['should_light' => true, 'reason' => 'full-week yahrzeit'];
    }

    if ($observance == "day" && yahrzeit_person_is_observed_today($person)) {
        return ['should_light' => true, 'reason' => 'yahrzeit today'];
    }

    return ['should_light' => false, 'reason' => 'not active'];
}

function yahrzeit_person_should_light_now($person, $timestamp = null)
{
    $decision = yahrzeit_person_lighting_decision($person, $timestamp);

    return $decision['should_light'];
}

/**
 * Load the memorial database and count records lit at the supplied timestamp.
 */
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

#!/usr/bin/env php
<?php
/*
 * NAME
 *      yahrzeit_engine.php
 *
 * SYNOPSIS
 *      php bin/yahrzeit_engine.php [options]
 *      bin/yahrzeit_engine.php [options]
 *
 * OPTIONS
 *      -a, --audit
 *          Validate the Yahrzeit memorial database against the static panel
 *          geometry.  Reports malformed locations, unknown panels, row/column
 *          values outside the physical panel size, and duplicate LED positions.
 *
 *      --report KIND
 *          Emit a human-readable report.  KIND may be:
 *
 *              day, daily
 *              week, weekly
 *              next-week
 *              month, this-month
 *              next-month
 *
 *      --date YYYY-MM-DD
 *          Use this Gregorian date as the report or command-generation base.
 *          If omitted, today is used.
 *
 *      -d N
 *          Set debug verbosity.  Debug lines are emitted as comments beginning
 *          with "#", so they remain harmless if mixed into a controller command
 *          stream.
 *
 *      -h, --help
 *          Display usage.
 *
 * DESCRIPTION
 *      Date, name, report, and audit engine for the CBS Yahrzeit Wall.
 *
 *      In normal mode, this script emits the line-oriented controller command
 *      stream that clears the wall, lights the currently active memorial LEDs,
 *      refreshes the display, and saves the controller state.
 *
 *      In audit mode, it validates memorial records against the static panel
 *      geometry.
 *
 *      In report mode, it lists yahrzeit names for a selected date range.
 *
 * BLUF
 *      cron decides WHEN scheduled phases run.
 *      yahrzeit_scheduler decides WHETHER the requested phase applies today.
 *      bin/yahrzeit decides HOW to run and, when appropriate, transmit.
 *      yahrzeit_engine.php decides WHAT names should be lit, audited, or
 *      reported.
 *
 *      This file should not know about cron timing, socket transport, nc,
 *      web-page layout, or manual operator buttons.
 *
 * NOTES
 *      "English date" means Gregorian calendar date.
 *
 *      Debug output is emitted as comment lines beginning with "#".  This
 *      keeps diagnostic output safe if it appears in a command-stream preview.
 *
 *      The normal command stream is consumed by bin/yahrzeit.  This engine
 *      does not talk directly to the embedded controller.
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

/*
 * TODO
 *      This file is now the central engine and has grown large.  A future
 *      refactor should split helper logic into smaller include files:
 *
 *          include/date_support.inc.php
 *              Gregorian/Hebrew date conversion, date-range matching,
 *              leap-year handling, Shabbat/week calculations.
 *
 *          include/name_support.inc.php or expanded names.inc.php
 *              person_name(), person_location(), person_options_text(),
 *              person date-field formatting, and other per-record helpers.
 *
 *          include/audit_support.inc.php
 *              panel/name validation and duplicate-location checks.
 *
 *      Keep yahrzeit_engine.php as the orchestration layer: parse options,
 *      select mode, read data, and call the appropriate engine/report/audit
 *      functions.
 */

require_once dirname(__DIR__) . "/include/misc.inc.php";
require_once site_root() . "/include/panels.inc.php";
require_once site_root() . "/include/names.inc.php";
require_once site_root() . "/include/leds.inc.php";


// ---------------------------------------------------------------------------
// Program entry and option handling
// ---------------------------------------------------------------------------

function usage($exit_status = 0)
{
    $message = <<<USAGE
Usage:
    php bin/yahrzeit_engine.php [options]
    bin/yahrzeit_engine.php [options]

Options:
    -a, --audit
        Validate the Yahrzeit database and panel geometry.

    --report KIND
        Emit a report.  KIND may be:
        day, daily, week, weekly, next-week, month, this-month, next-month.

    --date YYYY-MM-DD
        Use this Gregorian date as the base date.

    -d N
        Set debug verbosity.

    -h, --help
        Show this help.

Examples:
    php bin/yahrzeit_engine.php
    php bin/yahrzeit_engine.php --audit
    php bin/yahrzeit_engine.php --report week --date 2026-05-26

USAGE;

    if ($exit_status == 0) {
        fwrite(STDOUT, $message);
    } else {
        fwrite(STDERR, $message);
    }

    exit($exit_status);
}


function parse_options()
{
    $options = getopt("ad:h:", array("audit", "date:", "help", "report:"));

    if (isset($options['h']) || isset($options['help'])) {
        usage(0);
    }

    // Accept either -a or --audit.  The rest of the code tests -a.
    if (isset($options['audit'])) {
        $options['a'] = true;
    }

    return $options;
}


// Main function for yahrzeit_engine.php.
//
// Normal mode:
//   - compute today's Gregorian/Hebrew context
//   - emit a controller command stream for currently active names
//
// Audit/report modes:
//   - emit human-readable text only
//   - do not emit controller commands
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

    $selected_ts = selected_timestamp();
    setup_yahrzeit_date_context($selected_ts);

    if (isset($options['report'])) {
        return emit_yahrzeit_report($options['report'], $selected_ts);
    }

    if (audit_mode()) {
        return emit_audit_report();
    }

    // Normal command-stream path.
    echo "\n# Today is ";
    if (haYomShabbat()) {
        echo "SHABBAT ";
    }
    echo "$today_month/$today_day/$today_year ... ";
    echo "$hebrewDay $hebrewMonthName ($hebrewMonth) $hebrewYear\n";

    led_all("off");

    panel_readDB();
    $n = yahrzeit_readDB();

    for ($i = 0; $i < $n; $i++) {
        $person = yahrzeit_getObj($i);
        yz_process_person($person);
    }

    led_data_refresh();
    echo "\nsave\n";

    return 0;
}


// ---------------------------------------------------------------------------
// Logging and date context helpers
// ---------------------------------------------------------------------------

// Debug levels for yahrzeit_engine.php
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

function selected_timestamp()
{
    global $options;

    if (isset($options['date'])) {
        $ts = strtotime($options['date']);
        if ($ts === false) {
            warn_log("Invalid --date value '" . $options['date'] . "'; using today");
            return time();
        }
        return $ts;
    }

    return time();
}

function setup_yahrzeit_date_context($timestamp)
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

function hebrew_year_for_timestamp($timestamp)
{
    $jd = gregoriantojd(date('n', $timestamp), date('j', $timestamp), date('Y', $timestamp));
    $hebrewDate = jdtojewish($jd);
    $parts = explode('/', $hebrewDate);

    return isset($parts[2]) ? (int)$parts[2] : 0;
}



// ---------------------------------------------------------------------------
// Audit reporting
// ---------------------------------------------------------------------------

function build_panel_geometry_map()
{
    $n = panel_readDB();
    $panels = array();

    for ($i = 0; $i < $n; $i++) {
        $panel = panel_getObj($i);

        if (!isset($panel['panelId'])) {
            continue;
        }

        $panels[$panel['panelId']] = $panel;
    }

    return $panels;
}

function emit_audit_report()
{
    $panels = build_panel_geometry_map();
    $n = yahrzeit_readDB();

    $errors = 0;
    $warnings = 0;
    $seen_locations = array();

    echo "# AUDIT: Starting yahrzeit database audit\n";
    echo "# AUDIT: Panels defined: " . count($panels) . "\n";
    echo "# AUDIT: Memorial records: $n\n";

    for ($i = 0; $i < $n; $i++) {
        $person = yahrzeit_getObj($i);
        $name = person_name($person);
        $location = person_location($person);

        if ($name == "") {
            echo "# AUDIT WARNING: row $i has no name, location=$location\n";
            $warnings++;
            continue;
        }

        $panel_id = isset($person['panelId']) ? $person['panelId'] : "";
        $row = isset($person['row']) ? $person['row'] : "";
        $col = isset($person['column']) ? $person['column'] : "";

        if ($panel_id == "" || $row == "" || $col == "") {
            echo "# AUDIT ERROR: row $i $name has incomplete location '$location'\n";
            $errors++;
            continue;
        }

        if (!isset($panels[$panel_id])) {
            echo "# AUDIT ERROR: row $i $name uses unknown panel '$panel_id'\n";
            $errors++;
            continue;
        }

        $panel = $panels[$panel_id];

        if (!is_numeric($row) || !is_numeric($col)) {
            echo "# AUDIT ERROR: row $i $name has non-numeric row/column '$location'\n";
            $errors++;
            continue;
        }

        $row_i = (int)$row;
        $col_i = (int)$col;

        if ($row_i < 1 || $row_i > (int)$panel['nRows'] ||
            $col_i < 1 || $col_i > (int)$panel['nCols']) {
            echo "# AUDIT ERROR: row $i $name location '$location' outside geometry " .
                 $panel['nRows'] . "x" . $panel['nCols'] . "\n";
            $errors++;
            continue;
        }

        $key = "$panel_id-$row_i-$col_i";
        if (isset($seen_locations[$key])) {
            echo "# AUDIT ERROR: collision at $key: " .
                 $seen_locations[$key] . " and $name\n";
            $errors++;
        } else {
            $seen_locations[$key] = $name;
        }
    }

    echo "# AUDIT: Completed with $errors error(s), $warnings warning(s)\n";

    return ($errors == 0) ? 0 : 1;
}


// main function for the "yahrzeit_engine.php"
//    does some date calculations
//    reads entire yahrzeit database
//    for each person in database, calls yz_process_person



// ---------------------------------------------------------------------------
// Yahrzeit range reports
// ---------------------------------------------------------------------------

function emit_yahrzeit_report($kind, $base_timestamp)
{
    $range = report_date_range($kind, $base_timestamp);

    if ($range === false) {
        echo "# ERROR: Unknown report kind '$kind'\n";
        echo "# Valid report kinds: day, week, next-week, month, next-month\n";
        return 1;
    }

    list($start_ts, $end_ts, $normalized_kind) = $range;

    $n = yahrzeit_readDB();
    $rows = array();

    for ($i = 0; $i < $n; $i++) {
        $person = yahrzeit_getObj($i);
        $name = person_name($person);

        if ($name == "") {
            continue;
        }

        $dates = yahrzeit_candidate_dates_in_range($person, $start_ts, $end_ts);

        foreach ($dates as $ts) {
            $rows[] = array($ts, $person);
        }
    }

    usort($rows, function($a, $b) {
        if ($a[0] == $b[0]) {
            return strcmp(person_name($a[1]), person_name($b[1]));
        }
        return ($a[0] < $b[0]) ? -1 : 1;
    });

    echo "# Yahrzeit report: $normalized_kind\n";
    echo "# Range: " . date("l F j, Y", $start_ts) .
         " through " . date("l F j, Y", $end_ts) . "\n";
    echo "# Count: " . count($rows) . "\n\n";

    printf("%-12s  %-32s  %-18s  %-14s  %-12s  %s\n",
        "Date", "Name", "Hebrew Date", "English Date", "Location", "Options");
    printf("%'-12s  %'-32s  %'-18s  %'-14s  %'-12s  %s\n",
        "", "", "", "", "", "");

    foreach ($rows as $row) {
        $ts = $row[0];
        $person = $row[1];

        $name = person_name($person);

        $heb = trim(
            (isset($person['hebYzDD']) ? $person['hebYzDD'] : "") . " " .
            (isset($person['hebYzMonth']) ? $person['hebYzMonth'] : "") . " " .
            (isset($person['hebYzYYYY']) ? $person['hebYzYYYY'] : "")
        );

        $eng = trim(
            (isset($person['engYzMonth']) ? $person['engYzMonth'] : "") . "/" .
            (isset($person['engYzDD']) ? $person['engYzDD'] : "") . "/" .
            (isset($person['engYzYYYY']) ? $person['engYzYYYY'] : ""),
            "/"
        );

        printf("%-12s  %-32s  %-18s  %-14s  %-12s  %s\n",
            date("Y-m-d", $ts),
            substr($name, 0, 32),
            substr($heb, 0, 18),
            substr($eng, 0, 14),
            person_location($person),
            person_options_text($person)
        );
    }

    return 0;
}

function report_date_range($kind, $base_timestamp)
{
    $kind = strtolower($kind);

    if ($kind == "daily") {
        $kind = "day";
    }
    if ($kind == "weekly") {
        $kind = "week";
    }
    if ($kind == "this-month") {
        $kind = "month";
    }

    if ($kind == "day") {
        $start = strtotime(date("Y-m-d", $base_timestamp));
        $end   = $start;
    }
    else if ($kind == "week") {
        $start = strtotime(date("Y-m-d", $base_timestamp));
        $end   = strtotime("+6 days", $start);
    }
    else if ($kind == "next-week") {
        $start = strtotime("+7 days", strtotime(date("Y-m-d", $base_timestamp)));
        $end   = strtotime("+6 days", $start);
    }
    else if ($kind == "month") {
        $start = strtotime(date("Y-m-01", $base_timestamp));
        $end   = strtotime("last day of this month", $start);
    }
    else if ($kind == "next-month") {
        $start = strtotime("first day of next month", $base_timestamp);
        $end   = strtotime("last day of this month", $start);
    }
    else {
        return false;
    }

    return array($start, $end, $kind);
}

function yahrzeit_candidate_dates_in_range($person, $start_ts, $end_ts)
{
    global $hebrew_month_mapping;

    $candidates = array();

    if (isset($person['reserved']) && $person['reserved']) {
        return $candidates;
    }

    if (person_uses_hebrew_date($person)) {
        $month_name = isset($person['hebYzMonth'])
            ? closest_hebrew_month($person['hebYzMonth'])
            : "";

        if ($month_name == "" || !isset($hebrew_month_mapping[$month_name])) {
            return $candidates;
        }

        if (!isset($person['hebYzDD']) || !is_numeric($person['hebYzDD'])) {
            return $candidates;
        }

        $hmonth = (int)$hebrew_month_mapping[$month_name];
        $hday   = (int)$person['hebYzDD'];

        if ($hday < 1 || $hday > 30) {
            return $candidates;
        }

        $start_hyear = hebrew_year_for_timestamp($start_ts);
        $end_hyear   = hebrew_year_for_timestamp($end_ts);

        for ($hyear = $start_hyear - 1; $hyear <= $end_hyear + 1; $hyear++) {
            if ($hyear <= 0) {
                continue;
            }

            $jd = jewishtojd($hmonth, $hday, $hyear);
            if (!$jd) {
                continue;
            }

            $greg = JDToGregorian($jd);
            $parts = explode('/', $greg);
            if (count($parts) < 3) {
                continue;
            }

            list($mm, $dd, $yy) = $parts;
            $ts = mktime(0, 0, 0, (int)$mm, (int)$dd, (int)$yy);

            if ($start_ts <= $ts && $ts <= $end_ts) {
                $candidates[] = $ts;
            }
        }
    }
    else {
        $month = english_month_number(isset($person['engYzMonth']) ? $person['engYzMonth'] : "");
        $day   = isset($person['engYzDD']) ? (int)$person['engYzDD'] : 0;

        if ($month < 1 || $month > 12 || $day < 1 || $day > 31) {
            return $candidates;
        }

        $start_year = (int)date("Y", $start_ts);
        $end_year   = (int)date("Y", $end_ts);

        for ($year = $start_year; $year <= $end_year; $year++) {
            $candidate_day = $day;

            if ($month == 2 && $day == 29 && !isLeapYear($year)) {
                $candidate_day = 28;
            }

            $ts = mktime(0, 0, 0, $month, $candidate_day, $year);

            if ($start_ts <= $ts && $ts <= $end_ts) {
                $candidates[] = $ts;
            }
        }
    }

    sort($candidates);
    return $candidates;
}



// ---------------------------------------------------------------------------
// Normal command-stream processing
// ---------------------------------------------------------------------------

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



// ---------------------------------------------------------------------------
// Person and date helper functions
// ---------------------------------------------------------------------------

function person_name($person)
{
    $first = isset($person['firstName']) ? $person['firstName'] : "";
    $last  = isset($person['lastName'])  ? $person['lastName']  : "";

    return trim($first . " " . $last);
}

function person_location($person)
{
    $panel = isset($person['panelId']) ? $person['panelId'] : "";
    $row   = isset($person['row'])     ? $person['row']     : "";
    $col   = isset($person['column'])  ? $person['column']  : "";

    return "$panel-$row-$col";
}

function person_options_text($person)
{
    $opts = array();

    if (isset($person['useHeb']) && $person['useHeb']) {
        $opts[] = "HEB";
    }
    if (isset($person['useEng']) && $person['useEng']) {
        $opts[] = "ENG";
    }
    if (isset($person['manual']) && $person['manual']) {
        $opts[] = "MANUAL";
    }
    if (isset($person['reserved']) && $person['reserved']) {
        $opts[] = "RESERVED";
    }
    if (isset($person['yomhashoah']) && $person['yomhashoah']) {
        $opts[] = "HASHOAH";
    }
    if (isset($person['yomhazikaron']) && $person['yomhazikaron']) {
        $opts[] = "HAZIKARON";
    }

    return implode(",", $opts);
}

function person_uses_hebrew_date($person)
{
    global $minhag;

    if (isset($person['useHeb']) && $person['useHeb']) {
        return true;
    }

    if (isset($person['useEng']) && $person['useEng']) {
        return false;
    }

    return isset($minhag['yahrzeitEngOrHeb']) && $minhag['yahrzeitEngOrHeb'] == "heb";
}

function english_month_number($month)
{
    global $english_month_mapping;

    if ($month == "") {
        return 0;
    }

    if (is_numeric($month)) {
        return (int)$month;
    }

    return isset($english_month_mapping[$month]) ? (int)$english_month_mapping[$month] : 0;
}

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

// -----------------------------------------------------------------------------
// Program entry point
// -----------------------------------------------------------------------------

$options = parse_options();
$minhag = read_minhag_ini();

exit(yz_main());

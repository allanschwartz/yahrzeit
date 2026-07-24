#!/usr/bin/env php
<?php

/*
 * Verify calendar boundaries and confirm that week reports and automatic
 * weekly lighting select the same records.
 */

require_once dirname(__DIR__) . "/bin/yahrzeit_engine.php";

date_default_timezone_set("America/Los_Angeles");

$failures = array();

function test_expect_equal($actual, $expected, $description)
{
    global $failures;

    if ($actual !== $expected) {
        $failures[] = "$description: expected '$expected', got '$actual'";
    }
}

function test_range($kind, $anchor, $expected_start, $expected_end)
{
    $range = report_date_range($kind, strtotime($anchor));

    test_expect_equal(date("Y-m-d", $range[0]), $expected_start, "$kind start for $anchor");
    test_expect_equal(date("Y-m-d", $range[1]), $expected_end, "$kind end for $anchor");
}

// A Friday preparation run selects the Shabbat beginning that evening. The
// clock time does not change this policy. These cases also cover civil and
// Hebrew year boundaries.
$range_cases = array(
    array("week", "2026-07-15 12:00", "2026-07-11", "2026-07-17"),
    array("next-week", "2026-07-15 12:00", "2026-07-18", "2026-07-24"),
    array("week", "2026-07-17 15:00", "2026-07-18", "2026-07-24"),
    array("week", "2026-07-17 21:00", "2026-07-18", "2026-07-24"),
    array("next-week", "2026-07-17 15:00", "2026-07-25", "2026-07-31"),
    array("week", "2026-09-11 15:00", "2026-09-12", "2026-09-18"),
    array("week", "2026-09-11 21:00", "2026-09-12", "2026-09-18"),
    array("week", "2026-12-31 12:00", "2026-12-26", "2027-01-01"),
    array("next-week", "2026-12-31 12:00", "2027-01-02", "2027-01-08"),
    array("week", "2028-02-29 12:00", "2028-02-26", "2028-03-03"),
);

foreach ($range_cases as $case) {
    test_range($case[0], $case[1], $case[2], $case[3]);
}

// English-date/day-only matching must carry tomorrow into the next civil year.
set_yahrzeit_date_context(strtotime("2026-12-31 16:00"));
test_expect_equal(
    english_day_matches_today_or_tomorrow("Jan", 1),
    true,
    "January 1 should match tomorrow on December 31"
);

// Verify the Hebrew-year boundary independently of the report and lighting
// paths, so agreement between those paths cannot conceal a shared error.
$elul_29_5786 = civil_timestamp_for_hebrew_date(
    HEBREW_MONTH_MAPPING["Elul"],
    29,
    5786
);
$tishri_1_5787 = civil_timestamp_for_hebrew_date(
    HEBREW_MONTH_MAPPING["Tishri"],
    1,
    5787
);
test_expect_equal(
    date("Y-m-d", $elul_29_5786),
    "2026-09-11",
    "29 Elul 5786 civil date"
);
test_expect_equal(
    date("Y-m-d", $tishri_1_5787),
    "2026-09-12",
    "1 Tishri 5787 civil date"
);

$minhag = read_minhag_ini();

$tishri_record = array(
    'hebYzMonth' => "Tishri",
    'hebYzDD' => 1,
    'useHeb' => true,
);
$elul_record = array(
    'hebYzMonth' => "Elul",
    'hebYzDD' => 29,
    'useHeb' => true,
);

// On 29 Elul, a 1 Tishri memorial is tomorrow in the next Hebrew year.
$minhag['yahrzeitObservance'] = "day";
test_expect_equal(
    yahrzeit_person_should_light_now(
        $tishri_record,
        strtotime("2026-09-11 16:00")
    ),
    true,
    "1 Tishri should match tomorrow on 29 Elul"
);

// In 2025 the Hebrew year changed on a Tuesday. On Wednesday, the active
// Saturday-through-Friday week still contains the preceding 29 Elul.
$minhag['yahrzeitObservance'] = "week";
$boundary_anchor = strtotime("2025-09-24 12:00");
[$boundary_start, $boundary_end] =
    yahrzeit_lighting_week_range($boundary_anchor);
$elul_candidates = yahrzeit_candidate_dates_in_range(
    $elul_record,
    $boundary_start,
    $boundary_end
);
test_expect_equal(
    count($elul_candidates),
    1,
    "Elul memorial should have one occurrence in a week crossing into Tishri"
);
test_expect_equal(
    isset($elul_candidates[0]) ? date("Y-m-d", $elul_candidates[0]) : "",
    "2025-09-22",
    "29 Elul occurrence in a week crossing into Tishri"
);
test_expect_equal(
    yahrzeit_person_should_light_now($elul_record, $boundary_anchor),
    true,
    "29 Elul should remain in the active week during early Tishri"
);

$minhag['yahrzeitObservance'] = "week";
$record_count = yahrzeit_readDB();
$anchors = array(
    "2026-07-15 12:00",
    "2026-07-17 15:00",
    "2026-07-17 21:00",
    "2026-07-18 12:00",
    "2026-09-11 15:00",
    "2026-09-11 21:00",
    "2026-12-31 12:00",
    "2027-01-01 15:00",
    "2028-02-29 12:00",
);

$comparisons = 0;

foreach (array("heb", "eng") as $default_date_method) {
    $minhag['yahrzeitEngOrHeb'] = $default_date_method;

    foreach ($anchors as $anchor_text) {
        $anchor = strtotime($anchor_text);

        foreach (array("week", "next-week") as $kind) {
            [$start, $end] = report_date_range($kind, $anchor);
            $lighting_timestamp = ($kind == "week")
                ? $anchor
                : strtotime("+7 days", $anchor);

            for ($i = 0; $i < $record_count; $i++) {
                $person = yahrzeit_getObj($i);

                if (yahrzeit_person_name($person) == "") {
                    continue;
                }

                $reported = count(yahrzeit_candidate_dates_in_range($person, $start, $end)) > 0;
                $lit = yahrzeit_person_should_light_now($person, $lighting_timestamp);
                $comparisons++;

                if ($reported !== $lit) {
                    $failures[] = sprintf(
                        "%s default, %s, %s: report=%s lighting=%s for %s at %s",
                        $default_date_method,
                        $anchor_text,
                        $kind,
                        $reported ? "yes" : "no",
                        $lit ? "yes" : "no",
                        yahrzeit_person_name($person),
                        yahrzeit_person_location($person)
                    );
                }
            }
        }
    }
}

if (count($failures) > 0) {
    foreach ($failures as $failure) {
        fwrite(STDERR, "FAIL: $failure\n");
    }
    exit(1);
}

echo "PASS: week ranges and $comparisons report/lighting decisions agree.\n";

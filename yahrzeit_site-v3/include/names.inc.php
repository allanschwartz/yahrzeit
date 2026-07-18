<?php

/*
 * NAME
 *      names.inc.php
 *
 * DESCRIPTION
 *      Memorial-name database access functions for the CBS Yahrzeit Wall.
 *
 *      This file reads and maps the Yahrzeit CSV database into associative
 *      PHP records used by the web screens, reports, scheduler, and lighting
 *      engine.  Each record represents one memorialized person and includes
 *      name, English yahrzeit date, Hebrew yahrzeit date, per-person options,
 *      and physical LED location.
 *
 *      The original design used a small procedural "object" layer:
 *
 *          yahrzeit_readDB()
 *          yahrzeit_getObj($i)
 *          yahrzeit_putObj($i, $person)
 *
 *      This file uses a procedural object-store style rather than PHP classes.
 *      That style intentionally keeps the CSV file format and schema isolated
 *      from the rest of the application.  Most code should ask this file for
 *      a mapped record rather than parsing CSV columns directly.
 * 
 * NOTES
 *      The live database is:
 *
 *          data/yahrzeits-rev4.csv
 *
 *      The CSV column order is part of the long-lived data format.  Be careful
 *      when changing read/write behavior: reports, the names browser, audit
 *      checks, and controller command generation all depend on the mapped
 *      fields produced here.
 *
 *      This file should know how to read, validate, map, and write memorial
 *      records.  It should not contain calendar policy, Minhag rules, panel
 *      geometry, or LED command generation.
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

// Module-private state for the memorial-name CSV database.
// These are PHP globals for historical/procedural reasons only.
// Code outside names.inc.php should use the yahrzeit_*DB/get/put API.
global $yahrzeit_record_count;
global $yahrzeit_records;

// Mapped record-field name => token stored in the CSV OPTIONS column.
const YAHRZEIT_OPTION_FIELDS = [
    'useHeb'       => 'HEB',
    'useEng'       => 'ENG',
    'reserved'     => 'RESERVED',
    'yomhashoah'   => 'HASHOAH',
    'yomhazikaron' => 'HAZIKARON',
];


function yahrzeit_csv_field($rawrecord, $index)
{
    return trim($rawrecord[$index] ?? "");
}


function yahrzeit_split_full_name($full_name)
{
    $parts = preg_split('/\s+/', trim($full_name));

    if (!$parts || $parts[0] == "") {
        return ["", ""];
    }

    $lastname = array_pop($parts);

    return [implode(" ", $parts), $lastname];
}


function yahrzeit_parse_english_date_field($date_str)
{

    $date_str = trim($date_str);

    $result = array(
        'monthName' => "",
        'day'       => "",
        'year'      => ""
    );

    if ($date_str == "") {
        return $result;
    }

    $dod = date_parse($date_str);

    if (!isset($dod['month']) || !isset($dod['day']) || !isset($dod['year'])) {
        return $result;
    }

    if ($dod['month'] < 1 || $dod['month'] > 12) {
        return $result;
    }

    // Y2K conversion for old two-digit years.
    if (1 <= $dod['year'] && $dod['year'] <= 99) {
        $dod['year'] += 1900;
    }

    $result['monthName'] = ENGLISH_MONTH_NAMES[$dod['month'] - 1];
    $result['day']       = $dod['day'];
    $result['year']      = $dod['year'];

    return $result;
}


function yahrzeit_parse_hebrew_date_field($date_str)
{

    $date_str = trim($date_str);

    $result = array(
        'day'       => "",
        'monthName' => "",
        'monthNum'  => "",
        'year'      => ""
    );

    if ($date_str == "") {
        return $result;
    }

    $hdod = preg_split('/\s+/', $date_str);

    if ($hdod === false || count($hdod) < 2) {
        return $result;
    }

    // Special case for "Adar I" and "Adar II":
    //     17 Adar I 5764
    // becomes:
    //     [0] = 17, [1] = Adar I, [2] = 5764
    if (count($hdod) >= 4) {
        $hdod[1] = $hdod[1] . " " . $hdod[2];
        $hdod[2] = $hdod[3] ?? "";
    }

    $hebrew_month_name = closest_hebrew_month($hdod[1]);

    $result['day']       = $hdod[0];
    $result['monthName'] = $hebrew_month_name;
    $result['monthNum']  = ($hebrew_month_name != "" && isset(HEBREW_MONTH_MAPPING[$hebrew_month_name]))
                         ? HEBREW_MONTH_MAPPING[$hebrew_month_name]
                         : "";
    $result['year']      = $hdod[2] ?? "";

    return $result;
}


function yahrzeit_parse_location_field($location_str)
{
    [$panel, $column, $row] = array_pad(explode("-", trim($location_str)), 3, "");

    return [
        'panelId' => $panel,
        'column'  => $column,
        'row'     => $row
    ];
}


/**
 * Map one Rev4 CSV row to the associative memorial record used by the site.
 *
 * @param array<int, string|null> $rawrecord
 * @return array<string, mixed>
 */
function yahrzeit_person_from_csv_row($rawrecord)
{
    // Rev4 CSV fields:
    //
    //   0  DECEASED
    //   1  Last-Name-First
    //   2  Eng. DOD
    //   3  Heb. DOD
    //   4  YEAR
    //   5  OPTIONS
    //   6  OLD Location
    //   7  New Location

    $rawrecord = array_pad($rawrecord, 8, "");

    list($firstname, $lastname) =
        yahrzeit_split_full_name(yahrzeit_csv_field($rawrecord, 0));

    $eng = yahrzeit_parse_english_date_field(yahrzeit_csv_field($rawrecord, 2));
    $heb = yahrzeit_parse_hebrew_date_field(yahrzeit_csv_field($rawrecord, 3));
    $loc = yahrzeit_parse_location_field(yahrzeit_csv_field($rawrecord, 7));

    $newyear     = yahrzeit_csv_field($rawrecord, 4);
    $options     = yahrzeit_csv_field($rawrecord, 5);
    $oldLocation = yahrzeit_csv_field($rawrecord, 6);

    $person = array(
        'lastName'          => $lastname,
        'firstName'         => $firstname,
        'lastNameFirst'     => yahrzeit_csv_field($rawrecord, 1),
        'engYzMonth'        => $eng['monthName'],
        'engYzDD'           => $eng['day'],
        'engYzYYYY'         => $eng['year'],

        'hebYzDD'           => $heb['day'],
        'hebYzMonth'        => $heb['monthName'],
        'hebYzMM'           => $heb['monthNum'],
        'hebYzYYYY'         => $heb['year'],

        'useHeb'            => (stristr($options, 'HEB')       ? true : false),
        'useEng'            => (stristr($options, 'ENG')       ? true : false),
        'yomhashoah'        => (stristr($options, 'HASHOAH')   ? true : false),
        'yomhazikaron'      => (stristr($options, 'HAZIKARON') ? true : false),
        'reserved'          => (stristr($options, 'RESERVED')  ? true : false),
        'options'           => $options,

        'panelId'           => $loc['panelId'],
        'column'            => $loc['column'],
        'row'               => $loc['row'],

        'oldLocation'       => $oldLocation,
        'newyear'           => $newyear
    );

    return $person;
}

/** Return the CSV option tokens enabled on one mapped memorial record. */
function yahrzeit_person_options_text($person)
{
    $opts = [];

    foreach (YAHRZEIT_OPTION_FIELDS as $field => $label) {
        if (!empty($person[$field])) {
            $opts[] = $label;
        }
    }

    return implode(",", $opts);
}

/**
 * Serialize one mapped memorial record in Rev4 CSV column order.
 *
 * @return array<int, string|int>
 */
function yahrzeit_csv_row_from_person($person)
{
    $first = trim($person['firstName'] ?? "");
    $last  = trim($person['lastName']  ?? "");
    $name  = trim("$first $last");

    $lastNameFirst = ($last != "" && $first != "")
                   ? "$last, $first"
                   : $name;

    if (($person['engYzMonth'] ?? "") == "" || ($person['engYzDD'] ?? "") == "") {
        $dod = "";
    } else {
        $month = ENGLISH_MONTH_MAPPING[$person['engYzMonth']] ?? $person['engYzMonth'];
        $dod = $month . "/" . $person['engYzDD'] . "/" . ($person['engYzYYYY'] ?? "");
    }

    if (($person['hebYzDD'] ?? "") == "" || ($person['hebYzMonth'] ?? "") == "") {
        $hdod = "";
    } else {
        $hdod = trim(($person['hebYzDD'] ?? "") . " " .
             ($person['hebYzMonth'] ?? "") . " " .
             ($person['hebYzYYYY'] ?? ""));
    }

    $options = [];

    foreach (YAHRZEIT_OPTION_FIELDS as $field => $label) {
        if (!empty($person[$field])) {
            $options[] = $label;
        }
    }

    $panel  = $person['panelId'] ?? "";
    $column = $person['column']  ?? "";
    $row    = $person['row']     ?? "";

    $newLocation = ($panel == "") ? "" :
                   (($column == "" && $row == "") ? $panel : 
                    "$panel-$column-$row");

    return [
        $name,
        $lastNameFirst,
        $dod,
        $hdod,
        $person['newyear']     ?? "",
        implode(" ", $options),
        $person['oldLocation'] ?? "",
        $newLocation
    ];
}


// Ignore malformed rows, blank rows, and the CSV header row.
function yahrzeit_csv_row_is_invalid($rawrecord)
{
    $name = is_array($rawrecord) ? trim($rawrecord[0] ?? "") : "";

    return !is_array($rawrecord) ||
           count($rawrecord) < 8 ||
           $name == "" ||
           strcasecmp($name, "DECEASED") == 0;
}


/**
 * Load the live memorial CSV into this module's in-memory record store.
 *
 * @return int Number of memorial records loaded.
 */
function yahrzeit_readDB()
{
    global $yahrzeit_record_count;
    global $yahrzeit_records;

    $filename = site_root() . "/data/yahrzeits-rev4.csv";

    if ( ( $fp = fopen( $filename, "r" )) === false ) {
        die ("fopen failure");
    }

    $yahrzeit_record_count = 0;
    $yahrzeit_records = array();

    while ( ( $rawrecord = fgetcsv($fp, 512, ",", "\"", "") ) !== false ) {

        if (yahrzeit_csv_row_is_invalid($rawrecord)) {
            continue;
        }

        $person = yahrzeit_person_from_csv_row($rawrecord);
        $person['index'] = $yahrzeit_record_count;

        $yahrzeit_records[$yahrzeit_record_count] = $person;
        $yahrzeit_record_count++;
    }

    fclose($fp);
    return $yahrzeit_record_count;
}


/**
 * Rewrite the live memorial CSV from the complete in-memory record store.
 *
 * This legacy low-level writer does not create a backup or run an audit.
 * Request paths should use a protected save/upload workflow instead.
 */
function yahrzeit_writeDB()
{
    global $yahrzeit_records;
    $filename = site_root() . "/data/yahrzeits-rev4.csv";

    if ( ( $fp = fopen( $filename, "w" )) === false ) {
        die ("fopen failure");
    }

    fputcsv($fp, array(
        "DECEASED",
        "Last-Name-First",
        "Eng. DOD",
        "Heb. DOD",
        "YEAR",
        "OPTIONS",
        "OLD Location",
        "New Location",
        "",
        "",
        ""
    ), ",", "\"", "");

    fputcsv($fp, array("", "", "", "", "", "", "", "", "", "", ""), ",", "\"", "");

    foreach ( $yahrzeit_records as $person) {
        $record = yahrzeit_csv_row_from_person( $person );
        fputcsv($fp, $record, ",", "\"", "");
    }

    fclose($fp);
}

/** Return one loaded memorial record, or null when the index does not exist. */
function yahrzeit_getObj( $row )
{
    global $yahrzeit_records;

    return ( $yahrzeit_records[$row] ?? null );
}


// Store one mapped memorial record in memory; this does not write the CSV.
function yahrzeit_putObj( $row, $person )
{
    global $yahrzeit_records;

    $yahrzeit_records[$row] = $person;
}

/** Return a mapped-record template for a new or empty memorial. */
function yahrzeit_blank_person()
{
    return array(
        'lastName'       => "",
        'firstName'      => "",

        'engYzMonth'     => "",
        'engYzDD'        => "",
        'engYzYYYY'      => "",

        'hebYzDD'        => "",
        'hebYzMonth'     => "",
        'hebYzMM'        => "",
        'hebYzYYYY'      => "",

        'useHeb'         => true,
        'useEng'         => false,
        'yomhashoah'     => false,
        'yomhazikaron'   => false,
        'reserved'       => false,

        'panelId'        => "",
        'column'         => "",
        'row'            => "",

        'oldLocation'    => "",
        'newyear'        => ""
    );
}


// ---------------------------------------------------------------------------
// Person display and date-policy helpers
// ---------------------------------------------------------------------------
function yahrzeit_person_name($person)
{
    return trim(($person['firstName'] ?? "") . " " . ($person['lastName'] ?? ""));
}

function yahrzeit_person_location($person)
{
    $panel  = $person['panelId'] ?? "";
    $column = $person['column']  ?? "";
    $row    = $person['row']     ?? "";

    if ($panel == "") {
        return "";
    }

    if ($column == "" && $row == "") {
        return $panel;
    }

    return "$panel-$column-$row";
}

/**
 * Resolve the date method for one memorial.
 *
 * Per-record HEB/ENG options override the synagogue-wide Minhag default.
 */
function yahrzeit_person_uses_hebrew_date($person)
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

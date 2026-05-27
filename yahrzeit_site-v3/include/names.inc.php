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
 *
 *      This was intentionally used to keep the CSV file format and schema 
 *      isolated from the rest of the application.  Most code should ask this 
 *      file for a mapped record rather than parsing CSV columns directly.
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
 *      Modernized for PHP 8 and the yahrzeit_site-v3 directory layout in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

global $num_rows;
global $yahrzeitDB;


// 0     1              2                     3        4
// Name, Date of Death, Hebrew Date of Death, options, location
// Eheved Pinskaya,1/24/1970,17 SHEVAT 5730,Yes,10N3I

// note that is has been changed to an 8-column format in "rev4"
//  0  Name of DECEASED on plaque
//  1  Last-Name-First
//  2  Eng. DOD
//  3  Heb. DOD
//  4  empty (or year)
//  5  OPTIONS
//  6  OLD Location
//  7  New Location panel-col-row

function yahrzeit_safe_field($rawrecord, $index)
{
    return isset($rawrecord[$index]) ? trim($rawrecord[$index]) : "";
}


function yahrzeit_split_name($full_name)
{
    $parts = preg_split('/\s+/', trim($full_name));

    if ($parts === false || count($parts) == 0 || $parts[0] == "") {
        return array("", "");
    }

    $lastname = $parts[count($parts) - 1];
    unset($parts[count($parts) - 1]);

    $firstname = implode(" ", $parts);

    return array($firstname, $lastname);
}


function yahrzeit_parse_english_date($date_str)
{
    global $english_month_names;

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

    $result['monthName'] = $english_month_names[$dod['month'] - 1];
    $result['day']       = $dod['day'];
    $result['year']      = $dod['year'];

    return $result;
}


function yahrzeit_parse_hebrew_date($date_str)
{
    global $hebrew_month_mapping;

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

    if ($hdod === false || count($hdod) < 3) {
        return $result;
    }

    // Special case for "Adar I" and "Adar II":
    //     17 Adar I 5764
    // becomes:
    //     [0] = 17, [1] = Adar I, [2] = 5764
    if (count($hdod) >= 4) {
        $hdod[1] = $hdod[1] . " " . $hdod[2];
        $hdod[2] = $hdod[3];
    }

    $hebrew_month_name = closest_hebrew_month($hdod[1]);

    $result['day']       = $hdod[0];
    $result['monthName'] = $hebrew_month_name;
    $result['monthNum']  = ($hebrew_month_name != "" && isset($hebrew_month_mapping[$hebrew_month_name]))
                         ? $hebrew_month_mapping[$hebrew_month_name]
                         : "";
    $result['year']      = $hdod[2];

    return $result;
}


function yahrzeit_parse_location($location_str)
{
    $location = explode("-", trim($location_str));
    $location = array_pad($location, 3, "");

    return array(
        'panelId' => $location[0],
        'column'  => $location[1],
        'row'     => $location[2]
    );
}


function yahrzeit_map_internal($rawrecord)
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
        yahrzeit_split_name(yahrzeit_safe_field($rawrecord, 0));

    $eng = yahrzeit_parse_english_date(yahrzeit_safe_field($rawrecord, 2));
    $heb = yahrzeit_parse_hebrew_date(yahrzeit_safe_field($rawrecord, 3));
    $loc = yahrzeit_parse_location(yahrzeit_safe_field($rawrecord, 7));

    $newyear     = yahrzeit_safe_field($rawrecord, 4);
    $options     = yahrzeit_safe_field($rawrecord, 5);
    $oldLocation = yahrzeit_safe_field($rawrecord, 6);

    $person = array(
        'lastName'          => $lastname,
        'firstName'         => $firstname,
        'lastNameFirst'     => yahrzeit_safe_field($rawrecord, 1),
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
        'onnow'             => (stristr($options, 'ONNOW')     ? true : false),
        'reserved'          => (stristr($options, 'RESERVED')  ? true : false),
        'manual'            => (stristr($options, 'MANUAL')    ? true : false),
        'options'           => $options,

        'panelId'           => $loc['panelId'],
        'row'               => $loc['row'],
        'column'            => $loc['column'],

        'oldLocation'       => $oldLocation,
        'newyear'           => $newyear
    );

    return $person;
}


function yahrzeit_map_external($person)
{
    global $english_month_mapping;

    $first = isset($person['firstName']) ? trim($person['firstName']) : "";
    $last  = isset($person['lastName'])  ? trim($person['lastName'])  : "";

    $name = trim($first . " " . $last);

    if ($last != "" && $first != "") {
        $lastNameFirst = $last . ", " . $first;
    } else {
        $lastNameFirst = $name;
    }

    if ($person['engYzMonth'] == "" || $person['engYzDD'] == "") {
        $dod = "";
    } else {
        $month = isset($english_month_mapping[$person['engYzMonth']])
               ? $english_month_mapping[$person['engYzMonth']]
               : $person['engYzMonth'];

        $dod = $month . "/" . $person['engYzDD'] . "/" . $person['engYzYYYY'];
    }

    if ($person['hebYzDD'] == "" || $person['hebYzMonth'] == "") {
        $hdod = "";
    } else {
        $hdod = $person['hebYzDD'] . " " . $person['hebYzMonth'] . " " . $person['hebYzYYYY'];
    }

    $options =
            ( $person['useHeb']       ? 'HEB'       : "" ) . " " .
            ( $person['useEng']       ? 'ENG'       : "" ) . " " .
            ( $person['yomhashoah']   ? 'HASHOAH'   : "" ) . " " .
            ( $person['yomhazikaron'] ? 'HAZIKARON' : "" ) . " " .
            ( $person['onnow']        ? 'ONNOW'     : "" ) . " " .
            ( $person['reserved']     ? 'RESERVED'  : "" ) . " " .
            ( $person['manual']       ? 'MANUAL'    : "" );

    $options = trim(preg_replace('/\s+/', ' ', $options));

    $newLocation = "";

    if ($person['panelId'] != "") {
        if ($person['column'] == "" && $person['row'] == "") {
            $newLocation = $person['panelId'];
        } else {
            $newLocation = $person['panelId'] . "-" . $person['column'] . "-" . $person['row'];
        }
    }

    return array(
        $name,
        $lastNameFirst,
        $dod,
        $hdod,
        $person['newyear'],
        $options,
        $person['oldLocation'],
        $newLocation
    );
}


function yahrzeit_not_valid_csv_record($rawrecord)
{
    // Rev4 database records need at least:
    //   0  DECEASED
    //   1  Last-Name-First
    //   2  Eng. DOD
    //   3  Heb. DOD
    //   4  YEAR
    //   5  OPTIONS
    //   6  OLD Location
    //   7  New Location

    if ($rawrecord === false) {
        return true;
    }

    if (!is_array($rawrecord)) {
        return true;
    }

    if (count($rawrecord) < 8) {
        return true;
    }

    $name_field = isset($rawrecord[0]) ? trim($rawrecord[0]) : "";

    if ($name_field == "") {
        return true;
    }

    if (strcasecmp($name_field, "DECEASED") == 0) {
        return true;
    }

    return false;
}


function yahrzeit_readDB()
{
    global $num_rows;
    global $yahrzeitDB;
    global $yahrzeitHash;

    $filename = site_root() . "/data/yahrzeits-rev4.csv";

    if ( ( $fp = fopen( $filename, "r" )) === false ) {
        die ("fopen failure");
    }

    $num_rows = 0;
    $input_line = 0;
    $yahrzeitDB = array();
    $yahrzeitHash = array();

    while ( ( $rawrecord = fgetcsv($fp, 512, ",", "\"", "") ) !== false ) {
        $input_line++;

        if (yahrzeit_not_valid_csv_record($rawrecord)) {
            continue;
        }

        $person = yahrzeit_map_internal($rawrecord);
        $person['index'] = $num_rows;

        $yahrzeitDB[$num_rows] = $person;

        // Hash by physical plaque location too.
        // This is useful for locating a person by panel/row/column.
        $location = $person['panelId'] . "-" . $person['row'] . "-" . $person['column'];
        $yahrzeitHash[$location] = &$yahrzeitDB[$num_rows];

        $num_rows++;
    }

    fclose($fp);
    return $num_rows;
}


function yahrzeit_writeDB()
{
    global $yahrzeitDB;
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

    foreach ( $yahrzeitDB as $key => $value ) {
        $record = yahrzeit_map_external( $value );
        fputcsv($fp, $record, ",", "\"", "");
    }

    fclose($fp);
}

// Return the number of memorial records currently loaded by yahrzeit_readDB().
function yahrzeit_numrows()
{
    global $num_rows;

    return ( $num_rows );
}

// Return one memorial record as an associative array.
// This is the procedural equivalent of fetching a Name object.
function yahrzeit_getObj( $row )
{
    global $yahrzeitDB;

    return ( $yahrzeitDB[$row] );
}


// Store one mapped memorial record in memory.
// Used by the legacy 5singlename.php edit/save path.
// If 5singlename.php becomes read-only, this function and the CSV write-back
// path can probably be removed.
function yahrzeit_putObj( $row, $person )
{
    global $yahrzeitDB;

    $yahrzeitDB[$row] = $person;
}


?>

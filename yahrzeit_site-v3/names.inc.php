<?php

/*
 * NAME
 *      names.inc.php
 *
 * DESCRIPTION
 *      Yahrzeit Name Database
 *      class and functions.
 *
 * NOTES
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
 *	  53    function yahrzeit_map_internal( $rawrecord )
 *	 126    function yahrzeit_map_external( $person )
 *	 159    function yahrzeit_readDB()
 *	 184    function yahrzeit_writeDB()
 *	 200    function yahrzeit_numrows()
 *	 208    function yahrzeit_getObj( $row )
 *	 216    function yahrzeit_putObj( $row, $person )
 *	 224    function yahrzeit_delObj( $row )

 */


global $num_rows;
global $yahrzeitDB;


// 0     1              2                     3        4
// Name, Date of Death, Hebrew Date of Death, options, location
// Eheved Pinskaya,1/24/1970,17 SHEVAT 5730,Yes,10N3I


function yahrzeit_map_internal( $rawrecord )
{
    global $english_month_names;

    // first field, NAME
    $name = explode( " ", trim( $rawrecord[0] ) );
    $lastname = $name[ count($name)-1 ];
    unset( $name[ count($name)-1 ] );
    $firstname = implode (" ", $name );

    // second field -- ignore the Last-Name-First

    // third field, extract english date of death
    $dod = date_parse( trim( $rawrecord[2] ) );
    // Y2K conversion
    if (1 <= $dod['year'] && $dod['year'] <= 99) {
        $dod['year'] += 1900;
    }

    // fourth field, extract hebrew date of death
    $hdod = explode( " ", trim( $rawrecord[3] ) );

    // special algorithm for "Adar I" and "Adar II"
    if (count( $hdod ) == 4)  {
        $hdod[1] = $hdod[1] . " " . $hdod[2];
        $hdod[2] = $hdod[3];
    }

    // fifth field is the "new year"
    $newyear = trim( $rawrecord[4] );

    // sixth field is the options
    $options = trim( $rawrecord[5] );

    // missing field is the "next yahrzeit"
    //$fifth = trim( $rawrecord[4] );

    // seventh field is the old-location
    $oldLocation = trim( $rawrecord[6] );

    // eigth field, extract location
    $location = explode( "-", trim ( $rawrecord[7] ) );

    $hebrew_month_name = closest_hebrew_month( $hdod[1] );
    global $hebrew_month_mapping;

    $person = array (
            'lastName'          =>   $lastname,
            'firstName'         =>   $firstname,
            'engYzMonth'        =>   $english_month_names[$dod['month']-1],
            'engYzDD'           =>   $dod['day'],
            'engYzYYYY'         =>   $dod['year'],
            'hebYzDD'           =>   $hdod[0],
            'hebYzMonth'        =>   $hebrew_month_name,
            'hebYzMM'           =>   $hebrew_month_mapping[ $hebrew_month_name ],
            'hebYzYYYY'         =>   $hdod[2],
            'useHeb'            =>   (stristr( $options, 'HEB' ) ? true : false),
            'useEng'            =>   (stristr( $options, 'ENG' ) ? true : false),
            'yomhashoah'        =>   (stristr( $options, 'HASHOAH') ? true : false),
            'yomhazikaron'      =>   (stristr( $options, 'HAZIKARON' ) ? true : false),
            'onnow'             =>   (stristr( $options, 'ONNOW' ) ? true : false),
            'reserved'          =>   (stristr( $options, 'RESERVED' ) ? true : false),
            'manual'            =>   (stristr( $options, 'MANUAL' ) ? true : false),
            'panelId'           =>   $location[0],
            'row'               =>   $location[2],
            'column'            =>   $location[1],
            'oldLocation'       =>   $oldLocation,
            'newyear'           =>   $newyear
            );
    return  $person;
}


function yahrzeit_map_external( $person )
{
    global $english_month_mapping;

    $name = $person['firstName'] . " " . $person['lastName'];
    if ($person['engYzMonth'] == "" || $person['engYzDD'] == "" ) {
        $dod = " ";
    }
    else {
        $dod = $english_month_mapping[ $person['engYzMonth']] . "/" . $person['engYzDD'] . "/" . $person['engYzYYYY'];
    }
    //if ($person['hebYzDD'] == "" && $person['hebYzMonth'] = "Av":
    $hdod = $person['hebYzDD'] . " " . $person['hebYzMonth'] . " " . $person['hebYzYYYY'];
    $options = 
            ( $person['useHeb'      ] ? 'HEB' : "" ) . " " .
            ( $person['useEng'      ] ? 'ENG' : "" ) . " " .
            ( $person['yomhashoah'  ] ? 'HASHOAH': "" ) . " " .
            ( $person['yomhazikaron'] ? 'HAZIKARON' : "" ) . " " .
            ( $person['onnow'       ] ? 'ONNOW' : "" ) . " " .
            ( $person['reserved'    ] ? 'RESERVED' : "" ) . " " .
            ( $person['manual'      ] ? 'MANUAL' : "" );
    if ( $person['column'] == "" && $person['row'] == "" ) {
        $columnrow = " ";
    } 
    else {
        $columnrow = $person['column'] . "-" . $person['row'];
    }
    $location = $person['panelId'] . "-" . $columnrow;
    $rec =  array( $name, $dod, $hdod, $options, $location );
    return $rec;
}


function yahrzeit_readDB()
{
    global $num_rows;
    global $yahrzeitDB;
    $filename = "/Users/allan/Sites/yahrzeit/data/yahrzeits-rev4.csv";

    if ( ( $fp = fopen( $filename, "r" )) === false ) {
        die ("fopen failure");
    }
    $num_rows = 0;
    while ( ( $rawrecord = fgetcsv ($fp, 512, "," ) ) !== false ) {
        $person =  yahrzeit_map_internal( $rawrecord );
        $person['index'] = $num_rows;
        $yahrzeitDB[$num_rows] = $person;
        // hash via location, too
        $location = $person['panelId'] . "-" . $person['row'] . "-" . $person['column'];
        $yahrzeitHash[$location] = &$yahreitdb[$num_rows];

        $num_rows++;
    }
    fclose( $fp );
    return $num_rows;
}


function yahrzeit_writeDB()
{
    global $yahrzeitDB;
    $filename = "/Users/allan/Sites/yahrzeit/data/yahrzeits-rev4.csv";

    if ( ( $fp = fopen( $filename, "w" )) === false ) {
        die ("fopen failure");
    }
    foreach ( $yahrzeitDB as $key => $value ) {
        $record = yahrzeit_map_external( $value );
        fputcsv($fp, $record);
    }
    fclose($fp);
}


function yahrzeit_numrows()
{
    global $num_rows;

    return ( $num_rows );
}


function yahrzeit_getObj( $row )
{
    global $yahrzeitDB;

    return ( $yahrzeitDB[$row] );
}


function yahrzeit_putObj( $row, $person )
{
    global $yahrzeitDB;

    $yahrzeitDB[$row] = $person;
}


function yahrzeit_delObj( $row )
{
    global $yahrzeitDB;

    unset( $yahrzeitDB[$row] );
}

?>

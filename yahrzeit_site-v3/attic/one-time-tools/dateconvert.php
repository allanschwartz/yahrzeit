<?php

/*
 * NAME
 *      dateconvert.php
 *
 * DESCRIPTION
 *      Date conversion application -- to clean up the database, and 
 *      add english dates where there are none.
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
 *	  58    function yahrzeit_person_from_csv_row( $rawrecord )
 *	 131    function yahrzeit_csv_row_from_person( $person )
 *	 153    function yahrzeit_readDB()
 *	 178    function yahrzeit_writeDB()
 *	 194    function yahrzeit_record_cound()
 *	 202    function yahrzeit_getObj( $row )
 *	 210    function yahrzeit_putObj( $row, $person )
 *	 218    function yahrzeit_delObj( $row )
 *	 230    function dateconvert_main()
 *	 301    function yz_process_person( $i, $person )

 */


require_once "include/misc.inc.php";
//require_once "include/names.inc.php";

global $yahrzeit_record_count;
global $yahrzeit_records;


// 0     1              2                     3        4
// Name, Date of Death, Hebrew Date of Death, options, location
// Eheved Pinskaya,1/24/1970,17 SHEVAT 5730,Yes,10N3I


function yahrzeit_person_from_csv_row( $rawrecord )
{

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

    $person = array (
            'lastName'          =>   $lastname,
            'firstName'         =>   $firstname,
            'engYzMonth'        =>   ENGLISH_MONTH_NAMES[$dod['month']-1],
            'engYzDD'           =>   $dod['day'],
            'engYzYYYY'         =>   $dod['year'],
            'hebYzDD'           =>   $hdod[0],
            'hebYzMonth'        =>   $hebrew_month_name,
            'hebYzMM'           =>   HEBREW_MONTH_MAPPING[ $hebrew_month_name ],
            'hebYzYYYY'         =>   $hdod[2],
            'useHeb'            =>   (stristr( $options, 'HEB' ) ? true : false),
            'useEng'            =>   (stristr( $options, 'ENG' ) ? true : false),
            'yomhashoah'        =>   (stristr( $options, 'HASHOAH') ? true : false),
            'yomhazikaron'      =>   (stristr( $options, 'HAZIKARON' ) ? true : false),
            'onnow'             =>   (stristr( $options, 'ONNOW' ) ? true : false),
            'reserved'          =>   (stristr( $options, 'RESERVED' ) ? true : false),
            'manual'            =>   (stristr( $options, 'MANUAL' ) ? true : false),
            'panelId'           =>   $location[0],
            'row'               =>   $location[1],
            'column'            =>   $location[2],
            'oldLocation'       =>   $oldLocation,
            'newyear'           =>   $newyear
            );
    return  $person;
}


function yahrzeit_csv_row_from_person( $person )
{
    $name = $person['firstName'] . " " . $person['lastName'];
    $name2 = $person['lastName'] . ", " . $person['firstName'];
    $dod =  " " . $person['engYzMonth'] . " " . $person['engYzDD'] . ", " . $person['engYzYYYY'];
    $hdod = " " . $person['hebYzDD'] . " " . $person['hebYzMonth'] . " " . $person['hebYzYYYY'];
    $options = 
            ( $person['useHeb'      ] ? 'HEB' : "" ) . " " .
            ( $person['useEng'      ] ? 'ENG' : "" ) . " " .
            ( $person['yomhashoah'  ] ? 'HASHOAH': "" ) . " " .
            ( $person['yomhazikaron'] ? 'HAZIKARON' : "" ) . " " .
            ( $person['onnow'       ] ? 'ONNOW' : "" ) . " " .
            ( $person['reserved'    ] ? 'RESERVED' : "" ) . " " .
            ( $person['manual'      ] ? 'MANUAL' : "" );
    $location = $person['panelId'] . "-" . $person['row'] . "-" . $person['column'];
    $rec =  array( $name, $name2, $dod, $hdod, $person['newyear'], $options, $person['oldLocation'], $location );
    return $rec;
}


function yahrzeit_readDB()
{
    global $yahrzeit_record_count;
    global $yahrzeit_records;
    $filename = "/tmp/yahrzeits-rev3.csv";

    if ( ( $fp = fopen( $filename, "r" )) === false ) {
        die ("fopen failure");
    }
    $yahrzeit_record_count = 0;
    while ( ( $rawrecord = fgetcsv ($fp, 512, "," ) ) !== false ) {
        $person =  yahrzeit_person_from_csv_row( $rawrecord );
        $person['index'] = $yahrzeit_record_count;
        $yahrzeit_records[$yahrzeit_record_count] = $person;
        // hash via location, too
        $location = $person['panelId'] . "-" . $person['row'] . "-" . $person['column'];
        $yahrzeitHash[$location] = &$yahreitdb[$yahrzeit_record_count];

        $yahrzeit_record_count++;
    }
    fclose( $fp );
    return $yahrzeit_record_count;
}


function yahrzeit_writeDB()
{
    global $yahrzeit_records;
    $filename = "/tmp/yahrzeits-rev4.csv";

    if ( ( $fp = fopen( $filename, "w" )) === false ) {
        die ("fopen failure");
    }
    foreach ( $yahrzeit_records as $key => $value ) {
        $record = yahrzeit_csv_row_from_person( $value );
        fputcsv($fp, $record);
    }
    fclose($fp);
}


function yahrzeit_record_cound()
{
    global $yahrzeit_record_count;

    return ( $yahrzeit_record_count );
}


function yahrzeit_getObj( $row )
{
    global $yahrzeit_records;

    return ( $yahrzeit_records[$row] );
}


function yahrzeit_putObj( $row, $person )
{
    global $yahrzeit_records;

    $yahrzeit_records[$row] = $person;
}


function yahrzeit_delObj( $row )
{
    global $yahrzeit_records;

    unset( $yahrzeit_records[$row] );
}


//phpinfo();
dateconvert_main();


function dateconvert_main()
{
    global $today_month;
    global $today_day;
    global $today_year;

    global $hebrewDay;
    global $hebrewMonth;
    global $hebrewMonthName;
    global $hebrewYear;
    
    // calculate gregorian month, day, year
    $today_month = date('n');
    $today_day = date('j');
    $today_year = date('Y');
    echo "today is $today_month/$today_day/$today_year ... ";

    // calculate jewish month, day, year
    $jdDate = gregoriantojd($today_month,$today_day,$today_year);
    $hebrewMonthName = jdmonthname($jdDate,4);
    $hebrewDate = jdtojewish($jdDate);
    list($hebrewMonth, $hebrewDay, $hebrewYear) = explode('/',$hebrewDate);
    echo "$hebrewDay $hebrewMonthName ($hebrewMonth) $hebrewYear\n";

    $n2 = yahrzeit_readDB();

    for( $i = 0 ; $i < $n2 ; $i++ ) {
        $person = yahrzeit_getObj( $i );

        //echo "<pre> before"; print_r($person); echo "</pre>\n";

        yz_process_person( $i, $person );


        // count on location
        $person = yahrzeit_getObj( $i );
        $x = explode( "-", trim ( $person['oldLocation']  ));
        $key = $x[ 0 ];
        $loc_count[ $key ]++;
        $yr = $person['newyear'];
        if ($yr == "" || $yr == 0 ) {
            $yr = $person['engYzYYYY'];
        }
        if (1850 <= $yr && yr <= 2007 ) {
            $loc_count2[ $key ] ++;
            $loc_sigma[ $key ] += $yr;
            if ( 1997 <= $yr  && $yr <= 2007 ) {
                $loc_recnt[ $key ]++;
            }
        }


    }
    yahrzeit_writeDB();

    // summarize
    foreach ( $loc_count as $key => $value ) {
        if( $loc_count2[ $key ] ) {
            $ave = $loc_sigma[ $key ] / $loc_count2[ $key ];
        } else {
            $ave = 1;
        }
        printf(  "$key: %d (%d) , ave %d, recent %d \n",
            $loc_count[ $key ],
            $loc_count2[ $key ],
            $ave,
            $loc_recnt[ $key ] );
    }
}


function yz_process_person( $i, $person )
{
    global $english_month_names;

    $newyear = $person['newyear'];
    //echo "lastname " . $person['lastName'] . " directive $newyear \n";

    if ( $newyear == "no yr" ) {
        $person['engYzDD'] =  "";
        $person['engYzMonth'] = "";
        $person['engYzYYYY'] =  "";
        $person['hebYzYYYY'] = "";
    }
    else if ( $newyear == "no date" ) {
        $person['engYzDD'] =  "";
        $person['engYzMonth'] = "";
        $person['engYzYYYY'] =  "";
        $person['hebYzDD'] = "";
        $person['hebYzMonth'] = "";
        $person['hebYzYYYY'] = "";
    }
    else if ( ($newyear == "" || $newyear == "eng") && $person['oldLocation'] != "missing" ) {
        // just check the data
        $yr = $person['hebYzYYYY'];
        if ( 5561 <= $yr && $yr <= 5768 ) {
            $juliandatecount = jewishtojd(
                                        $person['hebYzMM'],
                                        $person['hebYzDD'],
                                        $person['hebYzYYYY']  );
            $caldate = jdtogregorian( $juliandatecount );
            $x = explode( "/", $caldate );
            //echo "caldate $caldate \n";
            $dod =  $person['engYzMonth'] . " " . $person['engYzDD'] . ", " . $person['engYzYYYY'];
            if ( ( $person['engYzMonth'] != ENGLISH_MONTH_NAMES[ $x[0] - 1 ] ) ||
               ( $person['engYzDD'] != $x[1] ) ||
               ( $person['engYzYYYY'] != $x[2] ) ) {
                echo "DATE ERROR... name " . $person['firstName'] . " " .  $person['lastName'] . " caldate $caldate recorded-date $dod\n";
            }
        }
    }
    else if ( ($newyear == "" || $newyear == "eng") ) {
    }
    else if ( 1800 <= $newyear && $newyear <= 2007 ) {
        $person['hebYzYYYY'] = $newyear + 3761;
        $juliandatecount = jewishtojd(
                                        $person['hebYzMM'],
                                        $person['hebYzDD'],
                                        $person['hebYzYYYY']  );
        $caldate = jdtogregorian( $juliandatecount );
        //echo "caldate $caldate \n";
        // "month/day/year"
        $x = explode( "/", $caldate );
        $person['engYzDD'] =  $x[1];
        $person['engYzMonth'] = ENGLISH_MONTH_NAMES[ $x[0] - 1 ];
        $person['engYzYYYY'] =  $x[2];
        //echo "lastname " .  $person['lastName'] . " year $x[2] \n";
        if ($x[2] > $newyear ) {

            $person['hebYzYYYY'] = $newyear + 3760;
            $juliandatecount = jewishtojd(
                                            $person['hebYzMM'],
                                            $person['hebYzDD'],
                                            $person['hebYzYYYY']  );
            $caldate = jdtogregorian( $juliandatecount );
            //echo "caldate $caldate \n";
            // "month/day/year"
            $x = explode( "/", $caldate );
            $person['engYzDD'] =  $x[1];
            $person['engYzMonth'] = ENGLISH_MONTH_NAMES[ $x[0] - 1 ];
            $person['engYzYYYY'] =  $x[2];
        }
        //echo "lastname " .  $person['lastName'] . " year $x[2] \n";

        assert ( $x[2] == $newyear );
    }
    else if ( 5561 <= $newyear && $newyear <= 5768 ) {
        $person['hebYzYYYY'] = $newyear;
        $juliandatecount = jewishtojd(
                                        $person['hebYzMM'],
                                        $person['hebYzDD'],
                                        $person['hebYzYYYY']  );
        $caldate = jdtogregorian( $juliandatecount );
        //echo "caldate $caldate \n";
        // "month/day/year"
        $x = explode( "/", $caldate );
        $person['engYzDD'] =  $x[1];
        $person['engYzMonth'] = ENGLISH_MONTH_NAMES[ $x[0] - 1 ];
        $person['engYzYYYY'] =  $x[2];
    }
    else {
        echo "unknown directive -- $newyear\n";
    }
    //echo "<pre> after"; print_r($person); echo "</pre>\n";
    yahrzeit_putObj( $i, $person );

    return;
}

?>

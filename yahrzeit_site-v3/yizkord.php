<?php

/*
 * NAME
 *      yizkord.php
 *
 * SYNOPSIS
 *      php yizkord.php ON|OFF
 *
 * DESCRIPTION
 *      This deamon runs at either sunset or sunrise or some other
 *      time, and turns on all LEDs representing a YIZKOR holiday.
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
 *    28    function yz_main()
 *    64    function yz_lighton( $person )
 *    78    function yz_lightoff( $person )
 *    92    function yz_process_person( $person )
 *   111    function haYomShabbat()

 */


require_once "misc.inc.php";
require_once "panels.inc.php";
require_once "names.inc.php";
require_once "leds.inc.php";

global $minhag;
$minhag = read_minhag_ini();

yz_main();



// main function for the "yizkord"
//  does some date calculations
//  reads entire yahrzeit database
//  for each person in database, calls yz_process_person
function yz_main()
{
    global $today_month;
    global $today_day;
    global $today_year;

    global $hebrewDay;
    global $hebrewMonth;
    global $hebrewMonthName;
    global $hebrewYear;
    global $argc;
    global $argv;
    global $leds_on;
    
    $leds_on = ($argv[1] == "ON");

    // calculate gregorian month, day, year
    $today_month = date('n');
    $today_day = date('j');
    $today_year = date('Y');
    echo "Today is ";
    if (haYomShabbat() ) echo "SHABBAT ";
    echo "$today_month/$today_day/$today_year ... ";

    // calculate jewish month, day, year
    $jdDate = gregoriantojd($today_month,$today_day,$today_year);
    $hebrewMonthName = jdmonthname($jdDate,4);
    $hebrewDate = jdtojewish($jdDate);
    list($hebrewMonth, $hebrewDay, $hebrewYear) = split('/',$hebrewDate);
    echo "$hebrewDay $hebrewMonthName ($hebrewMonth) $hebrewYear\n";

    $n1 = panel_readDB();
    $n2 = yahrzeit_readDB();

    for( $i = 0 ; $i < $n2 ; $i++ ) {
        $person = yahrzeit_getObj( $i );

        yz_process_person( $person );
    }
}


// main processing loop:
//  for each person in the data base, turn the LED on.
function yz_process_person( $person )
{
    global $minhag;
    global $leds_on;

    if ( $person['reserved'] == "YES" ) {
        yz_lightoff( $person );
        return;
    }
    if ( $person['manual']  == "YES" ) {
        yz_lighton( $person );
        return;
    }
    // else 'automatic' operation..
    // since this is a yizkor observance, the lights either all go on or all go off
    if ( $leds_on ) {
        yz_lighton( $person );
    }
    else {
        yz_lightoff( $person );
    }
    return;
}


// Boolean: is today the sabbath?
function haYomShabbat()
{
    global $today_month;
    global $today_day;
    global $today_year;

    $timestamp = mktime( 16, 0, 0, $today_month, $today_day, $today_year );
    $dateinfo = getdate( $timestamp );

    return ($dateinfo['weekday'] == "Friday" );
}

?>

<?php

/*
 * NAME
 *     yz_watcher.php
 *
 * SYNOPSIS
 *
 * DESCRIPTION
 *      This deamon runs once a day (at 2am or so),
 *      and schedules the yarhzeitd or the yizkord to run subsequently.
 *
 * NOTES
 *
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
 *    35    function yz_main()
 *    81    function yz_lighton( $person )
 *    95    function yz_lightoff( $person )
 *   109    function yz_process_person( $person )
 *   147    function engWeekMatch( $month, $day )
 *   180    function engDayMatch( $month, $day )
 *   206    function isYahrzeitThisWeek( $person )
 *   237    function isYahrzeitToday( $person)
 *   267    function isLeapYear( $year ) 
 *   286    function haYomShabbat()

 */

<?php

/*
    yz_watcher

    yahrzeit watcher


 */
/*


*/

require_once "misc.inc";
require_once "panels.inc";
global $minhag;
$minhag = read_minhag_ini();
yz_main();


function yz_main()
{
    global $minhag;
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

    // calculate jewish month, day, year
    $jdDate = gregoriantojd($today_month,$today_day,$today_year);
    $hebrewMonthName = jdmonthname($jdDate,4);
    $hebrewDate = jdtojewish($jdDate);
    list($hebrewMonth, $hebrewDay, $hebrewYear) = explode('/',$hebrewDate);
    echo "$hebrewDay $hebrewMonthName ($hebrewMonth) $hebrewYear";

    // time to run yahrzeitd... 
    if ( $minhag['yahrzeitLightTime'] == "atSunset" ) {
        ... calculate sunset time ...
        $hh = 
        $mm = 
    } else {
        $hh = $minhag['yahrzeitLightOnHH'];
        $mm = $minhag['yahrzeitLightOnMM'];
    }

    // calculate the next day we need to run 'yahrzeitd'
    if ( $minhag['yahrzeitFullWeek'] == "YES" ) {
        yz_date =  next erev shabbat
    } else if ( $minhag['yahrzeitPlusShabbat'] == "YES" ) [
        yz_date = today
    }


    // calculate the next day we need to run 'yizkord'
    :q
    for ( $today_day = 1; $today_day <=31; $today_day++ ) {
        if (haYomShabbat() ) echo "\n SHABBAT\n";
        echo "if today is $today_month/$today_day/$today_year ... ";
        // calculate jewish month, day, year
        $jdDate = gregoriantojd($today_month,$today_day,$today_year);
        $hebrewMonthName = jdmonthname($jdDate,4);
        $hebrewDate = jdtojewish($jdDate);
        echo " hebrewDate $hebrewDate ";
        list($hebrewMonth, $hebrewDay, $hebrewYear) = explode('/',$hebrewDate);
        echo "$hebrewDay $hebrewMonthName ($hebrewMonth) $hebrewYear\n";

        $n1 = panel_readDB();
    }
}



function isYahrzeitThisWeek( $person )
{
    global $minhag;
    global $hebrewDay;
    global $hebrewMonth;
    global $hebrewMonthName;
    global $hebrewYear;
    global $hebrew_month_mapping;

    if ( $minhag['yahrzeitEngOrHeb'] == "heb"  || $person['useHeb'] == "heb" ) {
        
        $m = $hebrew_month_mapping[ closest_hebrew_month( $person['hebYzMonth'] ) ];


        $yz_jd_date = jewishtojd ( $m, $person['hebYzDD'],  $hebrewYear );
        $yz_gregorian_date = JDToGregorian( $yz_jd_date );
        list( $mm, $dd, $yy ) = split ('/', $yz_gregorian_date );

        #echo "debug yz_jd_date $yz_jd_date yz_gregorian_date $yz_gregorian_date hebMon {person['hebYzMonth']} hebDay {person['hebYzDD']} mm $mm dd $dd yy $yy\n" ;

        return engWeekMatch( $mm, $dd );

    }
    else if ( $minhag['yahrzeitEngOrHeb'] == "eng"  || $person['useEng'] ) {

        return engWeekMatch( $person['engYzMonth'], $person['engYzDD'] );

    }
}


function isYahrzeitToday( $person)
{
    global $minhag;
    global $hebrewDay;
    global $hebrewMonth;
    global $hebrewMonthName;
    global $hebrewYear;
    global $hebrew_month_mapping;

    if ( $minhag['yahrzeitEngOrHeb'] == "heb"  || $person['useHeb'] == "heb" ) {

        $m = $hebrew_month_mapping[ closest_hebrew_month( $person['hebYzMonth'] ) ];

        $yz_jd_date = jewishtojd ( $m, $person['hebYzDD'],  $hebrewYear );
        $yz_gregorian_date = JDToGregorian( $yz_jd_date );
        list( $mm, $dd, $yy ) = split ('/', $yz_gregorian_date );

        #echo "debug yz_jd_date $yz_jd_date yz_gregorian_date $yz_gregorian_date hebMon ${person['hebYzMonth']} hebDay {person['hebYzDD']} mm $mm dd $dd yy $yy\n" ;

        return engDayMatch( $mm, $dd );

    }
    else if ( $minhag['yahrzeitEngOrHeb'] == "eng"  || $person['useEng'] ) {

        return engDayMatch( $person['engYzMonth'], $person['engYzDD'] );

    }
}


function isLeapYear( $year ) 
{
    if (($year % 4) == 0) {
        $leap = true;
        if (($year % 100 == 0)) {
            if (($year % 400 == 0)) 
                $leap = true;
            else 
                $leap = false;
        }
    }
    else  {
        $leap = false;
    }

    return $leap;
}


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

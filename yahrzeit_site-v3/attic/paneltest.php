<?php

/*
 * NAME
 *      paneltest.php
 *
 * SYNOPSIS
 *
 * DESCRIPTION
 *      various tests....
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
 *	  58    function testmain() 
 *	  86    function panel_test1( $panel )
 *	  93    function panel_test2( $panel )
 *	  99    function panel_test3( $panel )
 *	 108    function panel_test4( $panel )
 *	 117    function panel_test5( $panel )
 *	 127    function panel_test6( $panel ) 
 *	 137    function panel_test7( $panel ) 
 *	 151    function panel_test8( $panel ) 
 *	 165    function panel_test9( $panel ) 
 *	 179    function panel_test10( $panel ) 

 */


/* require_once "misc.inc.php";  */
/* require_once "panels.inc.php"; */
/* require_once "names.inc.php"; */
require_once "leds.inc.php";

/* global $minhag; */
/* $minhag = read_minhag_ini(); */

testmain();

function testmain() 
{
    global $argc;
    global $argv;

    if ( $argc != 3 ) {
        echo "debug61 argc $argc \n"; 
        echo "Usage: php paneltest.php _panelid_ _test_\n";
        exit();
    }
    $panelId = ($argv[1] + 0);
    $testId = ($argv[2] + 0);
    echo "debug67 panelId $panelId testId $testId\n"; 
    if ($testId == "st" ) { led_self_test(); }
    else if ($testId == "init" ) { led_init_packet( 56, 10 ); }
    else if ($testId == 1) { panel_test1 ( $panelId ) ; }
    else if ($testId == 2) { panel_test2 ( $panelId ) ; }
    else if ($testId == 3) { panel_test3 ( $panelId ) ; }
    else if ($testId == 4) { panel_test4 ( $panelId ) ; }
    else if ($testId == 5) { panel_test5 ( $panelId ) ; }
    else if ($testId == 6) { panel_test6 ( $panelId ) ; }
    else if ($testId == 7) { panel_test7 ( $panelId ) ; }
    else if ($testId == 8) { panel_test8 ( $panelId ) ; }
    else if ($testId == 9) { panel_test9 ( $panelId ) ; }
    else if ($testId == 10) { panel_test10 ( $panelId ) ; }
    else echo "Usage: php paneltest.php _panelid_ _test_\n";
}


// test 1 ... for a panel ... turn one LED on
function panel_test1( $panel )
{
                // panelid, rol, col
    led_lighton( $panel, 1, 1 );
    led_data_refresh();
}

// test 2 ... for a panel ... turn one LED off
function panel_test2( $panel )
{
    led_lightoff( $panel, 1, 1 );
    led_data_refresh();
}

// test 3 ... for a panel ... turn corner LEDs on
function panel_test3( $panel )
{
    led_lighton( $panel, 1, 1 );
    led_lighton( $panel, nrows_perpanel( $panel), 1 );
    led_lighton( $panel, 1, ncols_perpanel( $panel)  );
    led_lighton( $panel, nrows_perpanel( $panel), ncols_perpanel( $panel) );
    led_data_refresh();
}

// test 4 ... for a panel ... turn corner LEDs off
function panel_test4( $panel )
{
    led_lightoff( $panel, 1, 1 );
    led_lightoff( $panel, nrows_perpanel( $panel), 1 );
    led_lightoff( $panel, 1, ncols_perpanel( $panel)  );
    led_lightoff( $panel, nrows_perpanel( $panel), ncols_perpanel( $panel) );
    led_data_refresh();
}

// test 5 ... for each panel ... all lights on
function panel_test5( $panel )
{
    for( $col = 1; $col <= ncols_perpanel($panel) ; $col++ ) {
        for( $row = 1; $row <= nrows_perpanel($panel); $row++ ) {
            led_lighton( $panel, $row, $col );
        }
    }
    led_data_refresh();
}
    
// test 6 ... for each panel ... all lights off
function panel_test6( $panel ) 
{
    for( $col = 1; $col <= ncols_perpanel($panel) ; $col++ ) {
        for( $row = 1; $row <= nrows_perpanel($panel); $row++ ) {
            led_lightoff( $panel, $row, $col );
        }
    }
    led_data_refresh();
}

// test 7 ... marching rows, down ...  on/off
function panel_test7( $panel ) 
{
    for( $row = 1; $row <= nrows_perpanel($panel); $row++ ) {
        for( $col = 1; $col <= ncols_perpanel($panel) ; $col++ ) {
            led_lighton( $panel, $row, $col );
        }
        led_data_refresh();
        sleep(5);
        for( $col = 1; $col <= ncols_perpanel($panel) ; $col++ ) {
            led_lightoff( $panel, $row, $col );
        }
        led_data_refresh();
    }
}
    
// test 8 ... marching rows, up ... on/off
function panel_test8( $panel ) 
{
    for( $row = nrows_perpanel($panel); $row >= 1 ; $row-- ) {
        for( $col = 0; $col < 6; $col++ ) {
            led_lighton( $panel, $row, $col );
        }
        led_data_refresh();
        sleep(5);
        for( $col = 0; $col < 6; $col++ ) {
            led_lightoff( $panel, $row, $col );
        }
        led_data_refresh();
    }
}
    
// test 9 ... marching columns, right ... on/off
function panel_test9( $panel ) 
{
    for( $col = 1; $col <= ncols_perpanel($panel) ; $col++ ) {
        for( $row = 1; $row <= nrows_perpanel($panel); $row++ ) {
            led_lighton( $panel, $row, $col );
        }
        led_data_refresh();
        sleep(5);
        for( $row = 1; $row <= nrows_perpanel($panel); $row++ ) {
            led_lightoff( $panel, $row, $col );
        }
        led_data_refresh();
    }
}
    
// test 10 ... marching columns, left ...on/off
function panel_test10( $panel ) 
{
    for( $col = ncols_perpanel($panel); $col >= 1; $col-- ) {
        for( $row = 1; $row <= nrows_perpanel($panel); $row++ ) {
            led_lighton( $panel, $row, $col );
        }
        led_data_refresh();
        sleep(5);
        for( $row = 1; $row <= nrows_perpanel($panel); $row++ ) {
            led_lightoff( $panel, $row, $col );
        }
        led_data_refresh();
    }
}

?>

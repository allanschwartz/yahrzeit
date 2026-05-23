<?php

/*
 * NAME
 *      leds.inc.php
 *
 * DESCRIPTION
 *      functions dealing with the very low-level LEDs
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
 *	  98    function LED_number( $panelId, $row, $column ) 
 *	 120    function nrows_perpanel( $panelId )
 *	 127    function ncols_perpanel( $panelId )
 *	 134    function led_lighton( $panelId, $row, $column )
 *	 149    function led_lightoff( $panelId, $row, $column )
 *	 162    function led_protocol( $str )
 *	 173    function yz_lighton( $person )
 *	 196    function yz_lightoff( $person )
 *	 219    function led_self_test( $testnumber )
 *	 229    function led_all( $onoff )
 *	 239    function led_data_refresh()

 */

global $ttyname;
$ttyname = "/dev/tty.PL2303-0000103D";

// initialize the led-mapping datastructure
global $led_panel_mapping;
$led_panel_mapping = array (
    "col1a" => 1,  "col1b" => 2, "col1c" => 3,
    "col2a" => 4,  "col2b" => 5, "col2c" => 6,
    "col3a" => 7,  "col3b" => 8, "col3c" => 9,
    "col4a" => 10, "col4b" => 11, "col4c" => 12,
    "col5a" => 13, "col5b" => 14, "col5c" => 15,
    "col6a" => 16, "col6b" => 17, "col6c" => 18,
    "col7a" => 19, "col7b" => 20, "col7c" => 21,
    );

global $led_number_mapping;
$led_number_mapping = array (
    10000,
    1, 17, 39,          // panels 1, 2, 3 comprise column 1
    281, 297, 319,      // panels 4, 5, 6 comprise column 2
    617, 633, 655,      // panels 7, 8, 9 comprise column 3
    953, 969, 991,      // panels 10, 11, 12 comprise column 4
    1289, 1305, 1327,   // panels 13, 14, 15 comprise column 5
    1625, 1641, 1663,   // panels 16, 17, 18 comprise column 6
    1961, 1977, 1999,   // panels 19, 20, 21 comprise column 7
    );

global $led_nrows_perpanel;
$led_nrows_perpanel = array (
    56,
    16, 22, 18, 
    16, 22, 18, 
    16, 22, 18, 
    16, 22, 18, 
    16, 22, 18, 
    16, 22, 18, 
    16, 22, 18, 
    );

global $led_ncols_perpanel;
$led_ncols_perpanel = array (
    40,
    5, 5, 5,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    5, 5, 5,
    );

// map a {panelid, row, column} triple into the LED number
function LED_number( $panelId, $row, $column ) 
{
    global $led_panel_mapping;
    global $led_number_mapping;

    if ( !is_numeric( $panelId ) ) {
        $first = $led_number_mapping[ $led_panel_mapping[$panelId] ];
    }
    else {
        $first = $led_number_mapping[ $panelId ];
    }
    if ( $first == 0 ) return 0;
    if ( $row == 0 ) return 0;
    if ( $column == 0 ) return 0;

    $led_nmbr = $first + ($column - 1) * 56 + ($row - 1);
    return $led_nmbr;
}

function nrows_perpanel( $panelId )
{
    global $led_nrows_perpanel;
    return $led_nrows_perpanel[ $panelId ];
}


function ncols_perpanel( $panelId )
{
    global $led_ncols_perpanel;
    return $led_ncols_perpanel[ $panelId ];
}

// turn the specified LED on
function led_lighton( $panelId, $row, $column )
{
    $ledaddress = $panelId . "-" . $row . "-" . $column;
    $led_nmbr = LED_number( $panelId, $row, $column );
    global $led_panel_mapping;
    $panel = $led_panel_mapping[ $panelId ];
    echo "pixel on $row $column $panel \n";
}


// turn the specified LED off
function led_lightoff( $panelId, $row, $column )
{
    $ledaddress = $panelId . "-" . $row . "-" . $column;
    $led_nmbr = LED_number( $panelId, $row, $column );
    global $led_panel_mapping;
    $panel = $led_panel_mapping[ $panelId ];
    echo "pixel off $row $column $panel \n";
}

function led_protocol( $str )
{
    global $ttyname;
    $fp = fopen( $ttyname, "w" );
    fwrite( $fp, $str );
    $read = fgets( $fp, 1024 );
    echo $read;
    fclose ( $fp );
}

// turn the yahrzeit LED on
function yz_lighton( $person )
{
    $person['onnow'] = true;
    $name = $person['firstName']." ".$person['lastName'];
    $yz_date = $person['engYzMonth']."/".$person['engYzDD'] . " or " .
               $person['hebYzDD']."/".$person['hebYzMonth'];
    $ledaddress = $person['panelId'] . "-" . $person['row'] . "-" . $person['column'];
    echo "# $name, $yz_date, $ledaddress\n";
    $row = $person['row'];
    $column = $person['column'];
    global $led_panel_mapping;
    $panel = $led_panel_mapping[ $person['panelId'] ];
    echo "pixel on $row $column $panel \n\n";
}


// turn the yahrzeit LED off
function yz_lightoff( $person )
{
    if ( !$person['onnow'] ) {
        return;
    }
    $person['onnow'] = false;
    $name = $person['firstName']." ".$person['lastName'];
    $yz_date = $person['engYzMonth']."/".$person['engYzDD'] . " or " .
               $person['hebYzDD']."/".$person['hebYzMonth'];
    $ledaddress = $person['panelId'] . "-" . $person['row'] . "-" . $person['column'];
    echo "# $name, $yz_date, $ledaddress\n";
    $row = $person['row'];
    $column = $person['column'];
    global $led_panel_mapping;
    $panel = $led_panel_mapping[ $person['panelId'] ];
    echo "pixel off $row $column $panel \n\n";
}

 
// start self-test 
function led_self_test( $testnumber )
{
    echo "test $testnumber\n";
}


// all LEDs on or off
function led_all( $onoff )
{
    echo "all $onoff\n\n";
}


// refresh the LEDs 
function led_data_refresh()
{
    echo "refresh\n";
}

?>


<?php

/*
 * NAME
 *      leds.inc.php
 *
 * DESCRIPTION
 *      Low-level command emitters for the Yahrzeit embedded controller.
 *
 *      This file no longer talks directly to serial hardware. It emits the
 *      line-oriented controller command stream consumed by bin/yahrzeit, which
 *      transmits the stream with nc.
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
 *	 229    function led_all( $onoff )
 *	 239    function led_data_refresh()

 */


global $led_panel_mapping;
$led_panel_mapping = array (
    "col1a" => 1,  "col1b" => 2,  "col1c" => 3,
    "col2a" => 4,  "col2b" => 5,  "col2c" => 6,
    "col3a" => 7,  "col3b" => 8,  "col3c" => 9,
    "col4a" => 10, "col4b" => 11, "col4c" => 12,
    "col5a" => 13, "col5b" => 14, "col5c" => 15,
    "col6a" => 16, "col6b" => 17, "col6c" => 18,
    "col7a" => 19, "col7b" => 20, "col7c" => 21,
);


function yz_panel_number($panelId)
{
    global $led_panel_mapping;

    return isset($led_panel_mapping[$panelId]) ? $led_panel_mapping[$panelId] : "";
}


// turn the yahrzeit LED on
function yz_lighton($person)
{
    $name = $person['firstName'] . " " . $person['lastName'];
    $yz_date = $person['engYzMonth'] . "/" . $person['engYzDD'] . " or " .
               $person['hebYzDD'] . "/" . $person['hebYzMonth'];
    $ledaddress = $person['panelId'] . "-" . $person['row'] . "-" . $person['column'];

    $row = $person['row'];
    $column = $person['column'];
    $panel = yz_panel_number($person['panelId']);

    if ($panel == "") {
        echo "# WARNING: unknown panelId {$person['panelId']} for $name\n";
        return;
    }

    echo "# $name, $yz_date, $ledaddress\n";
    echo "pixel on $row $column $panel\n\n";
}


// turn the yahrzeit LED off
function yz_lightoff($person)
{
    $name = $person['firstName'] . " " . $person['lastName'];
    $yz_date = $person['engYzMonth'] . "/" . $person['engYzDD'] . " or " .
               $person['hebYzDD'] . "/" . $person['hebYzMonth'];
    $ledaddress = $person['panelId'] . "-" . $person['row'] . "-" . $person['column'];

    $row = $person['row'];
    $column = $person['column'];
    $panel = yz_panel_number($person['panelId']);

    if ($panel == "") {
        echo "# WARNING: unknown panelId {$person['panelId']} for $name\n";
        return;
    }

    echo "# $name, $yz_date, $ledaddress\n";
    echo "pixel off $row $column $panel\n\n";
}

// all LEDs on or off
function led_all( $onoff )
{
    echo "all $onoff\n";
}


// refresh the LEDs 
function led_data_refresh()
{
    echo "refresh\n";
}

?>


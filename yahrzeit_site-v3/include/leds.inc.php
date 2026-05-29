<?php

/*
 * NAME
 *      leds.inc.php
 *
 * DESCRIPTION
 *      Low-level command emitters for the Yahrzeit embedded LED controller.
 *
 *      This file no longer talks directly to serial hardware, sockets, or
 *      device files.  It emits the line-oriented controller command stream
 *      consumed by bin/yahrzeit, which is responsible for transmitting the
 *      stream to the Arduino V3 controller.
 *
 *      Higher-level code decides what should be lit. This file only translates
 *      those decisions into controller commands.
 * 
 * NOTES
 *      Keep this file deliberately small.  Calendar rules, Minhag policy,
 *      report generation, and database validation belong elsewhere.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized for the Arduino V3 controller and the bin/yahrzeit wrapper
 *      in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */


const LED_PANEL_MAPPING = [
    "col1a" => 1,  "col1b" => 2,  "col1c" => 3,
    "col2a" => 4,  "col2b" => 5,  "col2c" => 6,
    "col3a" => 7,  "col3b" => 8,  "col3c" => 9,
    "col4a" => 10, "col4b" => 11, "col4c" => 12,
    "col5a" => 13, "col5b" => 14, "col5c" => 15,
    "col6a" => 16, "col6b" => 17, "col6c" => 18,
    "col7a" => 19, "col7b" => 20, "col7c" => 21,
];


function yz_panel_number($panelId)
{
    return LED_PANEL_MAPPING[$panelId] ?? "";
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


<?php

/*
 * NAME
 *      panels.inc.php
 *
 * DESCRIPTION
 *      Static Congregation Beth Sholom Yahrzeit Wall panel geometry.
 *
 * NOTES
 *      This is no longer a writable CSV-backed "database".
 *
 *      The V3 embedded controller owns physical LED addressing.  The PHP
 *      appliance only needs stable panel geometry and panelId lookup for
 *      display, validation, and legacy GUI support.
 *
 *      Panel IDs are physical wall locations such as col1a, col1b, col1c.
 *      Controller panel numbers are maintained in leds.inc.php.
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
 *	  48    function panel_readDB()
 *	  79    function panel_writeDB()
 *	  95    function panel_numrows()
 *	 103    function panel_getObj( $row )
 *	 111    function panel_getObj_byId( $panelId )
 *	 120    function panel_putObj( $row, $panel )
 *	 128    function panel_delObj( $row )
 *	 135    function panel_delObj_byId( $panelId )

 */

global $num_rows;
global $panelsDB;
global $panelHash;

global $panel_static_geometry;
$panel_static_geometry = array(
    array('panelId' => 'col1a', 'nRows' => 16, 'nCols' => 5, 'nNames' => 80),
    array('panelId' => 'col1b', 'nRows' => 22, 'nCols' => 5, 'nNames' => 110),
    array('panelId' => 'col1c', 'nRows' => 18, 'nCols' => 5, 'nNames' => 90),

    array('panelId' => 'col2a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96),
    array('panelId' => 'col2b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132),
    array('panelId' => 'col2c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108),

    array('panelId' => 'col3a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96),
    array('panelId' => 'col3b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132),
    array('panelId' => 'col3c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108),

    array('panelId' => 'col4a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96),
    array('panelId' => 'col4b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132),
    array('panelId' => 'col4c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108),

    array('panelId' => 'col5a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96),
    array('panelId' => 'col5b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132),
    array('panelId' => 'col5c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108),

    array('panelId' => 'col6a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96),
    array('panelId' => 'col6b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132),
    array('panelId' => 'col6c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108),

    array('panelId' => 'col7a', 'nRows' => 16, 'nCols' => 5, 'nNames' => 80),
    array('panelId' => 'col7b', 'nRows' => 22, 'nCols' => 5, 'nNames' => 110),
    array('panelId' => 'col7c', 'nRows' => 18, 'nCols' => 5, 'nNames' => 90),
);

function panel_readDB()
{
    global $num_rows;
    global $panelsDB;
    global $panelHash;
    global $panel_static_geometry;

    $num_rows = 0;
    $panelsDB = array();
    $panelHash = array();

    foreach ($panel_static_geometry as $panel) {
        $panelsDB[$num_rows] = $panel;
        $panelHash[$panel['panelId']] = $num_rows;
        $num_rows++;
    }

    return $num_rows;
}

function panel_writeDB()
{
    die("panel_writeDB disabled: panel geometry is now static in panels.inc.php");
}

function panel_numrows()
{
    global $num_rows;

    return $num_rows;
}

function panel_getObj($row)
{
    global $panelsDB;

    return isset($panelsDB[$row]) ? $panelsDB[$row] : null;
}

function panel_getObj_byId($panelId)
{
    global $panelHash;
    global $panelsDB;

    if (!isset($panelHash[$panelId])) {
        return null;
    }

    return $panelsDB[$panelHash[$panelId]];
}

function panel_putObj($row, $panel)
{
    die("panel_putObj disabled: panel geometry is static");
}


function panel_delObj($row)
{
    die("panel_delObj disabled: panel geometry is static");
}

?>

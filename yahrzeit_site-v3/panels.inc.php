<?php

/*
 * NAME
 *      panels.inc.php
 *
 * DESCRIPTION
 *      The Yahrzeit Panels "database":
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


function panel_readDB()
{
    $filename = "/Users/allan/Sites/yahrzeit/data/panels.csv";
    global $num_rows;
    global $panelsDB;
    global $panelHash;

    if ( ( $fp = fopen( $filename, "r" )) === false ) {
        die ("fopen failure");
    }
    $num_rows = 0;
    while ( ( $rawrecord = fgetcsv ($fp, 512, "," ) ) !== false ) {
        $panel = array (
            'panelId'          =>   trim( $rawrecord[0] ),
            'nRows'            =>   trim( $rawrecord[1] ),
            'nCols'            =>   trim( $rawrecord[2] ),
            'nExtra'           =>   trim( $rawrecord[3] ),
            'nNames'           =>   trim( $rawrecord[4] ),
            'connectedTo'      =>   trim( $rawrecord[5] ),
            'firstLedId'       =>   trim( $rawrecord[6] ), 
            'addressingMode'   =>   trim( $rawrecord[7] )
            );
        $panelsDB[$num_rows] = $panel;
        $panelHash[$panel['panelId']] = $num_rows;
        $num_rows++;
    }
    fclose( $fp );
    return $num_rows;
}


function panel_writeDB()
{
    $filename = "/Users/allan/Sites/yahrzeit/data/panels.csv";
    global $panelsDB;

    if ( ( $fp = fopen( $filename, "w" )) === false ) {
        die ("fopen failure");
    }
    foreach ( $panelsDB as $key => $value ) {
        fputcsv($fp, $panelsDB[$key] );
    }
    fclose($fp);
    return true;
}


function panel_numrows()
{
    global $num_rows;

    return ( $num_rows );
}


function panel_getObj( $row )
{
    global $panelsDB;

    return ( $panelsDB[$row] );
}


function panel_getObj_byId( $panelId )
{
    global $panelHash;
    global $panelsDB;
    $row = $panelHash[$panelId];

    return ( $panelsDB[$row] );
}

function panel_putObj( $row, $panel )
{
    global $panelsDB;

    $panelsDB[$row] = $panel;
}


function panel_delObj( $row )
{
    global $panelsDB;

    unset( $panelsDB[$row] );
}

function panel_delObj_byId( $panelId )
{
    global $panelHash;
    global $panelsDB;
    $row = $panelHash[$panelId];

    unset ( $panelsDB[$row] );
}
?>

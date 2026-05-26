<?php

/*
 * NAME
 *      panels.inc.php
 *
 * DESCRIPTION
 *      Panel geometry definitions for the CBS Yahrzeit Wall.
 *
 *      This file defines the physical wall layout: panel IDs, panel names,
 *      and the number of LED rows and columns on each panel.  The rest of
 *      the application uses this information to validate memorial locations,
 *      draw panel views, and translate a person's stored location into a
 *      controller command.
 *
 *      The original application treated panel data much like the memorial
 *      name database, with a small procedural object-store API:
 *
 *          panel_readDB()
 *          panel_numrows()
 *          panel_getObj($i)
 *          panel_getObj_byId($panelId)
 *
 *      In modern terms, this file combines the roles of a Panel record class
 *      and a PanelGeometry repository.  The panel_ prefix acts as a small
 *      module namespace.
 *
 * NOTES
 *      Panel geometry is now static application data, not an editable CSV
 *      file.  The old panel add/modify/delete workflow was removed because
 *      the physical wall layout is fixed.
 *
 *      Valid panel IDs are part of the memorial database contract.  The
 *      audit report uses this file to detect invalid panel IDs, out-of-range
 *      row/column values, and duplicate LED locations.
 *
 *      This file should know the wall geometry only.  It should not contain
 *      Yahrzeit date logic, Minhag policy, name-database parsing, or LED
 *      command transmission.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized in 2026 by replacing the old editable panel database with
 *      static geometry for the Arduino V3 controller and yahrzeit_site-v3.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

global $num_rows;
global $panelsDB;
global $panelHash;

// Static CBS wall geometry.
// These panel IDs and dimensions must match the physical wall and the
// locations stored in data/yahrzeits-rev4.csv.
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

// Load the static CBS panel geometry into memory.
// Returns the number of panels defined.
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

// Return the number of panels currently loaded by panel_readDB().
function panel_numrows()
{
    global $num_rows;

    return $num_rows;
}

// Return one panel record by zero-based index.
function panel_getObj($row)
{
    global $panelsDB;

    return isset($panelsDB[$row]) ? $panelsDB[$row] : null;
}

// Return one panel record by panel ID, such as "col3b".
// Returns false if the panel ID is unknown.
function panel_getObj_byId($panelId)
{
    global $panelHash;
    global $panelsDB;

    if (!isset($panelHash[$panelId])) {
        return null;
    }

    return $panelsDB[$panelHash[$panelId]];
}

?>

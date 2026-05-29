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

// Static CBS wall geometry.
// These panel IDs and dimensions must match the physical wall and the
// locations stored in data/yahrzeits-rev4.csv.
const PANEL_STATIC_GEOMETRY = [
    ['panelId' => 'col1a', 'nRows' => 16, 'nCols' => 5, 'nNames' => 80],
    ['panelId' => 'col1b', 'nRows' => 22, 'nCols' => 5, 'nNames' => 110],
    ['panelId' => 'col1c', 'nRows' => 18, 'nCols' => 5, 'nNames' => 90],

    ['panelId' => 'col2a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96],
    ['panelId' => 'col2b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132],
    ['panelId' => 'col2c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108],

    ['panelId' => 'col3a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96],
    ['panelId' => 'col3b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132],
    ['panelId' => 'col3c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108],

    ['panelId' => 'col4a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96],
    ['panelId' => 'col4b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132],
    ['panelId' => 'col4c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108],

    ['panelId' => 'col5a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96],
    ['panelId' => 'col5b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132],
    ['panelId' => 'col5c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108],

    ['panelId' => 'col6a', 'nRows' => 16, 'nCols' => 6, 'nNames' => 96],
    ['panelId' => 'col6b', 'nRows' => 22, 'nCols' => 6, 'nNames' => 132],
    ['panelId' => 'col6c', 'nRows' => 18, 'nCols' => 6, 'nNames' => 108],

    ['panelId' => 'col7a', 'nRows' => 16, 'nCols' => 5, 'nNames' => 80],
    ['panelId' => 'col7b', 'nRows' => 22, 'nCols' => 5, 'nNames' => 110],
    ['panelId' => 'col7c', 'nRows' => 18, 'nCols' => 5, 'nNames' => 90],
];

// Load the static CBS panel geometry into memory.
// Returns the number of panels defined.
function panel_readDB()
{
    // Compatibility shim. Geometry is now static and already loaded.
    return count(PANEL_STATIC_GEOMETRY);
}

function panel_numrows()
{
    return count(PANEL_STATIC_GEOMETRY);
}

function panel_getObj($row)
{
    return PANEL_STATIC_GEOMETRY[$row] ?? null;
}

function panel_getObj_byId($panelId)
{
    foreach (PANEL_STATIC_GEOMETRY as $panel) {
        if ($panel['panelId'] === $panelId) {
            return $panel;
        }
    }

    return null;
}

?>

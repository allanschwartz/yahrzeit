<?php

/*
 * NAME
 *      panels.inc.php
 *
 * DESCRIPTION
 *      Panel geometry definitions for the CBS Yahrzeit Wall.
 *
 *      The static panel IDs and dimensions are used to validate memorial
 *      locations, render panel views, and map locations to controller LEDs.
 *
 * NOTES
 *      These values must remain synchronized with the physical wall and the
 *      locations stored in data/yahrzeits-rev4.csv.
 *
 * HISTORY
 *      Version 1 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net.
 *
 *      Modernized for PHP 8 and the Yahrzeit V3 release in 2026.
 *
 * COPYRIGHT NOTICE
 *      Copyright (c) 2008, 2026, by Allan M. Schwartz.
 *      All rights reserved.
 */

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

/**
 * Return the panel count through the legacy database-loading API.
 *
 * Geometry is now static, so no load operation is required.
 */
function panel_readDB()
{
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

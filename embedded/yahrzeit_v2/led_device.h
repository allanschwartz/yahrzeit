/**
 * @file        led_device.h
 *
 * @brief       Logical pixel addressing, panel geometry, and EEPROM storage.
 *
 *              Declares the wall-level abstraction above the yyz_pixel driver:
 *              one-based pixel and panel addressing, whole-panel operations,
 *              brightness, and framebuffer persistence in EEPROM.
 *
 *              Panel tables include PANEL0 for the whole display. Geometry
 *              selection remains in yahrzeit_v2.h; physical signal generation
 *              remains in the yyz_pixel class.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 2.1 interface extracted in November 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#pragma once

#include <Arduino.h>

enum { PANEL0 = 0 };

enum ResultIds {
    NO_ERROR = 0, ERR_SYNTAX, ERR_MISSING, ERR_ROW,
    ERR_COL, ERR_PANEL, ERR_BRIGHT, ERR_TESTNUM,
};

/**
 * @brief   Clear the complete pixel buffer.
 */
void    led_init( void );

/**
 * @brief   Set or clear a pixel using one-based wall coordinates.
 */
int     led_store_in_array( boolean pixelbit, byte row, byte col );

/**
 * @brief   Read a pixel using one-based wall coordinates.
 */
boolean led_pixel_value( byte row, byte col );

/**
 * @brief   Set or clear a pixel using one-based panel-local coordinates.
 */
int     led_store_in_panel( boolean pixelbit, byte row, byte col, byte panel );

/**
 * @brief   Save the complete pixel buffer to EEPROM.
 */
void    led_savedata( void );

/**
 * @brief   Restore the complete pixel buffer from EEPROM.
 */
void    led_loaddata( void );

/**
 * @brief   Set active-low OE PWM: 0 is full brightness, 255 is off.
 */
int     led_set_intensity( int intensity );

/**
 * @brief   Set or clear all pixels in a panel, or PANEL0 for the whole wall.
 */
int     led_all_on( boolean pixelbit, byte panel );

/**
 * @brief   Panel origins and dimensions; index PANEL0 describes the whole wall.
 */
extern const byte led_row_of_panel[];
extern const byte led_col_of_panel[];
extern const byte nrows_perpanel[];
extern const byte ncols_perpanel[];

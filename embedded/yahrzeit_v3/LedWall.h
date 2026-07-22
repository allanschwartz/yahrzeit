/**
 * @file        LedWall.h
 *
 * @brief       Logical LED wall abstraction.
 *
 *              LedWall presents the Yahrzeit display as rows, columns, and
 *              panels. It translates those logical coordinates into operations
 *              on the underlying YYZ_PIXEL hardware driver.
 *
 *              Higher-level command code should use this interface rather
 *              than addressing the hardware driver directly. This keeps the
 *              controller command language expressed in wall terms: all,
 *              panel, row, column, pixel, refresh, save, and load.
 *
 *              LedWall owns geometry validation and panel mapping. It
 *              delegates framebuffer storage, EEPROM persistence, brightness,
 *              and physical refresh operations to YyzPixel.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 3.0 revised in April 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#pragma once

#include <Arduino.h>
#include "YyzPixel.h"

enum ResultIds : byte {
    NO_ERROR = 0, ERR_SYNTAX, ERR_MISSING, ERR_ROW,
    ERR_COL, ERR_PANEL, ERR_BRIGHT, ERR_TESTNUM,
};

class LedWall
{
public:
    /**
     * @brief   Construct the logical wall around one YYZ_PIXEL driver.
     */
    explicit LedWall(YyzPixel& pixels);

    /**
     * @brief   Initialize the logical wall with an empty framebuffer.
     */
    void begin();

    /**
     * @brief   Set or clear one pixel in the full logical display.
     */
    ResultIds setPixel(bool pixelBit, byte row, byte col);

    /**
     * @brief   Return one pixel value from the full logical display.
     */
    bool pixelValue(byte row, byte col) const;

    /**
     * @brief   Set or clear one pixel by panel-local coordinates.
     */
    ResultIds setPixelInPanel(bool pixelBit, byte row, byte col, byte panel);

    /**
     * @brief   Return one pixel value by panel-local coordinates.
     */
    bool pixelValueInPanel(byte row, byte col, byte panel) const;

    /**
     * @brief   Save the complete framebuffer to EEPROM.
     */
    void savePixels();

    /**
     * @brief   Load the complete framebuffer from EEPROM.
     */
    void loadPixels();

    /**
     * @brief   Set display brightness.
     */
    void setBrightness(byte brightness);

    /**
     * @brief   Set every framebuffer pixel in the selected panel or wall.
     */
    ResultIds allOn(bool pixelBit, byte panel);

    /**
     * @brief   Return the number of rows in a panel.
     */
    byte rowsInPanel(byte panel) const;

    /**
     * @brief   Return the number of columns in a panel.
     */
    byte colsInPanel(byte panel) const;

    /**
     * @brief   Clear the full logical display.
     */
    void clear();

    /**
     * @brief   Refresh the physical display from the framebuffer.
     */
    void refresh();

private:

    YyzPixel& pixels_;
}; 

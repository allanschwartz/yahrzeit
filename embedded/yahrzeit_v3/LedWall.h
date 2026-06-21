#pragma once

#include <Arduino.h>
#include "YyzPixel.h"

enum ResultIds : byte {
    NO_ERROR = 0, ERR_SYNTAX, ERR_MISSING, ERR_ROW,
    ERR_COL, ERR_PANEL, ERR_BRIGHT, ERR_TESTNUM,
};

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
 *              LedWall owns the logical framebuffer behavior, EEPROM
 *              save/load operations, brightness control, and refresh path.
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

class LedWall
{
public:
    /**
     * @brief   Construct the logical LED wall abstraction
     */
    explicit LedWall(YyzPixel& pixels);

    /**
     * @brief   Initialize the logical LED wall.
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
     * @brief   Save the display framebuffer to EEPROM.
     */
    void savePixels();

    /**
     * @brief   Load the display framebuffer from EEPROM.
     */
    void loadPixels();

    /**
     * @brief   Set display brightness.
     */
    void setBrightness(byte brightness);

    /**
     * @brief   Turn all pixels on or off.
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

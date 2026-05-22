#pragma once

#include <Arduino.h>
#include "YyzPixel.h"

/**
 * @file        LedWall.h
 *
 * @brief       Interface for the logical LED wall abstraction.
 *
 *              LedWall presents the display as rows, columns, and panels.
 *              It maps logical coordinates onto the underlying YYZ_PIXEL
 *              hardware driver.
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
    byte setPixel(bool pixelBit, byte row, byte col);

    /**
     * @brief   Return one pixel value from the full logical display.
     */
    bool pixelValue(byte row, byte col) const;

    /**
     * @brief   Set or clear one pixel by panel-local coordinates.
     */
    byte setPixelInPanel(bool pixelBit, byte row, byte col, byte panel);

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
    byte allOn(bool pixelBit, byte panel);

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

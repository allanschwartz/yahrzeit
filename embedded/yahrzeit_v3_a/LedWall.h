#pragma once

#include <Arduino.h>
#include "YyzPixel.h"

/**
 * @file        LedWall.h
 *
 * @brief       Logical LED display abstraction above the YYZ PIXEL driver.
 *
 *              YyzPixel knows how to shift bits into the physical YYZ PIXEL
 *              board chain. LedWall knows about logical rows, columns,
 *              panels, brightness, save/restore, and display-wide operations.
 */

class LedWall {
public:
    /**
     * Construct the logical LED device.
     *
     * @param pixels          underlying YYZ PIXEL chain driver
     */
    LedWall(YyzPixel& pixels);

    void begin();

    int storeInArray(boolean pixelBit, byte row, byte col);
    boolean pixelValue(byte row, byte col);

    int storeInPanel(boolean pixelBit, byte row, byte col, byte panel);
    boolean pixelValueInPanel(byte row, byte col, byte panel);

    void savePixels();
    void loadPixels();

    int setBrightness(byte brightness);

    int allOn(boolean pixelBit, byte panel);

    void clear();
    void refresh();

    byte rowsInPanel(byte panel) const;
    byte colsInPanel(byte panel) const;

private:
    YyzPixel& pixels_;

    int validatePanel(byte panel) const;
};

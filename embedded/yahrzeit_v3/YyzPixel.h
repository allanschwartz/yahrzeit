/**
 * @file        YyzPixel.h
 *
 * @brief       Device driver for the YYZ_PIXEL shift-register display chain.
 *
 *              This class owns the packed pixel framebuffer and knows how to
 *              shift that framebuffer into a chain of YYZ_PIXEL boards built
 *              from 74HC595-style serial-in / parallel-out registers.
 */

#pragma once

#include <Arduino.h>

class YyzPixel
{
public:

    // ----------------------------------------------------------------------------
    //            C O N S T R U C T O R
    // ----------------------------------------------------------------------------

    /**
     * @brief Construct the YYZ_PIXEL driver.
     */
    explicit YyzPixel(uint8_t dataPin, uint8_t oePin,
                      uint8_t cpPin, uint8_t stPin);


    // ----------------------------------------------------------------------------
    //            P U B L I C   F U N C T I O N S
    // ----------------------------------------------------------------------------

    /**
     * @brief   Initialize hardware interface.
     */
    void begin();

    /**
     * @brief   Set or clear a pixel in the framebuffer.
     */
    void setPixel(uint16_t x, uint16_t y, uint8_t pixel);

    /**
     * @brief   Get a pixel value from the framebuffer.
     */
    bool getPixel(uint16_t x, uint16_t y) const;


    /**
     * @brief   Clear the framebuffer.
     */
    void clear();

    /**
     * @brief   Refresh the physical display from the framebuffer.
     */
    void refresh();

    /**
     * @brief   Toggle logical inversion of pixel output.
     */
    void reverse();

    /**
     * @brief   Check whether pixel output is active-low.
     */
    bool isActiveLow() const;

    /**
     * @brief   Set display brightness.
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief   Enable display output.
     */
    void on();

    /**
     * @brief   Disable display output.
     */
    void off();

    /**
     * @brief   Save framebuffer to EEPROM.
     */
    void savePixels(int eepromOffset);

    /**
     * @brief   Load framebuffer from EEPROM.
     */
    void loadPixels(int eepromOffset);


private:

    // ----------------------------------------------------------------------------
    //            D A T A   M E M B E R S
    // ----------------------------------------------------------------------------

    uint8_t dataPin_;           // Arduino digital output connected to DI / SER.
    uint8_t oePin_;             // Arduino digital output connected to OE, active LOW.
    uint8_t cpPin_;             // Arduino digital output connected to CP / SRCLK.
    uint8_t stPin_;             // Arduino digital output connected to ST / RCLK.

    bool activeLow_ = true;     // true if shifted pixel data is active-low.
    bool displayOn_ = false;    // is the display ON or OFF
};

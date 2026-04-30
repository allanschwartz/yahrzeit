/**
 * @file        YyzPixel.h
 *
 * @brief       The device driver and abstraction of the YYZ PIXEL board
 *              this includes an abstraction for an Array of Pixels,
 *              created from hundreds of YYZ boards chained together.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 3.0 revised in April 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 *
 * @notes       see project notes in file yahrzeit_v3.h
 */

#pragma once
#include <stdint.h>

class YyzPixel {
public:
    /**
     * initialize the class, define the digital ports used
     * @param DATA          (data out) digital port number
     * @param OE            (output enable) digital port number
     * @param CP            (clock) digital port number
     * @param ST            (strobe) digital port number
     */
    YyzPixel(uint8_t DATA, uint8_t OE, uint8_t CP, uint8_t ST);

    /**
     * set the display's display buffer
     * @param width         number of columns
     * @param height        number of rows
     */
    void begin(uint16_t width, uint16_t height);

    /**
     * draw a point
     * @param x     x
     * @param y     y
     * @param pixel 0: led off, >0: led on
     */
    void setPixel(uint16_t x, uint16_t y, uint8_t pixel);

    /**
     * return a point's value
     * @param x     x
     * @param y     y
     * @returns 0: led off, >0: led on
     */
    bool getPixel(uint16_t x, uint16_t y);

    /**
     * draw a rect
     * @param (x1, y1)   top-left position
     * @param (x2, y2)   bottom-right position, not included in the rect
     * @param pixel      0: rect off, >0: rect on
     */
    void drawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t pixel);

    /**
     * Set screen buffer to zero
     */
    void clear(void);

    /**
     * refresh the LEDs with their ON/OFF values
     */
    void refresh(void);

    /**
     * reverse the pixels ... the 1/0 on/off sense is inverted
     */
    void reverse(void);

    /**
     * Is the pixel logic active low, that is inverted (or normal)?
     */
    bool isActiveLow(void);

    /**
     * set the Brightness, via the Pulse Width Modulation intensity of the display: 0..255
     */
    void setBrightness(uint8_t intensity);

    /**
     * turn on the display
     */
    void on(void);

    /**
     * turn off the display
     */
    void off(void);

    /**
     * save the framebuffer, that is all Pixels data, to the EEPROM
     *   in case of power failure, it all comes back to the same state.
     */
    void savePixels(int eepromOffset);

    /**
     * restore the framebuffer, from yesterday's state
     */
    void loadPixels(int eepromOffset);

private:
    // four signal names on the YYZ PIXEL board
                                      // except DATA is DI on the board
                                      // note: CP is SRCP on the 74595 chips
    const uint8_t dataPin_;           // known as DI on the YYZ PIXEL board
    const uint8_t oePin_;             // also known as OE (output enable) active low
    const uint8_t cpPin_;             // also known as CP (clock pulse)
    const uint8_t stPin_;             // also known as ST (store)

    uint16_t width_ = 0;
    uint16_t height_ = 0;
    uint8_t pwmLevel_ = 128;          // default to half bright
    bool activeLow_ = true;           // our logic is inverted
    bool displayOn_ = true;
};

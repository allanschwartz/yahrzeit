/**
 * @file  	YyzPixel.cpp
 *
 * @brief	The device driver and abstraction of the YYZ PIXEL board
 *              this includes an abstraction for an Array of Pixels,
 *              created from hundreds of YYZ boards chained together.
 *
 *              This same device driver works for our fixture, which s
 *              simulates 9 YYZ PIXEL boards
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


#include "Arduino.h"
#include <EEPROM.h>
#include "yahrzeit_v3.h"
#include "YyzPixel.h"
#include "panic.h"

// Display Buffer up to 64 rows and 64 columns, 512 bytes = 64 * 64 / 8
static constexpr size_t kFrameBufferBytes = (NROWS * NCOLS) / 8;
struct FrameBuffer {
    byte pixelBits_[kFrameBufferBytes];
} frameBuffer_ = {};


/**
 * initialize the class, define the digital ports used
 * @param dataPin       D (data out) digital port number
 * @param oePin         OE (output enable) digital port number
 * @param cpPin         CP (clock) digital port number
 * @param stPin         ST (strobe) digital port number
 */
YyzPixel::YyzPixel(uint8_t dataPin, uint8_t oePin, uint8_t cpPin, uint8_t stPin)
    : dataPin_(dataPin),
      oePin_(oePin),
      cpPin_(cpPin),
      stPin_(stPin)
{
}


/**
 * set the display's display buffer
 * @param width         number of columns
 * @param height        number of rows
 */
void YyzPixel::begin(uint16_t width, uint16_t height)
{
    ASSERT(width > 0);
    ASSERT(height > 0);

    width_ = width;
    height_ = height;

    pinMode(dataPin_, OUTPUT);
    pinMode(oePin_, OUTPUT);
    pinMode(cpPin_, OUTPUT);
    pinMode(stPin_, OUTPUT);

    digitalWrite(dataPin_, LOW);
    digitalWrite(oePin_, HIGH);     // OE is active low: HIGH disables LEDs
    digitalWrite(cpPin_, LOW);
    digitalWrite(stPin_, LOW);

    displayOn_ = true;

    // // we use Modulino pixels to indicate status
    // Modulino.begin();
    // statusPixels.begin();
    // statusPixels.clear();
    // statusPixels.show();
}

/**
 * draw a point
 * @param x     x
 * @param y     y
 * @param pixel 0: led off, >0: led on
 */
void YyzPixel::setPixel(uint16_t x, uint16_t y, uint8_t pixel)
{
    ASSERT(width_ > x);
    ASSERT(height_ > y);

    if (x >= width_ || y >= height_) {
        return;
    }

    uint8_t *byte = frameBuffer_.pixelBits_ + x * height_ / 8 + y / 8;
    uint8_t bit = y % 8;
    uint8_t mask = 0x80 >> bit;

    if (pixel) {
        *byte |= mask;
    } else {
        *byte &= ~mask;
    }
}

/**
 * return a point's value
 * @param x     x
 * @param y     y
 * @returns 0: led off, >0: led on
 */
bool YyzPixel::getPixel(uint16_t x, uint16_t y)
{
    ASSERT(width_ > x);
    ASSERT(height_ > y);

    if (x >= width_ || y >= height_) {
        return false;
    }

    const uint8_t *byte = frameBuffer_.pixelBits_ + x * height_ / 8 + y / 8;
    uint8_t bit = y % 8;
    uint8_t mask = 0x80 >> bit;

    return ((*byte & mask) != 0);
}

/**
 * draw a rect
 * @param (x1, y1)   top-left position
 * @param (x2, y2)   bottom-right position, included in the rect
 * @param pixel      0: rect off, >0: rect on
 */
void YyzPixel::drawRect(uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2,
                        uint8_t pixel)
{
    for (uint16_t x = x1; x < x2; ++x) {
        for (uint16_t y = y1; y < y2; ++y) {
            setPixel(x, y, pixel);
        }
    }
}

/**
 * Set screen buffer to zero
 */
void YyzPixel::clear()
{
    memset(frameBuffer_.pixelBits_, 0, sizeof frameBuffer_.pixelBits_);
}

/**
 * refresh the LEDs with their ON/OFF values
 */
void YyzPixel::refresh()
{
    if (!displayOn_) {
        return;
    }

    digitalWrite(stPin_, LOW);
    digitalWrite(cpPin_, LOW);
    digitalWrite(oePin_, HIGH);     // Disable LEDs while shifting

    for (uint16_t col = width_; col > 0; --col) {
        for (uint16_t row = height_; row > 0; --row) {
            bool pixel = getPixel(col - 1, row - 1);

            digitalWrite(dataPin_, pixel ^ activeLow_);

            digitalWrite(cpPin_, HIGH);     //   on rising edge...  ____
            delayMicroseconds(1);           // 1 u-sec pulse    ___/    \___
            digitalWrite(cpPin_, LOW);
        }
        delayMicroseconds(20);
    }

    // latch data by pulsing "ST"

    digitalWrite(stPin_, HIGH);          //   on rising edge...   ____
    delayMicroseconds(10);               // 10 u-sec pulse    ___/    \___
    digitalWrite(stPin_, LOW);           // Shift-register data is STored in the storage register
    // OE is active low.  Lower pwmLevel_ should mean brighter.
    analogWrite(oePin_, pwmLevel_);
}

/**
 * reverse the pixels ... the 1/0 on/off sense is inverted
 */
void YyzPixel::reverse()
{
    activeLow_ = !activeLow_;
}

/**
 * Are we inverted (or normal)?
 */
bool YyzPixel::isActiveLow()
{
    return activeLow_;
}

/**
 * set the Brightness, via the Pulse Width Modulation blanking of the display: 0..255
 * brightness is 1..254.    1 is full bright, 254 is full dim.
 */
void YyzPixel::setBrightness(uint8_t brightness)
{
    pwmLevel_ = brightness;
    analogWrite(oePin_, pwmLevel_);
}

/**
 * turn on the display
 */
void YyzPixel::on()
{
    displayOn_ = true;
    digitalWrite(oePin_, LOW);     // OE active low: LOW enables LEDs
}

/**
 * turn off the display
 */
void YyzPixel::off()
{
    displayOn_ = false;
    digitalWrite(oePin_, HIGH);     // OE active low: HIGH disables LEDs
}

/**
 * save the framebuffer, that is all Pixels data, to the EEPROM
 *   in case of power failure, it all comes back to the same state.
 */
void YyzPixel::savePixels(int eepromOffset)
{
    EEPROM.put(eepromOffset, frameBuffer_);
}

/**
 * restore the framebuffer, from yesterday's state
 */
void YyzPixel::loadPixels(int eepromOffset)
{
    EEPROM.get(eepromOffset, frameBuffer_);
}

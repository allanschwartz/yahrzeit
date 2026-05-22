/**
 * @file        YyzPixel.cpp
 *
 * @brief       Device driver for the YYZ_PIXEL shift-register display chain.
 *
 *              This class owns the packed pixel framebuffer and knows how to
 *              shift that framebuffer into a chain of YYZ_PIXEL boards built
 *              from 74HC595-style serial-in / parallel-out registers.
 *
 *              Higher-level row/column/panel mapping belongs in LedWall.
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

#include "yahrzeit_v3.h"

// Display buffer capacity: up to 64 rows x 64 columns.
// Actual active geometry is displayConfig.nRows x displayConfig.nCols.
static constexpr size_t kFrameBufferBytes = 
    (static_cast<size_t>(MAXNROWS * MAXNCOLS) / 8);

struct FrameBuffer {
    byte pixelBits_[kFrameBufferBytes] {};
};

static FrameBuffer frameBuffer_;


// ----------------------------------------------------------------------------
//            C O N S T R U C T O R
// ----------------------------------------------------------------------------

/**
 * @brief Construct the YYZ_PIXEL hardware driver.
 *
 * @param dataPin      Arduino digital output connected to DI / SER.
 * @param oePin        Arduino digital output connected to OE, active LOW.
 * @param cpPin        Arduino digital output connected to CP / SRCLK.
 * @param stPin        Arduino digital output connected to ST / RCLK.
 *
 * @note The constructor records pin assignments only.  Pin modes and initial
 *       signal levels are established by begin().
 */
YyzPixel::YyzPixel(uint8_t dataPin, uint8_t oePin, uint8_t cpPin, uint8_t stPin)
    : dataPin_(dataPin),
      oePin_(oePin),
      cpPin_(cpPin),
      stPin_(stPin)
{
}


// ----------------------------------------------------------------------------
//            P U B L I C   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * @brief Initialize the YYZ_PIXEL hardware interface.
 *
 * Sets the four control pins to OUTPUT and leaves the display blanked while
 * the rest of the application initializes.
 *
 * @note The active display geometry comes from displayConfig.  The framebuffer
 *       itself is fixed at the maximum supported size so EEPROM layout remains
 *       stable across different test-fixture geometries.
 */
void YyzPixel::begin()
{
    pinMode(dataPin_, OUTPUT);
    pinMode(oePin_, OUTPUT);
    pinMode(cpPin_, OUTPUT);
    pinMode(stPin_, OUTPUT);

    digitalWrite(dataPin_, LOW);
    digitalWrite(oePin_, HIGH);     // OE is active low: HIGH disables LEDs
    digitalWrite(cpPin_, LOW);
    digitalWrite(stPin_, LOW);

    displayOn_ = true;
}

/**
 * @brief Set or clear one framebuffer pixel.
 *
 * @param x       Zero-based column index.
 * @param y       Zero-based row index.
 * @param pixel   0 = off, nonzero = on.
 *
 * @note This modifies only the packed framebuffer.  The physical display is
 *       not updated until refresh() is called.
 */
void YyzPixel::setPixel(uint16_t x, uint16_t y, uint8_t pixel)
{
    ASSERT(displayConfig.nCols > x);
    ASSERT(displayConfig.nRows > y);

    if (x >= displayConfig.nCols || y >= displayConfig.nRows) {
        return;
    }

    byte *const bytePtr = frameBuffer_.pixelBits_ + x * displayConfig.nRows / 8 + y / 8;
    const uint8_t bit = y % 8;
    const uint8_t mask = 0x80 >> bit;

    if (pixel) {
        *bytePtr |= mask;
    } else {
        *bytePtr &= ~mask;
    }
}

/**
 * @brief Return one framebuffer pixel value.
 *
 * @param x       Zero-based column index.
 * @param y       Zero-based row index.
 *
 * @returns       true if the pixel is logically on, false otherwise.
 *
 * @note This reads the framebuffer only.  It does not read back physical
 *       hardware state from the YYZ_PIXEL chain.
 */
bool YyzPixel::getPixel(uint16_t x, uint16_t y)  const
{
    ASSERT(displayConfig.nCols > x);
    ASSERT(displayConfig.nRows > y);

    if (x >= displayConfig.nCols || y >= displayConfig.nRows) {
        return false;
    }

    const uint8_t *const bytePtr = frameBuffer_.pixelBits_ + x * displayConfig.nRows / 8 + y / 8;
    const uint8_t bit = y % 8;
    const uint8_t mask = 0x80 >> bit;

    return ((*bytePtr & mask) != 0);
}


/**
 * @brief Clear the entire framebuffer.
 *
 * Sets every stored pixel bit to zero.  The physical display is not updated
 * until refresh() is called.
 */
void YyzPixel::clear()
{
    memset(frameBuffer_.pixelBits_, 0, sizeof frameBuffer_.pixelBits_);
}

/**
 * @brief Shift the framebuffer into the YYZ_PIXEL chain and update outputs.
 *
 * The framebuffer is shifted through the 74HC595 chain using:
 *
 *      DI      serial data
 *      CP      shift-register clock
 *      ST      storage-register latch clock
 *      OE      active-LOW output enable
 *
 * OE is held disabled while the serial data is shifted, then ST is pulsed to
 * copy the shift-register contents into the 74HC595 storage registers.  Finally
 * OE is restored to its configured PWM brightness value.
 *
 * @note 74HC595 timing requirements are in the tens of ns.  The µs-scale
 *       delays here are intentionally generous for robustness and visibility
 *       on a logic analyzer.
 */
void YyzPixel::refresh()
{
    if (!displayOn_) {
        return;
    }

    digitalWrite(stPin_, LOW);
    digitalWrite(cpPin_, LOW);
    digitalWrite(oePin_, HIGH);     // Disable LEDs while shifting

    for (byte col = displayConfig.nCols; col > 0; --col) {
        for (byte row = displayConfig.nRows; row > 0; --row) {
            const bool pixel = getPixel(col - 1, row - 1);

            digitalWrite(dataPin_, pixel ^ activeLow_);

            digitalWrite(cpPin_, HIGH);     //   on rising edge...  ____
            delayMicroseconds(1);           // 1 u-sec pulse    ___/    \___
            digitalWrite(cpPin_, LOW);
        }
        // Diagnostic gap: makes column/grouping boundaries visible
        // on a logic analyzer.  Not required by the 74HC595.
        delayMicroseconds(4);
    }

    // Latch data by pulsing ST.
    // ST/RCLK rising edge transfers shift-register contents into
    // the 74HC595 storage register.

    digitalWrite(stPin_, HIGH);          //  on rising edge...   ____
    delayMicroseconds(1);                // 1 u-sec pulse    ___/    \___
    digitalWrite(stPin_, LOW);           // Storage register now drives the outputs

    // OE (Output Enable) is active LOW and driven by PWM.
    // Lower values → less blanking → brighter display
    // Higher values → more blanking → dimmer display
    analogWrite(oePin_, displayConfig.brightness);
}

/**
 * @brief Reverse the logical on/off sense of shifted pixel data.
 *
 * The YYZ_PIXEL hardware is normally active-low at the LED output stage.
 * This flag controls whether logical framebuffer pixels are inverted as they
 * are shifted into the 74HC595 chain.
 */
void YyzPixel::reverse()
{
    activeLow_ = !activeLow_;
}

/**
 * @brief Return whether shifted pixel data is currently inverted.
 *
 * @returns true if logical pixels are shifted as active-low data.
 */
bool YyzPixel::isActiveLow() const
{
    return activeLow_;
}

/**
 * @brief Set display brightness by changing OE PWM blanking.
 *
 * @param brightness   Raw PWM value written to OE, 0..255.
 *
 * OE is active LOW, so this value represents blanking time:
 *
 *      0     fully enabled / brightest
 *      255   fully disabled / off
 *
 * The value is stored in displayConfig and applied immediately.
 */
void YyzPixel::setBrightness(uint8_t brightness)
{
    displayConfig.brightness = brightness;
    analogWrite(oePin_, displayConfig.brightness);
}

/**
 * @brief Enable the display outputs.
 *
 * Sets the display state to on and drives OE LOW.  A later call to refresh()
 * will restore the configured PWM brightness.
 */
void YyzPixel::on()
{
    displayOn_ = true;
    digitalWrite(oePin_, LOW);     // OE active low: LOW enables LEDs
}

/**
 * @brief Disable the display outputs.
 *
 * Sets the display state to off and drives OE HIGH so the LED outputs are
 * blanked regardless of framebuffer contents.
 */
void YyzPixel::off()
{
    displayOn_ = false;
    digitalWrite(oePin_, HIGH);     // OE active low: HIGH disables LEDs
}

/**
 * @brief Save the complete framebuffer to EEPROM.
 *
 * @param eepromOffset   EEPROM byte offset where the framebuffer is stored.
 *
 * @note The full maximum-size framebuffer object is saved, not just the
 *       currently active display geometry.  This keeps EEPROM offsets stable
 *       as test-fixture geometry changes.
 */
void YyzPixel::savePixels(int eepromOffset)
{
    EEPROM.put(eepromOffset, frameBuffer_);
}

/**
 * @brief Load the complete framebuffer from EEPROM.
 *
 * @param eepromOffset   EEPROM byte offset where the framebuffer is stored.
 *
 * @note The caller is responsible for ensuring the EEPROM contents are valid
 *       for the current firmware/configuration.
 */
void YyzPixel::loadPixels(int eepromOffset)
{
    EEPROM.get(eepromOffset, frameBuffer_);
}

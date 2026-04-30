/**
 * @file        LedWall.cpp
 *
 * @brief       LED Wall (or display) class
 *
 *          This is an abstraction layer above the "YyzPixel" class
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

#include <Arduino.h>
#include "yahrzeit_v3.h"
#include "LedWall.h"

// ----------------------------------------------------------------------------
//            F I L E - L O C A L   P A N E L   G E O M E T R Y
// ----------------------------------------------------------------------------

namespace {

/**
 * Starting row of each logical panel in the full LED array.
 *
 * Public panel numbering is:
 *
 *      PANEL0      whole display
 *      1..NPANELS  individual panels
 *
 * Row and column values in this file are 1-based at the public API boundary,
 * because that matches the original Yahrzeit command protocol and field notes.
 */
static constexpr byte ledRowOfPanel[NPANELS + 1] = {
#if CBS_56x40_WALL
    1,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
#endif

#if TEST_3x24_HARNESS
    1, 1
#endif
};

/**
 * Starting column of each logical panel in the full LED array.
 */
static constexpr byte ledColOfPanel[NPANELS + 1] = {
#if CBS_56x40_WALL
    1,
    1, 1, 1,
    6, 6, 6,
    12, 12, 12,
    18, 18, 18,
    24, 24, 24,
    30, 30, 30,
    36, 36, 36,
#endif

#if TEST_3x24_HARNESS
    1, 1
#endif
};

/**
 * Number of rows in each panel.
 */
static constexpr byte nRowsPerPanel[NPANELS + 1] = {
#if CBS_56x40_WALL
    56,     // whole display: 16 + 22 + 18
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
#endif

#if TEST_3x24_HARNESS
    NROWS, NROWS
#endif
};

/**
 * Number of columns in each panel.
 */
static constexpr byte nColsPerPanel[NPANELS + 1] = {
#if CBS_56x40_WALL
    40,     // whole display: 5 + 6 + 6 + 6 + 6 + 6 + 5
    5, 5, 5,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    5, 5, 5,
#endif

#if TEST_3x24_HARNESS
    NCOLS, NCOLS
#endif
};

}   // anonymous namespace

// we might structure the EEPROM into a couple of sections
static constexpr int kEepromDisplayOffset = 0;     // future, there may be other items in EEPROM

// ----------------------------------------------------------------------------
//            C O N S T R U C T O R
// ----------------------------------------------------------------------------

/**
 * Construct the logical LED device.
 *
 * @param pixels          underlying YYZ PIXEL chain driver
 */
LedWall::LedWall(YyzPixel& pixels)
    : pixels_(pixels)
{
}

// ----------------------------------------------------------------------------
//            P U B L I C   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * begin ... initialize the logical LED device
 *
 * This does not own the low-level pins.  Those are owned by YyzPixel.
 * This layer simply clears the logical display state.
 */
void LedWall::begin()
{
    pixels_.clear();
}

/**
 * storeInArray ... store a single LED-pixel of data
 *
 * @param pixelBit  boolean value to store
 * @param row       row address, 1-based
 * @param col       column address, 1-based
 *
 * @returns         NO_ERROR or an error code
 */
int LedWall::storeInArray(boolean pixelBit, byte row, byte col)
{
    if (row < 1 || row > NROWS) {
        return ERR_ROW;
    }
    if (col < 1 || col > NCOLS) {
        return ERR_COL;
    }

    pixels_.setPixel(col - 1, row - 1, pixelBit);
    return NO_ERROR;
}

/**
 * pixelValue ... recall a single LED-pixel of data
 *
 * @param row       row address, 1-based
 * @param col       column address, 1-based
 *
 * @returns         boolean value of pixel data
 */
boolean LedWall::pixelValue(byte row, byte col)
{
    if (row < 1 || row > NROWS) {
        return false;
    }
    if (col < 1 || col > NCOLS) {
        return false;
    }

    return pixels_.getPixel(col - 1, row - 1);
}

/**
 * storeInPanel ... store a single LED pixel bit by panel-local row/col
 *
 * @param pixelBit  boolean value to store
 * @param row       panel-local row address, 1-based
 * @param col       panel-local column address, 1-based
 * @param panel     panel number 0 or [1..NPANELS]
 *
 * @returns         NO_ERROR or an error code
 */
int LedWall::storeInPanel(boolean pixelBit, byte row, byte col, byte panel)
{
    int err = validatePanel(panel);
    if (err != NO_ERROR) {
        return err;
    }

    if (row < 1 || row > nRowsPerPanel[panel]) {
        return ERR_ROW;
    }
    if (col < 1 || col > nColsPerPanel[panel]) {
        return ERR_COL;
    }

    return storeInArray(pixelBit,
                        ledRowOfPanel[panel] + row - 1,
                        ledColOfPanel[panel] + col - 1);
}

/**
 * pixelValueInPanel ... recall a single LED pixel bit by panel-local row/col
 *
 * @param row       panel-local row address, 1-based
 * @param col       panel-local column address, 1-based
 * @param panel     panel number 0 or [1..NPANELS]
 *
 * @returns         pixelBit  boolean value stored
 */
boolean LedWall::pixelValueInPanel(byte row, byte col, byte panel)
{
    if (validatePanel(panel) != NO_ERROR) {
        return false;
    }

    if (row < 1 || row > nRowsPerPanel[panel]) {
        return false;
    }

    if (col < 1 || col > nColsPerPanel[panel]) {
        return false;
    }

    return pixelValue(ledRowOfPanel[panel] + row - 1,
                      ledColOfPanel[panel] + col - 1);
}

/**
 * savePixels ... save the whole display buffer in EEPROM
 */
void LedWall::savePixels()
{
    pixels_.savePixels(kEepromDisplayOffset);
}

/**
 * loadPixels ... restore the whole display buffer from EEPROM
 */
void LedWall::loadPixels()
{
    pixels_.loadPixels(kEepromDisplayOffset);
}


/**
 * setIntensity ... set the intensity of the LED brightness
 *
 * @param intensity  brightness level, 1 through 10.
 *                        1 .. least bright
 *                        10 .. most bright
 *
 * @returns          NO_ERROR or ERR_BRIGHT
 */
int LedWall::setBrightness(byte brightness)
{
    /*
     * YyzPixel::setBrightness() takes a raw PWM value.  Preserve the original
     * user-facing 1..255 brightness convention here, reverse the number
     */

    pixels_.setBrightness(255 - brightness);

    return NO_ERROR;
}

/**
 * allOn ... for a panel, turn all LEDs on/off
 *
 * @param pixelBit  boolean value to store
 * @param panel     panel number 0 or [1..NPANELS]
 *
 * @returns         NO_ERROR or ERR_PANEL
 */
int LedWall::allOn(boolean pixelBit, byte panel)
{
    int err = validatePanel(panel);
    if (err != NO_ERROR) {
        return err;
    }

    if (panel == PANEL0) {
        for (byte col = 1; col <= NCOLS; ++col) {
            for (byte row = 1; row <= NROWS; ++row) {
                storeInArray(pixelBit, row, col);
            }
        }
    } else {
        for (byte col = 1; col <= nColsPerPanel[panel]; ++col) {
            for (byte row = 1; row <= nRowsPerPanel[panel]; ++row) {
                storeInPanel(pixelBit, row, col, panel);
            }
        }
    }

    return NO_ERROR;
}
byte LedWall::rowsInPanel(byte panel) const
{
    if (panel > NPANELS) return 0;
    return nRowsPerPanel[panel];
}

byte LedWall::colsInPanel(byte panel) const
{
    if (panel > NPANELS) return 0;
    return nColsPerPanel[panel];
}
/**
 * clear ... clear the full logical display
 */
void LedWall::clear()
{
    pixels_.clear();
}

/**
 * refresh ... shift the current logical display state into the hardware
 */
void LedWall::refresh()
{
    pixels_.refresh();
}

// ----------------------------------------------------------------------------
//            P R I V A T E   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * validatePanel ... validate a panel number
 *
 * @param panel     panel number 0 or [1..NPANELS]
 *
 * @returns         NO_ERROR or ERR_PANEL
 */
int LedWall::validatePanel(byte panel) const
{
    if (panel > NPANELS) {
        return ERR_PANEL;
    }

    return NO_ERROR;
}

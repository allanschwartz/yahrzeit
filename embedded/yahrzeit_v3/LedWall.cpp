/**
 * @file        LedWall.cpp
 *
 * @brief       Logical LED wall abstraction above the YYZ_PIXEL driver.
 *
 *              LedWall presents the display as rows, columns, and optional
 *              physical panels.  It owns the mapping between panel-local
 *              coordinates and the full logical LED array.
 *
 *              YyzPixel owns the packed framebuffer and the physical
 *              shift-register interface.
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

// ----------------------------------------------------------------------------
//            F I L E - L O C A L   P A N E L   G E O M E T R Y
// ----------------------------------------------------------------------------

namespace {

#if CBS_56x40_WALL
constexpr byte N_PANELS = 21;  // Seven columns of three physical panels.
constexpr byte N_ROWS = 56;
constexpr byte N_COLS = 40;
#endif

#if TEST_FIXTURE
constexpr byte N_PANELS = 2;   // Two 24-row by 3-column fixture boards.
constexpr byte N_ROWS = 24;
constexpr byte N_COLS = 6;
#endif

/**
 * @brief Starting row of each logical panel in the full LED array.
 *
 * Panel numbering is:
 *
 *      PANEL0       whole active display
 *      1..displayConfig.nPanels   individual physical/logical panels
 *
 * Row and column values in LedWall's public API are 1-based.  This preserves
 * the original Yahrzeit command language and makes console/debugging output
 * match human-facing row/column numbers.
 */
static constexpr byte LED_ROW_OF_PANEL[N_PANELS + 1] = {
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

#if TEST_FIXTURE
    1, 
    1, 1, 
#endif
};

/**
 * @brief Starting column of each logical panel in the full LED array.
 */
static constexpr byte LED_COL_OF_PANEL[N_PANELS + 1] = {
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

#if TEST_FIXTURE
    1, 
    1, 4,
#endif
};

/**
 * @brief Number of rows in each panel.
 */
static constexpr byte N_ROWS_PER_PANEL[N_PANELS + 1] = {
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

#if TEST_FIXTURE
    24, 
    24, 24,
#endif
};

/**
 * @brief Number of columns in each panel.
 */
static constexpr byte N_COLS_PER_PANEL[N_PANELS + 1] = {
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

#if TEST_FIXTURE
    6,
    3, 3
#endif
};

/**
 * @brief EEPROM layout.  The display framebuffer is stored first.
 *
 * Future configuration/preferences can be placed at later fixed offsets. 
 */
constexpr int EEPROM_DISPLAY_OFFSET = 0;

}

// ----------------------------------------------------------------------------
//            C O N S T R U C T O R
// ----------------------------------------------------------------------------

/**
 * @brief   Construct the logical LED wall abstraction.
 *
 * @param pixels   underlying YYZ_PIXEL hardware driver
 *
 * @note LedWall does not own the hardware pins or framebuffer.  It delegates
 *       those responsibilities to YyzPixel.
 */
LedWall::LedWall(YyzPixel& pixels)
    : pixels_(pixels)
{
    displayConfig.nRows = N_ROWS;
    displayConfig.nCols = N_COLS;
    displayConfig.nPanels = N_PANELS;
}

// ----------------------------------------------------------------------------
//            P U B L I C   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * @brief   Initialize the logical LED wall.
 *
 * Clears the framebuffer through the underlying YyzPixel driver.
 */
void LedWall::begin()
{
    clear();
}

/**
 * @brief   Set or clear one pixel in the full logical display.
 *
 * @param pixelBit   false = off, true = on
 * @param row        display row, 1-based
 * @param col        display column, 1-based
 *
 * @returns          NO_ERROR, ERR_ROW, or ERR_COL
 *
 * @note This updates the framebuffer only.  The physical display is updated
 *       by calling refresh().
 */
ResultIds LedWall::setPixel(bool pixelBit, byte row, byte col)
{
    if (debugPixel) {
        char outputBuffer[64];
        snprintf( outputBuffer, sizeof outputBuffer, 
                  "sPix bit %d, row %d, col %d",
                  pixelBit,  row,  col);
        Serial.println(outputBuffer);
    }
    if (row < 1 || row > displayConfig.nRows) {
        Serial.println("sPix: ERR_ROW");
        return ERR_ROW;
    }
    if (col < 1 || col > displayConfig.nCols) {
        Serial.println("sPix: ERR_COL");
        return ERR_COL;
    }

    pixels_.setPixel(col - 1, row - 1, pixelBit);
    return NO_ERROR;
}

/**
 * @brief   Return one pixel value from the full logical display.
 *
 * @param row        display row, 1-based
 * @param col        display column, 1-based
 *
 * @returns          true if the pixel is on, false otherwise
 */
bool LedWall::pixelValue(byte row, byte col) const
{
    if (row < 1 || row > displayConfig.nRows) {
        Serial.println("pixVal: ERR_row");
        return false;
    }
    if (col < 1 || col > displayConfig.nCols) {
        Serial.println("pixVal: ERR_col");
        return false;
    }

    return pixels_.getPixel(col - 1, row - 1);
}

/**
 * @brief   Set or clear one pixel by panel-local coordinates.
 *
 * @param pixelBit   false = off, true = on
 * @param row        panel-local row, 1-based
 * @param col        panel-local column, 1-based
 * @param panel      PANEL0 or panel number 1..displayConfig.nPanels
 *
 * @returns          NO_ERROR, ERR_PANEL, ERR_ROW, or ERR_COL
 *
 * @note PANEL0 is accepted, but callers normally use setPixel() for full-wall
 *       coordinates and setPixelInPanel() for individual panels.
 */
ResultIds LedWall::setPixelInPanel(bool pixelBit, byte row, byte col, byte panel)
{
    char outputBuffer[96];

    if (debugPixel) {
        snprintf( outputBuffer, sizeof outputBuffer,
                  "sPixInP bit %d, row %d, col %d panel %d",
                  pixelBit, row, col, panel);
        Serial.println(outputBuffer);
    }

    if (panel > displayConfig.nPanels) {
        snprintf(outputBuffer, sizeof outputBuffer,
                 "setPixelInPanel: bad panel=%d nPanels=%d",
                 panel, displayConfig.nPanels);
        Serial.println(outputBuffer);
        return ERR_PANEL;
    }

    if (panel == PANEL0) {
        return setPixel(pixelBit, row, col);
    }

    if (row < 1 || row > N_ROWS_PER_PANEL[panel]) {
        snprintf(outputBuffer, sizeof outputBuffer,
                 "setPixelInPanel: bad row=%d panel=%d maxRows=%d",
                 row, panel, N_ROWS_PER_PANEL[panel]);
        Serial.println(outputBuffer);
        return ERR_ROW;
    }

    if (col < 1 || col > N_COLS_PER_PANEL[panel]) {
        snprintf(outputBuffer, sizeof outputBuffer,
                 "setPixelInPanel: bad col=%d panel=%d maxCols=%d",
                 col, panel, N_COLS_PER_PANEL[panel]);
        Serial.println(outputBuffer);
        return ERR_COL;
    }

    return setPixel(pixelBit,
                    LED_ROW_OF_PANEL[panel] + row - 1,
                    LED_COL_OF_PANEL[panel] + col - 1);
}

/**
 * @brief   Return one pixel value by panel-local coordinates.
 *
 * @param row        panel-local row, 1-based
 * @param col        panel-local column, 1-based
 * @param panel      PANEL0 or panel number 1..displayConfig.nPanels
 *
 * @returns          true if the pixel is on, false otherwise
 */
bool LedWall::pixelValueInPanel(byte row, byte col, byte panel) const
{
    char outputBuffer[96];

    if (panel > displayConfig.nPanels) {
        snprintf(outputBuffer, sizeof outputBuffer,
                 "pixelValueInPanel: bad panel=%d nPanels=%d",
                 panel, displayConfig.nPanels);
        Serial.println(outputBuffer);
        return false;
    }

    if (panel == PANEL0) {
        return pixelValue(row, col);
    }

    if (row < 1 || row > N_ROWS_PER_PANEL[panel]) {
        snprintf(outputBuffer, sizeof outputBuffer,
                 "pixelValueInPanel: bad row=%d panel=%d maxRows=%d",
                 row, panel, N_ROWS_PER_PANEL[panel]);
        Serial.println(outputBuffer);
        return false;
    }
    if (col < 1 || col > N_COLS_PER_PANEL[panel]) {
        snprintf(outputBuffer, sizeof outputBuffer,
                 "pixelValueInPanel: bad col=%d panel=%d maxCols=%d",
                 col, panel, N_COLS_PER_PANEL[panel]);
        Serial.println(outputBuffer);
        return false;
    }

    return pixelValue(LED_ROW_OF_PANEL[panel] + row - 1,
                      LED_COL_OF_PANEL[panel] + col - 1);
}

/**
 * @brief   Save the display framebuffer to EEPROM.
 *
 * Delegates persistence to YyzPixel, which owns the framebuffer layout.
 */
void LedWall::savePixels()
{
    pixels_.savePixels(EEPROM_DISPLAY_OFFSET);
}

/**
 * @brief   Load the display framebuffer from EEPROM.
 *
 * Delegates persistence to YyzPixel, which owns the framebuffer layout.
 */
void LedWall::loadPixels()
{
    pixels_.loadPixels(EEPROM_DISPLAY_OFFSET);
}

/**
 * @brief   Set display brightness.
 *
 * @param brightness   raw OE PWM value, 0..255
 *
 * OE is active-low in the YYZ_PIXEL hardware:
 *
 *      0     brightest
 *      255   off
 *
 * The value is stored in displayConfig and applied immediately.
 */
void LedWall::setBrightness(byte brightness)
{
    displayConfig.brightness = brightness;
    pixels_.setBrightness(brightness);
}

/**
 * @brief   Turn all pixels on or off.
 *
 * @param pixelBit   false = off, true = on
 * @param panel      PANEL0 for whole display, or panel number 1..displayConfig.nPanels
 *
 * @returns          NO_ERROR or ERR_PANEL
 */
ResultIds LedWall::allOn(bool pixelBit, byte panel)
{
    if (panel > displayConfig.nPanels) {
        return ERR_PANEL;
    }

    if (panel == PANEL0) {
        for ( byte col = 1; col <= displayConfig.nCols; ++col) {
            for ( byte row = 1; row <= displayConfig.nRows; ++row) {
                const ResultIds rc = setPixel(pixelBit, row, col);
                ASSERT(rc == NO_ERROR);
            }
        }
    } else {
        for ( byte col = 1; col <= N_COLS_PER_PANEL[panel]; ++col) {
            for ( byte row = 1; row <= N_ROWS_PER_PANEL[panel]; ++row) {
                const ResultIds rc = setPixelInPanel(pixelBit, row, col, panel);
                ASSERT(rc == NO_ERROR);
            }
        }
    }

    return NO_ERROR;
}

/**
 * @brief   Return the number of rows in a panel.
 *
 * @param panel      PANEL0 or panel number 1..displayConfig.nPanels
 *
 * @returns          row count, or 0 for an invalid panel
 */
byte LedWall::rowsInPanel(byte panel) const
{
    if (panel > displayConfig.nPanels) {
        return 0;
    }
    return N_ROWS_PER_PANEL[panel];
}

/**
 * @brief   Return the number of columns in a panel.
 *
 * @param panel      PANEL0 or panel number 1..displayConfig.nPanels
 *
 * @returns          column count, or 0 for an invalid panel
 */
byte LedWall::colsInPanel(byte panel) const
{
    if (panel > displayConfig.nPanels) {
        return 0;
    }
    return N_COLS_PER_PANEL[panel];
}

/**
 * @brief   Clear the full logical display.
 *
 * Clears the underlying framebuffer.  The physical display is not updated
 * until refresh() is called.
 */
void LedWall::clear()
{
    pixels_.clear();
}

/**
 * @brief   Refresh the physical display from the framebuffer
 */
void LedWall::refresh()
{
    pixels_.refresh();
}

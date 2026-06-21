
/**
 * @file        selftest.ino
 *
 * @brief       Self-test patterns for the Yahrzeit wall display.
 *
 *              These tests exercise the logical LedWall interface and,
 *              indirectly, the YYZ_PIXEL hardware driver underneath it.
 *              The tests are intended for bring-up, manufacturing checks,
 *              wiring verification, and field diagnostics.
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
 *              Note that these selftests overwrite the current framebuffer contents
 */

#include "yahrzeit_v3.h"


// ----------------------------------------------------------------------------
//            P R I V A T E   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * @brief   SELF TEST 1: turn on the four corner pixels.
 *
 * @param panel   PANEL0 for the whole display, or panel number 1..displayConfig.nPanels.
 *
 * @note This is the first bring-up test because it verifies both ends of
 *       the logical addressing range without lighting every pixel.
 */
static void selftest_corners(byte panel)
{
    ASSERT(panel <= displayConfig.nPanels);

    if (panel == PANEL0) {
        // Exercise the full-wall coordinate path.
        ledWall.setPixel(1, 1, 1);
        ledWall.setPixel(1, displayConfig.nRows, 1);
        ledWall.setPixel(1, 1, displayConfig.nCols);
        ledWall.setPixel(1, displayConfig.nRows, displayConfig.nCols);
    } else {
        // Exercise the panel-local coordinate path.
        ledWall.setPixelInPanel(1, 1, 1, panel);
        ledWall.setPixelInPanel(1, ledWall.rowsInPanel(panel), 1, panel);
        ledWall.setPixelInPanel(1, 1, ledWall.colsInPanel(panel), panel);
        ledWall.setPixelInPanel(1, ledWall.rowsInPanel(panel),
                                ledWall.colsInPanel(panel), panel);
    }

    // intentional visible pacing for operator observation
    sleep_ms(true, 500);
}

/**
 * @brief   SELF TEST 2/3 helper: turn all pixels on or off.
 *
 * @param pixelBit   false = off, true = on.
 * @param panel      PANEL0 for the whole display, or panel number 1..displayConfig.nPanels.
 */
static void selftest_all_on(bool pixelBit, byte panel)
{
    ASSERT(panel <= displayConfig.nPanels);

    if (panel == PANEL0) {
        for (byte col = 1; col <= displayConfig.nCols; ++col) {
            for (byte row = 1; row <= displayConfig.nRows; ++row) {
                const ResultIds rc = ledWall.setPixel(pixelBit, row, col);
                ASSERT(rc == NO_ERROR);
            }
        }
    } else {
        for (byte col = 1; col <= ledWall.colsInPanel(panel); ++col) {
            for (byte row = 1; row <= ledWall.rowsInPanel(panel); ++row) {
                const ResultIds rc = ledWall.setPixelInPanel(pixelBit, row, col, panel);
                ASSERT(rc == NO_ERROR);
            }
        }
    }

    // intentional visible pacing for operator observation
    sleep_ms(true, 500);
}

/**
 * @brief   SELF TEST 4: checkerboard pattern.
 *
 * Alternates pixels across rows and columns.  This is more diagnostic than
 * a simple all-on/all-off flash because adjacent pixels are driven to opposite
 * states.
 *
 * @param panel   PANEL0 for the whole display, or panel number 1..displayConfig.nPanels.
 *
 * @note This test is useful for finding row/column swaps, off-by-one mapping
 *       errors, and adjacent-bit coupling or ghosting.
 */
static void selftest_checkerboard(byte panel)
{
    ASSERT(panel <= displayConfig.nPanels);

    const byte nRows = (panel == PANEL0)
                     ? displayConfig.nRows
                     : ledWall.rowsInPanel(panel);

    const byte nCols = (panel == PANEL0)
                     ? displayConfig.nCols
                     : ledWall.colsInPanel(panel);

    for (byte row = 1; row <= nRows; ++row) {
        for (byte col = 1; col <= nCols; ++col) {
            const bool pixel = ((row + col) & 1) ? 1 : 0;

            if (panel == PANEL0) {
                ledWall.setPixel(pixel, row, col);
            } else {
                ledWall.setPixelInPanel(pixel, row, col, panel);
            }
        }
    }

    // intentional visible pacing for operator observation
    sleep_ms(true, 500);
}

/**
 * @brief   SELF TEST 5: marching row pattern.
 *
 * Turns on one row at a time, pauses, then turns that row off before moving
 * to the next row.
 *
 * @param panel   PANEL0 for the whole display, or panel number 1..displayConfig.nPanels.
 */

void selftest_marching_row_in_panel(byte panel)
{
    ASSERT(panel >= 1);
    ASSERT(panel <= displayConfig.nPanels);

    const byte nRows = ledWall.rowsInPanel(panel);
    const byte nCols = ledWall.colsInPanel(panel);

    ASSERT(nRows > 0);
    ASSERT(nCols > 0);

    snprintf(outputBuf, sizeof outputBuf,
             "selftest: marching row panel=%u rows=%u cols=%u",
             panel, nRows, nCols);
    serial_log(outputBuf);

    for (byte row = 1; row <= nRows; ++row) {
        for (byte col = 1; col <= nCols; ++col) {
            const ResultIds rc = ledWall.setPixelInPanel(1, row, col, panel);
            ASSERT(rc == NO_ERROR);
        }

        sleep_ms(true, 300);

        for (byte col = 1; col <= nCols; ++col) {
            const ResultIds rc = ledWall.setPixelInPanel(0, row, col, panel);
            ASSERT(rc == NO_ERROR);
        }

        sleep_ms(true, 1);
    }
}

void    selftest_marching_row_all_panels()
{
    for (byte panel = 1; panel <= displayConfig.nPanels; panel++) {
        snprintf(outputBuf, sizeof outputBuf,
                 "selftest: panel %u/%u",
                 panel, displayConfig.nPanels);
        serial_log(outputBuf);
        selftest_marching_row_in_panel(panel);
        sleep_ms(true, 500);
    }
}

void    selftest_marching_row( byte panel ) {
    selftest_marching_row_all_panels();
}

/**
 * @brief   SELF TEST 6: marching column pattern.
 *
 * Turns on one column at a time, pauses, then turns that column off before
 * moving to the next column.
 *
 * @param panel   PANEL0 for the whole display, or panel number 1..displayConfig.nPanels.
 */
static void selftest_marching_col(byte panel)
{
    ASSERT(panel <= displayConfig.nPanels);

    for (byte col = 1; col <= ledWall.colsInPanel(panel); ++col) {
        for (byte row = 1; row <= ledWall.rowsInPanel(panel); ++row) {
            const ResultIds rc = ledWall.setPixelInPanel(1, row, col, panel);
            ASSERT(rc == NO_ERROR);
        }

        sleep_ms(true, 300);

        for (byte row = 1; row <= ledWall.rowsInPanel(panel); ++row) {
            const ResultIds rc = ledWall.setPixelInPanel(0, row, col, panel);
            ASSERT(rc == NO_ERROR);
        }

        sleep_ms(true, 1);
    }

    // intentional visible pacing for operator observation
    sleep_ms(true, 500);
}


/**
 * @brief   Print a one-line description of the selected self-test.
 *
 * @param streamID          output stream, such as SOCKET or CONSOLE.
 * @param testNumber        self-test number.
 * @param testDescription   human-readable test description.
 * @param panel             PANEL0 for whole display, or panel number 1..displayConfig.nPanels.
 */
static void selftest_description(byte streamID,
                                 byte testNumber,
                                 const char *testDescription,
                                 byte panel)
{
    if (panel > PANEL0) {
        snprintf(outputBuf, sizeof outputBuf,
                 "selftest %d - %s panel %d\n",
                 testNumber, testDescription, panel);
    } else {
        snprintf(outputBuf, sizeof outputBuf,
                 "selftest %d - %s\n",
                 testNumber, testDescription);
    }

    my_puts(streamID, outputBuf);
}


// ----------------------------------------------------------------------------
//            P U B L I C   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * @brief   Dispatch one of the LED self-test patterns.
 *
 * The command interface supports:
 *
 *      TEst 1 [panel]   four corner pixels on
 *      TEst 2 [panel]   all pixels on
 *      TEst 3 [panel]   all pixels off
 *      TEst 4 [panel]   checkerboard pattern
 *      TEst 5 [panel]   marching row pattern
 *      TEst 6 [panel]   marching column pattern
 *
 * If panel is omitted by the command layer, PANEL0 is used and the test
 * applies to the entire active display.
 *
 * @param streamID     output stream, such as SOCKET or CONSOLE.
 * @param testNumber   self-test number, 1..7.
 * @param panel        PANEL0 for whole display, or panel number 1..displayConfig.nPanels.
 *
 * @returns            NO_ERROR, ERR_PANEL, or ERR_TESTNUM.
 */
ResultIds selftest(byte streamID, byte testNumber, byte panel)
{
    if (panel > displayConfig.nPanels) {
        return ERR_PANEL;
    }

    switch (testNumber) {
        case 1:
            selftest_description(streamID, testNumber,
                                    "corner pixels on", panel);
            selftest_corners(panel);
            break;

        case 2:
            selftest_description(streamID, testNumber,
                                    "turn pixels ON", panel);
            selftest_all_on(1, panel);
            break;

        case 3:
            selftest_description(streamID, testNumber,
                                    "turn pixels OFF", panel);
            selftest_all_on(0, panel);
            break;

        case 4:
            selftest_description(streamID, testNumber,
                                    "checkerboard pattern", panel);
            selftest_checkerboard(panel);
            break;

        case 5:
            selftest_description(streamID, testNumber,
                                    "marching row pattern", panel);
            selftest_marching_row(panel);
            break;

        case 6:
            selftest_description(streamID, testNumber,
                                    "marching column pattern", panel);
            selftest_marching_col(panel);
            break;

        default:
            return ERR_TESTNUM;
    }

    return NO_ERROR;
}

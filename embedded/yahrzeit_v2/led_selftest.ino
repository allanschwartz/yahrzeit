/**
 * @file        led_selftest.ino
 *
 * @brief       Self-test patterns for the Yahrzeit wall display.
 *
 *              Exercises the YYZ pixel interface for bench testing, wiring
 *              verification, and field diagnostics. Startup uses marching rows
 *              before restoring the saved display data.
 *
 * @note        Tests modify the framebuffer and refresh the visible display.
 *              They do not save patterns to EEPROM. Delays are intentional
 *              so an operator can observe each pattern.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 */

#include "yahrzeit_v2.h"

// ----------------------------------------------------------------------------
//            F U N C T I O N S
// ----------------------------------------------------------------------------

// Dimension tables include PANEL0 at index zero for the whole display.
// Panel-origin tables likewise map PANEL0 to row 1, column 1.


/**
 * @brief   SELF TEST 1: clear the selected area, then light its four corners.
 *
 * @param panel     PANEL0 for the whole display, or panel number 1..NPANELS
 */
static void  selftest_corners( byte panel )
{
    ASSERT (panel <= NPANELS );

    const int rc = led_all_on( 0, panel );
    ASSERT(rc == NO_ERROR);

    if ( panel == PANEL0 ) {
                // bit, row, col
        led_store_in_array( 1, 1, 1 );
        led_store_in_array( 1, NROWS, 1 );
        led_store_in_array( 1, 1, NCOLS );
        led_store_in_array( 1, NROWS, NCOLS );
    }
    else {
                        // bit, row, col, panel
        led_store_in_panel( 1, 1, 1, panel);
        led_store_in_panel( 1, nrows_perpanel[panel], 1, panel );
        led_store_in_panel( 1, 1,  ncols_perpanel[panel], panel );
        led_store_in_panel( 1, nrows_perpanel[panel], ncols_perpanel[panel], panel );
    }
    sleep_ms( true, 500);
}


/**
 * @brief   SELF TEST 2/3 helper: turn the selected area on or off.
 *
 * @param pixelbit    0: off, 1: on
 * @param panel       PANEL0 for the whole display, or panel number 1..NPANELS
 */
static void  selftest_all_on( boolean pixelbit, byte panel )
{
    ASSERT (panel <= NPANELS );

    const byte nRows = nrows_perpanel[panel];
    const byte nCols = ncols_perpanel[panel];

    if ( panel == PANEL0 ) {

        for ( byte col = 1; col <= NCOLS; col++ ) {
            for ( byte row = 1; row <= NROWS; row++ ) {
                const int rc = led_store_in_array( pixelbit, row, col );
                ASSERT(rc == NO_ERROR);
            }
        }
    }
    else {

        for ( byte col = 1; col <= nCols; col++ ) {
            for ( byte row = 1; row <= nRows; row++ ) {
                const int rc = led_store_in_panel( pixelbit, row, col, panel );
                ASSERT(rc == NO_ERROR);
            }
        }
    }
    sleep_ms( true, 500);
}


/**
 * @brief   SELF TEST 4: display a checkerboard, inverting it on each call.
 *
 * @param panel     PANEL0 for the whole display, or panel number 1..NPANELS
 */
static void  selftest_checkerboard( byte panel )
{
    ASSERT (panel <= NPANELS );

    int rc = led_all_on( 0, panel );
    ASSERT(rc == NO_ERROR);

    static boolean invertPattern = false;
    const byte nRows = nrows_perpanel[panel];
    const byte nCols = ncols_perpanel[panel];

    for (byte row = 1; row <= nRows; row++) {
        for (byte col = 1; col <= nCols; col++) {
            const boolean pixel = ((row + col) & 1) ^ invertPattern;
            rc = (panel == PANEL0)
               ? led_store_in_array(pixel, row, col)
               : led_store_in_panel(pixel, row, col, panel);
            ASSERT(rc == NO_ERROR);
        }
    }

    invertPattern = !invertPattern;
    sleep_ms( true, 500 );
}


/**
 * @brief   SELF TEST 5: light and extinguish each row in sequence.
 *
 * @note    Does not clear the area first; other existing pixels remain until
 *          their row is visited.
 *
 * @param panel     PANEL0 for the whole display, or panel number 1..NPANELS
 */
void  selftest_marching_row( byte panel )
{
    ASSERT (panel <= NPANELS );

    const byte nRows = nrows_perpanel[panel];
    const byte nCols = ncols_perpanel[panel];

    for ( byte row = 1; row <= nRows; row++ ) {
        for ( byte col = 1; col <= nCols; col++ ) {
            const int rc = led_store_in_panel( 1, row, col, panel );
            ASSERT(rc == NO_ERROR);
        }
        sleep_ms( true, 300 );

        for ( byte col = 1; col <= nCols; col++ ) {
            const int rc = led_store_in_panel( 0, row, col, panel );
            ASSERT(rc == NO_ERROR);
        }
        sleep_ms( true, 1 );
    }
    sleep_ms( true, 500 );
}


/**
 * @brief   SELF TEST 6: light and extinguish each column in sequence.
 *
 * @note    Does not clear the area first; other existing pixels remain until
 *          their column is visited.
 *
 * @param panel     PANEL0 for the whole display, or panel number 1..NPANELS
 */
static void  selftest_marching_col( byte panel )
{
    ASSERT (panel <= NPANELS );

    const byte nRows = nrows_perpanel[panel];
    const byte nCols = ncols_perpanel[panel];

    for ( byte col = 1; col <= nCols; col++ ) {
        for ( byte row = 1; row <= nRows; row++ ) {
            const int rc = led_store_in_panel( 1, row, col, panel );
            ASSERT(rc == NO_ERROR);
        }
        sleep_ms( true, 300 );

        for ( byte row = 1; row <= nRows; row++ ) {
            const int rc = led_store_in_panel( 0, row, col, panel );
            ASSERT(rc == NO_ERROR);
        }
        sleep_ms( true, 1 );
    }
    sleep_ms( true, 500 );
}


/**
 * @brief   Write a one-line description of the selected self-test.
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param testNumber  selected test number
 * @param testDescription  human-readable pattern description
 * @param panel       PANEL0 for the whole display, or panel number 1..NPANELS
 */
void  selftest_description( byte streamID, byte testNumber, const char * testDescription, byte panel )
{
    if ( panel > PANEL0 ) {
        snprintf(CmdOutput, sizeof CmdOutput,
                 "selftest %d - %s panel %d\n",
                 testNumber, testDescription, panel);
    }
    else {
        snprintf(CmdOutput, sizeof CmdOutput,
                 "selftest %d - %s\n",
                 testNumber, testDescription);
    }
    writeOutput( streamID, CmdOutput);
}


/**
 * @brief   Dispatch one of the six display self-tests.
 *
 * @par Test patterns
 *      We have implemented the following 6 test patterns.
 *
 *      The commands supported are:
 *
 *      TEst 1 [<panel>] --   4 corners ON
 *      TEst 2 [<panel>] --   all pixels ON
 *      TEst 3 [<panel>] --   all pixels OFF
 *      TEst 4 [<panel>] --   checkerboard pattern
 *      TEst 5 [<panel>] --   marching row pattern
 *      TEst 6 [<panel>] --   marching column pattern
 *
 *      <panel> omitted means apply the test to the entire pixel array
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param testNumber  testNumber, see above
 * @param panel       PANEL0 for the whole display, or panel number 1..NPANELS
 * @returns           NO_ERROR, ERR_PANEL, or ERR_TESTNUM
 */
int  selftest( byte streamID, byte testNumber, byte panel )
{
    if (panel > NPANELS ) {
        return ERR_PANEL;
    }
    switch ( testNumber ) {
        case 1:
            selftest_description( streamID, testNumber, "corner LEDs on", panel );
            selftest_corners( panel );
            break;
        case 2:
            selftest_description( streamID, testNumber, "turn pixels ON", panel );
            selftest_all_on( 1, panel );
            break;
        case 3:
            selftest_description( streamID, testNumber, "turn pixels OFF", panel );
            selftest_all_on( 0, panel );
            break;
        case 4:
            selftest_description( streamID, testNumber, "checkerboard pattern", panel );
            selftest_checkerboard( panel );
            break;
        case 5:
            selftest_description( streamID, testNumber, "marching row pattern", panel );
            selftest_marching_row( panel );
            break;
        case 6:
            selftest_description( streamID, testNumber, "marching column pattern", panel );
            selftest_marching_col( panel );
            break;
        default:
        case 0:
            return ERR_TESTNUM;
    }
    return NO_ERROR;
}

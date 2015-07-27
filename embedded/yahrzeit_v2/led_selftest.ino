/**
 * @file        led_selftest.c
 *
 * @brief       Test framework for the Yahrzeit wall project
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

#if 1
#define ASSERT(e)   if (!(e)) { Serial.println(#e); while (1); }
#else
#define ASSERT(e)
#endif

// ----------------------------------------------------------------------------
//            F U N C T I O N S
// ----------------------------------------------------------------------------


/**
 * SELF TEST 1, for a panel ... turn corner LEDs on
 *
 * @param panel     panel number 0 or [1..NPANELS]
 */
static void  selftest_corners( byte panel )
{
    ASSERT (panel <= NPANELS );
    
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
 * SELF TEST 2, 3: for a panel, turn all LEDs on / off
 *
 * @param pixelbit    0: off, 1: on 
 * @param panel       panel number 0 or [1..NPANELS]
 */
static void  selftest_all_on( boolean pixelbit, byte panel )
{
    ASSERT (panel <= NPANELS );
    
    if ( panel == PANEL0 ) {

        for ( byte col = 1; col <= NCOLS; col++ ) {
            for ( byte row = 1; row <= NROWS; row++ ) {
                int rc = led_store_in_array( pixelbit, row, col );
                ASSERT(rc == NO_ERROR);
            }
        }     
        sleep_ms( true, 1);
    }
    else {

        for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
            for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
                int rc = led_store_in_panel( pixelbit, row, col, panel );
                ASSERT(rc == NO_ERROR);
            }
        }
        sleep_ms( true, 1);
    }
    sleep_ms( true, 500);
}


/**
 * SELF TEST 4: all LEDs on, then off
 *
 * @param panel     panel number 0 or [1..NPANELS]
 */
static void  selftest_flashes( byte panel )
{
    ASSERT (panel <= NPANELS );
    
    selftest_all_on( 1, panel );

    selftest_all_on( 0, panel );
}


/**
 * SELF TEST 5: marching row pattern
 *
 * @param panel     panel number 0 or [1..NPANELS]
 */
void  selftest_marching_row( byte panel )
{
    ASSERT (panel <= NPANELS );
    
//    if ( panel == PANEL0 ) {
//        for ( byte row = 1; row <= NROWS; row++ ) {
//            for ( byte col = 1; col <= NCOLS; col++ ) {
//                led_store_in_array( 1, row, col );
//            }
//            sleep_ms( true, 200 );
//
//            for ( byte col = 1; col <= NCOLS; col++ ) {
//                led_store_in_array( 0, row, col );
//            }
//            sleep_ms( true, 10 );
//        }
//    }
//    else {
        for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
            for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
                int rc = led_store_in_panel( 1, row, col, panel );
                ASSERT(rc == NO_ERROR);
            }
            sleep_ms( true, 300 );

            for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
                int rc = led_store_in_panel( 0, row, col, panel );
                ASSERT(rc == NO_ERROR);
            }
            sleep_ms( true, 1 );
        }
//    }
    sleep_ms( true, 500 );
}


/**
 * SELF TEST 6: marching column pattern, repeat 3 times
 *
 * @param panel     panel number 0 or [1..NPANELS]
 */
static void  selftest_marching_col( byte panel )
{
    ASSERT (panel <= NPANELS );
    
//    if ( panel == PANEL0 ) {
//        for ( byte col = 1; col <= NCOLS; col++ ) {
//            for ( byte row = 1; row <= NROWS; row++ ) {
//                led_store_in_array( 1, row, col );
//            }
//            sleep_ms( true, 200 );
//
//            for ( byte row = 1; row <= NROWS; row++ ) {
//                led_store_in_array( 0, row, col );
//            }
//            sleep_ms( true, 10 );
//        }
//    }
//    else {
        for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
            for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
                int rc = led_store_in_panel( 1, row, col, panel );
                ASSERT(rc == NO_ERROR);
            }
            sleep_ms( true, 300 );

            for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
                int rc = led_store_in_panel( 0, row, col, panel );
                ASSERT(rc == NO_ERROR);
            }
            sleep_ms( true, 1 );
        }
//    }
    sleep_ms( true, 500 );
}


/**
 * SELF TEST 7: Cylon pattern
 *
 * @param panel     panel number 0 or [1..NPANELS]
 */
static void  selftest_cylon( byte panel )
{
    ASSERT (panel <= NPANELS );
    
    for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
        for ( byte row = 1; row <= (nrows_perpanel[panel] - 1); row++ ) {
            led_store_in_panel( 1, row, col, panel );
            led_store_in_panel( 1, row+1, col, panel );
            sleep_ms( true, 100 );
            led_store_in_panel( 0, row, col, panel );
            led_store_in_panel( 0, row+1, col, panel );
            sleep_ms( true, 1 );
        }
        for ( byte row = nrows_perpanel[panel]; row > 1; row-- ) {
            led_store_in_panel( 1, row, col, panel );
            led_store_in_panel( 1, row-1, col, panel );
            sleep_ms( true, 100 );
            led_store_in_panel( 0, row, col, panel );
            led_store_in_panel( 0, row-1, col, panel );
            sleep_ms( true, 1 );
        }
        sleep_ms( true, 250 );
    }
    sleep_ms( true, 500 );
}


/**
 * selftest_description -- displays, for example, 
 *                        "selftest 6 - marching column pattern panel 2"
 *                     to the correct stream
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param panel       panel number [1..NPANELS]
 */
void  selftest_description( byte streamID, byte testNumber, const char * testDescription, byte panel )
{   
    if ( panel > PANEL0 ) {
        sprintf( CmdOutput, "selftest %d - %s panel %d\n", testNumber, testDescription, panel );
    } 
    else {
        sprintf( CmdOutput, "selftest %d - %s\n", testNumber, testDescription );
    }
    my_puts( streamID, CmdOutput);
}


/**
 * SELF TEST DISPATCH
 *
 * @TEST PATTERNS
 *      We have implemented the following 7 test patterns.
 *
 *      The commands supported are:
 *
 *      TEst 1 [<panel>] --   4 corners ON
 *      TEst 2 [<panel>] --   all pixels ON
 *      TEst 3 [<panel>] --   all pixels OFF
 *      TEst 4 [<panel>] --   all ON / all OFF
 *      TEst 5 [<panel>] --   marching row pattern
 *      TEst 6 [<panel>] --   marching column pattern
 *      TEst 7 [<panel>] --   Cylon pattern
 *
 *      <panel> omitted means apply the test to the entire pixel array
 *
 * @param streamID    display output on the SOCKET or CONSOLE
 * @param testNumber  testNumber, see above
 * @param panel       which panel [1..NPANELS] (0 means using entire LED arrary)
 * @param repeat           repeat count
 *
 * @returns           n/a
 */
int  selftest( byte streamID, byte testNumber, byte panel, int repeat )
{
    if (panel > NPANELS ) {
        return ERR_PANEL;
    }
    for ( int i = 0; i < repeat; i++ ) {
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
                selftest_description( streamID, testNumber, "all pixels ON then OFF", panel );
                selftest_flashes( panel );
                break;
            case 5:
                selftest_description( streamID, testNumber, "marching row pattern", panel );
                selftest_marching_row( panel );
                break;
            case 6:
                selftest_description( streamID, testNumber, "marching column pattern", panel );
                selftest_marching_col( panel );
                break;
            case 7:
                selftest_description( streamID, testNumber, "Cylon pattern", panel );
                selftest_cylon( panel );
                break;
            default:
            case 0:
                return ERR_TESTNUM;
        }
    }
    return NO_ERROR;
}


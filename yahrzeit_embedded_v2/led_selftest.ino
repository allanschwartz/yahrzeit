#include "led_wall2.h"
    
    
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
 * @param testnumber    testnumber, see above
 * @param panel     which panel [1..NPANELS] (0 means using entire LED arrary)
 * @param k         repeat count
 * @returns         n/a
 */
void  led_selftest( byte testnumber, byte panel, byte k )
{
    for ( ; k; k-- ) {
        switch ( testnumber ) {
            case 1:
                led_selftest_corners( panel);
                break;
            case 2:
                Serial.print( "selftest 2 - turn pixels " );
                Serial.print( "ON" );
                panel_n(panel);
                led_selftest_all_on( 1, panel);
                break;
            case 3:
                Serial.print( "selftest 2 - turn pixels " );
                Serial.print( "OFF" );
                panel_n(panel);
                led_selftest_all_on( 0, panel);
                break;
            case 4:
                led_selftest_flashes( panel );
                break;
            case 5:
                led_selftest_marching_row( panel );
                break;
            case 6:
                led_selftest_marching_col( panel );
                break;
            case 7:
                led_selftest_cylon( panel);
                break;
            case 8:
                led_selftest_corners( panel);
                led_selftest_marching_row( panel );
                led_selftest_marching_col( panel );
                led_selftest_cylon( panel);
                break;
            default:
            case 0:
                Serial.println("eh?");
                break;
        }
    }
    Serial.println("OK");
}


/**
 * display " panel %d\n" without using printf
 *
 * @param panel     panel number [1..NPANELS]
 * @returns         n/a
 */
void  panel_n( byte panel )
{
    if ( panel > 0 ) {
        Serial.print( " panel ");
        Serial.print( panel, DEC );
    }
    Serial.println( "" );
}


/**
 * SELF TEST 1, for a panel ... turn corner LEDs on
 *
 * @param panel     panel number [1..NPANELS]
 * @returns         n/a
 */
void  led_selftest_corners( byte panel )
{
    Serial.print( "selftest 1 - corners LEDs on" );
    panel_n(panel);

    if ( panel == 0 ) {
                // bit, row, col
        led_store1( 1, 1, 1 );
        led_store1( 1, NROWS, 1 );
        led_store1( 1, 1, NCOLS );
        led_store1( 1, NROWS, NCOLS );
    } 
    else {
                        // bit, panel, row, col
        led_store_in_panel( 1, panel, 1, 1 );
        led_store_in_panel( 1, panel, nrows_perpanel[panel], 1 );
        led_store_in_panel( 1, panel, 1,  ncols_perpanel[panel] );
        led_store_in_panel( 1, panel, nrows_perpanel[panel], ncols_perpanel[panel] );
    }
    sleep_ms(1000);
}


/**
 * SELF TEST 2, 3: for a panel, turn all LEDs on / off
 *
 * @param panel     panel number [1..NPANELS]
 * @returns         n/a
 */
void  led_selftest_all_on( boolean pixelbit, byte panel )
{
    //Serial.print( "selftest 2 - turn pixels " );
    //Serial.print( pixelbit ? "ON" : "OFF" );
    //panel_n(panel);
        
    if ( panel == 0 ) {
    
        for ( int col = 1; col <= NCOLS; col++ ) {
            for ( byte row = 1; row <= NROWS; row++ ) {
                led_store1( pixelbit, row, col );
                sleep_ms(1);
            }
        }
    } 
    else {

        for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
            for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
                led_store_in_panel( pixelbit, panel, row, col );
                sleep_ms(1);
            }
        }
    }
    sleep_ms(1000);
}


/**
 * SELF TEST 4: all LEDs on, then off
 *
 * @param panel     panel number [1..NPANELS]
 * @returns         n/a
 */
void  led_selftest_flashes( byte panel )
{
    Serial.print( "selftest 4 - all pixels ON then OFF" );
    panel_n(panel);
    
    led_selftest_all_on( 1, panel );

    led_selftest_all_on( 0, panel );
}


/**
 * SELF TEST 5: marching row pattern
 *
 * @param panel     panel number [1..NPANELS]
 * @returns         n/a
 */
void  led_selftest_marching_row( byte panel )
{
    Serial.print( "selftest 5 - marching row pattern" );
    panel_n(panel);

    if ( panel == 0 ) {
        for ( byte row = 1; row <= NROWS; row++ ) {
            for ( byte col = 1; col <= NCOLS; col++ ) {
                led_store1( 1, row, col );
            }
            sleep_ms( 200 );

            for ( byte col = 1; col <= NCOLS; col++ ) {
                led_store1( 0, row, col );
            }
            sleep_ms( 1 );
        }
    } 
    else {
        for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
            for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
                led_store_in_panel( 1, panel, row, col );
            }
            sleep_ms( 200 );

            for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
                led_store_in_panel( 0, panel, row, col );
            }
            sleep_ms( 1 );
        }
    }
    sleep_ms( 1000 );
}


/**
 * SELF TEST 6: marching column pattern, repeat 3 times
 *
 * @param panel     panel number [1..NPANELS]
 * @returns         n/a
 */
void  led_selftest_marching_col( byte panel )
{
    Serial.print( "selftest 6 - marching column pattern" );
    panel_n(panel);

    if ( panel == 0 ) {
        for ( byte col = 1; col <= NCOLS; col++ ) {
            for ( byte row = 1; row <= NROWS; row++ ) {
                led_store1( 1, row, col );
            }
            sleep_ms( 200 );

            for ( byte row = 1; row <= NROWS; row++ ) {
                led_store1( 0, row, col );
            }
            sleep_ms( 1 );
        }
    } 
    else {
        for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
            for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
                led_store_in_panel( 1, panel, row, col );
            }
            sleep_ms( 200 );

            for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
                led_store_in_panel( 0, panel, row, col );
            }
            sleep_ms( 1 );
        }
    }
    sleep_ms( 1000 );

}


/**
 * SELF TEST 7: Cylon pattern
 *
 * @param panel     panel number [1..NPANELS]
 * @returns         n/a
 */
void  led_selftest_cylon( byte panel )
{
    Serial.print( "selftest 7 - Cylon pattern" );
    panel_n(panel);

    for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
        for ( byte row = 1; row <= (nrows_perpanel[panel] - 1); row++ ) {
            led_store_in_panel( 1, panel, row, col );
            led_store_in_panel( 1, panel, row+1, col );
            sleep_ms( 50 );
            led_store_in_panel( 0, panel, row, col );
            led_store_in_panel( 0, panel, row+1, col );
        }
        for ( byte row = nrows_perpanel[panel]; row > 1; row-- ) {
            led_store_in_panel( 1, panel, row, col );
            led_store_in_panel( 1, panel, row-1, col );
            sleep_ms( 50 );
            led_store_in_panel( 0, panel, row, col );
            led_store_in_panel( 0, panel, row-1, col );
        }
        sleep_ms( 250 );
    }
    sleep_ms( 1000 );
}


/**
 * sleep_ms delays the specified number of milliseconds, however, the display
 * is refreshed periodically.
 *
 * @param ms        number of milliseconds to delay
 * @returns         n/a
 */
void sleep_ms( unsigned int ms )
{
    unsigned long t0 = millis();

    while ( (unsigned int)(millis() - t0) < ms ) {
        delayMicroseconds(100);
        matrix.scan();
    }
}



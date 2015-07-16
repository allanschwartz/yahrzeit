/**
 * @file  	led_device.c
 *
 * @brief	LED Driver device specific code.
 *
 *      This is an abstraction layer above the "YYZ_PIXEL" class,
 *      (or the LEDMatrix class).
 *
 * @history
 *              version 0.4 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 */

#include "yahrzeit_r2.h"


// ----------------------------------------------------------------------------
//            G L O B A L   S T O R A G E
// ----------------------------------------------------------------------------
 

const  byte led_row_of_panel[ NPANELS+1 ] = {
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
  #if TEST_16x64_LED_MATRIX
    1,
    1,9, 1,9, 1,9, 1,9, 1,9
  #endif
  #if TEST_2x24_HARNESS
    1, 1
  #endif
};

const  byte led_col_of_panel[ NPANELS+1 ] = {
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
  #if TEST_16x64_LED_MATRIX
    1,
    1, 1,
    9, 9,
    17, 17,
    25, 25,
    33, 33,
  #endif
  #if TEST_2x24_HARNESS
    1, 1
  #endif
};

const  byte nrows_perpanel[ NPANELS+1 ] = {
  #if CBS_56x40_WALL
    56,     // 16+22+18
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
  #endif
  #if TEST_16x64_LED_MATRIX
    16,
    8,8, 8,8, 8,8, 8,8, 8,8,
  #endif
  #if TEST_2x24_HARNESS
    NROWS, NROWS
  #endif
 };

const  byte ncols_perpanel[ NPANELS+1 ] = {
  #if CBS_56x40_WALL
    40,     // 5+6+6+6+6+6+5
    5, 5, 5,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    5, 5, 5,
  #endif
  #if TEST_16x64_LED_MATRIX
    40,
    8,8, 8,8, 8,8, 8,8, 8,8,
  #endif
  #if TEST_2x24_HARNESS
    NCOLS, NCOLS
  #endif
};


// ----------------------------------------------------------------------------
//            P U B L I C   F U N C T I O N S
// ----------------------------------------------------------------------------

/**
 * led_init ... clear all the pixel data
 */
void  led_init( void )
{
    LedMatrix.clear();
}


/**
 * led_store1 ... store a single LED-pixel of data
 *
 * @param singlebit  boolean value to store
 * @param row       row address
 * @param col       column address
 */
void  led_store1( boolean singlebit, byte row, byte col )
{
    if ( row > NROWS ) {
        Serial.println( "ERR row" );
        return;
    }
    if ( col > NCOLS ) {
        Serial.println( "ERR col" );
        return;
    }

    LedMatrix.drawPoint(col-1, row-1, singlebit);
}


/**
 * led_data1 ... recall a single LED-pixel of data
 *
 * @param row       row address
 * @param col       column address
 *
 * @returns         boolean value of pixel data
 */
boolean  led_data1( byte row, byte col )
{
    if ( row > NROWS ) {
        Serial.println( "ERR row" );
        return 0;
    }
    if ( col > NCOLS ) {
        Serial.println( "ERR col" );
        return 0;
    }
    
    return ( LedMatrix.getPoint(col-1, row-1) );
}


/**
 * led_store_in_panel ... function to store a single LED pixel bit
 *     basic on the row,col
 *
 * @param singlebit  boolean value to store
 * @param panel     panel number, [1..21]
 * @param row       row address
 * @param col       column address
 */
void  led_store_in_panel( boolean singlebit, byte panel, byte row, byte col )
{
    if ( col > NCOLS ) {
        Serial.println( "ERR col" );
        return;
    }
    if ( row > NROWS ) {
        Serial.println( "ERR row" );
        return;
    }
    
    led_store1( singlebit, led_row_of_panel[panel] + row - 1,
                           led_col_of_panel[panel] + col - 1 );
}


/**
 * led_savedata ... save the whole Displaybuf in EEPROM
 */
void  led_savedata( void )
{
    EEPROM.put( 0, Displaybuf );
}


/**
 * led_loaddata ... restore the whole Displaybuf from EEPROM
 */
void led_loaddata( void )
{
    // restore the last pattern stored yesterday
    EEPROM.get( 0, Displaybuf );
}


/**
 * led_set_intensity ... set the intensity of the LED brightness
 *
 * @param intensity  brightness level, 1 through 10.
 *                        1 .. least bright
 *                        10 .. most bright
 */
void  led_set_intensity( byte intensity )
{
    if (intensity < 1 || intensity > 10) {
        Serial.println( "ERR Brightness" );
        return;
    }
    int pwm_n = (10-intensity)*(255/9);
    LedMatrix.set_pwm( pwm_n );
}


/**
 * @file        led_device.ino
 *
 * @brief       LED Driver device specific code.
 *
 *          This is an abstraction layer above the "yyz_pixel" class.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 */

#include "led_device.h"
#include "yahrzeit_v2.h"


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
  #if TEST_2x24_HARNESS
    1, 1
  #endif
  #if TEST_FIXTURE
    1, 1, 1
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
  #if TEST_2x24_HARNESS
    1, 1
  #endif
  #if TEST_FIXTURE
    1, 1, 4
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
  #if TEST_2x24_HARNESS
    NROWS, NROWS
  #endif
   #if TEST_FIXTURE
    24, 24, 24
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
  #if TEST_2x24_HARNESS
    NCOLS, NCOLS
  #endif
  #if TEST_FIXTURE
    6, 3, 3
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
 * led_store_in_array ... store a single LED-pixel of data
 *
 * @param pixelbit  boolean value to store
 * @param row       row address
 * @param col       column address
 *
 * @returns        NO_ERROR or an error code
 */
int  led_store_in_array( boolean pixelbit, byte row, byte col )
{
    if ( row > NROWS ) {
        return ERR_ROW;
    }
    if ( col > NCOLS ) {
        return ERR_COL;
    }

    LedMatrix.drawPoint(col-1, row-1, pixelbit);
    return NO_ERROR;
}


/**
 * led_pixel_value ... recall a single LED-pixel of data
 *
 * @param row       row address
 * @param col       column address
 *
 * @returns         boolean value of pixel data
 */
boolean  led_pixel_value( byte row, byte col )
{
    if ( row > NROWS ) {
        return 0;
    }
    if ( col > NCOLS ) {
        return 0;
    }

    return ( LedMatrix.getPoint(col-1, row-1) );
}


/**
 * led_store_in_panel ... function to store a single LED pixel bit
 *     basic on the row,col
 *
 * @param pixelbit  boolean value to store
 * @param row       row address
 * @param col       column address
 * @param panel     panel number 0 or [1..NPANELS]
 *
 * @returns        NO_ERROR or an error code
 */
int  led_store_in_panel( boolean pixelbit, byte row, byte col, byte panel )
{
    if (panel > NPANELS ) {
        return ERR_PANEL;
    }
    if ( row > NROWS ) {
        return ERR_ROW;
    }
    if ( col > NCOLS ) {
        return ERR_COL;
    }
    return led_store_in_array( pixelbit,
                       led_row_of_panel[panel] + row - 1,
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
 * led_set_intensity ... set the raw active-low OE PWM value
 *
 * @param intensity  raw OE PWM value, 0 through 255.
 *                        0   .. continuously enabled / full brightness
 *                        255 .. continuously disabled / off
 *
 * @returns        NO_ERROR or ERR_BRIGHT
 */
int  led_set_intensity( int intensity )
{
    if (intensity < 0 || intensity > 255) {
        return ERR_BRIGHT;
    }
    LedMatrix.set_pwm( (byte)intensity );
    return NO_ERROR;
}

/**
 * led_all_on    ... for a panel, turn all LEDs on / off
 *
 * @param pixelbit  boolean value to store
 * @param panel     panel number 0 or [1..NPANELS]
 *
 * @returns        NO_ERROR or ERR_PANEL
 */
int  led_all_on( boolean pixelbit, byte panel )
{
    if (panel > NPANELS ) {
        return ERR_PANEL;
    }
    if ( panel == PANEL0 ) {

        for ( byte col = 1; col <= NCOLS; col++ ) {
            for ( byte row = 1; row <= NROWS; row++ ) {
                led_store_in_array( pixelbit, row, col );
            }
        }
    }
    else {

        for ( byte col = 1; col <= ncols_perpanel[panel]; col++ ) {
            for ( byte row = 1; row <= nrows_perpanel[panel]; row++ ) {
                led_store_in_panel( pixelbit, row, col, panel );
            }
        }
    }
    return NO_ERROR;
}

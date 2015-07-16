/**
 * NAME
 *      led_device.c
 *
 * DESCRIPTION
 *      LED Driver device specific code.
 *
 *      This includes code to control the "YPX" (Yahrzeit Pixel Board).
 *      The YPX is also referred to as the light engine
 *
 * NOTES
 *
 * HISTORY
 *      version 0.4 created for Congregation Beth Sholom, 2007-2008
 *      by Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * COPYRIGHT NOTICE
 *      copyright (c) 2008, by Allan M. Schwartz
 *      All rights reserved.
 *
 * BUGS
 *
 *
 * TODO
 *
 *
 * CONTENTS
 *
 *  line    Funtion Declarations
 *  ----    ------------------------------------


 */

#include "led_wall2.h"

/*
 * ----------------------------------------------------------------------------
 *            G L O B A L   S T O R A G E
 * ----------------------------------------------------------------------------
 */
 

const  byte led_row_of_panel[ NPANELS+1 ] = {
  #if CBS
    1,      
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
  #else
    1,
    1,9, 1,9, 1,9, 1,9, 1,9
  #endif
    };

const  byte led_col_of_panel[ NPANELS+1 ] = {
  #if CBS
    1,
    1, 1, 1,
    6, 6, 6,
    12, 12, 12,
    18, 18, 18,
    24, 24, 24,
    30, 30, 30,
    36, 36, 36,
  #else
    1,
    1, 1,
    9, 9,
    17, 17,
    25, 25,
    33, 33,
  #endif
    };

const  byte nrows_perpanel[ NPANELS+1 ] = {
  #if CBS
    56,     // 16+22+18
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
  #else
    16,
    8,8, 8,8, 8,8, 8,8, 8,8,
  #endif
    };

const  byte ncols_perpanel[ NPANELS+1 ] = {
  #if CBS
    40,     // 5+6+6+6+6+6+5
    5, 5, 5,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    5, 5, 5,
  #else
    40,
    8,8, 8,8, 8,8, 8,8, 8,8,
  #endif
    };


/*
 * ----------------------------------------------------------------------------
 *            L E D   S T O R A G E
 * ----------------------------------------------------------------------------
 */

void  led_init(void)
{
    matrix.clear();
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
    matrix.drawPoint(col-1, row-1, singlebit);
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
    return ( matrix.getPoint(col-1, row-1) );
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
    led_store1( singlebit, led_row_of_panel[panel] + row - 1,
                           led_col_of_panel[panel] + col - 1 );
}


void  led_savedata(void)
{
    EEPROM.put( 0, displaybuf );
}

// assume stored by column
byte led_data8( byte row, byte col )
{
    byte val = 0;
    for ( byte i = 0; i < 8; i++ ) {
        val <<= 1;
        val |= led_data1( row+i, col );
    }
    return val;
}


void  led_store8( byte val, byte row, byte col )
{
    for ( byte i = 0; i < 8; i++ ) {
    }
}



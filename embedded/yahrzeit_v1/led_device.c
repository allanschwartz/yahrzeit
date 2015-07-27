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
 *          void  led_store8_offset( uint8_t databits, unsigned int offset )
 *          void  led_store8( uint8_t databits, uint8_t row, uint8_t col )
 *          uint8_t  led_data8( uint8_t row, uint8_t col )
 *          void  led_store1( BIT singlebit, uint8_t row, uint8_t col )
 *          uint8_t  led_data1( uint8_t row, uint8_t col )
 *          void  led_store_in_panel( BIT singlebit, uint8_t panel, uint8_t row, uint8_t col )
 *          void  led_datarefresh()
 *          void  gate_pixels_out ( uint8_t XDATA *pixeldata, unsig...
 *          void  putchar( char c )
 *          void  getchar()
 *          void  sleep_ms( unsigned int n )

 */

#ifdef SDCC
#include "ser_ir.h"
#include "mcs51/at89x52.h"
#define XDATA __xdata
#define CONST  __code const
#define NOP __asm nop __endasm;
#define BIT bit
#define SLEEP_5SEC  sleep_ms( 5000 )
#define SLEEP_1SEC  sleep_ms( 1000 )
#else
#define XDATA
#define CONST  const
#define NOP
#define BIT char
#define SLEEP_5SEC  sleep( 5 )
#define SLEEP_1SEC  sleep( 1 )
#endif  /* SDCC */

#include "ctype.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define uint8_t      uint8_t


// forward define functions ... to help the compiler
    void  led_store8_offset( uint8_t databits, unsigned int offset );
    void  led_store8( uint8_t databits, uint8_t row, uint8_t col);
    uint8_t led_data8( uint8_t row, uint8_t col );
    void  led_store1( BIT singlebit, uint8_t row, uint8_t col);
    uint8_t led_data1( uint8_t row, uint8_t col );
    void  led_store_in_panel( BIT singlebit, uint8_t panel, uint8_t row, uint8_t col );
    void  led_datarefresh( int n );
    void  gate_pixels_out ( uint8_t XDATA *pixeldata, int nbytes );
    void  putchar( char c );
    char  getchar();
    void  sleep_ms( unsigned int n );


/*
 * ----------------------------------------------------------------------------
 *            G L O B A L   S T O R A G E
 * ----------------------------------------------------------------------------
 */

// global variables, these are in the __near/__data or directly addressable internal 8051 registers
extern uint8_t   nrows ;                 // number of rows (56)
extern uint8_t   nrows_div_8;            // number of bytes or pixel boards in a column
extern uint8_t   ncols ;                 // a column is 56 rows of pixel data

extern int      nled_boards ;       // number of LED pixel boards connected to this controller
                                    // each LED pixel board is 8 pixels, therefore 1 byte of data

// big structure ... this must be in xdata, which is the 8052's
// on chip Expeanded Ram (XRAM).   There are 1024 bytes of XRAM on the AT89C51
XDATA  uint8_t led_data[ 64 * 64 / 8];


/*
 * ----------------------------------------------------------------------------
 *            L E D   S T O R A G E
 * ----------------------------------------------------------------------------
 */

/**
 * led_store8_offset ...store 8 LED-pixels of data, given the calculated offset into led_data[]
 *
 * @param databits  raw binary of 8 pixels
 * @param offset    byte offset (not pixel number) into the led_data
 */
void  led_store8_offset( uint8_t databits, uint16_t offset )
{
    led_data[offset] = databits;
}


/**
 * led_store8 ...store 8 LED-pixels of data, given the row, col address of the led_data[]
 *
 * @param databits  raw binary of 8 pixels
 * @param row       row address
 * @param col       column address
 */
void  led_store8( uint8_t databits, uint8_t row, uint8_t col )
{
    uint16_t offset = ( ( col - 1 ) * nrows_div_8 ) + (( row - 1) >> 3 );
    led_data[offset] = databits;
}


/**
 * led_data8 ... recall 8 LED-pixels of data
 *
 * @param row       row address
 * @param col       column address
 *
 * @returns databits  raw binary of 8 pixels
 */
uint8_t  led_data8( uint8_t row, uint8_t col )
{
    uint16_t offset = ( ( col - 1 ) * nrows_div_8 ) + (( row - 1) >> 3 );
    uint8_t databits  = led_data[offset];
    //printf( "in led_data8: databits %02X row %d col %d offset %d\n",
    //                       databits,     row,   col,   offset );
    return ( databits );
}


/**
 * led_store1 ... store a single LED-pixel of data
 *
 * @param singlebit  boolean value to store
 * @param row       row address
 * @param col       column address
 */
void  led_store1( BIT singlebit, uint8_t row, uint8_t col )
{
    uint8_t tmp = led_data8( row, col );
    CONST uint8_t masks[8]= { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
    uint8_t mask = masks[row-1 & 7];

    //printf( "debug led_store1 singlebit %d row %d col %d tmp %02.2X mask %02.2X\n",
    //                          singlebit,   row,   col,   tmp,       mask );
    if ( singlebit ) {
        tmp |= mask;
    } 
    else {
        tmp &= ~mask;
    }
    led_store8( tmp, row, col );
}


/**
 * led_data1 ... recall a single LED-pixel of data
 *
 * @param row       row address
 * @param col       column address
 *
 * @returns         boolean value of pixel data
 */
uint8_t  led_data1( uint8_t row, uint8_t col )
{
    uint8_t tmp = led_data8( row, col );
    CONST uint8_t masks[8]= { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
    uint8_t mask = masks[row-1 & 7];
    return ( (tmp & mask ) ? 1 : 0 );
}


CONST  uint8_t led_row_of_panel[22] = {
    1,      
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    1, 17, 39,
    };

CONST  uint8_t led_col_of_panel[22] = {
    1,
    1, 1, 1,
    6, 6, 6,
    12, 12, 12,
    18, 18, 18,
    24, 24, 24,
    30, 30, 30,
    36, 36, 36,
    };

CONST  uint8_t nrows_perpanel[22] = {
    56,     // 16+22+18
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    16, 22, 18,
    };

CONST  uint8_t ncols_perpanel[22] = {
    40,     // 5+6+6+6+6+6+5
    5, 5, 5,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    6, 6, 6,
    5, 5, 5,
    };


/**
 * led_store_in_panel ... function to store a single LED pixel bit
 *     basic on the row,col
 *
 * @param singlebit  boolean value to store
 * @param panel     panel number, [1..21]
 * @param row       row address
 * @param col       column address
 */
void  led_store_in_panel( BIT singlebit, uint8_t panel, uint8_t row, uint8_t col )
{
    led_store1( singlebit, led_row_of_panel[panel] + row - 1,
                           led_col_of_panel[panel] + col - 1 );
}


/*
 * ----------------------------------------------------------------------------
 *            L E D   D A T A R E F R E S H
 * ----------------------------------------------------------------------------
 */

/**
 * led_datarefresh ... data refresh all the LED pixel data
 *
 * @param n       number of pixel bytes or LED light engines to refresh
 */
void  led_datarefresh( uint16_t n )
{
    if ( n == 0 ) {
        n = nled_boards;
    }

    gate_pixels_out ( &led_data[n-1], /*nbytes*/ n ) ;
}

#if SDCC

// registers we use ... this is just for function gate_pixels_out

#define     DOUT    P1_1        // Data Out      (connected to SER on the 74595)
#define     EN      P1_3        // Output Enable (connected to E  on the 74595)
#define     CLK     P1_5        // (shift register) Clock Pulse   (connected to SCLK on the 74595)
#define     LATCH   P1_7        // (storage register) Clock Pulse (connected to RCLK on the 74595)


/*
 * gate_pixels_out ... function to gate an entire column or chain
 *    of pixel data out of the ports
 *
 * @param pixeldata points to the LAST byte in the chain
 * @param nbytes    is the length of the pixel chain divided by 8
 */
void gate_pixels_out ( uint8 XDATA *pixeldata, uint8 nbytes )
{
    EN = 0;         // output enable latch is active-low
    NOP;
    CLK = 0;
    for ( ; nbytes > 0 ; nbytes--, pixeldata-- ) {

        // retrieve the 8 pixel bits
        uint8 databits = *pixeldata;

        // gate the 8 bits out through the IO lines
        //  high-order bit, first (invert the bits first)
        for ( uint8 k = 0 ; k < 8 ; k++ ) {
            bit pixel;
            pixel = (databits & 0x80) ? 0 : 1;     // take (and invert) the high order bit
            DOUT = pixel;            // serial data out
            CLK = 1;                 // clock shift          ____
            NOP;                     // 2 u-sec pulse    ___/    \___
            CLK = 0;
            databits <<= 1;
        }
    }

    EN = 1;                         // output enable storage register
    LATCH = 1;                      // clock latch          ____
    NOP;                            // 2 u-sec pulse    ___/    \___
    LATCH = 0;
    
}
#endif


#if SDCC

/**
 * putchar(), replaces the libc function, because we are using our
 *      own serial i/o package
 *
 * @param   c       character to output
 */
void  putchar( char c )
{
    ser_putc(c);
    if ( c == '\n' ) ser_putc( '\r' );
}

/**
 * getchar(), replaces the libc function, because we are using our
 *      own serial i/o package
 *
 * @returns     the next character in the received buffer
 */
char  getchar()
{
    return (ser_getc());
}

#endif


/**
 * sleep_ms()
 * wait approximately n milli seconds
 */

void  sleep_ms( uint16_t n )
{
    uint16_t k;

    for ( n; --n; ) {
        for ( k = 320; --k; ) {
            ;
        }
    }
}


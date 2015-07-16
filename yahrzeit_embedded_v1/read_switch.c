/**
 * NAME
 *      read_switch.c
 *
 * SYNOPSIS
 *
 * DESCRIPTION
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
 *
 * CONTENTS
 *
 *  line    Funtion Declarations
 *  ----    ------------------------------------

 */

#ifdef SDCC
#include "ser_ir.h"
#include "mcs51/at89x52.h"
#define XDATA __xdata
#define CONST  __code static const
#define NOP __asm nop __endasm;
#define BIT bit
#else
#define XDATA
#define CONST  static const
#define NOP
#define BIT char
#endif  /* SDCC */

#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"


#define u_int8      unsigned char


// define as extern certain functions from "led_ctrl.c"
extern    void  led_selftest( u_int8 testnumber, u_int8 panel, u_int8 k );
extern    int   enforce_range ( int desiredval, int minlimit, int maxlimit, char *itemname );

// define as extern certain functions from "led_driver.c"
extern    void  led_datarefresh( int n );
extern    void  sleep_ms( unsigned int n );

/*
 * ----------------------------------------------------------------------------
 *            G L O B A L   S T O R A G E
 * ----------------------------------------------------------------------------
 */

// global variables, these are in the __near/__data or directly addressable internal 8051 registers
extern u_int8     nrows ;             // number of rows
extern u_int8     nrows_div_8 ;
#define MAXNROWS     64
extern u_int8     ncols ;             // a column is 56 rows of pixel data
#define MAXNCOLS     64
extern int        nled_boards ;       // number of LED light engines connected to this controller
#define MAXBOARDS   64 * 64 / 8

/*
 * ----------------------------------------------------------------------------
 *            R E A D   S W I T C H
 * ----------------------------------------------------------------------------
 */


/**
 * blink_n_times() ... blink a specified number of times
 */
void blink_n_times( unsigned char n )
{
#define LED    P2_6                // Self-Test LED output

    unsigned char i;

    for ( i = 0; i < n; i++ ) {
        sleep_ms( 200 );
        LED = !LED;
        sleep_ms( 200 );
        LED = !LED;
    }
}


/**
 * read switch, read the DIP switch
 *
 * @NOTE
 *   add documentation here
 */
void  read_switch() 
{ 
    u_int8 sw0, sw = 0, i, j;

    // debounce up to 4 times, taking at most 100ms per debounce
    for ( i = 4; i; i-- ) {
        sw0 = ~P0;
        if ( !sw0 ) return;         // if no button is pressed, return immediately
        for ( j = 5; i; i-- ) {
            sleep_ms(20);           // wait 10ms
            sw = ~P0;
            if ( sw != sw0 ) 
                goto again; // debounce 
                        // if, during a period of 100ms, the switches change, ignore it
        }
        // we fall through this loop, if the switches stayed constant for 100ms
        if ( sw ) goto switch_down;
again:
    }
    return;

switch_down:

    //assert ( sw != 0 );
    if ( sw == 0x40 ) {
        led_datarefresh( 0 );
        blink_n_times(1);
    }
    else if ( sw == 0x01 ||
              sw == 0x02 ||
              sw == 0x03 ||
              sw == 0x04 ||
              sw == 0x05 ||
              sw == 0x06 ||
              sw == 0x07 || 
              sw == 0x08 ) {
        led_selftest( /* testnumber */ sw & 0x0f, /* panelnumber */ 0, 3 );
        // if a test was done!
        blink_n_times(2);
    }
    // ignore all other switch patterns
}



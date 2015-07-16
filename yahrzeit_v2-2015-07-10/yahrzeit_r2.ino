

/**
 * ASSEMBLY
 *      LED Matrix panel has 64x16 pixels.
 *                  connections to Arduino Uno
 
 *      LED Matrix PCB:      LA, LB, LC, LD,    EN, R1, LT, SK
 *      Arduino Digital pin: 3,  4,  5,  6,     2, 11,  8,  13
 */

#include <Arduino.h>
#include <EEPROM.h>
#include "yahrzeit_r2.h"


// we are supporting two different physical interfaces to LED ARRAYS
#if TEST_16x64_LED_MATRIX
#include "LEDMatrix.h"
#endif
#include "yyz_pixel.h"


#define WIDTH  NCOLS        // 2 in our home lab
#define HEIGHT NROWS        // 24 in our home lab


#if TEST_16x64_LED_MATRIX
// constructor for the LedMatrix, an instance of the 'LEDMatrix' class
// LEDMatrix    ( a, b, c, d, oe, r1, stb, clk);
LEDMatrix LedMatrix( 3, 4, 5, 6, 2,  11, 8,   13);
#endif

// constructor for the LedMatrix, an instance of the 'yyz_pixel' class
//yyz_pixel     ( DI, OE, CP, ST )
yyz_pixel LedMatrix(   48, // blue ->  DI,
                       46, // green->  OE, 
                       42, // yellow-> CP
                       44);// orange-> ST


// ----------------------------------------------------------------------------
//            G L O B A L   S T O R A G E
// ----------------------------------------------------------------------------

// Display Buffer 512 bytes = 64 * 64 / 8
byte Displaybuf[MAXDATA] = { 0 };


// ----------------------------------------------------------------------------
//           M A I N
// ----------------------------------------------------------------------------

/**
 * setup ... initializatin code, to run once:
 */
void setup() 
{
    // initialize the LED matrix package
    LedMatrix.begin( Displaybuf, WIDTH, HEIGHT );
    
    // initialize the USB serial port
    Serial.begin( 9600 );
    
    // initialize the console logic
    console_init();
    
    // restore the last pattern stored yesterday
    led_loaddata();
}


/**
 * loop ... main code, to run repeatedly:
 */
void loop() 
{
#if TEST_16x64_LED_MATRIX
    LedMatrix.scan();                     // most important task -- refresh the display
#endif

    console_thread();
}


/**
 * sleep_ms delays the specified number of milliseconds, however, the display
 * is refreshed periodically.
 *
 * @param doRefresh   should we do a refresh, before we wait? 
 * @param ms        number of milliseconds to delay
 */
void sleep_ms( bool doRefresh, unsigned int ms )
{
    unsigned long t0 = millis();

    if ( doRefresh) 
        LedMatrix.refresh();
    while ( (unsigned int)(millis() - t0) < ms ) {
        delayMicroseconds(100);
        
        #if TEST_16x64_LED_MATRIX
        LedMatrix.scan();
        #endif
    }
}


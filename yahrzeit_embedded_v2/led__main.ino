

/**
 * ASSEMBLY
 *      LED Matrix panel has 64x16 pixels.
 *                  connections to Arduino Uno
 *      LED Matrix PCB:      LA, LB, LC, LD,    EN, R1, LT, SK
 *      Arduino Digital pin: 3,  4,  5,  6,     2, 11,  8,  13
 */

#include <Arduino.h>
#include <EEPROM.h>
#include "led_wall2.h"
#include "LEDMatrix.h"

#define WIDTH 64
#define HEIGHT 16

// LEDMatrix    ( a, b, c, d, oe, r1, stb, clk);
LEDMatrix matrix( 3, 4, 5, 6, 2,  11, 8,   13);

// Display Buffer 128 bytes = 64 * 16 / 8
byte displaybuf[WIDTH * HEIGHT / 8] = { 0 };

/*
 * ----------------------------------------------------------------------------
 *            G L O B A L   S T O R A G E
 * ----------------------------------------------------------------------------
 */

// the command from the host
#define MAXCMDLINE 64
char     cmd_buffer[ MAXCMDLINE ];
byte     cmd_pos = 0;            // 0..63

// parsed into arguments
#define MAXARGS 5
char *   arguments[ MAXARGS ];


/*
 * ----------------------------------------------------------------------------
 *            M A I N
 * ----------------------------------------------------------------------------
 */


void setup() {
    // put your setup code here, to run once:
    
    // initialize the LED matrix package
    matrix.begin( displaybuf, WIDTH, HEIGHT );
    
    // initialize the USB serial port
    Serial.begin( 9600 );
    
    // initialize the console logic
    console_init();
    
    // restore the last pattern stored yesterday
    EEPROM.get( 0, displaybuf );
}

void loop() {
    // put your main code here, to run repeatedly:

    matrix.scan();                     // most important task -- refresh the display

    console_thread();
}



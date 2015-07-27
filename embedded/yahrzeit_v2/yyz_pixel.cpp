/**
 * @file  	yyz_pixel.cpp
 *
 * @brief	The device driver and abstraction of the YYZ PIXEL board
 *              this includes an abstraction for an Array of Pixels,
 *              created from hundreds of YYZ boards chained together.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 */

#include "yyz_pixel.h"
#include "Arduino.h"

#if 0
#define ASSERT(e)   if (!(e)) { Serial.println(#e); while (1); }
#else
#define ASSERT(e)
#endif

/**
 * initialize the class, define the digital ports used
 * @param DATA          (data out) digital port number
 * @param OE            (output enable) digital port number
 * @param CP            (clock) digital port number
 * @param ST            (strobe) digital port number
 */
yyz_pixel::yyz_pixel(uint8_t DATA, uint8_t OE, uint8_t CP, uint8_t ST)
{
    this->DATA = DATA;
    this->OE = OE;
    this->CP = CP;
    this->ST = ST;
    mask = true;
    isDisplayOn = false;
    pwm_intensity = 128;
}

/**
 * set the display's display buffer
 * @param displaybuf    display buffer
 * @param width         number of columns
 * @param height        number of rows
 */
void yyz_pixel::begin(uint8_t *displaybuf, uint16_t width, uint16_t height)
{
    ASSERT(displaybuf != NULL);
    ASSERT(width > 0);
    ASSERT(height > 0);

    this->displaybuf = displaybuf;
    this->width = width;
    this->height = height;

    pinMode(DATA, OUTPUT);
    pinMode(OE, OUTPUT);
    pinMode(CP, OUTPUT);
    pinMode(ST, OUTPUT);

    digitalWrite(DATA, LOW);
    digitalWrite(OE, HIGH);              // OutputEnable is active low
    digitalWrite(CP, LOW);
    digitalWrite(ST, LOW);


    isDisplayOn = true;
}

/**
 * draw a point
 * @param x     x
 * @param y     y
 * @param pixel 0: led off, >0: led on
 */
void yyz_pixel::drawPoint(uint16_t x, uint16_t y, uint8_t pixel)
{
    ASSERT(width > x);
    if( x >= width) return;
    ASSERT(height > y);
    if (y >= height) return;

    uint8_t *byte = displaybuf + x * height / 8 + y / 8;
    uint8_t  bit = y % 8;

    if (pixel) {
        *byte |= 0x80 >> bit;
    } else {
        *byte &= ~(0x80 >> bit);
    }
}

/**
 * return a point's value
 * @param x     x
 * @param y     y
 * @returns 0: led off, >0: led on
 */
bool yyz_pixel::getPoint(uint16_t x, uint16_t y)
{
    ASSERT(width > x);
    if( x >= width) return false;
    ASSERT(height > y);
    if (y >= height) return false;

    uint8_t *byte = displaybuf + x * height / 8 + y / 8;
    uint8_t  bit = y % 8;

    uint8_t  val = *byte & (0x80 >> bit );
    return (val ? 1 : 0);
}

/**
 * draw a rect
 * @param (x1, y1)   top-left position
 * @param (x2, y2)   bottom-right position, included in the rect
 * @param pixel      0: rect off, >0: rect on
 */
void yyz_pixel::drawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t pixel)
{
    for (uint16_t x = x1; x <= x2; x++) {
        for (uint16_t y = y1; y <= y2; y++) {
            drawPoint(x, y, pixel);
        }
    }
}

/**
 * draw a image
 * @param (xoffset, yoffset)   top-left offset of image
 * @param (width, height)      image's width and height
 * @param image     contents, 1 bit to 1 led
 */
void yyz_pixel::drawImage(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image)
{
    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            const uint8_t *byte = image + (x + y * width) / 8;
            uint8_t  bit = 7 - x % 8;
            uint8_t  pixel = (*byte >> bit) & 1;

            drawPoint(x + xoffset, y + yoffset, pixel);
        }
    }
}

/**
 * Set screen buffer to zero
 */
void yyz_pixel::clear()
{
    memset( displaybuf, 0, width * height / 8 );
}

/**
 * rreverse the pixels ... the 1/0 on/off sense is inverted
 */
void yyz_pixel::reverse()
{
    mask = !mask;
}

/**
 * Are we inverted (or normal)?
 */
bool yyz_pixel::isReversed()
{
    return mask;
}

/**
 * set the Pulse Width Modulation intensity of the display: 0..255
 */
void yyz_pixel::set_pwm(uint8_t pwm_intensity)
{
    this->pwm_intensity = pwm_intensity;
}

/**
 * refresh the LEDs with their ON/OFF values
 */
void yyz_pixel::refresh()
{
    if (!isDisplayOn) {
        return;
    }

    digitalWrite(ST, LOW);
    digitalWrite(CP, LOW);
    digitalWrite(OE, HIGH);               // OutputEnable is active low

    bool pixel = 0;

    for ( uint8_t col = width; col > 0; col-- ) {

        for (uint8_t row = height; row > 0; row-- ) {

            // gate the bit out through the DI port

            extern bool led_pixel_value( byte row, byte col);
            bool pixel = led_pixel_value( row, col );
            digitalWrite(DATA, pixel ^ mask);
            digitalWrite(CP, HIGH);      //   on rising edge...  ____
            delayMicroseconds(2);        // 2 u-sec pulse    ___/    \___
            digitalWrite(CP, LOW);
        }
        delayMicroseconds(20);
    }

    // latch data by pulsing "ST"

    digitalWrite(ST, HIGH);              //   on rising edge...   ____
    delayMicroseconds(10);               // 10 u-sec pulse    ___/    \___
    digitalWrite(ST, LOW);               // Shift-register data is STored in the storage register
    //digitalWrite(OE, LOW);             // enable OE (active low), turning on the LEDs
    analogWrite(OE, pwm_intensity);
}

/**
 * turn on the display
 */
void yyz_pixel::on()
{
    isDisplayOn = true;
}

/**
 * turn off the display
 */
void yyz_pixel::off()
{
    isDisplayOn = false;
}

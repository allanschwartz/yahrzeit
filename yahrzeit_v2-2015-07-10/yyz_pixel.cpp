/**
 * @file  	yyz_pixel.cpp
 *
 * @brief	The device driver and abstraction of the YYZ PIXEL board
 *              this includes an abstraction for an Array of Pixels,
 *              created from hundreds of YYZ boards chained together.
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

#include "yyz_pixel.h"
#include "Arduino.h"

#if 0
#define ASSERT(e)   if (!(e)) { Serial.println(#e); while (1); }
#else
#define ASSERT(e)
#endif
                    
yyz_pixel::yyz_pixel(uint8_t DATA, uint8_t EN, uint8_t CLK, uint8_t ST)
{
    this->DATA = DATA;
    this->EN = EN;
    this->CLK = CLK;
    this->ST = ST;
    mask = true;
    isDisplayOn = false;
    pwm_intensity = 128;
}

void yyz_pixel::begin(uint8_t *displaybuf, uint16_t width, uint16_t height)
{
    ASSERT(width > 0);
    ASSERT(height > 0);

    this->displaybuf = displaybuf;
    this->width = width;
    this->height = height;

    pinMode(DATA, OUTPUT);
    pinMode(EN, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(ST, OUTPUT);
    
    digitalWrite(DATA, LOW); 
    digitalWrite(EN, HIGH);              // OutputEnable is active low
    digitalWrite(CLK, LOW);
    digitalWrite(ST, LOW);


    isDisplayOn = true;
}

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

void yyz_pixel::drawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t pixel)
{
    for (uint16_t x = x1; x <= x2; x++) {
        for (uint16_t y = y1; y <= y2; y++) {
            drawPoint(x, y, pixel);
        }
    }
}

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

void yyz_pixel::clear()
{
    memset( displaybuf, 0, width * height / 8 );
}

void yyz_pixel::reverse()
{
    mask = !mask;
}

bool yyz_pixel::isReversed()
{
    return mask;
}

void yyz_pixel::set_pwm(uint8_t pwm_intensity)
{
    this->pwm_intensity = pwm_intensity;
}

void yyz_pixel::refresh()
{
    if (!isDisplayOn) {
        return;
    }

    digitalWrite(ST, LOW);
    digitalWrite(CLK, LOW);
    digitalWrite(EN, HIGH);               // OutputEnable is active low
     
    bool pixel = 0;

    for ( uint8_t col = width; col > 0; col-- ) {

        for (uint8_t row = height; row > 0; row-- ) {

            // gate the bit out through the DI port

            extern bool led_data1( byte row, byte col);
            bool pixel = led_data1( row, col );
            digitalWrite(DATA, pixel ^ mask); 
            digitalWrite(CLK, HIGH);     //   on rising edge...  ____
            delayMicroseconds(2);        // 2 u-sec pulse    ___/    \___
            digitalWrite(CLK, LOW);
        }
        delayMicroseconds(20);
    }

    // latch data by pulsing "ST"
 
    digitalWrite(ST, HIGH);              //   on rising edge...   ____
    delayMicroseconds(10);               // 10 u-sec pulse    ___/    \___
    digitalWrite(ST, LOW);               // Shift-register data is STored in the storage register 
    //digitalWrite(EN, LOW);             // enable EN (active low), turning on the LEDs
    analogWrite(EN, pwm_intensity);
}

void yyz_pixel::on()
{
    isDisplayOn = true;
}

void yyz_pixel::off()
{
    isDisplayOn = false;
}

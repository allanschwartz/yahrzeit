/**
 * @file  	yyz_pixel.h
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


#ifndef __YYZ_PIXEL_H__
#define __YYZ_PIXEL_H__

#include <stdint.h>

class yyz_pixel {
public:
    /**
     * initialize the class, define the digital ports used
     * @param DATA          (data out) digital port number
     * @param EN            (output enable) digital port number
     * @param CLK           (clock) digital port number
     * @param STB           (strobe) digital port number
     */
    yyz_pixel(uint8_t DATA, uint8_t EN, uint8_t CLK, uint8_t STB);

    /**
     * set the display's display buffer
     * @param displaybuf    display buffer
     */
    void begin(uint8_t *displaybuf, uint16_t width, uint16_t height);

    /**
     * draw a point
     * @param x     x
     * @param y     y
     * @param pixel 0: led off, >0: led on
     */
    void drawPoint(uint16_t x, uint16_t y, uint8_t pixel);

    /**
     * return a point's value
     * @param x     x
     * @param y     y
     * @returns 0: led off, >0: led on
     */
    bool getPoint(uint16_t x, uint16_t y);

    /**
     * draw a rect
     * @param (x1, y1)   top-left position
     * @param (x2, y2)   bottom-right position, not included in the rect
     * @param pixel      0: rect off, >0: rect on
     */
    void drawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t pixel);

    /**
     * draw a image
     * @param (xoffset, yoffset)   top-left offset of image
     * @param (width, height)      image's width and height
     * @param pixels     contents, 1 bit to 1 led
     */
    void drawImage(uint16_t xoffset, uint16_t yoffset, uint16_t width, uint16_t height, const uint8_t *image);

    /**
     * Set screen buffer to zero
     */
    void clear(void);

    /**
     * refresh the LEDs with their ON/OFF values
     */
    void refresh(void);

    /**
     * rreverse the pixels ... the 1/0 on/off sense is inverted
     */
    void reverse(void);

    /**
     * Are we inverted (or normal)?
     */
    bool isReversed(void);
    
    /**
     * set the Pulse Width Modulation intensity of the display: 0..255
     */
    void set_pwm(uint8_t pwm_intensity);

    /**
     * turn on the display
     */
    void on(void);
    
    /**
     * turn off the display
     */
    void off(void);

private:
    uint8_t  DATA, EN, CLK, ST;        // these are the signal names printed on the YYZ PIXEL board
                                       // except CLK is CP on the board, and SRCLK on the 74595
    uint8_t *displaybuf;
    uint16_t width;
    uint16_t height;
    uint8_t  pwm_intensity;
    bool     mask;
    bool     isDisplayOn;
};

#endif

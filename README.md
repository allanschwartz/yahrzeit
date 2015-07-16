# yahrzeit

/**
 * @brief       Yarhzeit Wall Server and Embedded controller code
 *
 * @history     version 0.4 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in 2015
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008-15, by Allan M. Schwartz
 *              All rights reserved.
 */

/**
 * ARCHITECTURE
 *      Yahrzeit Wall - The LED array representing that many 
 *          souls to be remembered by illuminating the LEDs at
 *          at the appropriate time.
 *      Yahrzeit Appliance - LAMP (or equiv.) machine which presents
 *          a browsable GUI to the the end-user.
 *      Yahrzeit Embedded Controller - The Arduino based microcontroller 
 *          assembly which sits behind the Yahrzeit wall and controls
 *          the LED array.
 *      Yahrzeit Pixel board (YPX) - the YPX board is a small
 *          number of LEDs on a single narrow board.  We have
 *          implemented 6-pixel, 8-pixel and 10-pixel YPX boards.
 *          A string of 7 YPX boards comprise each column,
 *          and 40 columns (a total 280 YPX boards) are connected 
 *          to comprise the entire LED array and the Yahrzeit Wall.
 *          The YPX is sometimes referred to as the "light engine."
 *      Arduino Ethenet Shield -- A communications device
 *          which provides a TCP/IP connection between the
 *          LED Embedded Controller and the Yahrzeit Appliance.
 *          With this device, the Yahrzeit Wall (the Yahrzeit 
 *          Embedded Controller) is reachable with "telnet".
 *          
 * FURTHER DETAILS
 *      Version 1 of the Yahrzeit Embedded Controller was implemented with an 
 *      Atmel 89C51ED2 custom board.
 *
 *      The 89C51ED2 is an 8051-compatible 8bit microprocessor with 32K Flash.
 *      The AT89C51ED2 provides the following standard features: 32K
 *      bytes of Flash, 2KB EEPROM, 256 bytes of SRAM, 1792 bytes of XRAM, 
 *      32 I/O lines, three 16-bit timer/counters, full-duplex serial port, 
 *      on-chip oscillator, and clock circuitry.
 *
 *      Version 2 of the Yahrzeit Embedded Controller is implemented with
 *      a Arduino Mega, using a ATmega2560 chip.  This chip has 54 Digital
 *      I/O pins, 256 KB of Flash Memory, 8KB SRAM, 4 KB EEPROM, and runs
 *      at a 16 MHz clock speed.
 *
 *      However, the most important feature of the Arduino family is the
 *      open-source hardware design, the easy-to-use development enviroment,
 *      and the well supported libraries.
 *
 * CLI
 *      The LED CONTROLLER implements the following console commands,
 *      to facilitate the yahrzeit appliance controlling the LED array.
 *          All on|off [<panel>]
 *          BRightness <n> (1:low, 10:high)
 *          DAta <row> <col> <binary data>
 *          DUmp
 *          HElp
 *          LOad
 *          PIxel on|off <row> <col> [<panel>]
 *          REfresh
 *          SAve
 *          TEst <testnumber> [<panel>]
 */


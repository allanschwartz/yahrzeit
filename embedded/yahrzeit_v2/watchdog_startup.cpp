/**
 * @file        watchdog_startup.cpp
 *
 * @brief       Disable a surviving watchdog before application initialization.
 *
 *              The AVR .init3 hook clears reset flags and disables the watchdog
 *              before data and C++ initialization. It supports panic recovery;
 *              the bootloader runs first and must also handle watchdog resets.
 *
 *              We never call this function explicitly. The linker places it
 *              in the AVR startup sequence, before Arduino reaches setup().
 *              Startup sections run consecutively: execution falls through
 *              this code into the next section, rather than calling it and
 *              returning.
 *
 * @verbatim
 *              Reset -> bootloader -> application startup
 *                                         |
 *                          .init2: basic CPU/stack setup
 *                          .init3: our watchdog cleanup
 *                          .init4: initialize global/static storage
 *                                         |
 *                          C++ constructors -> main() -> setup() -> loop()
 * @endverbatim
 *
 * @history     created in November 2026 for AVR watchdog recovery
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#include <avr/io.h>
#include <avr/wdt.h>

extern "C" void disableWatchdogEarly()
    __attribute__((naked, used, section(".init3")));

/**
 * @brief   Clear reset flags and disable the watchdog during AVR startup.
 *
 * @note    Runs in .init3, before data and C++ initialization. The bootloader
 *          runs first and must also handle watchdog resets correctly.
 */
extern "C" void disableWatchdogEarly()
{
    // MCUSR (MCU Status Register) holds reset-cause flags. Clear them before
    // disabling the watchdog: WDRF (watchdog reset) overrides WDE (enable).
    // This also discards the other reset-cause flags; we do not preserve them.
    MCUSR = 0;
    wdt_disable();
    // Naked .init functions fall through into the next startup section.
}

/**
 * @file    watchdog_startup.cpp
 *
 * @brief   Disable a surviving watchdog before application initialization.
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
    // WDRF overrides WDE, so clear reset flags before disabling the watchdog.
    MCUSR = 0;
    wdt_disable();
    // Naked .init functions fall through into the next startup section.
}

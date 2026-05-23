#pragma once

#include <Arduino.h>

/**
 * @file        selftest.h
 *
 * @brief       Self-test patternss for the Yahrzeit wall display.
 *
 *              These tests exercise the logical LedWall interface and,
 *              indirectly, the YYZ_PIXEL hardware driver underneath it.
 *              The tests are intended for bring-up, manufacturing checks,
 *              wiring verification, and field diagnostics.
 */

/**
 * @brief   Dispatch one of the LED self-test patterns.
 */
int     selftest(byte streamID, byte testNumber, byte panel);

/**
 * @brief   SELF TEST 5: marching row pattern.
 */
void    selftest_marching_row( byte panel );


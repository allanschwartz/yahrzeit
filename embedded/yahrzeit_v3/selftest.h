#pragma once

#include <Arduino.h>
#include "LedWall.h"

/**
 * @file        selftest.h
 *
 * @brief       Operator-triggered LED wall test patterns.
 *
 *              These tests exercise the logical LedWall interface and,
 *              indirectly, the YYZ_PIXEL hardware driver underneath it.
 *
 *              The tests are intended for bring-up, wiring verification,
 *              panel diagnostics, and field troubleshooting. They are invoked
 *              by command, not automatically as part of normal startup.
 *
 *              Test patterns may change the visible wall state. Normal
 *              application code should restore the desired display after
 *              testing.
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

/**
 * @brief   Dispatch one of the LED self-test patterns.
 */
ResultIds selftest(byte streamID, byte testNumber, byte panel);

/**
 * @brief   SELF TEST 5: marching row pattern.
 */
void    selftestMarchingRow( byte panel );

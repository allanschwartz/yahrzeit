/**
 * @file        selftest.h
 *
 * @brief       Operator-triggered LED wall test patterns.
 *
 *              These tests exercise the logical LedWall interface and,
 *              indirectly, the YYZ_PIXEL hardware driver underneath it.
 *
 *              The tests are intended for bring-up, wiring verification,
 *              panel diagnostics, and field troubleshooting. The command
 *              processor exposes all six tests; startup also uses the
 *              marching-row pattern before restoring the saved framebuffer.
 *
 *              Test patterns may change the visible wall state. Normal
 *              application code should restore the desired display after
 *              testing.
 *
 * @history     version 1.0 created for Congregation Beth Sholom, 2007-2008
 *              version 2.0 revised in July 2015
 *              version 3.0 revised in April 2026
 *
 * @author      Allan M. Schwartz, allanschwartz@sbcglobal.net
 *
 * @copyright   copyright (c) 2008,2015,2026, by Allan M. Schwartz
 *              All rights reserved.
 */

#pragma once

#include <Arduino.h>
#include "LedWall.h"

/**
 * @brief   Dispatch one of the LED self-test patterns.
 */
ResultIds selftest(byte streamID, byte testNumber, byte panel);

/**
 * @brief   Run the marching-row pattern across every configured panel.
 *
 * @note The panel argument is retained for the common self-test interface but
 *       is currently ignored.
 */
void    selftestMarchingRow( byte panel );

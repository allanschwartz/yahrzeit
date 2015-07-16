/*
 * ----------------------------------------------------------------------------
 *            S E L F    T E S T
 * ----------------------------------------------------------------------------
 */

// forward define functions ... to help the compiler

void  led_selftest( byte testnumber, byte panel, byte k );
void  led_selftest_corners( byte panel );
void  led_selftest_all_on( boolean singlebit, byte panel );
void  led_selftest_flashes( byte panel );
void  led_selftest_marching_row( byte panel );
void  led_selftest_marching_col( byte panel );
void  led_selftest_cylon( byte panel );

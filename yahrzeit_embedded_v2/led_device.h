// forward define functions ... to help the compiler

void  led_store1( boolean singlebit, byte row, byte col);
boolean  led_data1( byte row, byte col );
void  led_store_in_panel( boolean singlebit, byte panel, byte row, byte col );;
void  led_savedata( void );


extern const  byte led_row_of_panel[];
extern const  byte led_col_of_panel[];
extern const  byte nrows_perpanel[];
extern const  byte ncols_perpanel[];

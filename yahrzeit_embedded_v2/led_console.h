
    void console_init(void);
    void  console_loop(void);
    byte  match_cmd_verb(void);
    void  parse_cmd_buffer(void);
    boolean    onoff_bool( char *s );
    byte  xdigitvalue ( char c );
    byte  xdigits_decoded( char c1, char c2 );
    void  led_data_cmd( int offset, int nbytes, char *hexdata );


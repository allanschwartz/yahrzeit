
#include "led_wall2.h"

/*
 * ----------------------------------------------------------------------------
 *            G L O B A L   S T O R A G E
 * ----------------------------------------------------------------------------
 */



// constant character data is placed in the CODE segment
static const char version_string[] =
    "LED Controller, V2.0, 2015-04-26\n"
    "Copywrite (c) 2008-15, AMS Consulting\n"
    "----------";

static const char help_string[] =
    "All  on|off [<panel>]\n"
    "DAta <byteoffset> <nbytes> <hexified data>\n"
    "DUmp\n"
    "HElp\n"
    "PIxel 0|1 <row> <col> [<panel>]\n"
    "REfresh\n"
    "SAve\n"
    "TEst <testnumber> [<panel>]\n"
    ;


/*
 * ----------------------------------------------------------------------------
 *            C O N S O L E
 * ----------------------------------------------------------------------------
 */

/**
 * MAIN ... execution starts here.
 *
 * @TODO
 *      - [DONE] experiment with other serial port settings
 *      - [DONE] make sure interrupt driven character I/O works
 *      - [  ]   experiment with a watchdog timer
 *      - [DONE] experiment with a microsleep() function
 *      - [DONE] does the XRAM have to be software selected to full size?
 *              (crt does this)
 *      - [  ]  play with LCD display, make that work.
 *
 * @returns n/a
 */

void console_init(void) {
    led_init();

    // splash
    Serial.println( version_string );

    // some initial pattern, on power-up (as a self-test)
    led_selftest_all_on( 0, /* panel */ 0);         // all off
    led_selftest_marching_row( /* panel */ 0 );
    //led_selftest_marching_col( /* panel */ 0 );

    Serial.print("> ");
    memset( cmd_buffer, 0, sizeof cmd_buffer );
    cmd_pos = 0;
}



/*
 * ----------------------------------------------------------------------------
 *            C O N S O L E
 * ----------------------------------------------------------------------------
 */

enum cmd_ids  {
    CMD_ALL     = 1,
    CMD_DATA,
    CMD_DUMP,
    CMD_HELP,
    CMD_PIXEL,
    CMD_REFRESH,
    CMD_SAVE,
    CMD_TEST,
    CMD_NOP,    // this was required on the slower 8051 inplementation
};

const struct cmd_s {
    uint8_t  cmd_identifier;            // see enum cmd_ids above
    uint8_t  cmd_required_args;
    const char * cmd_string;
} commands[] = {
    { CMD_ALL, 1, "all" },
    { CMD_DATA, 3, "data" },
    { CMD_DUMP, 0, "dump" },
    { CMD_HELP, 0, "help" },
    // { CMD_HELP, 0, "?" },        // this works without a table entry
    { CMD_PIXEL, 3, "pixel" },
    { CMD_REFRESH, 0, "refresh" },
    { CMD_SAVE, 0, "save" },
    { CMD_TEST, 0, "test" },
    { CMD_NOP, 0, "nop" },
    { 0, 0, 0 }
};


/**
 * console_loop .. implement the console i/o gets(), puts() loop.
 *      Execution falls into here from main and never returns.
 *
 * @COMMAND LANGUAGE
 *      We have implemented a simple command interpreter that recognizes
 *      several commands (which can be abbrieviated to 2 characters).
 *      The commands supported are:
 *
 *          All on|off [<panel>]
 *          DAta <address> <nbytes> <hexified data>
 *          PIxel on|off <row> <col> [<panel>]
 *          TEst <testnumber> [<panel>]
 *          REfresh
 *          SAve
 *          HElp
 *          NOp
 *          ?
 *
 *
 * @RETURN
 *      no return from console_loop
 */
void  console_thread(void)
{
    if ( console_read() ) {
        console_execute();
        Serial.print("> ");
        memset( cmd_buffer, 0, sizeof cmd_buffer );
        cmd_pos = 0;
    }
    else {
        sleep_ms(2);
    }
}


boolean console_read(void) {
    char c;

    if ( Serial.available() > 0 ) {
        c = Serial.read();
        if (c == '\r') {
            Serial.write(c);
            c = '\n';
        }
        Serial.write(c);               // echoplex
        cmd_buffer[cmd_pos++] = c;
        cmd_buffer[cmd_pos] = '\0';
        if (c == '\n')
            return true;               // NL completes the line
    }
    return false;
}


void console_execute(void) {

    int arg1 = 0, arg2 = 0, arg3 = 0;

    parse_cmd_buffer();         // array DATA.arguments[] assigned pointers to tokens

    // as an code-size optimization, lets convert argv[1] & argv[2] from ascii to an int
    if ( OPTIONAL_ARG(1) ) {
        arg1 = atoi( arguments[1] );
    }
    if ( OPTIONAL_ARG(2) ) {
        arg2 = atoi( arguments[2] );
    }
    if ( OPTIONAL_ARG(3) ) {
        arg3 = atoi( arguments[3] );
    }

    if ( arguments[0] == NULL ) {
        goto not_understood;
    }
    if ( arguments[0][0] == '?' ) {
        goto help_user;
    }


    switch ( match_cmd_verb() ) {

        case CMD_ALL:
            // all on|off [<panel>]
            led_selftest_all_on(
                onoff_bool( arguments[1] ),  /* "on"|"off" or "0"|"1" */
                OPTIONAL_ARG(2) ? arg2 : 0  /* optional <panel> */
            );
            break;

        case CMD_DATA:
            // data <address> <length> <hex-data>
            #if 0
            led_data_cmd( arg1,                 /* address */
                          arg2,                 /* length */
                          arguments[3] );       /* hexified data */
            #endif
            break;
            
        case CMD_DUMP:
            // dump
            led_debug();
            break;

        case CMD_HELP:
            // help or ?
help_user:
            Serial.println( version_string );
            Serial.println( help_string );
            break;     

        case CMD_PIXEL:
            // pixel 0|1 <row> <col> [<panel>]
            if ( OPTIONAL_ARG(4) ) {
                led_store_in_panel(
                    onoff_bool( arguments[1] ),  /* "on"|"off" or "0"|"1" */
                    atoi( arguments[4] ),   /* panel */
                    arg2,                   /* row */
                    arg3 );                 /* column */
            }
            else {
                led_store1(
                    onoff_bool( arguments[1] ),  /* "on"|"off" or "0"|"1" */
                    arg2,                   /* row */
                    arg3 );                 /* column */
            }
            break;

        case CMD_REFRESH:
            // refresh [<nbytes>]
            //led_datarefresh( OPTIONAL_ARG(1) ? arg1 : 0 );
            break;

        case CMD_SAVE:
            // save
            led_savedata();
            Serial.println( "OK" );
            break;

        case CMD_TEST:
            // test <testnumber> [<panel>]
            if ( REQUIRED_ARG(1) ) {
                led_selftest(
                    arg1,                           /* test number */
                    OPTIONAL_ARG(2) ? arg2 : 0 ,    /* optional <panel> */
                    OPTIONAL_ARG(3) ? arg3 : 3 );   /* optional repeat */
            }
            else {
                Serial.println(
                    "TEst 1 [<panel>]     --   4 corners ON\n"
                    "TEst 2 [<panel>]     --   all pixels ON\n"
                    "TEst 3 [<panel>]     --   all pixels OFF\n"
                    "TEst 4 [<panel>] [k] --   all ON / all OFF\n"
                    "TEst 5 [<panel>] [k] --   marching row pattern\n"
                    "TEst 6 [<panel>] [k] --   marching column pattern\n"
                    "TEst 7 [<panel>] [k] --   Cylon pattern\n"
                    "TEst 8 [<panel>] [k] --   lots of tests" );
            }
            break;

        case CMD_NOP:
            break;

        case 0:
        default:
not_understood:
            Serial.println("eh?");
            return;
    }
}


/**
 * match command verb
 *    uses the globals DATA.arguments and the table commands
 *
 * @returns unique command identifier
 */
byte  match_cmd_verb(void)
{
    for ( const struct cmd_s *p = &commands[0]; *p->cmd_string; p++ ) {
        if ( STRMATCH2( arguments[0], p->cmd_string ) ) {
            if ( REQUIRED_ARG(p->cmd_required_args) ) {
                return p->cmd_identifier;
            }
            else {
                Serial.println( "missing arg" );
                return 0;
            }
        }
    }
    return 0;
}


/**
 * parse_cmd_bufer ... function to break apart the DATA.arguments
 *    uses the globals cmd_buffer and DATA.arguments
 *
 * @returns n/a
 */
void  parse_cmd_buffer(void)
{
    char *p = cmd_buffer;

    memset( arguments, 0, sizeof arguments );
    for ( byte i = 0; i < MAXARGS; i++ ) {
        // skip white space
        while ( *p && isspace(*p) ) {
            p++;
        }
        if ( *p == 0 )
            return;
        arguments[i] = p;       // record where arg starts
        // walk over this arg
        while ( *p && !isspace(*p) ) {
            p++;
        }
        if ( *p == 0 )
            return;
    }
}


/**
 * onoff_bool -- convert a string representing on/off, or true/false, or 1/0
 * to a bool
 *
 * @returns true or false
 */
boolean    onoff_bool( char *s )
{
    if ( s == NULL )
        return false;
    if ( STRMATCH2( s, "true" ) )
        return true;
    if ( STRMATCH2( s, "on" ) )
        return true;
    if ( STRMATCH2( s, "false" ) )
        return false;
    if ( STRMATCH2( s, "off" ) )
        return false;
    if ( isdigit( *s ) )
        return ( atoi( s ) );

    return false;
}


/**
 * xdigitvalue -- dehexify an ascii character representing 4-bits
 *
 * @returns the 4-bit value of the hex digit
 */
byte  xdigitvalue( char c )
{
    byte val;

    if ( isdigit( c ) )
        val = c - '0';
    else if ( 'a' <= c && c <= 'f' )
        val = c - 'a' + 10;
    else if ( 'A' <= c && c <= 'F' )
        val = c - 'A' + 10;
    else
        val = 0;
    return ( val );
}


/*
 * xdigits_decoded -- 2 hex digits decoded into a single byte
 *
 * @returns the 8-bit value of the 2 hex digits
 */
byte  xdigits_decoded( char c1, char c2 )
{
    return ( (xdigitvalue( c1 ) << 4) | xdigitvalue( c2 ) );
}


/**
 * led_data_cmd ... performs the action of the "DATA" command.
 *    The given data is decoded (dehexified, stored into led data XRAM data,
 *    then the display is refreshed.
 *
 * @param col       start column in the LED array
 * @param bytes     number of bytes (each representing 8 pixels) provided
 * @param hexdata   hexified data. i.e., hex data encoded in ascii chars
 *
 * @returns n/a
 */
#if 0
void  led_data_cmd( byte col, int nbytes, char *hexdata )
{
    char c1, c2;
    uint8_t x;

    if ( col >= NCOLS ) {
        Serial.println( "ERR col" );
        return;
    }
    if ( (nbytes < 1) || ( nbytes >= 16 ) ) {
        Serial.println( "ERR len" );
        return;
    }

    for ( char *p = hexdata; nbytes > 0; nbytes--, offset++ ) {
        c1 = *p++;
        c2 = *p++;
        if ( !isxdigit(c1)  || !isxdigit(c2) ) {
            Serial.println( "ERR non-hex" );
            return;
        }
        x = xdigits_decoded( c1, c2 );
        led_store8( x, row + offset, col );
    }
    Serial.println( "OK" );
}
#endif


#if 0
/**
 * hexdump -- a general purpose routine to dump a section of memory in hex
 */
void  hexdump( unsigned char *p, unsigned int len )
{
    for ( unsigned int offset = 0; len; offset++, len-- ) {
        if ( (offset & 31 )  == 0 )  {
            printf( "%04x: ", offset );
        }
        printf ( "%02x ", p[offset] );
    }
    printf( "\n" );
}
#endif


/**
 * led_debug ... debug routine to display the LED data area
 */
void  led_debug()
{
    char buf[32];
    Serial.println( "DEBUG data .. column data:" );

    // dump_led_data ... display the LED data in hex
    for ( byte col = 1; col <= NCOLS; col++ ) {

        sprintf( buf, "%04d: ", col ); Serial.print(buf);
        for ( byte row = 1; row <= NROWS; row += 8 ) {
            sprintf ( buf, "%02x ", led_data8( row, col ) ); Serial.print(buf);
        }
        if ( (col & 3) == 0 ) {
            Serial.println("");
        } 
        else {
            Serial.print("  ");
        }
    }
    Serial.println( "" );
    Serial.println( "OK" );
}




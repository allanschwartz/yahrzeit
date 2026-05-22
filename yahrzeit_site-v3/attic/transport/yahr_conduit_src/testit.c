#include <stdio.h>
#include <errno.h>

static char stest[] = "[S]\r\0";
static char itest[] = "[I 56 40]\r\0";
static char dtest[] = "[D1 8 789<]]\r\0";

void led_open( char * filename );
void led_write( char * buf, int len );
void led_read( char * buf, int len );

FILE *fp;

main(int argc, char * argv[])
{
    char *ttyfilename = argv[1];
    char *p;
    int rc;
    char buf[100];

    led_open( ttyfilename );

    led_write( stest, sizeof stest );

    led_read( buf, sizeof buf );

    led_write( itest, sizeof itest );

    led_read( buf, sizeof buf );

    led_write( dtest, sizeof dtest );

    led_read( buf, sizeof buf );

    rc = fclose( fp );
    if ( rc != 0 ) {
        printf( "fclose error rc %d\n", errno );
        return;
    }
}

void led_open( char * filename )
{
    //p = fstat( filename );
    printf( "before fopen\n" );
    fp = fopen( filename, "w+" );
    if ( fp == NULL ) {
        printf( "fopen error errno %d\n", errno );
        return;
    }
    printf( "fopen ok\n" );
}

void led_write( char * buf, int len )
{
    int rc; 

    rc = fwrite( buf, len, 1, fp);
    if ( rc == 0 ) {
        printf( "rwrite error rc %d errno %d\n", rc, errno );
    }
    printf( "fwrite rc %d\n", rc );
}

void led_read( char * buf, int len )
{
    char *p;

    p = fgets( buf, len, fp );
    if ( p == NULL ) {
        printf( "fgets error rc %d\n", errno );
        return;
    }
    printf( "fgets ok\n" );
    printf( "%s\n", buf );
}


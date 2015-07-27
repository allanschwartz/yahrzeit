/*-------------------------------------------------------------------------
  ser_ir.c - source file for serial routines 
  
  Written By - Josef Wolf <jw@raven.inka.de> (1999) 
  
	 This program is free software; you can redistribute it and/or modify it
	 under the terms of the GNU General Public License as published by the
	 Free Software Foundation; either version 2, or (at your option) any
	 later version.
	 
	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.
	 
	 You should have received a copy of the GNU General Public License
	 along with this program; if not, write to the Free Software
	 Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
	 
	 In other words, you are welcome to use, share and improve this program.
	 You are forbidden to forbid anyone else to use, share and improve
	 what you give them.   Help stamp out software-hoarding!

-------------------------------------------------------------------------*/
#include "ser_ir.h"

/* This file implements a serial interrupt handler and its supporting
* routines. Compared with the existing serial.c and _ser.c it has
* following advantages:
* - You can specify arbitrary buffer sizes (umm, up to 255 bytes),
*   so it can run on devices with _little_ memory like at89cx051.
* - It won't overwrite characters which already are stored in the
*   receive-/transmit-buffer.
*/

/* BUG: those definitions (and the #include) should be set dynamically
* (while linking or at runtime) to make this file a _real_ library.
*/
#include <8051.h>
__sfr __at (0x8F) CKCON ;               /* Clock Control Register 0 */

#define XBUFLEN 4
#define RBUFLEN 40

/* You might want to specify idata, pdata or xdata for the buffers */
static unsigned char __pdata rbuf[RBUFLEN], xbuf[XBUFLEN];
static unsigned char rcnt, xcnt, rpos, xpos;
static __bit busy;

extern void read_switch();

void ser_init (void)
{
   ES = 0;                          /* disable serial interupt */
   rcnt = xcnt = rpos = xpos = 0;   /* init buffers */
   busy = 0;

   /* set up the SFRs */
   CKCON &= 0xFE;                   /* set clock = Standard Mode (12 clock) */
   TMOD = 0x21;                     /* Timer1(8bit-Auto) & Timer0(16 bit) */
   SCON = 0x50;                     /* serial port mode 1 */
#if 0
   PCON |= 0x80;                    /* SMOD1 = 1; serial port mode bit 1  */
#endif
   PS = 1;                          /* serial interrupt high priority */
   ES = 0;                          /* disable serial interrupt */
   ET0 = 0;                         /* disable Timer 0 interrupt */
   ET1 = 0;                         /* disable Timer 1 interrupt */
#if 0
   TMOD &= 0x0f;                    /* use timer 1 */
   TMOD |= 0x20;
   TL1 = -3; TH1 = -3;              /* 19200bps with 11.059MHz crystal */
#endif
   TH1 = 0xFB;                      /* set baudrate 9600 (18.432Mhz/12 Clock) */
   TL1 = 0xFB;
   TR0 = 1;                         /* start Timer 0 */
   TR1 = 1;                         /* start Timer 1 */
   ES = 1;                          /* enable serial interrupt */
   EA = 1;                          /* enable global interrupts */
}

void ser_handler (void) __interrupt 4
{
   if (RI) {
	   RI = 0;
	   /* don't overwrite chars already in buffer */
	   if (rcnt < RBUFLEN)
		   rbuf [(unsigned char)(rpos+rcnt++) % RBUFLEN] = SBUF;
   }
   if (TI) {
	   TI = 0;
	   if (busy = xcnt) {   /* Assignment, _not_ comparison! */
		   xcnt--;
		   SBUF = xbuf [xpos++];
		   if (xpos >= XBUFLEN)
			   xpos = 0;
	   }
   }
}

void ser_putc (unsigned char c)
{
   while (xcnt >= XBUFLEN) /* wait for room in buffer */
	   ;
   ES = 0;
   if (busy) {
	   xbuf[(unsigned char)(xpos+xcnt++) % XBUFLEN] = c;
   } else {
	   SBUF = c;
	   busy = 1;
   }
   ES = 1;
}

unsigned char ser_getc (void)
{
   unsigned char c;
   while (!rcnt)   /* wait for character */
	   ;
   ES = 0;
   rcnt--;
   c = rbuf [rpos++];
   if (rpos >= RBUFLEN)
	   rpos = 0;
   ES = 1;
   return (c);
}
#pragma save
#pragma noinduction
void ser_puts (unsigned char *s)
{
   unsigned char c;
   while (c=*s++) {
	   if (c == '\n') ser_putc ('\r');
	   ser_putc (c);
   }
}
#pragma restore
void ser_gets (unsigned char *s, unsigned char len)
{
   unsigned char pos, c;

   pos = 0;
   while (pos <= len) {
       while (!rcnt) {           /* while waiting for a character */
           read_switch();
       }
	   c = ser_getc ();
	   //if (c == '\r') continue;        /* discard CR's */
       if (c == '\r') {
          ser_putc(c);
          c = '\n';
       }
       ser_putc(c);                 // echoplex
	   s[pos++] = c;
       if (c == '\n') break;           /* NL terminates */
   }
   s[pos] = '\0';
}

unsigned char ser_can_xmt (void)
{
   return XBUFLEN - xcnt;
}

unsigned char ser_can_rcv (void)
{
   return rcnt;
}

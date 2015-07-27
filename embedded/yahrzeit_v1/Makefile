INCLUDE=-I/Developer/sdcc/bin/../share/sdcc/lib/small
PROCESSOR_FLAG=-l msc51
CC=sdcc
AS=gpasm
CFLAGS=-S

HEX_FILES=led_ctrl.ihx

led_ctrl:            led_device.rel read_switch.rel ser_ir.rel
	$(CC) led_ctrl.c led_device.rel read_switch.rel ser_ir.rel

all: $(HEX_FILES)

%.rel: %.c
	$(CC) -c $(INCLUDE) $(PROCESSOR_FLAG) $<

%.asm: %.c
	$(CC) $(INCLUDE) $(PROCESSOR_FLAG) $(CFLAGS) $<

%.hex: %.asm
	$(AS) -a inhx32 $*.asm

clean:
	$(RM) *.asm *.mem *.rel *.rst *.sym *.lnk *~


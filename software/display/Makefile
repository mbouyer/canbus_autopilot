.SUFFIXES: .o .hex

.PATH: ${.CURDIR}/../../../pic18_n2k ${.CURDIR}/../lib/eeprom

CC= sdcc  --use-non-free --Werror --fsigned-char
CFLAGS= --fomit-frame-pointer --denable-peeps --optimize-cmp --optimize-df
CFLAGS+= -I${.CURDIR} -I${.CURDIR}/../../../pic18_n2k -I${.CURDIR}/../lib/eeprom
#CFLAGS+= -DDISPLAY_FAKE
OBJECTS= main.o serial.o autopilot_spi.o font5x8.o font10x16.o font20x24.o menu.o nmea2000.o eeprom.o raddeg.o
HEADERS= autopilot_serial.h autopilot_spi.h font5x8.h font10x16.h font20x24.h nmea2000.h nmea2000_pgn.h nmea2000_user.h menu.h  eeprom.h raddeg.h

all: display.bin

obj:
	mkdir obj

display.bin: display.hex
	hex2bin1k ${.ALLSRC} ${.TARGET}
	@ls -lh ${.TARGET}

display.hex: ${OBJECTS}
	${CC} -V -Wl,-s${.CURDIR}/pic18f26k80.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -mpic16 -p18f26k80 -Wl,libc18f.lib ${.ALLSRC} -o ${.TARGET} -llibm18f.lib

${OBJECTS}: ${HEADERS} Makefile

.c.o:
	${CC} ${CFLAGS} -V --no-crt --ivt-loc=0x400 --obanksel=2 -mpic16 -p18f26k80 -c ${.IMPSRC} -o ${.TARGET}

clean:
	rm -f display.bin *.o *.hex *.cod *.lst *.asm

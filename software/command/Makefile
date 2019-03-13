.SUFFIXES: .o .hex

.PATH: ${.CURDIR}/../lib/nmea2000 ${.CURDIR}/../lib/eeprom
.PATH: ${.CURDIR}/../lib/radanddeg

CC= sdcc  --use-non-free --Werror --fsigned-char
CFLAGS= -I${.CURDIR} -I${.CURDIR}/../lib/nmea2000 -I${.CURDIR}/../lib/eeprom
CFLAGS+= -I${.CURDIR} -I${.CURDIR}/../lib/radanddeg
CFLAGS+= -DFAKE
OBJECTS= main.o serial.o nmea2000.o raddeg.o eeprom.o
HEADERS= autopilot_serial.h raddeg.h eeprom.h nmea2000.h nmea2000_pgn.h nmea2000_user.h

all: command.bin

obj:
	mkdir obj

command.bin: command.hex
	hex2bin1k ${.ALLSRC} ${.TARGET}
	@ls -lh ${.TARGET}

command.hex: ${OBJECTS}
	${CC} -V -Wl,-s${.CURDIR}/pic18f26k80.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -mpic16 -p18f26k80 -Wl,libc18f.lib ${.ALLSRC} -o ${.TARGET} -llibm18f.lib

${OBJECTS}: ${HEADERS} Makefile

.c.o:
	${CC} ${CFLAGS} -V --no-crt --ivt-loc=0x400 --obanksel=2 -mpic16 -p18f26k80 -c ${.IMPSRC} -o ${.TARGET}

clean:
	rm -f ${.ALLTARGETS}

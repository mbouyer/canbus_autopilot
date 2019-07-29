.SUFFIXES: .o .hex

.PATH: ${.CURDIR}/../lib/nmea2000
.PATH: ${.CURDIR}/../lib/radanddeg

CC= sdcc  --use-non-free --Werror --fsigned-char
ASM = gpasm
CFLAGS= -I${.CURDIR} -I${.CURDIR}/../lib/nmea2000 -I${.CURDIR}/../lib/eeprom
CFLAGS+= -I${.CURDIR}/../lib/radanddeg
CFLAGS+= --fomit-frame-pointer --denable-peeps --optimize-cmp --optimize-df
CFLAGS+= -DCAPTEUR_FAKE
OBJECTS= main.o serial.o nmea2000.o raddeg.o
HEADERS= autopilot_serial.h nmea2000.h nmea2000_pgn.h nmea2000_user.h raddeg.h

all: receiver.bin

obj:
	mkdir obj

receiver.bin: receiver.hex
	hex2bin1k ${.ALLSRC} ${.TARGET}
	@ls -lh ${.TARGET}

receiver.hex: ${OBJECTS}
	${CC} -V -Wl,-s${.CURDIR}/pic18f26k80.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -mpic16 -p18f26k80 -Wl,libc18f.lib ${.ALLSRC} -o ${.TARGET} -llibm18f.lib

${OBJECTS}: ${HEADERS} Makefile

.c.o:
	${CC} ${CFLAGS} -V --no-crt --ivt-loc=0x400 --obanksel=2 -mpic16 -p18f26k80 -c ${.IMPSRC} -o ${.TARGET}

.s.o:
	${ASM} -c -o ${.TARGET} ${.IMPSRC}

clean:
	rm -f ${.ALLTARGETS}
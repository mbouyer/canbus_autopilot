.SUFFIXES: .o .hex

.PATH: ${.CURDIR}/../lib/nmea2000 ${.CURDIR}/../lib/eeprom

CC= sdcc  --use-non-free --Werror --fsigned-char
ASM = gpasm
CFLAGS= -I${.CURDIR} -I${.CURDIR}/../lib/nmea2000 -I${.CURDIR}/../lib/eeprom
CFLAGS+= --fomit-frame-pointer --denable-peeps --optimize-cmp --optimize-df
CFLAGS+= -DCAPTEUR_FAKE
OBJECTS= main.o marg_filter.o serial.o autopilot_i2c.o nmea2000.o eeprom.o
OBJECTS+= fsmul.o fsadd.o fssub.o mullong.o
HEADERS= marg_filter.h autopilot_serial.h autopilot_i2c.h l3dg20reg.h lsm303.h nmea2000.h nmea2000_pgn.h nmea2000_user.h eeprom.h

all: capteur.bin

obj:
	mkdir obj

capteur.bin: capteur.hex
	hex2bin ${.ALLSRC} ${.TARGET}
	@ls -lh ${.TARGET}

capteur.hex: ${OBJECTS}
	${CC} -V -Wl,-s${.CURDIR}/pic18f26k80.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -mpic16 -p18f26k80 -Wl,libc18f.lib ${.ALLSRC} -o ${.TARGET} -llibm18f.lib

${OBJECTS}: ${HEADERS} Makefile

.c.o:
	${CC} ${CFLAGS} -V --no-crt --ivt-loc=0x400 --obanksel=2 -mpic16 -p18f26k80 -c ${.IMPSRC} -o ${.TARGET}

.s.o:
	${ASM} -c -o ${.TARGET} ${.IMPSRC}

clean:
	rm -f ${.ALLTARGETS}
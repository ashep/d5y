APP_NAME = cronus
APP_VER = 0.1
APP_AUTH = Alexander Shepetko
APP_URL = https://github.com/ashep/cronus

MCU = atmega328p
DUDE_MCU = m328p
F_CPU = 8000000
BAUD=38400

DUDE_PROGR_USBASP = -c usbasp -P USB

CC=avr-gcc
LD=avr-ld
OBJCOPY=avr-objcopy
CFLAGS=\
	-std=c18 \
	-pedantic \
	-Os \
	-mmcu=${MCU} \
	-I. \
	-D F_CPU=${F_CPU} \
	-D BAUD=${BAUD} \
	-D APP_NAME='"${APP_NAME}"' \
	-D APP_VER='"${APP_VER}"' \
	-D APP_AUTH='"${APP_AUTH}"' \


BUILD_DIR=build
TARGET=${BUILD_DIR}/${APP_NAME}
SRC=main.c

bin:
	mkdir -p ${BUILD_DIR}
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SRC}

hex: bin
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex
	ls -l ${TARGET}.hex

upload: hex
	avrdude -V -p ${DUDE_MCU} -U flash:w:${TARGET}.hex:i ${DUDE_PROGR_USBASP}

clean:
	rm -f ${BUILD_DIR}/*.bin ${BUILD_DIR}/*.hex ${BUILD_DIR}/*.o

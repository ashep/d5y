cmake_minimum_required(VERSION 3.15)
project(cronus C)

SET(OBJ_COPY /usr/bin/avr-objcopy)
SET(AVRDUDE /usr/bin/avrdude)

SET(MCU "atmega328p")
SET(DUDE_MCU "m328p")
SET(F_CPU "8000000")

SET(CMAKE_C_COMPILER /usr/bin/avr-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/avr-g++)

SET(CMAKE_C_FLAGS "-std=c99 -Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU}")
SET(CMAKE_C_LINK_FLAGS "-mmcu=${MCU}")

INCLUDE_DIRECTORIES(.)

SET(SOURCE_FILES
        src/main.c
        lib/util.c
        lib/usart.c
        lib/fb.c
        lib/max7219.c)

add_executable(cronus ${SOURCE_FILES})

add_custom_target(hex ALL COMMAND ${OBJ_COPY} -j .text -j .data -O ihex ${PROJECT_NAME} ${PROJECT_NAME}.hex)

add_custom_target(flash ALL
        COMMAND ${AVRDUDE} -c usbasp -v -V -P usb -p ${DUDE_MCU} -D -U flash:w:${PROJECT_NAME}.hex:i)
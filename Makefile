#15/02/2011 Stefano Burchietti

VERSION = 0.1
CC = gcc
CFLAGS=-Wall
LFLAGS=
SRC_DIR=src/
BUILD_DIR=build/
CAMPANARO_BIN = Campanaro
CAMPANARO_SRC = *.c
$LIBS=

all:	Campanaro

Campanaro:
	${CC} $(CFLAGS) $(LFLAGS) -o ${BUILD_DIR}${CAMPANARO_BIN} ${SRC_DIR}${CAMPANARO_SRC} $(LIBS)

clean:       cleanbin
	rm -f ${BUILD_DIR}*.o  ${SRC_DIR}*~ 

cleanbin:
	rm -f ${BUILD_DIR}${CAMPANARO_BIN}

tar:
	tar czvf Campanaro.tgz Makefile src/ README TODO

# DO NOT DELETE

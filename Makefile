#15/02/2011 Stefano Burchietti

VERSION = 0.1
CC = gcc
CFLAGS=-Wall -g
LFLAGS=
SRC_DIR=src/
BUILD_DIR=build/
CAMPANARO_BIN = Campanaro
CAMPANARO_SRC = *.c
CAMPANARO_GLUE = */*.c
$LIBS=
INCLUDE=-I src/SocketGlue


all:
	${CC} $(INCLUDE) $(CFLAGS) $(LFLAGS) -o ${BUILD_DIR}${CAMPANARO_BIN} ${SRC_DIR}${CAMPANARO_SRC} ${SRC_DIR}${CAMPANARO_GLUE} $(LIBS)

clean:	cleanbin
	rm -f ${BUILD_DIR}*.o  ${SRC_DIR}*~ 

cleanbin:
	rm -f ${BUILD_DIR}${CAMPANARO_BIN}

tar:
	tar czvf Campanaro.tgz Makefile src/ README TODO

# DO NOT DELETE

CC=g++
CFLAGS=-Wall -O2 -std=c++0x
LFLAGS=-Wall -O2 -lopencv_core -lopencv_highgui

all: cgm

main.o: main.cpp common.h
	${CC} ${CFLAGS} -c main.cpp
GestureMap.o: GestureMap.cpp GestureMap.h common.h
	${CC} ${CFLAGS} -c GestureMap.cpp
FrameBuffer.o: FrameBuffer.cpp FrameBuffer.h
	${CC} ${CFLAGS} -c FrameBuffer.cpp

cgm: GestureMap.o FrameBuffer.o main.o
	${CC} ${LFLAGS} main.o GestureMap.o FrameBuffer.o -o cgm


clean:
	rm -f *.o cgm


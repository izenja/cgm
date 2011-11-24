CFLAGS=-O2 -std=c++0x
LFLAGS=-O2 -lopencv_core -lopencv_highgui

all: cgm

main.o: main.cpp common.h
	gcc ${CFLAGS} -c main.cpp
GestureMap.o: GestureMap.cpp GestureMap.h common.h
	gcc ${CFLAGS} -c GestureMap.cpp

cgm: GestureMap.o main.o
	gcc ${LFLAGS} main.o GestureMap.o -o cgm


clean:
	rm -f *.o cgm


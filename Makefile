CFLAGS=-O2 -std=c++0x
LFLAGS=-O2 -lopencv_core -lopencv_highgui

all: cgm

main.o: main.cpp
	gcc ${CFLAGS} -c main.cpp

cgm: main.o
	gcc ${LFLAGS} main.o -o cgm


clean:
	rm -f main.o cgm


OGLCFLAGS=-lGL -lGLU -lglut -lGLEW -lSOIL

OPTIONS=$(OGLCFLAGS) -std=c++11 -g

main.cpp:
	g++ src/main.cpp $(OPTIONS) -o shadergif

all: main.cpp

install:
	cp shadergif /usr/bin/

uninstall:
	rm /usr/bin/shadergif

clean:
	rm -f src/*.o
	rm -f shadergif

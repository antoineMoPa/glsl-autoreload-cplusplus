OGLCFLAGS=-lGL -lGLU -lglut -lGLEW -lSOIL

OPTIONS=$(OGLCFLAGS) -std=c++11 -g

all: main.o
	g++ main.o Shader.o ShaderGif.o platform.o Image.o $(OPTIONS) -o shadergif

main.o: Shader.o ShaderGif.o platform.o Image.o
	g++ -c src/main.cpp $(OPTIONS)

ShaderGif.o:
	g++ -c src/ShaderGif.cpp $(OPTIONS)

platform.o:
	g++ -c src/platform.cpp $(OPTIONS)

Shader.o:
	g++ -c src/Shader.cpp $(OPTIONS)

Image.o:
	g++ -c src/Image.cpp $(OPTIONS)

install:
	cp shadergif /usr/bin/

uninstall:
	rm /usr/bin/shadergif

clean:
	rm -f *.o
	rm -f shadergif

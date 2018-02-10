OGLCFLAGS=-lGL -lGLU -lglut -lGLEW -lSOIL

OPTIONS=$(OGLCFLAGS) -std=c++11 -g

all: main.o
	g++ main.o Shader.o ShaderGif.o platform.o Image.o $(OPTIONS) -o shadergif

main.o: Shader.o ShaderGif.o platform.o Image.o src/main.cpp
	g++ -c src/main.cpp $(OPTIONS)

ShaderGif.o: src/ShaderGif.cpp
	g++ -c src/ShaderGif.cpp $(OPTIONS)

platform.o: src/platform.cpp
	g++ -c src/platform.cpp $(OPTIONS)

Shader.o: src/Shader.cpp
	g++ -c src/Shader.cpp $(OPTIONS)

Image.o: src/Image.cpp
	g++ -c src/Image.cpp $(OPTIONS)

install:
	cp shadergif /usr/bin/

uninstall:
	rm /usr/bin/shadergif

clean:
	rm -f *.o
	rm -f shadergif

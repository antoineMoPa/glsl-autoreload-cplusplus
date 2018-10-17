PACKAGES=gtk+-3.0 gtksourceview-3.0 gtkglext-3.0
LIBS=-lGL -lGLU -lGLEW

all:
	export PACKAGES="$(PACKAGES)"
	gcc `pkg-config --cflags "$(PACKAGES)"` -o shader shader.cpp `pkg-config --libs "$(PACKAGES)"` $(LIBS)

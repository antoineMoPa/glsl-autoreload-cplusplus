# ShaderGif native

A simple command line tool to run shaders. Good for offline shader sketches.

Why use a command line tool for graphical stuff? With a command-line tool, you can use your preferred editor, you can automate it, things are simple. Command lines are actually made to be used by humans.

# Building:

	sudo apt-get install freeglut3-dev
	make all
	sudo make install
	./shadergif

# Removing

	sudo make uninstall

# Running

	shadergif

You will need a vertex and a fragment shader in your working directory. You can just grab `vertex.glsl` and `fragment.glsl` from the repo.

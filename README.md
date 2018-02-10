# ShaderGif native

A simple linux command line tool to run shaders. Good for offline shader sketches.

Why use a command line tool for graphical stuff? With a command-line tool, you can use your preferred editor, you can automate it, things are simple. Command lines are actually made to be used by humans and I want to promote the terminal.

# Building:

	sudo apt-get install freeglut3-dev
	make all
	sudo make install	# (optional)
	./shadergif			# or just `shadergif` if you did `make install`

# Removing

	sudo make uninstall

# Running

	shadergif

You will need a vertex and a fragment shader in your working directory. You can just grab `vertex.glsl` and `fragment.glsl` from the repo.

# Render a frame

	# Change 0.0 to the time uniform you want
	# Image will be saved to image.bmp
	./shadergif --time=0.0
	# Image will be saved to test.bmp
	./shadergif --time=0.0 --filename=test.bmp

# Rendering a gif with imagemagick

You can use/tweak this bash one-liner:

	for i in `LANG=en_US seq 0.0 0.1 1.0`; do ./shadergif --time=$i --filename=image-$i.bmp; done; convert image-*.bmp anim.gif

This will create a file named anim.gif.


# Rendering a video with avconv

You can use/tweak this bash one-liner:

	j=0;for i in `LANG=en_US seq 0.0 0.1 1.0`; do ./shadergif --time=$i --filename=image-`printf "%04d" $j`.bmp; j=$((1+$j)); done; avconv -i image-%04d.bmp anim.mp4

To watch the video (with looping):

	mplayer --loop=0 anim.mp4


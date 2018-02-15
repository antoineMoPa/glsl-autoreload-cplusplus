# ShaderGif native

A simple linux command line tool that runs shaders. Good for offline shader sketches. Shaders are watched for modifications and updated automatically. This is coded in C++, unlike some alternatives that run a browser behind the scene. Works fine on many machines, including a chromebook with crouton.

# Installing dependencies and building:

This should work on  Ubuntu and Debian. For other distributions, you'll have to find these dependencies, it should not be too difficult.

	sudo apt-get install freeglut3-dev libglm-dev libglew-dev libsoil-dev
	git clone https://github.com/antoineMoPa/shadergif-native.git
	cd shadergif-native
	make all
	sudo make install	# (optional)
	./shadergif		# or just `shadergif` if you did `make install`

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


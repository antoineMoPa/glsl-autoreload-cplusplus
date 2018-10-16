# Dependencies

"Simply":

	sudo apt-get install -y gobject-introspection libgtk-3-dev libmutter-dev libwnck-3-dev libgnome-menu-3-dev libupower-glib-dev gobject-introspection gtk-doc-tools libmath-libm-perl libgtksourceview-3.0 pkg-config libgtk-3-0 libsdl2-dev
	git clone git@github.com:alexmurray/gtkglext.git
	cd gtkglext
	autoreconf --install
	./configure --prefix=/usr/
	make
	sudo make install



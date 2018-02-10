#ifndef SHADERGIF_H
#define SHADERGIF_H

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <map>
#include <vector>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <algorithm>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>

#include "platform.h"
#include "Shader.h"
#include "Image.h"

/*
  Contains data about an opengl framebuffer
  And the texture it gets rendered to
*/
class FrameBuffer{
public:
	Image * rendered_tex;
	GLuint fb_id;
	GLuint depth_buf;
	
	void delete_ressources();
	
	void create(int w, int h);
	
	void resize(int w, int h);
};

using namespace std;

using TextureMap = std::map<std::string, Image>;
using ShaderMap = std::map<std::string, Shader>;

class ShaderGif{
public:
	static Shader * current_shader;
	static Shader post_process_shader;
	// This frame counter resets at frame resize
	static int frame_count;

	// The texture that we can post process
	// (and render on the quad)
	
	static TextureMap textures;
	static int inotifyFd, wd;
	static char *p; // TODO: what is this?
	static struct inotify_event *event;
	static bool enable_2_pass_pp;
	static const int pass_total = 3;
	static FrameBuffer fbs [pass_total + 1];

	// Default app path
	static string app_path;
	static ShaderMap shaders;

	// Window width
	static int w;
	// Window height
	static int h;
	
	// The depth buffer
	static GLuint depth_buf;
	static FrameBuffer fb;
	
	// The data of the render-to-texture quad
	static GLuint quad_vertexbuffer;

	static int argc;
	static char ** argv;
	static float curr_time;

	static void resize(int rhs_w, int rhs_h);

	static void load_default_shaders();

	/*
	  Returns current time, either as given by 
	  argument or form system clock
	*/
	static float get_frame_time();
	
	static void main_render();
	
	static void post_process_render(int pass);

	static void two_pass_pp();
	
	static void render();
	
	/**
	   Creates the plane that will be used to render everything on
	 */
	static void create_render_quad();

	/*
	  Gets a numeric arg like "--meow=2" or "--frame=42.0"
	  where name is "meow" or "frame"
	  
	  returns the number, if present & valid
	  returns -1.0 if arg is not present
	  returns -2.0 if an error is detected, in which case it 
	  will print an error message.
	*/
	static float get_positive_numeric_arg(const char * name);

	/*
	  Mandatory refs.:
	  https://stackoverflow.com/questions/5616092/non-blocking-call-for-reading-descriptor
	  http://man7.org/tlpi/code/online/diff/inotify/demo_inotify.c.html
	 */
	static void init_watch_files();
	
	static void apploop();

	static void start(int _argc, char ** _argv);
};

#endif

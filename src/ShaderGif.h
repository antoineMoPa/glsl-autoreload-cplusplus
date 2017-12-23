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

namespace ShaderGif{
	Shader * current_shader = nullptr;
	Shader post_process_shader;
	// This frame counter resets at frame resize
	int frame_count = 0;
}

#include "Image.h"

namespace ShaderGif{
	// The texture that we can post process
	// (and render on the quad)
	using TextureMap = std::map<std::string, Image>;
	TextureMap textures;
	int inotifyFd, wd;
	char *p;
	struct inotify_event *event;
	
	/*
	  Contains data about an opengl framebuffer
	  And the texture it gets rendered to
	*/
	class FrameBuffer{
	public:
		Image * rendered_tex;
		GLuint fb_id;
		GLuint depth_buf;

		void delete_ressources(){
			rendered_tex->delete_ressources();
			glDeleteRenderbuffers(1, &depth_buf);
			glDeleteRenderbuffers(1, &fb_id);
		}
		
		void create(int w, int h){
			// Create framebuffer
			glGenFramebuffers(1, &fb_id);
			glGenRenderbuffers(1, &depth_buf);
			glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
			
			rendered_tex = new Image(w,h);
			
			// Poor filtering. Needed !
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			
			glFramebufferTexture2D(
								   GL_FRAMEBUFFER,
								   GL_COLOR_ATTACHMENT0,
								   GL_TEXTURE_2D,
								   rendered_tex->get_id(),0);
			
			GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
			glDrawBuffers(1, DrawBuffers);

			glBindRenderbuffer(GL_RENDERBUFFER, depth_buf);
			glRenderbufferStorage(GL_RENDERBUFFER,
								  GL_DEPTH_COMPONENT, w, h);
			
			glFramebufferRenderbuffer(
									  GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buf
									  );
			 
			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
				cerr << "Framebuffer setup error" << endl;
			}
		}
		
		void resize(int w, int h){
			delete_ressources();
			create(w, h);
			rendered_tex->resize(w, h);
		}
	};

	bool enable_2_pass_pp = false;
	const int pass_total = 3;
	FrameBuffer fbs [pass_total + 1];
}

using namespace std;

namespace ShaderGif{
	// Default app path
	string app_path = "./";

	using ShaderMap = std::map<std::string,Shader>;
	ShaderMap shaders;

	// Window width
	int w = 540;
	// Window height
	int h = 540;
}

namespace ShaderGif{
	// The depth buffer
	GLuint depth_buf;
	FrameBuffer fb;
	
	// The data of the render-to-texture quad
	GLuint quad_vertexbuffer;

	int argc;
	char ** argv;
	float curr_time;

	/**
	   Window resize callback
	 */
	static void resize(int rhs_w, int rhs_h){
		w = rhs_w;
		h = rhs_h;
		
		for(int i = 0; i < pass_total + 1; i++){
			fbs[i].resize(w, h);
		}
		frame_count = 0;
	}

	static void load_default_shaders(){
		// default shaders
		char * vertex_path =
			strdup("./vertex.glsl");
		char * frag_path =
			strdup("./fragment.glsl");

		Shader s;
		cout << "loading" << frag_path << "\n";
		cout << "loading" << vertex_path << "\n";
		if(!s.load(vertex_path,frag_path)){
			// No shader for now
			// the app will retry when file is modified
			return;
		}
		s.bind();

		using new_el = ShaderMap::value_type;

		shaders.erase("default");
		shaders.insert(new_el("default",s));
	}

	/*
	  Returns current time, either as given by 
	  argument or form system clock
	*/
	float get_frame_time(){
		if(curr_time < 0.0){
			return get_timestamp();
		} else {
			return curr_time;
		}
	}
	
	void main_render(){
		current_shader = &shaders[string("default")];
		current_shader->bind();
		
		// Add aspect ratio
		GLuint loc = current_shader
			->get_uniform_location("ratio");
		
		// Bind it
		glUniform1f(loc,(float)w/(float)h);

		// Add screen dimensions
		// Width
		loc = current_shader
			->get_uniform_location("screen_w");
		
		glUniform1i(loc,w);

		// Height
		loc = current_shader
			->get_uniform_location("screen_h");
		
		glUniform1i(loc,h);

		float time = get_frame_time();
		
		// Add timestamp
		loc = current_shader
			->get_uniform_location("time");
		
		// Bind timestamp to variable
		glUniform1f(loc, time);

		// Add render pass number
		loc = current_shader
			->get_uniform_location("pass");
		
		// Bind pass number
		glUniform1i(loc, 0);

		// Add frame count
		loc = current_shader
			->get_uniform_location("frame_count");

		glUniform1i(loc,frame_count);
		
		glViewport(0,0,w,h);
		
		// Actual rendering
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		
		fbs[0].rendered_tex->bind(3,"rendered_tex");

		// Render the plane
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(0);
		
		glFlush();
		
		// Internet copy pasta
		// to watch shader files
		const int BUF_LEN = 1024;
		char buf[BUF_LEN];
		int numRead = -1;
		
		numRead = read(inotifyFd, &buf, 1);
		bool shaders_changed = false;

		while(true){
			numRead = read(inotifyFd, buf, BUF_LEN);

			if(numRead == 0){
				cout << "read() from inotify fd returned 0!\n";
				exit(-1);
			}
			
			if(numRead == -1){
				break;
			}
			
			for(int i = 0; i < numRead; i++){
				p = &buf[i];
				event = (struct inotify_event *) p;
				shaders_changed = true;
			}
		}

		if(shaders_changed){
			load_default_shaders();
		}
	}
	
	void post_process_render(int pass){
		glViewport(0,0,w,h);

		// TODO document all these defined values in some place
		// for new users
		

		if(pass < 1){
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		} else {
			glClear(GL_DEPTH_BUFFER_BIT);
		}
		
		// Render the plane
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(0);

		glFlush();
	}

	void two_pass_pp(){
		int pass;
		GLuint pps = post_process_shader.get_id();
		int last_fb = 0;
		int curr_fb;
		
		post_process_shader.bind();
		
		int limit = pass_total;

		/*
		  Render each pass
		  Bind texture of last pass to pass_0/pass_1/etc.
		  pass_0 is the main rendered scene
		 */
		for(pass = 1; pass <= limit; pass++){
			curr_fb = pass;
			
			string num = "0";
			num[0] += pass - 1;
			string pass_name("pass_");
			pass_name += num;

			// bind last texture
			fbs[last_fb]
				.rendered_tex->bind(pps,3,"last_pass");

			fbs[last_fb]
				.rendered_tex->bind(pps,3 + pass,pass_name.c_str());

			if(pass != pass_total){
				// Render texture on a plane
				glBindFramebuffer( GL_FRAMEBUFFER,
								   fbs[curr_fb].fb_id );
			} else {
				// Last pass: render to screen
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
			
			post_process_render(pass);
			last_fb = curr_fb;
		}
	}
	
	static void render(){
		// Prepare to render to a texture
		glBindFramebuffer(GL_FRAMEBUFFER, fbs[0].fb_id);
		main_render();

		if(enable_2_pass_pp == true){
			two_pass_pp();
		} else {
			GLuint pps = post_process_shader.get_id();
			fbs[0]
				.rendered_tex->bind(pps,0,"renderedTexture");

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			post_process_shader.bind();
			post_process_render(0);
		}

		glutSwapBuffers();
		frame_count++;
	}
	
	/**
	   Creates the plane that will be used to render everything on
	 */
	static void create_render_quad(){
		GLuint quad_vertex_array_id;
		// Create a quad
		glGenVertexArrays(1, &quad_vertex_array_id);
		glBindVertexArray(quad_vertex_array_id);

		static const GLfloat quad_vertex_buffer_data[] = {
			-1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			-1.0f,  -1.0f, 0.0f,
			1.0f,  -1.0f, 0.0f
		};

		// Put the data in buffers
		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER,
					 sizeof(quad_vertex_buffer_data),
					 quad_vertex_buffer_data,
					 GL_STATIC_DRAW);
	}

	/*
	  Gets a numeric arg like "--meow=2" or "--frame=42.0"
	  where name is "meow" or "frame"
	  
	  returns the number, if present & valid
	  returns -1.0 if arg is not present
	  returns -2.0 if an error is detected, in which case it 
	  will print an error message.
	*/
	static float get_positive_numeric_arg(const char * name){
		string needle = string("--") + string(name);
		for(int i = 0; i < argc; i++){
			string arg = string(argv[i]);

			if(int pos = arg.find(needle) == 0){
				// find '=' sign
				pos = arg.find("=") + 1;
				
				// Get the value
				// todo: check bound and potential of by one
				// error because it is 1h00 AM
				int number = stof(arg.substr(pos, arg.length() - pos + 1));
				
				if(number < 0){
					cout << "Invalid number given for argument '";
					cout << name << "'\n";
					return -2.0;
				}
				
				return number;
			}
		}

		return -1.0;
	}

	/*
	  Mandatory refs.:
	  https://stackoverflow.com/questions/5616092/non-blocking-call-for-reading-descriptor
	  http://man7.org/tlpi/code/online/diff/inotify/demo_inotify.c.html
	 */
	static void init_watch_files(){
		inotifyFd = inotify_init();
		
		wd = inotify_add_watch(inotifyFd, "fragment.glsl", IN_MODIFY);

		if(wd < 0){
			cout << "error while creating file listener\n";
			exit(-1);
		}
		
		int flags = fcntl(inotifyFd, F_GETFL, 0);
		fcntl(inotifyFd, F_SETFL, flags | O_NONBLOCK);
	}
	
	static void apploop(){
		glutInit(&argc,argv);
		glutInitWindowSize(w,h);
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
		glutCreateWindow("shadergif-native");

		init_watch_files();
		
		// http://gamedev.stackexchange.com/questions/22785/
		GLenum err = glewInit();
		if (err != GLEW_OK){
			cout << "GLEW error: " << err << "\n";
			cout <<  glewGetErrorString(err) << "\n";
		}

		load_default_shaders();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Alpha mixing setup
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		// TODO: make post processing work
		// using if(false) for now, to skip
		// maybe it already works, but I have to sleep
		if(false){
			// Load post processing shaders
			post_process_shader.load(
				(app_path+"post-vertex.glsl").c_str(),
				(app_path+"post-fragment.glsl").c_str());
		}

		// Assign callbacks
		glutReshapeFunc(resize);
		glutDisplayFunc(render);
		glutIdleFunc(render);

		// Create the plane for render-to-texture
		create_render_quad();
		
		// Init buffers for render-to-texture
		// and post process
		for(int i = 0; i < pass_total + 1; i++){
			fbs[i].create(w, h);
		}

		curr_time = get_positive_numeric_arg("time");

		if(curr_time < 0.0){
			// Enter continuous render mode
			// The app becomes alive here
			glutMainLoop();
			
		} else {
			// Render only one frame
			render();
			fbs[0].rendered_tex->save("image.bmp");
		}
		
		
	}

	static void start(int _argc, char ** _argv){
		argc = _argc;
		argv = _argv;
	}
}

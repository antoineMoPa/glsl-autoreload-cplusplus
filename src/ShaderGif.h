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

    void main_render(){
        glViewport(0,0,w,h);
        
        // Actual rendering
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        current_shader = &shaders[string("default")];
        current_shader->bind();
        fbs[0].rendered_tex->bind(3,"rendered_tex");

		// Render the plane
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(0);
		
        glFlush();        
    }

    void post_process_render(int pass){
        glViewport(0,0,w,h);

        // TODO document all these defined values in some place
        // for new users
        
        // Add aspect ratio
        GLuint loc = post_process_shader
            .get_uniform_location("ratio");
        
        // Bind it
        glUniform1f(loc,(float)w/(float)h);

        // Add screen dimensions
        // Width
        loc = post_process_shader
            .get_uniform_location("screen_w");
        
        glUniform1i(loc,w);

        // Height
        loc = post_process_shader
            .get_uniform_location("screen_h");
        
        glUniform1i(loc,h);

        
        // Add timestamp
        loc = post_process_shader
            .get_uniform_location("time");
        
        // Bind timestamp to variable
        glUniform1i(loc,get_timestamp());

        // Add render pass number
        loc = post_process_shader
            .get_uniform_location("pass");
        
        // Bind pass number
        glUniform1i(loc,pass);

        // Add frame count
        loc = post_process_shader
            .get_uniform_location("frame_count");

        glUniform1i(loc,frame_count);

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
	
    static void load_default_shaders(){
        // default shaders
        char * vertex_path =
            strdup("./vertex.glsl");
        char * frag_path =
            strdup("./fragment.glsl");

        Shader s;
        cout << frag_path << "\n";
        cout << vertex_path << "\n";
        if(!s.load(vertex_path,frag_path)){
            cout << "No default vertex & fragment shader found." << "\n";
            exit(0);
            return;
        }
        s.bind();

        using new_el = ShaderMap::value_type;

        shaders.insert(new_el("default",s));
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

    static void apploop(){
        glutInit(&argc,argv);
        glClearColor(0.0f,0.0f,0.0f,0.0f);
        glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
        glutInitWindowSize(w,h);

        glutCreateWindow("shadergif-native");

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
        
        // The app becomes alive here
        glutMainLoop();
    }

    static void start(int _argc, char ** _argv){
        argc = _argc;
        argv = _argv;
		
    }
}

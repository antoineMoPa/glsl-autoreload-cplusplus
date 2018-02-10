#ifndef SHADER_H
#define SHADER_H

#include <GL/gl.h>
#include <iostream>
#include "ShaderGif.h"

using namespace std;

// TODO: move vars to snake_case

class Shader{
public:
	
/**
 * shader type is either:
 * GL_VERTEX_SHADER
 * GL_FRAGMENT_SHADER
 **/
	GLuint load_shader(const char * file_path,GLenum shaderType);
	
	GLuint get_uniform_location(const char * name);
	
	bool load(const char * vertex_path, const char * fragment_path);
	
	GLuint get_id();
	
	
	bool bind();
	
	bool used = false;
	GLuint ProgramID = -1;
};

#endif

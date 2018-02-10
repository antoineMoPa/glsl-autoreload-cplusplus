#ifndef IMAGE_H
#define IMAGE_H

#include <GL/glew.h>
#include <GL/gl.h>

using namespace std;

/*
  Thanks to
  http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/
*/
class Image{
public:
	Image();
	
	/* generate empty image */
	Image(int w, int h);

	void init(int w, int h);

	void delete_ressources();

	void resize(int w, int h);
	
	GLuint get_id();

	/* 
	   Loads a bitmap (.bmp) file 
	   Todo: Load other formats (like png)
	*/
	bool load(const char * filename);

	/* 
	   Todo: Load other formats (like png)
	*/
	bool save(const char * filename);
	
	void generate();
	
	void bind();

	void bind(GLuint index, const char * name);
	
	void bind(GLuint shader_id, GLuint index,const char * name);
	
	~Image();
	
	GLuint textureID;
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int imageSize; // width*height*3
	unsigned char * data = nullptr;
};

#endif

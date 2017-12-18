#include "ShaderGif.h"

int main(int argc, char ** argv)
{
    ShaderGif::w = 640;
    ShaderGif::h = 480;
    ShaderGif::start(argc,argv);

	ShaderGif::apploop();
    
    return 0;
}


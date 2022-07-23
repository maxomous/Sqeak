#include "glcore.h"
using namespace std; 
 
 
void glClearErrors() 
{
	while(glGetError() != GL_NO_ERROR);
}

bool glErrors(const char* function, const char* file, int line) 
{
	bool noErrors = true;
	while(GLenum err = glGetError()) 
	{
		noErrors = false;
		string error;
		switch(err) {
			case GL_INVALID_OPERATION:				error="INVALID_OPERATION";				break;
			case GL_INVALID_ENUM:					error="INVALID_ENUM";					break;
			case GL_INVALID_VALUE:					error="INVALID_VALUE";					break;
			case GL_OUT_OF_MEMORY:					error="OUT_OF_MEMORY";					break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:	error="INVALID_FRAMEBUFFER_OPERATION";	break;
			case GL_STACK_OVERFLOW:					error="GL_STACK_OVERFLOW";				break;
			case GL_STACK_UNDERFLOW:				error="GL_STACK_UNDERFLOW";				break;
			case GL_CONTEXT_LOST:					error="GL_CONTEXT_LOST";				break;
			case GL_TABLE_TOO_LARGE:				error="GL_TABLE_TOO_LARGE";				break;
		}
		cerr << "[OpenGL Error] (" << err << "): GL_" << error.c_str() << "\n" << function << " " << file << ":" << line << endl;
	}
	return noErrors;
}

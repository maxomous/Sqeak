
#include <iostream>
#include <string>
using namespace std;

#include "glcore.h"
#include "renderer.h"

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
		cerr << "{OpenGL Error] (" << err << "): GL_" << error.c_str() << "\n" << function << " " << file << ":" << line << endl;
	}
	return noErrors;
}

Renderer::Renderer()
	: m_Primitive(GL_TRIANGLES)
{}
Renderer::Renderer(int primitive)
	: m_Primitive(primitive)
{}

void Renderer::Clear()
{
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
    Draw(va, ib, shader, ib.GetCount());
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader, uint drawCount) const
{
	// bind shader
	shader.Bind();
	// bind vertex array
	va.Bind();
	// bind index buffer
	ib.Bind();
	
	// number of indices (not vertices)
	GLCall(glDrawElements(m_Primitive, drawCount, GL_UNSIGNED_INT, nullptr));
}

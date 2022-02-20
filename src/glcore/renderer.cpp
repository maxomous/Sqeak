
#include <iostream>
#include <string>
using namespace std;

#include "glcore.h"
#include "renderer.h"
/*
Renderer::Renderer(int primitive)
	: m_Primitive(primitive)
{}
*/
void Renderer::Clear()
{
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::Draw(uint primitive, const VertexArray& va, const IndexBuffer& ib, const Shader& shader)
{
    Draw(primitive, va, ib, shader, 0, ib.GetCount());
}

void Renderer::Draw(uint primitive, const VertexArray& va, const IndexBuffer& ib, const Shader& shader, uint offset, uint drawCount)
{
	// bind shader
	shader.Bind();
	// bind vertex array
	va.Bind();
	// bind index buffer
	ib.Bind();
	    
	// number of indices (not vertices)
	GLCall(glDrawElements(primitive, drawCount, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * offset)));
}

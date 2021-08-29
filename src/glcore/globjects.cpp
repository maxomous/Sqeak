
#include <iostream>
#include <string>

#include "glcore.h"
#include "globjects.h"

// static
VertexBuffer::VertexBuffer(const void* data, uint size)
{
	// generate a vertex buffer
	// bind the buffer
    GLCall(glGenBuffers(1, &m_RendererID));
    // bind data to buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
    // state how data should be read
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW)); // use dynamic for constantly changing
}
// dynamic
VertexBuffer::VertexBuffer(uint size)
{
	// generate a vertex buffer
	// bind the buffer
    GLCall(glGenBuffers(1, &m_RendererID));
    // bind data to buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
    // state how data should be read
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW)); // use dynamic for constantly changing
}

VertexBuffer::~VertexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
    // bind data to buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}
void VertexBuffer::Unbind() const
{
    // bind data to buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::DynamicUpdate(GLintptr offset, GLsizeiptr size, const void* data) const
{
	Bind();
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}


IndexBuffer::IndexBuffer(const uint* data, uint count)
	: m_Count(count)
{
	ASSERT(sizeof(uint) == sizeof(GLuint));
	// generate a index buffer object
	// bind the buffer
    GLCall(glGenBuffers(1, &m_RendererID));
    // bind data to buffer
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
    // state how data should be read
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint), data, GL_STATIC_DRAW));
}
IndexBuffer::~IndexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void IndexBuffer::Bind() const
{
    // bind data to buffer
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
}
void IndexBuffer::Unbind() const
{
    // bind data to buffer
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

uint VertexBufferElement::GetSizeOfType(uint type) 
{
	switch (type)
	{
		case GL_FLOAT: 			return 4;
		case GL_UNSIGNED_INT: 	return 4;
		case GL_UNSIGNED_BYTE: 	return 1;
	}
	ASSERT(false);
	return 0;
}


VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &m_RendererID));
	
} 
VertexArray::~VertexArray()
{
	GLCall(glGenVertexArrays(1, &m_RendererID));
}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(m_RendererID));
}
 
void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray(0));
}
	
void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	Bind();
	vb.Bind();
	const auto& elements = layout.GetElements();
	uint offset = 0;
	for (uint i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		
		GLCall(glEnableVertexAttribArray(element.id));
		GLCall(glVertexAttribPointer(element.id, element.count, element.type, element.normalised, layout.GetStride(), (const void*)offset));
		/* glsl (3.3+)
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalised, layout.GetStride(), (const void*)offset));
		*/
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}

#pragma once

#include <stdlib.h>

#define ASSERT(x) if (!(x)) exit(1);
#define GLCall(x) glClearErrors();\
	x;\
	ASSERT(glErrors(#x, __FILE__, __LINE__));

void glClearErrors();
bool glErrors(const char* function, const char* file, int line);

//forward declaration
class VertexArray;
class IndexBuffer;
class Shader;
   
class Renderer
{
public:
	Renderer();
	Renderer(int primitive);
	static void Clear();
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader, uint drawCount) const;
private:
	uint m_Primitive;
};

#pragma once

#include <stdlib.h>

//forward declaration
class VertexArray;
class IndexBuffer;
class Shader;
   
class Renderer
{
public:
	//Renderer(int primitive = GL_TRIANGLES);
	static void Clear();
	static void Draw(uint primitive, const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
    static void Draw(uint primitive, const VertexArray& va, const IndexBuffer& ib, const Shader& shader, uint offset, uint drawCount);	
};

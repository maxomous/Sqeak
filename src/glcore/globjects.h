#pragma once

#include <vector>

class VertexBuffer
{
private:
	uint m_RendererID;
    bool isDynamicBuffer;
    void Resize(uint size, const void* data = nullptr) ;
	
public:
	// static
	VertexBuffer(uint size, const void* data);
	// dynamic
	VertexBuffer(uint size);
	~VertexBuffer();
    
	void Bind() const;
	void Unbind() const;
	void DynamicUpdate(GLintptr offset, GLsizeiptr size, const void* data) const;
};


class IndexBuffer
{
private:
	uint m_RendererID;
	uint m_Count;
    void Resize(uint count, const uint* data);
	
public:
	IndexBuffer(uint count, const uint* data);
	~IndexBuffer();
	
	void Bind() const;
	void Unbind() const;
	
	inline uint GetCount() const { return m_Count; };
};

struct VertexBufferElement
{
	uint type;
	uint count;
	unsigned char normalised;
	// this should be removed for glsl(3.3+)
	uint id;
	
	static uint GetSizeOfType(uint type);
};

template<typename T>
struct identity { typedef T type; };

class VertexBufferLayout
{
public:
	VertexBufferLayout()
		: m_Stride(0) {}
		
	template <typename T>
	void Push(uint id, uint count)
	{
		Push(id, count, identity<T>());
	}

	inline const std::vector<VertexBufferElement> GetElements() const { return m_Elements; };
	inline uint GetStride() const { return m_Stride; };
	
private:
	std::vector<VertexBufferElement> m_Elements;
	uint m_Stride;
	
	
	template <typename T>
	void Push(uint id, uint count, identity<T>)
	{
        std::cerr << "Error: Unknown push type" << std::endl;
		exit(1);
	}
	void Push(uint id, uint count, identity<float>)
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE, id });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
	}
	void Push(uint id, uint count, identity<uint>)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE, id });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
	}
	void Push(uint id, uint count, identity<unsigned char>)
	{
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE, id });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
	}
};


class VertexArray
{
public:
	VertexArray();
	~VertexArray();
	
	void Bind() const;
	void Unbind() const;
	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	
private:
	uint m_RendererID;
};

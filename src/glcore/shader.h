
#pragma once

#include <unordered_map>

class Shader
{
public:
    Shader(const std::string& vertexShader, const std::string& fragmentShader);
	~Shader();
	
	void Bind() const;
	void Unbind() const;
	// only required for a testbasic - can be removed
	int GetID() { return m_RendererID; }
	// in later versions of glsl (3.3+) we dont require this function as we can use glEnableVertexAttribArray (see VertexArray::AddBuffer()) 
	int GetAttribLocation(const std::string& name);
	
	void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform2f(const std::string& name, float v0, float v1);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniform1iv(const std::string& name, int size, int values[]);
    void SetUniform2fv(const std::string& name, glm::vec2 values);
    void SetUniform3fv(const std::string& name, glm::vec3 values);
    void SetUniform4fv(const std::string& name, glm::vec4 values);
    void SetUniformMat3f(const std::string& name, const glm::mat3 matrix);
	void SetUniformMat4f(const std::string& name, const glm::mat4 matrix);
	
private:
	std::string m_FilePath; // kept for debugging
	uint m_RendererID;
	std::unordered_map<std::string, int> m_UniformLocationCache;

	uint CompileShader(uint type, const std::string& source);
	uint CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	int GetUniformLocation(const std::string& name);
};

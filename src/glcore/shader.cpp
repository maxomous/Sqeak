
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <assert.h>
using namespace std;  

#include "glcore.h"
#include "shader.h"
 
 
Shader::Shader(const string& vertexShader, const string& fragmentShader)
	: m_RendererID(0)
{
	m_RendererID = CreateShader(vertexShader, fragmentShader);
}
Shader::~Shader()
{
	GLCall(glDeleteProgram(m_RendererID));
}

uint Shader::CompileShader(uint type, const string& source)
{
	uint id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);
	
	int result = GL_FALSE;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    
    if(result == GL_FALSE) { 
		int length = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        
        std::vector<char> message(length);
		glGetShaderInfoLog(id, length, &length, &message[0]);

		cout << "Failed to compile ";
        if(type == GL_VERTEX_SHADER) cout << " Vertex shader:";
        else if(type == GL_FRAGMENT_SHADER) cout << "Fragment shader:";
        else cout << "Unkown shader:";
        cout << "Length: " << length << " Message: " << message.data() << endl << endl;
        
		glDeleteShader(id);
		return 0;
	}
	return id;
}
 
uint Shader::CreateShader(const string& vertexShader, const string& fragmentShader)
{
	uint program = glCreateProgram();
    
	uint vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	assert(vs);
	
	uint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
	assert(fs);
    
	glAttachShader(program, vs);
	glAttachShader(program, fs);
        
	glLinkProgram(program);
	glValidateProgram(program);
	
	glDeleteShader(vs);
	glDeleteShader(fs);     
    
	return program;
}

void Shader::Bind() const
{
	GLCall(glUseProgram(m_RendererID));
}
void Shader::Unbind() const
{
	GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
	GLCall(glUniform1i(GetUniformLocation(name), value));
}
void Shader::SetUniform1f(const string& name, float value)
{
	GLCall(glUniform1f(GetUniformLocation(name), value));
}
void Shader::SetUniform2f(const string& name, float v0, float v1)
{
	GLCall(glUniform2f(GetUniformLocation(name), v0, v1));
}
void Shader::SetUniform3f(const string& name, float v0, float v1, float v2)
{
	GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}
void Shader::SetUniform4f(const string& name, float v0, float v1, float v2, float v3)
{
	GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}
void Shader::SetUniform1iv(const std::string& name, int size, int values[])
{
	GLCall(glUniform1iv(GetUniformLocation(name), size, values));
}
void Shader::SetUniform2fv(const std::string& name, glm::vec2 values)
{
	GLCall(glUniform2fv(GetUniformLocation(name), 1 /*vector*/, &values[0]));
}
void Shader::SetUniform3fv(const std::string& name, glm::vec3 values)
{
	GLCall(glUniform3fv(GetUniformLocation(name), 1 /*vector*/, &values[0]));
}
void Shader::SetUniform4fv(const std::string& name, glm::vec4 values)
{
	GLCall(glUniform4fv(GetUniformLocation(name), 1 /*vector*/, &values[0]));
}
void Shader::SetUniformMat3f(const string& name, const glm::mat3 matrix)
{
	GLCall(glUniformMatrix3fv(GetUniformLocation(name), 1, /*true if not colomn major*/ GL_FALSE, &matrix[0][0]));
}
void Shader::SetUniformMat4f(const string& name, const glm::mat4 matrix)
{
	GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, /*true if not colomn major*/ GL_FALSE, &matrix[0][0]));
}

int Shader::GetUniformLocation(const string& name)
{
	// instead of requesting the id from gpu each time, we can cache it if we have searched for it already
	if(m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[name];
		
	GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
	ASSERT(location != -1);	
	 
	m_UniformLocationCache[name] = location;
	return location;
}

	// in later versions of glsl (3.3+) we dont require this function as we can use glEnableVertexAttribArray (see VertexArray::AddBuffer()) 
int Shader::GetAttribLocation(const string& name)
{
	GLCall(int id = glGetAttribLocation(m_RendererID, name.c_str()));
	if(id < 0) {
		cout << "Error: Attribute not found: " << name << " = " << id << endl;
		exit(1); 
	}
	return id;
}

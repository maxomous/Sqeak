
#pragma once

extern GLuint LoadShadersStd();
extern GLuint LoadShadersFile(const char* vertex_file_path, const char* fragment_file_path);
extern GLuint LoadShaders(std::string VertexShaderCode, std::string FragmentShaderCode);


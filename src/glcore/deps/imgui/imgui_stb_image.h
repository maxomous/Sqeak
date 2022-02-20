// imgui wrapper for image loading - MPW 2021


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#pragma once

extern bool LoadIconFromFile(GLFWwindow* window, const char* filename) ;

class ImageTexture 
{
public:
    int w = 0;
    int h = 0;
    GLuint textureID = 0;
    
    void Init(const char* location);
};

namespace ImGui
{
    IMGUI_API void  Image(ImageTexture imgT, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
    IMGUI_API void  Image(ImageTexture imgT, const float scale = 1.0f, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
    IMGUI_API bool  ImageButton(ImageTexture imgT, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding
    IMGUI_API bool  ImageButton(const ImVec2& buttonSize, const ImVec2& imgSize, ImageTexture imgT, const char* id = nullptr, const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding
    IMGUI_API bool  ImageButton(ImageTexture imgT, const float scale = 1.0f, const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding
}

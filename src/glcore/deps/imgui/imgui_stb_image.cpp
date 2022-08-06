
// imgui wrapper for image loading - MPW 2021

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "deps/stb_image/stb_image.h" 
//#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_stb_image.h"


using namespace std;

bool LoadIconFromFile(GLFWwindow* window, const char* filename) 
{  
    GLFWimage* img = new GLFWimage;
    img->width = 0;
    img->height = 0;
    // Load from file
    img->pixels = stbi_load(filename, &img->width, &img->height, NULL, 4);
    if (img->pixels == NULL)
        return false;
    // set image as icon
    glfwSetWindowIcon(window, 1, img);
    delete(img);
    return true;
}


// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture`
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void ImageTexture::Init(const char* location) 
{ 
    if(!LoadTextureFromFile(location, &textureID, &w, &h)) {
        cout << "Error: Could not find image " << location << endl;
    } 
}

namespace ImGui
{
    void  Image(ImageTexture imgT, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
        return Image((void*)(intptr_t)imgT.textureID, size, uv0, uv1, tint_col, border_col);
    }
    void  Image(ImageTexture imgT, const float scale, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
        return Image((void*)(intptr_t)imgT.textureID, ImVec2(scale*imgT.w, scale*imgT.h), uv0, uv1, tint_col, border_col);
    }
    bool  ImageButton(ImageTexture imgT, const ImVec2& size, const ImVec2& uv0,  const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col) {
        return ImageButton((void*)(intptr_t)imgT.textureID, size, uv0, uv1, frame_padding, bg_col, tint_col);
    }
    bool  ImageButton(const ImVec2& buttonSize, const ImVec2& imgSize, ImageTexture imgT, const char* id, const ImVec2& uv0,  const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col) {
        if(id) { PushID(id); }
        PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2((buttonSize.x-imgSize.x)/2.0f, (buttonSize.y-imgSize.y)/2.0f));
            bool clicked = ImageButton((void*)(intptr_t)imgT.textureID, imgSize, uv0, uv1, -1, bg_col, tint_col);
        PopStyleVar();
        if(id) { PopID(); }
        return clicked;
    }
    bool  ImageButton(ImageTexture imgT, const float scale, const ImVec2& uv0,  const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col) {
        return ImageButton((void*)(intptr_t)imgT.textureID, ImVec2(scale*imgT.w, scale*imgT.h), uv0, uv1, frame_padding, bg_col, tint_col);
    }
    
}


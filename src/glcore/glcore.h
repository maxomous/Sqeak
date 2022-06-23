#pragma once

// Imgui
#define IMGUI_DEFINE_MATH_OPERATORS 
#include "deps/imgui/imgui.h"
#include "deps/imgui/imgui_internal.h" // for vector math overrides
#include "deps/imgui/imgui_impl_glfw.h"
#include "deps/imgui/imgui_impl_opengl3.h"
#include "deps/imgui/imgui_stdlib.h"
#include "deps/imgui/imgui_stb_image.h"
// Fonts for Imgui
#include "deps/imgui/fonts/font_geomanist.h"
// Imgui add-ons
#include "deps/imgui_modules/imgui_modules.h"

// OpenGL Specification
#include <GL/glew.h>
// Windowing
#include <GLFW/glfw3.h>

// GL Maths Library
#include "glm.h"
#include "camera.h"
#include "glsys.h"
#include "renderer.h"
#include "shader.h"
#include "globjects.h"
#include "texture.h"
#include "modelloader.h"
#include "material.h"


#define ASSERT(x) if (!(x)) exit(1);
#define GLCall(x) glClearErrors();\
	x;\
	ASSERT(glErrors(#x, __FILE__, __LINE__));

void glClearErrors();
bool glErrors(const char* function, const char* file, int line);

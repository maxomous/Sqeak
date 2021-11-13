#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS 
#include "deps/imgui/imgui.h"
#include "deps/imgui/imgui_internal.h" // for vector math overrides
#include "deps/imgui/imgui_impl_glfw.h"
#include "deps/imgui/imgui_impl_opengl3.h"
#include "deps/imgui/imgui_stdlib.h"
#include "deps/imgui/imgui_stb_image.h"
#include "deps/imgui/font_geomanist.h"

// OpenGL Specification
#include <GL/glew.h>
// Windowing
#include <GLFW/glfw3.h>
// GL Maths Library
#define GLM_FORCE_CTOR_INIT // ensure 
#include <glm/glm.hpp> 						
#include <glm/gtc/matrix_transform.hpp> 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp> 

#include "camera.h"
#include "glsys.h"
#include "renderer.h"
#include "shader.h"
#include "globjects.h"
#include "texture.h"
#include "modelloader.h"
#include "material.h"

static inline std::ostream& operator<<(std::ostream& os, const glm::vec2& p) { os << "(" << p.x << ", " << p.y << ")"; return os; }
static inline std::ostream& operator<<(std::ostream& os, const glm::vec3& p) { os << "(" << p.x << ", " << p.y << ", " << p.z << ")"; return os; }
static inline std::ostream& operator<<(std::ostream& os, const glm::vec4& p) { os << "(" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")"; return os; }

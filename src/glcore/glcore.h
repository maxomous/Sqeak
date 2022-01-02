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


// This takes a std::string of 2/3 values seperated by commas (.000,0.000,2.222) and will return a vec2 / vec3
glm::vec2 stoVec2(const std::string& msg);
glm::vec3 stoVec3(const std::string& msg);


#define IM_VEC2_CLASS_EXTRA                                                     \
        ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }                        \
        operator glm::vec2() const { return glm::vec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                     \
        ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }      \
        operator glm::vec4+() const { return glm::vec4(x,y,z,w); }


static inline std::ostream& operator<<(std::ostream& os, const glm::vec2& p) { os << "(" << p.x << ", " << p.y << ")"; return os; }
static inline std::ostream& operator<<(std::ostream& os, const glm::vec3& p) { os << "(" << p.x << ", " << p.y << ", " << p.z << ")"; return os; }
static inline std::ostream& operator<<(std::ostream& os, const glm::vec4& p) { os << "(" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")"; return os; }

static inline glm::vec2 Vec2(glm::vec3 v) { return { v.x, v.y }; }
static inline glm::vec3 Vec3(glm::vec2 v) { return { v.x, v.y, 0.0f }; }

namespace glm {
    static inline bool operator <(const glm::vec2& l, const glm::vec2& r) { return l.x < r.x && l.y < r.y; }
    static inline bool operator >(const glm::vec2& l, const glm::vec2& r) { return l.x > r.x && l.y > r.y; }
}

static inline float hypot(glm::vec2 v) { return sqrtf(v.x*v.x + v.y*v.y); }
static inline float hypot(glm::vec3 v) { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); }

static inline glm::vec2 roundVec2(float roundto, const glm::vec2& input) {
    double x = (double)input.x / (double)roundto;
    double y = (double)input.y / (double)roundto;
    x = round(x) * (double)roundto;
    y = round(y) * (double)roundto;
    return {x, y};
}

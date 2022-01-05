#pragma once
#include <ostream>

// GL Maths Library
#define GLM_FORCE_CTOR_INIT // ensure 
#include <glm/glm.hpp> 						
#include <glm/gtc/matrix_transform.hpp> 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp> 

// This takes a std::string of 2/3 values seperated by commas (.000,0.000,2.222) and will return a vec2 / vec3
glm::vec2 stoVec2(const std::string& msg);
glm::vec3 stoVec3(const std::string& msg);

// define additional ImGui constructors
#define IM_VEC2_CLASS_EXTRA                                                 \
    ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }                        \
    operator glm::vec2() const { return glm::vec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                 \
    ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }      \
    operator glm::vec4+() const { return glm::vec4(x,y,z,w); }


/*
struct glm::vec2 
{
    operator point2D() const;
    float x, y;
};
*/
/*
namespace glm {
    vec2(point2D p) { return glm::vec2(p.x, p.y); }
    vec3(point3D p) { return glm::vec3(p.x, p.y, p.z); }
}


class glm {
 public:
    void draw();
};

glm::vec2 glm::vec2() { return new Shape(); }
*/

//operator glm::vec2() { return glm::vec2(x,y); }
//operator glm::vec3() { return glm::vec3(x,y,z); }

//point2D(const glm::vec2& p) { x = p.x; y = p.y; }         
//point3D(const glm::vec3& p) { x = p.x; y = p.y; z = p.z; }


static inline std::ostream& operator<<(std::ostream& os, const glm::vec2& p) { os << "(" << p.x << ", " << p.y << ")"; return os; }
static inline std::ostream& operator<<(std::ostream& os, const glm::vec3& p) { os << "(" << p.x << ", " << p.y << ", " << p.z << ")"; return os; }
static inline std::ostream& operator<<(std::ostream& os, const glm::vec4& p) { os << "(" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")"; return os; }

namespace glm {
    static inline bool operator <(const glm::vec2& l, const glm::vec2& r) { return l.x < r.x && l.y < r.y; }
    static inline bool operator >(const glm::vec2& l, const glm::vec2& r) { return l.x > r.x && l.y > r.y; }
}

static inline glm::vec2 Vec2(glm::vec3 v) { return { v.x, v.y }; }
static inline glm::vec3 Vec3(glm::vec2 v) { return { v.x, v.y, 0.0f }; }



static inline float hypot(glm::vec2 v) { return sqrtf(v.x*v.x + v.y*v.y); }
static inline float hypot(glm::vec3 v) { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); }

static inline glm::vec2 roundVec2(float roundto, const glm::vec2& input) {
    double x = (double)input.x / (double)roundto;
    double y = (double)input.y / (double)roundto;
    x = round(x) * (double)roundto;
    y = round(y) * (double)roundto;
    return {x, y};
}

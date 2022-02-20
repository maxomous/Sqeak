
#pragma once

#include "../glcore/glcore.h"

// basic shaders for just position and colour. No lighting. No normals, No texture coords
static const std::string Viewer_BasicVertexShader = R"(
// #version 140
#version 300 es

in vec3 in_Position;
in vec3 in_Colour;

out vec3 v_Colour;

uniform mat4 u_MVP;

void main(void)
{
	gl_Position = u_MVP * vec4(in_Position, 1.0f);
    v_Colour = in_Colour;
}
)";

static std::string Viewer_BasicFragmentShader = R"(
// #version 140
#version 300 es

precision highp float; // needed only for version 1.30

in vec3 v_Colour;
out vec4 out_Colour;

void main(void)
{
	out_Colour = vec4(v_Colour, 1.0f);
}
)";




struct Vertex {
    Vertex(glm::vec3 pos, glm::vec3 col) : position(pos), colour(col) {}
    glm::vec3 position;
    glm::vec3 colour;
};


class Shape
{
public:     
    void Append(const Shape& shape);
    // returns a new transformed Shape of Shape
    // translate, scale & rotate about x axis and then z axis
    Shape Transform(const glm::vec3& translate = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, const glm::vec2& rotate = { 0.0f, 0.0f }) const;
    // returns vertex by index (e.g. shape[2])
    const glm::vec3& operator [](size_t i) const    { assert(i < m_Vertices.size()); return m_Vertices[i]; }
    // returns number of vertices
    size_t Size() const                             { return m_Vertices.size(); }
    
private:
    void AddVertex(const glm::vec3& vertex);
    //void AddIndices(uint indices);
    //void AddNormal(const glm::vec3& normal);
    //void AddTextureCoord(const glm::vec2& textureCoord);
    
    std::vector<glm::vec3> m_Vertices;
    //std::vector<uint> m_Indices;
    //std::vector<glm::vec3> m_Normals;
    //std::vector<glm::vec2> m_TextureCoords;
    
    friend class Shapes;
};

class Shapes
{
public:
    Shapes(float arcAngle = 10.0f);
    
    // use GL_LINES
    const Shape& Wireframe_Circle()     { return m_Wireframe_Circle; }
    const Shape& Wireframe_Square()     { return m_Wireframe_Square; }
    const Shape& Wireframe_Cylinder()   { return m_Wireframe_Cylinder; }
    const Shape& Wireframe_Cube()       { return m_Wireframe_Cube; }
    // use GL_TRIANGLES
    const Shape& Face_Circle()          { return m_Face_Circle; }
    const Shape& Face_Square()          { return m_Face_Square; }
    // use GL_TRIANGLES
    const Shape& Body_Cylinder()        { return m_Body_Cylinder; }
    const Shape& Body_Cube()            { return m_Body_Cube; }
    const Shape& Body_Sphere()          { return m_Body_Sphere; }

private:
    // angle in degrees used for determining points on arcs
    float m_ArcAngle;
    // use GL_LINES
    Shape m_Wireframe_Circle;
    Shape m_Wireframe_Square;
    Shape m_Wireframe_Cylinder;
    Shape m_Wireframe_Cube;
    // use GL_TRIANGLES
    Shape m_Face_Circle;
    Shape m_Face_Square;
    // use GL_TRIANGLES
    Shape m_Body_Cylinder;
    Shape m_Body_Cube;
    Shape m_Body_Sphere;
    // wireframes
    void Init_Wireframe_Circle();
    void Init_Wireframe_Square();
    void Init_Wireframe_Cylinder();
    void Init_Wireframe_Cube();
    // faces
    void Init_Face_Circle();
    void Init_Face_Square();
    // bodies
    void Init_Body_Cylinder();
    void Init_Body_Cube();
    void Init_Body_Sphere();
};



class DynamicBuffer
{
public:
    struct DynamicVertexList {
        std::vector<glm::vec3> position; 
        glm::vec3 colour; 
    };

    DynamicBuffer(GLenum primitiveType, int maxVertices, int maxIndices);
    
    void Resize(int maxVertices, int maxIndices);
    void ClearVertices();
    void AddVertex(const glm::vec3& position, const glm::vec3& colour, bool isOutline = false);
    void AddCursor(Settings& settings, bool isValid, glm::vec2 pos);
    void AddGrid(Settings& settings);
    void AddAxes(float size, glm::vec3 origin);

    void AddShapeOutline(const Shape& shape, glm::vec3 colour, const glm::vec3& translate = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, const glm::vec2& rotate = { 0.0f, 0.0f }); 
    void AddShape(const Shape& shape, glm::vec3 colour, const glm::vec3& translate = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, const glm::vec2& rotate = { 0.0f, 0.0f }, bool isOutline = false);
    void AddPathAsLines(const std::vector<glm::vec3>& vertices, glm::vec3 colour, const glm::vec3& zeroPosition);
    void AddDynamicVertexList(const std::vector<DynamicBuffer::DynamicVertexList>* dynamicLineLists, const glm::vec3& zeroPosition);

    void Update();
    void Draw(glm::mat4& proj, glm::mat4& view, bool isDrawOutline = false);
    
private:
    GLenum m_PrimitiveType;
    uint m_MaxVertexCount;
    uint m_MaxIndexCount;
    uint m_VertexCount = 0;
    uint m_OutlineVertexCount = 0;
    
    std::vector<Vertex> m_Vertices;
    
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VertexBuffer;
    std::unique_ptr<IndexBuffer> m_IndexBuffer;
}; 

// forward declare
class Event_Viewer_AddLineLists;
class Event_Viewer_AddPointLists;
class Event_Set2DMode;

class Viewer
{
public:
    Viewer();

    glm::vec3 GetWorldPosition(glm::vec2 px);
    float ScaleToPx(float size) { return size * (m_Camera.GetZoom() / Window::GetHeight()); } 
    
    void SetCursor(bool isValid, glm::vec2 worldCoords);
    void SetPath(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& colours);
    void Clear();    
    
    void Update(Settings& settings, float dt);
    void Render(Settings& settings);
    void ImGuiRender(Settings& settings);
       
private:
    bool m_Initialised = false;
    bool m_Show = true;
    
    Camera_CentreObject m_Camera;
    
    std::unique_ptr<EventHandler<Event_WindowResize>> event_WindowResize;
    std::unique_ptr<EventHandler<Event_MouseScroll>> event_MouseScroll;
    std::unique_ptr<EventHandler<Event_MouseMove>> event_MouseDrag;
    std::unique_ptr<EventHandler<Event_KeyInput>> event_Keyboard;
    std::unique_ptr<EventHandler<Event_Viewer_AddLineLists>> event_AddLineLists;
    std::unique_ptr<EventHandler<Event_Viewer_AddPointLists>> event_AddPointLists;
    std::unique_ptr<EventHandler<Event_Set2DMode>> event_Set2DMode;
    //std::unique_ptr<EventHandler<Event_UpdateCamera>> event_UpdateCamera;
    
    // 3d shape primitives
    Shapes m_Shapes;
    // tool
    Shape m_Shape_Tool;
    Shape m_Shape_ToolHolder;; 
    Shape m_Shape_Tool_Wireframe;
    Shape m_Shape_ToolHolder_Wireframe;
    
    DynamicBuffer m_DynamicPoints = { GL_POINTS, 512, 512 };
    DynamicBuffer m_DynamicLines = { GL_LINES, 1024, 1024 };
    DynamicBuffer m_DynamicBodies = { GL_TRIANGLES, 4096, 4096 };
    
    std::vector<DynamicBuffer::DynamicVertexList>* m_DynamicLineLists = nullptr;
    std::vector<DynamicBuffer::DynamicVertexList>* m_DynamicPointLists = nullptr;
    //std::vector<glm::vec2> m_Shape;
    //std::vector<glm::vec2> m_ShapeOffset;
    //bool m_ShapeIsLoop = false;
    
    std::pair<bool, glm::vec2> m_Cursor2DPos;
    
    // static buffer for path
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VertexBuffer;
    std::unique_ptr<IndexBuffer> m_IndexBuffer;
    
    glm::mat4 m_Proj;
    glm::mat4 m_View;
    
    int m_DrawCount = 0;
    int m_DrawMax = 0;
    
    
    MaterialPicker m_MaterialPicker;
      // set initial material
    Material m_Material = m_MaterialPicker.GetCurrentMaterial();
    
    float m_LineWidth_Lines = 1.0f;
    float m_LineWidth_Bodies = 1.0f;
    int m_DepthFunction = 3;
    
    void Draw2DText(const char* label, glm::vec3 position);
    void Draw2DAxesLabels(glm::vec3 position, float axisLength);
    
    void ClearDynamicVertices();
    void AddVertex(const glm::vec3& position, const glm::vec3& colour);
    void AddDynamicAxes(float axisSize, DynamicBuffer& buffer, glm::vec3 origin);
    void DrawGrid(Settings& settings, DynamicBuffer& buffer);
    
    void DynamicBufferInit();
    void DrawDynamicBuffer();
    void DrawPath();

};


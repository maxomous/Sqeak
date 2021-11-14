
#pragma once

#include "../glcore/glcore.h"





static std::string Viewer_VertexShader = R"(
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

// phong lighting model

static std::string Viewer_FragmentShader = R"(
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
    Vertex(glm::vec3 p, glm::vec3 c) : position(p), colour(c) {}
    glm::vec3 position;
    glm::vec3 colour;
};


class DynamicBuffer
{
public:
    DynamicBuffer(GLenum primitiveType, int maxVertices, int maxIndices);
    
    void Resize(int maxVertices, int maxIndices);
    void ClearVertices();
    void AddVertex(const glm::vec3& position, const glm::vec3& colour);
    void AddGrid(Settings& settings);
    void AddAxes(float size, glm::vec3 origin);
    void AddShape(const std::vector<glm::vec3>& shape, glm::vec3 colour, const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f), float rotateX = 0.0f, float rotateZ = 0.0f);

    void Update();
    void Draw(glm::mat4& proj, glm::mat4& view);
    
private:
    GLenum m_PrimitiveType;
    uint m_MaxVertexCount;
    uint m_MaxIndexCount;
    uint m_VertexCount = 0;
    
    std::vector<Vertex> m_Vertices;
    
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VertexBuffer;
    std::unique_ptr<IndexBuffer> m_IndexBuffer;
}; 


class Viewer
{
public:
    Viewer();
    ~Viewer();

    void SetPath(Settings& settings, std::vector<glm::vec3>& vertices, std::vector<uint>& indices);
    void Clear();    
    
    void Update(Settings& settings, float dt);
    void Render();
    void ImGuiRender(Settings& settings);
    
private:
    bool m_Initialised = false;
    bool m_Show = true;
    
    Camera_CentreObject m_Camera;
    
    std::unique_ptr<EventHandler<Event_WindowResize>> event_WindowResize;
    std::unique_ptr<EventHandler<Event_MouseScroll>> event_MouseScroll;
    std::unique_ptr<EventHandler<Event_MouseMove>> event_MouseDrag;
    std::unique_ptr<EventHandler<Event_KeyInput>> event_Keyboard;
    //std::unique_ptr<EventHandler<Event_UpdateCamera>> event_UpdateCamera;
    
    DynamicBuffer m_DynamicLines = { GL_LINES, 600, 600 };
    DynamicBuffer m_DynamicFaces = { GL_TRIANGLES, 444, 444 };
    
    // static buffer for path
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VertexBuffer;
    std::unique_ptr<IndexBuffer> m_IndexBuffer;
    
    glm::mat4 m_Proj;
    glm::mat4 m_View;
    
    int m_DrawCount = 0;
    int m_DrawMax = 0;
    
        int m_LinePolygonType = 0;
        float m_Offset = 6.0f;
        int m_QuadrantSegments = 30;
        
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


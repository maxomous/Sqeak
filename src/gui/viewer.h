
#pragma once

#include "../glcore/glcore.h"





static std::string Viewer_VertexShader = R"(
#version 140

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
#version 140

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

class Viewer
{
public:
    Viewer();
    ~Viewer();
    
    void SetPath(std::vector<glm::vec3>& vertices, std::vector<uint>& indices);
    
    void Update(float dt, const glm::vec3& currentMPos, const glm::vec3& coordSysPos);
    void Render();
    void ImGuiRender(GRBLVals& grblVals);
    
    void SetGridPosition(glm::vec3 position) { m_GridPosition = position; }
    void SetGridSize(glm::vec2 size) { m_GridSize = size; }
    void SetGridSpacing(float spacing) { m_GridSpacing = spacing; }
    void SetGridColour(glm::vec3 colour) { m_GridColour = colour; }
    
private:
    bool m_Initialised = false;
    bool m_Show = true;
    
    Camera_CentreObject m_Camera;
    
    std::unique_ptr<EventHandler<Event_WindowResize>> event_WindowResize;
    std::unique_ptr<EventHandler<Event_MouseScroll>> event_MouseScroll;
    std::unique_ptr<EventHandler<Event_MouseMove>> event_MouseDrag;
    std::unique_ptr<EventHandler<Event_KeyInput>> event_Keyboard;
    //std::unique_ptr<EventHandler<Event_UpdateCamera>> event_UpdateCamera;
   
    std::unique_ptr<Shader> m_Shader_Dynamic;
    std::unique_ptr<VertexArray> m_VAO_Dynamic;
    std::unique_ptr<VertexBuffer> m_VertexBuffer_Dynamic;
    std::unique_ptr<IndexBuffer> m_IndexBuffer_Dynamic;
    
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<VertexArray> m_VAO;
    std::unique_ptr<VertexBuffer> m_VertexBuffer;
    std::unique_ptr<IndexBuffer> m_IndexBuffer;
    
    glm::mat4 m_Proj;
    glm::mat4 m_View;
    
    std::vector<Vertex> m_Vertices; // for dynamic buffer
    
    int m_DrawCount = 0;
    int m_DrawMax = 0;
    
    float m_AxisSize = 50.0f;
    
    glm::vec3   m_GridPosition = glm::vec3(-949.0f, -529.0f, -1.0f);
    glm::vec2   m_GridSize = glm::vec2(1200.0f, 600.0f);
    float       m_GridSpacing = 100.0f;
    glm::vec3   m_GridColour = glm::vec3(0.6f, 0.6f, 0.6f);
    
    void Draw2DText(const char* label, glm::vec3 position);
    void Draw2DAxesLabels(glm::vec3 position, float axisLength);
    
    void ClearDynamicVertices();
    void AddDynamicVertex(const glm::vec3& position, const glm::vec3& colour);
    void AddDynamicAxes(glm::vec3 origin);
    void DrawGrid();
    
    void DynamicBufferInit();
    void DrawDynamicBuffer();
    void DrawPath();

};


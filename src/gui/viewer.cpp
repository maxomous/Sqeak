
#include <iostream>
using namespace std;

#include "../common.h" 

/*
float angleIncr = 10; // degs

vector<glm::vec3> shape_Cylinder;
void initShape_Cylinder()
{
    for (float th = 0; th <= M_2_PI; i += angleIncr)
    {
        glm::vec3 v = { 0.5f*cos(th), -0.5f*sin(th) };
        shape_Cylinder.push_back(v);
        glm::vec3 v = { 0.5f*cos(th+angleIncr), -0.5f*sin(th+angleIncr) };
        shape_Cylinder.push_back(v);
        glm::vec3 v = { 0.0f, 0.0f };
        shape_Cylinder.push_back(v);
    }
}
*/

Viewer::Viewer() 
  : m_Camera(Window::GetWidth(), Window::GetHeight(), glm::vec3(0.0f, 0.0f, 0.0f), 80.0f)
{     
    auto WindowResizeEvent = [&](Event_WindowResize data) {
        m_Camera.SetViewport(0, 0, data.Width, data.Height);
    };
    auto MouseScrollEvent = [&](Event_MouseScroll data) {
        if(!ActiveItem::IsViewportHovered(m_Camera))
            return;
        m_Camera.ProcessMouseScroll(data.OffsetY);
    };
    auto MouseDragEvent = [&](Event_MouseMove data) {
        // we can just use the data already sent to mouse
        (void)data;
        // rotate on mouse click
        if(Mouse::IsLeftClicked() && ActiveItem::IsViewport(m_Camera)) {
            m_Camera.ChangeDirection(Mouse::GetPositionDif());
        } 
        // pan on mouse middle click
        else if(Mouse::IsMiddleClicked() && ActiveItem::IsViewport(m_Camera)) { 
            m_Camera.Move(m_Camera.GetWorldVector(Mouse::GetPositionDif()));		
        } 
    };
    auto KeyboardEvent = [&](Event_KeyInput data) {
        //bool ctrl = data.Modifier & GLFW_MOD_CONTROL;
        
        if(data.Action == GLFW_PRESS || data.Action == GLFW_REPEAT)  // GLFW_RELEASE / GLFW_REPEAT
        {
           /* switch (data.Key)
            {
                case GLFW_KEY_W:
                    break;
            }*/
        }
    };
   /* auto UpdateCameraEvent = [&](Event_SettingsUpdated data) {
        if(data.type != Event_SettingType::CoordSystems)
            return;
        m_Camera.SetCentre(data.position);
        // data.angle
        grblVals.coords.homeCoords[0], 0.0f 
    };
    */
    event_WindowResize = make_unique<EventHandler<Event_WindowResize>>(WindowResizeEvent);
    event_MouseScroll = make_unique<EventHandler<Event_MouseScroll>>(MouseScrollEvent);
    event_MouseDrag = make_unique<EventHandler<Event_MouseMove>>(MouseDragEvent);
    event_Keyboard = make_unique<EventHandler<Event_KeyInput>>(KeyboardEvent);
    //event_UpdateCamera = make_unique<EventHandler<Event_SettingsUpdated>>(UpdateCameraEvent);
    
    // dont draw vertices outside of our visible depth
    glEnable(GL_DEPTH_TEST); 
    // dont draw triangles facing the wrong way 
    glEnable(GL_CULL_FACE);  
    
    m_Camera.SetNearFar(0.1f, 5000.0f);
    m_Camera.SetZoomMinMax(1.0f, 3000.0f);
    m_Camera.SetZoom(2000.0f);
    
    DynamicBufferInit();
}
Viewer::~Viewer()
{
}



const int m_DynVertexMax = 100;
const int maxDynIndexCount = m_DynVertexMax;
uint m_DynVertexCount = 0;


void Viewer::DynamicBufferInit()
{
    std::array<uint, maxDynIndexCount> indices;
    for (int i = 0; i < maxDynIndexCount; i++)
        indices[i] = i;
    
    m_Shader_Dynamic = make_unique<Shader>(Viewer_VertexShader, Viewer_FragmentShader);
    // make dynamic vertex buffer
    m_VertexBuffer_Dynamic = make_unique<VertexBuffer>(m_DynVertexMax * sizeof(Vertex));
    VertexBufferLayout layout;
    
    layout.Push<float>(m_Shader_Dynamic->GetAttribLocation("in_Position"), 3);
    layout.Push<float>(m_Shader_Dynamic->GetAttribLocation("in_Colour"), 3);
    
    m_VAO_Dynamic = make_unique<VertexArray>();
    m_VAO_Dynamic->AddBuffer(*m_VertexBuffer_Dynamic, layout);
    
    m_IndexBuffer_Dynamic = make_unique<IndexBuffer>(indices.data(), maxDynIndexCount);

    m_Vertices.reserve(m_DynVertexMax);
}

void Viewer::SetPath(std::vector<glm::vec3>& positions, std::vector<uint>& indices)
{
    vector<Vertex> vertices;
    vertices.reserve(positions.size());
    
    glm::vec3 pathColour = { 0.0f, 0.2f, 0.8f };
    
    for (size_t i = 0; i < positions.size(); i++)
    {
        vertices.emplace_back(positions[i], pathColour);
    }
    
    
    m_Shader.reset(new Shader(Viewer_VertexShader, Viewer_FragmentShader));
   
    m_VertexBuffer.reset(new VertexBuffer(vertices.data(), vertices.size() * sizeof(Vertex)));
    VertexBufferLayout layout;
    
    layout.Push<float>(m_Shader->GetAttribLocation("in_Position"), 3);
    layout.Push<float>(m_Shader->GetAttribLocation("in_Colour"), 3);
    
    m_VAO.reset(new VertexArray());
    m_VAO->AddBuffer(*m_VertexBuffer, layout);
    
    m_IndexBuffer.reset(new IndexBuffer(indices.data(), indices.size()));
    
    m_DrawCount = m_DrawMax = m_IndexBuffer->GetCount();
    
    m_Initialised = true;
}

void Viewer::Draw2DText(const char* label, glm::vec3 position)
{
    pair<bool, glm::vec2> LabelPos = m_Camera.GetScreenCoords(position);
    // centre letters
    ImVec2 charSize = ImGui::CalcTextSize(label);
    glm::vec2 charOffset = { charSize.x/2, charSize.y/2 };
    
    if(LabelPos.first) {
        glm::vec2 pos2D = Window::InvertYCoord(LabelPos.second) - charOffset;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        drawList->AddText(ImVec2(pos2D.x, pos2D.y), IM_COL32(255, 255, 255, 255), label);
    }
}

void Viewer::Draw2DAxesLabels(glm::vec3 position, float axisLength)
{
    Draw2DText("X", glm::vec3(axisLength, 0.0f, 0.0f) + position);
    Draw2DText("Y", glm::vec3(0.0f, axisLength, 0.0f) + position);
    Draw2DText("Z", glm::vec3(0.0f, 0.0f, axisLength) + position);
}

void Viewer::ClearDynamicVertices()
{
    m_Vertices.clear();
    m_DynVertexCount = 0;
}

void Viewer::AddDynamicVertex(const glm::vec3& position, const glm::vec3& colour)
{
    m_Vertices.emplace_back( position, colour ); 
    m_DynVertexCount++;
}

void Viewer::AddDynamicAxes(glm::vec3 origin)
{
    // draw axis
    AddDynamicVertex(origin + glm::vec3(0.0f,        0.0f,        0.0f), { 1.0f, 0.0f, 0.0f });
    AddDynamicVertex(origin + glm::vec3(m_AxisSize, 0.0f,        0.0f), { 1.0f, 0.0f, 0.0f });
    AddDynamicVertex(origin + glm::vec3(0.0f,        0.0f,        0.0f), { 0.0f, 1.0f, 0.0f });
    AddDynamicVertex(origin + glm::vec3(0.0f,    m_AxisSize,     0.0f), { 0.0f, 1.0f, 0.0f });
    AddDynamicVertex(origin + glm::vec3(0.0f,        0.0f,        0.0f), { 0.0f, 0.0f, 1.0f });
    AddDynamicVertex(origin + glm::vec3(0.0f,        0.0f, m_AxisSize), { 0.0f, 0.0f, 1.0f });
}
 

void Viewer::DrawGrid()
{   
    if(m_GridSpacing <= 0)
        return;
    glm::vec2 gridSizeSign = glm::vec2(sign(m_GridSize.x), sign(m_GridSize.y));
        
    for (float i = 0.0f; i <= abs(m_GridSize.x); i += m_GridSpacing) {
        AddDynamicVertex(m_GridPosition + glm::vec3(gridSizeSign.x * i, 0.0f, 0.0f), m_GridColour);
        AddDynamicVertex(m_GridPosition + glm::vec3(gridSizeSign.x * i, m_GridSize.y, 0.0f), m_GridColour);
    }
    for (float j = 0.0f; j <= abs(m_GridSize.y); j += m_GridSpacing) {
        AddDynamicVertex(m_GridPosition + glm::vec3(0.0f, gridSizeSign.y * j, 0.0f), m_GridColour);
        AddDynamicVertex(m_GridPosition + glm::vec3(m_GridSize.x, gridSizeSign.y * j, 0.0f), m_GridColour);
    }
}

void Viewer::Update(float dt, const glm::vec3& currentMPos, const glm::vec3& coordSysPos)
{    
    (void)dt;
    // add axis letters
    Draw2DAxesLabels(coordSysPos, m_AxisSize);
    // reset buffer
    ClearDynamicVertices();
// ---------------------------------
    // could be in static buffer    
    DrawGrid();
    
    // draw machine zero axis
    //AddDynamicAxes(glm::vec3(0.0f, 0.0f, 0.0f));
    // draw coord system axis
    AddDynamicAxes(coordSysPos);
    
    float w = 2;
    glm::vec3 currentPosColour = { 0.8f, 0.8f, 0.2f };
    
    // bottom square
    AddDynamicVertex(currentMPos + glm::vec3(-w, -w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w, -w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w, -w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w,  w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w,  w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w,  w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w,  w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w, -w, -w), currentPosColour);
    // top square           
    AddDynamicVertex(currentMPos + glm::vec3(-w, -w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w, -w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w, -w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w,  w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w,  w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w,  w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w,  w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w, -w,  w), currentPosColour);
    // sides                
    AddDynamicVertex(currentMPos + glm::vec3(-w, -w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w, -w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w, -w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w, -w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w,  w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3( w,  w,  w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w,  w, -w), currentPosColour);
    AddDynamicVertex(currentMPos + glm::vec3(-w,  w,  w), currentPosColour);
    
    m_VertexBuffer_Dynamic->DynamicUpdate(0, m_Vertices.size() * sizeof(Vertex), m_Vertices.data());
}


void Viewer::Render()
{
    m_Proj = m_Camera.GetProjectionMatrix();
    m_View = m_Camera.GetViewMatrix();
    DrawDynamicBuffer();
    DrawPath();
}

void Viewer::DrawDynamicBuffer()
{
    Renderer renderer(GL_LINES);
    renderer.Clear();
    
    m_Shader_Dynamic->Bind();
    m_Shader_Dynamic->SetUniformMat4f("u_MVP", m_Proj * m_View * glm::mat4(1.0f));
    
    // We need more vertices
    if(m_DynVertexCount > maxDynIndexCount) {
        Log::Error("Too many vertices to display");
        return;
    }
    renderer.Draw(*m_VAO_Dynamic, *m_IndexBuffer_Dynamic, *m_Shader_Dynamic, m_DynVertexCount);
}

void Viewer::DrawPath()
{ 
    if(!m_Initialised || !m_Show)
        return;
    Renderer renderer(GL_LINE_STRIP);
    
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_MVP", m_Proj * m_View * glm::mat4(1.0f));
    
    renderer.Draw(*m_VAO, *m_IndexBuffer, *m_Shader, (uint)m_DrawCount);
}
void Viewer::ImGuiRender(GRBLVals& grblVals)
{
    ImGui::Begin("GCode Viewer", NULL, ImGuiWindowFlags_None);
    
    ImGui::TextUnformatted("GCode Viewer");
    ImGui::Checkbox("Show", &m_Show);
    ImGui::SliderInt("Vertices", &m_DrawCount, 0, m_DrawMax);
    
    ImGui::Dummy(ImVec2());
    
    ImGui::TextUnformatted("Grid");
    
    HereButton(grblVals, m_GridPosition);
    ImGui::SliderFloat3("Position", &m_GridPosition[0], -3000.0f, 3000.0f);
    ImGui::SliderFloat2("Size", &m_GridSize[0], -3000.0f, 3000.0f);
    ImGui::SliderFloat("Spacing", &m_GridSpacing, 0.0f, 1000.0f);
    ImGui::SliderFloat3("Colour", &m_GridColour[0], 0.0f, 1.0f);
    /*
    glm::vec3   m_GridPosition;
    glm::vec2   m_GridSize = glm::vec2(1200, 600);
    float       m_GridSpacing = 100.0f;
    glm::vec3   m_GridColour = glm::vec3(0.6f, 0.6f, 0.6f);
    */
    ImGui::End();
}

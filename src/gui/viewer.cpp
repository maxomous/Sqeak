
#include <iostream>
using namespace std;

#include "../common.h" 

float dAngle = 10.0f; // degrees
 
vector<glm::vec3> shape_Cylinder;
vector<glm::vec3> shape_Cylinder_Outline;
void initShape_Cylinder()
{
    // bottom face - circle facing down 
    for (float th = 0.0f; th <= 360.0f; th += dAngle) {
        float th2 = th + dAngle;
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th), -0.5f * Sin(th), 0.0f));
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th2), -0.5f * Sin(th2), 0.0f));
        shape_Cylinder.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    }
    // walls
    for (float th = 0.0f; th <= 360.0f; th += dAngle) {
        float th2 = th + dAngle;
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th), -0.5f * Sin(th), 0.0f));
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th), -0.5f * Sin(th), 1.0f));
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th2), -0.5f * Sin(th2), 0.0f));
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th), -0.5f * Sin(th), 1.0f));
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th2), -0.5f * Sin(th2), 1.0f));
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th2), -0.5f * Sin(th2), 0.0f));
    }
    // top face - circle facing up
    for (float th = 0.0f; th <= 360.0f; th += dAngle) {
        float th2 = th + dAngle;
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th), -0.5f * Sin(th), 1.0f));
        shape_Cylinder.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
        shape_Cylinder.push_back(glm::vec3(0.5f * Cos(th2), -0.5f * Sin(th2), 1.0f));
    }
    
    // outline bottom
    for (float th = 0.0f; th <= 360.0f; th += dAngle) {
        float th2 = th + dAngle;
        shape_Cylinder_Outline.push_back(glm::vec3(0.5f * Cos(th), -0.5f * Sin(th), 0.0f));
        shape_Cylinder_Outline.push_back(glm::vec3(0.5f * Cos(th2), -0.5f * Sin(th2), 0.0f));
    }
    // outline top
    for (float th = 0.0f; th <= 360.0f; th += dAngle) {
        float th2 = th + dAngle;
        shape_Cylinder_Outline.push_back(glm::vec3(0.5f * Cos(th), -0.5f * Sin(th), 1.0f));
        shape_Cylinder_Outline.push_back(glm::vec3(0.5f * Cos(th2), -0.5f * Sin(th2), 1.0f));
    }
    
}

    /* Square
    // bottom square
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w, -w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w, -w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w, -w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w,  w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w,  w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w,  w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w,  w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w, -w, -w), currentPosColour);
    // top square           
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w, -w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w, -w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w, -w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w,  w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w,  w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w,  w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w,  w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w, -w,  w), currentPosColour);
    // sides                
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w, -w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w, -w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w, -w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w, -w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w,  w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3( w,  w,  w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w,  w, -w), currentPosColour);
    m_DynamicFaces.AddVertex(currentMPos + glm::vec3(-w,  w,  w), currentPosColour);
    */
   
DynamicBuffer::DynamicBuffer(GLenum primitiveType, int maxVertices, int maxIndices)
    : m_PrimitiveType(primitiveType), m_MaxVertexCount(maxVertices), m_MaxIndexCount(maxIndices) 
{ 
    std::vector<uint> indices;
    indices.reserve(m_MaxIndexCount);
    for (uint i = 0; i < m_MaxIndexCount; i++)
        indices.push_back(i);
    m_Vertices.reserve(m_MaxVertexCount);
    
    m_Shader = make_unique<Shader>(Viewer_VertexShader, Viewer_FragmentShader);
    // make dynamic vertex buffer
    m_VertexBuffer = make_unique<VertexBuffer>(m_MaxVertexCount * sizeof(Vertex));
    VertexBufferLayout layout;
    
    layout.Push<float>(m_Shader->GetAttribLocation("in_Position"), 3);
    layout.Push<float>(m_Shader->GetAttribLocation("in_Colour"), 3);
    
    m_VAO = make_unique<VertexArray>();
    m_VAO->AddBuffer(*m_VertexBuffer, layout);
    
    m_IndexBuffer = make_unique<IndexBuffer>(m_MaxIndexCount, indices.data());
    
}
  
void DynamicBuffer::Resize(int maxVertices, int maxIndices)
{ 
    m_MaxVertexCount = maxVertices;
    m_MaxIndexCount = maxIndices;
    
    std::vector<uint> indices;
    indices.reserve(m_MaxIndexCount);
    for (uint i = 0; i < m_MaxIndexCount; i++)
        indices.push_back(i);
    m_Vertices.reserve(m_MaxVertexCount);
    
    m_Shader.reset(new Shader(Viewer_VertexShader, Viewer_FragmentShader));
    // make dynamic vertex buffer
    m_VertexBuffer.reset(new VertexBuffer(m_MaxVertexCount * sizeof(Vertex)));
    VertexBufferLayout layout;
    
    layout.Push<float>(m_Shader->GetAttribLocation("in_Position"), 3);
    layout.Push<float>(m_Shader->GetAttribLocation("in_Colour"), 3);
    
    m_VAO.reset(new VertexArray());
    m_VAO->AddBuffer(*m_VertexBuffer, layout);
    
    m_IndexBuffer.reset(new IndexBuffer(m_MaxIndexCount, indices.data()));
    
}
void DynamicBuffer::ClearVertices()
{
    m_Vertices.clear();
    m_VertexCount = 0;
}

void DynamicBuffer::AddVertex(const glm::vec3& position, const glm::vec3& colour)
{
    m_Vertices.emplace_back( position, colour ); 
    m_VertexCount++;
}

void DynamicBuffer::AddGrid(Settings& settings)
{   
    ParametersList::Viewer3DParameters::Grid& grid = settings.p.viewer.grid;
    if(grid.Spacing <= 0)
        return;
    glm::vec2 gridOrientation = glm::vec2(sign(grid.Size.x), sign(grid.Size.y));
    glm::vec3 offset = settings.grblVals.ActiveCoordSys() + grid.Position;
    
    for (float i = 0.0f; i <= abs(grid.Size.x); i += grid.Spacing) {
        AddVertex(offset + glm::vec3(gridOrientation.x * i, 0.0f, 0.0f), grid.Colour);
        AddVertex(offset + glm::vec3(gridOrientation.x * i, grid.Size.y, 0.0f), grid.Colour);
    }
    for (float j = 0.0f; j <= abs(grid.Size.y); j += grid.Spacing) {
        AddVertex(offset + glm::vec3(0.0f, gridOrientation.y * j, 0.0f), grid.Colour);
        AddVertex(offset + glm::vec3(grid.Size.x, gridOrientation.y * j, 0.0f), grid.Colour);
    }
}

void DynamicBuffer::AddAxes(float size, glm::vec3 origin)
{
    // draw axis
    AddVertex(origin + glm::vec3(0.0f,      0.0f,       0.0f),  { 1.0f, 0.0f, 0.0f });
    AddVertex(origin + glm::vec3(size,      0.0f,       0.0f),  { 1.0f, 0.0f, 0.0f });
    AddVertex(origin + glm::vec3(0.0f,      0.0f,       0.0f),  { 0.0f, 1.0f, 0.0f });
    AddVertex(origin + glm::vec3(0.0f,      size,       0.0f),  { 0.0f, 1.0f, 0.0f });
    AddVertex(origin + glm::vec3(0.0f,      0.0f,       0.0f),  { 0.0f, 0.0f, 1.0f });
    AddVertex(origin + glm::vec3(0.0f,      0.0f,       size),  { 0.0f, 0.0f, 1.0f });
} 

void DynamicBuffer::AddShape(const vector<glm::vec3>& shape, glm::vec3 colour, const glm::vec3& position, const glm::vec3& scale, float rotateX, float rotateZ) 
{
    for (const glm::vec3& vertex : shape) 
    {
        glm::vec3 v = vertex * scale;
        v = glm::rotate(v, glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
        v = glm::rotate(v, glm::radians(rotateZ), glm::vec3(0.0f, 0.0f, 1.0f));
        v += position;
        AddVertex(move(v), colour);
    }
}   

void DynamicBuffer::AddPath(const vector<glm::vec2>& vertices, glm::vec3 colour, const glm::vec3& position, bool isLoop) 
{    
    for (size_t i = 1; i < vertices.size(); i++) {
        AddVertex(position + glm::vec3(vertices[i-1], 0.0f), colour);
        AddVertex(position + glm::vec3(vertices[i], 0.0f), colour);
    }
    // join end and beginning for loop
    if(isLoop && vertices.size() > 2) { 
        AddVertex(position + glm::vec3(vertices[vertices.size()-1], 0.0f), colour);
        AddVertex(position + glm::vec3(vertices[0], 0.0f), colour);
    }
}  

void DynamicBuffer::Update() {
    m_VertexBuffer->DynamicUpdate(0, m_Vertices.size() * sizeof(Vertex), m_Vertices.data());
}

void DynamicBuffer::Draw(glm::mat4& proj, glm::mat4& view) {
    Renderer renderer(m_PrimitiveType); 
    
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_MVP", proj * view * glm::mat4(1.0f));
    
    if(m_VertexCount > m_MaxIndexCount || m_VertexCount > m_MaxVertexCount) {
        Log::Info("Too many vertices to display, resizing buffer to %d vertices, %d indices", m_MaxVertexCount*2, m_MaxIndexCount*2);
        // double size of buffer
        Resize(m_MaxVertexCount*2, m_MaxIndexCount*2);
    }
    renderer.Draw(*m_VAO, *m_IndexBuffer, *m_Shader, m_VertexCount);
}

 
    
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
        // ignore if a window is hovered over
        if(ImGui::GetIO().WantCaptureMouse)
            return;
        
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
    auto DisplayShapeOffsetEvent = [&](Event_DisplayShapeOffset data) {
        m_Shape = data.shape;
        m_ShapeOffset = data.shapeOffset;
        m_ShapeIsLoop = data.isLoop;
    };
    
   /* auto UpdateCameraEvent = [&](Event_SettingsUpdated data) {
        if(data.type != Event_SettingType::CoordSystems)
            return;
        m_Camera.SetCentre(data.position);
        // data.angle
        grblVals.coords.homeCoords[0], 0.0f 
    };
    */
    event_WindowResize          = make_unique<EventHandler<Event_WindowResize>>(WindowResizeEvent);
    event_MouseScroll           = make_unique<EventHandler<Event_MouseScroll>>(MouseScrollEvent);
    event_MouseDrag             = make_unique<EventHandler<Event_MouseMove>>(MouseDragEvent);
    event_Keyboard              = make_unique<EventHandler<Event_KeyInput>>(KeyboardEvent);
    event_DisplayShapeOffset    = make_unique<EventHandler<Event_DisplayShapeOffset>>(DisplayShapeOffsetEvent); 
    //event_UpdateCamera = make_unique<EventHandler<Event_SettingsUpdated>>(UpdateCameraEvent);
    
    // dont draw vertices outside of our visible depth
    glEnable(GL_DEPTH_TEST);
    // this will always draw the latest thing on top, prevent lines overlapping and looking jittery
    glDepthFunc(GL_ALWAYS);
    // dont draw triangles facing the wrong way 
    glEnable(GL_CULL_FACE);  
    
    m_Camera.SetNearFar(0.1f, 5000.0f);
    m_Camera.SetZoomMinMax(1.0f, 3000.0f);
    m_Camera.SetZoom(2000.0f);
    
    initShape_Cylinder();
    
}
Viewer::~Viewer()
{
}





void Viewer::SetPath(Settings& settings, std::vector<glm::vec3>& positions, std::vector<uint>& indices)
{
    vector<Vertex> vertices;
    vertices.reserve(positions.size());
    
    for (size_t i = 0; i < positions.size(); i++) {
        vertices.emplace_back(positions[i], settings.p.viewer.ToolpathColour);
    }
    
    m_Shader.reset(new Shader(Viewer_VertexShader, Viewer_FragmentShader));
   
    m_VertexBuffer.reset(new VertexBuffer(vertices.size() * sizeof(Vertex), vertices.data()));
    VertexBufferLayout layout;
    
    layout.Push<float>(m_Shader->GetAttribLocation("in_Position"), 3);
    layout.Push<float>(m_Shader->GetAttribLocation("in_Colour"), 3);
    
    m_VAO.reset(new VertexArray());
    m_VAO->AddBuffer(*m_VertexBuffer, layout);
    
    m_IndexBuffer.reset(new IndexBuffer(indices.size(), indices.data()));
    
    m_DrawCount = m_DrawMax = m_IndexBuffer->GetCount();
    
    m_Initialised = true;
}

void Viewer::Clear()
{
    m_Shader.reset(new Shader(Viewer_VertexShader, Viewer_FragmentShader));
    m_VertexBuffer.reset(new VertexBuffer(0, nullptr));
    m_VAO.reset(new VertexArray());
    m_IndexBuffer.reset(new IndexBuffer(0, nullptr));
    
    m_DrawCount = m_DrawMax = 0;
    m_Initialised = false;
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


         
        
void Viewer::Update(Settings& settings, float dt)
{    
    (void)dt;
    GRBLVals& grblVals = settings.grblVals;
    float axisSize = settings.p.viewer.axis.Size;
    const glm::vec3& zeroPos = grblVals.ActiveCoordSys();
    
    // Add axis letters
    Draw2DAxesLabels(zeroPos, axisSize);
    Draw2DText("H", grblVals.coords.homeCoords[0]);
    // Reset buffer
    m_DynamicLines.ClearVertices();
    m_DynamicFaces.ClearVertices();
// ---------------------------------
    // this could be in static buffer...
    m_DynamicLines.AddGrid(settings);
    
    // Draw Current Position
    glm::vec3 scaleTool = settings.p.tools.GetToolScale();
    m_DynamicFaces.AddShape(shape_Cylinder,           settings.p.viewer.spindle.toolColour,         grblVals.status.MPos, scaleTool);
    m_DynamicLines.AddShape(shape_Cylinder_Outline,   settings.p.viewer.spindle.toolColourOutline,  grblVals.status.MPos, scaleTool);
    
    // add shape and offset path
    m_DynamicLines.AddPath(m_Shape,        settings.p.pathCutter.ShapeColour,         zeroPos, m_ShapeIsLoop);
    m_DynamicLines.AddPath(m_ShapeOffset,  settings.p.pathCutter.ShapeOffsetColour,   zeroPos, false); // geos offsetPolygon() closes path for us
    
    // Draw coord system axis
    m_DynamicLines.AddAxes(axisSize, zeroPos);
    
    m_DynamicFaces.Update();
    m_DynamicLines.Update();
}


void Viewer::Render()
{    
    m_Proj = m_Camera.GetProjectionMatrix();
    m_View = m_Camera.GetViewMatrix();
 
    Renderer::Clear();
    
    m_DynamicFaces.Draw(m_Proj, m_View);
    m_DynamicLines.Draw(m_Proj, m_View);
    DrawPath();
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
  
void Viewer::ImGuiRender(Settings& settings)  
{ 

    if (!ImGui::Begin("GCode Viewer", NULL, general_window_flags)) {
        ImGui::End();
        return;
    }  
     
    ImGuiModules::KeepWindowInsideViewport();
        
    ImGui::PushItemWidth(-130.0f);
            
        
        ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs;
            
        ImGui::SliderInt("Vertices", &m_DrawCount, 0, m_DrawMax); 
        ImGui::ColorEdit3("Toolpath Colour", &settings.p.viewer.ToolpathColour[0], flags);
         
            ImGui::Separator();
            
        if(ImGui::Button("Clear")) {
            Clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Show", &m_Show);
        
            ImGui::Separator();
            
        ImGui::TextUnformatted("GCode Viewer"); 
            
            ImGui::Separator();
            
        ImGui::ColorEdit3("Background Colour", &settings.p.viewer.BackgroundColour[0], flags);
        
            ImGui::Separator();
            
        ImGui::SliderFloat("Axis Size", &settings.p.viewer.axis.Size, 0.0f, 500.0f);
        
            ImGui::Separator();
            
        ImGui::ColorEdit3("Tool Colour", &settings.p.viewer.spindle.toolColour[0], flags);
        ImGui::ColorEdit3("Tool Colour Outline", &settings.p.viewer.spindle.toolColourOutline[0], flags);
        
            ImGui::Separator();
        
        ImGui::TextUnformatted("Grid");
        
        ImGui::SliderFloat3("Position", &settings.p.viewer.grid.Position[0], -3000.0f, 3000.0f);
        ImGui::SameLine();
        ImGuiModules::HereButton(settings.grblVals, settings.p.viewer.grid.Position);
        ImGui::SliderFloat2("Size", &settings.p.viewer.grid.Size[0], -3000.0f, 3000.0f);
        ImGui::SliderFloat("Spacing", &settings.p.viewer.grid.Spacing, 0.0f, 1000.0f);
        ImGui::ColorEdit3("Colour", &settings.p.viewer.grid.Colour[0], flags);
            
    ImGui::PopItemWidth();
        
    ImGui::End();
}

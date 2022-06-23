
#include <iostream>
using namespace std;

#include "../common.h" 

namespace Sqeak { 
    
  
void Shape::AddVertex(const glm::vec3& vertex) 
{ 
    m_Vertices.push_back(vertex); 
}

void Shape::Append(const Shape& shape) 
{                      
    for(size_t i = 0; i < shape.Size(); i++) {
        AddVertex(shape[i]);
    }
}
// returns a new transformed Shape of Shape 
// rotate about x axis and then z axis
Shape Shape::Transform(const glm::vec3& translate, const glm::vec3& scale, const glm::vec2& rotate) const
{
    Shape newShape;
    for(const glm::vec3& vertex : m_Vertices) {
        glm::vec3 v = glm::Transform(vertex, translate, scale, rotate);
        newShape.AddVertex(move(v));
    }
    return newShape;
}   


Shapes::Shapes(float arcAngle) 
    :   m_ArcAngle(arcAngle) {
    // wireframes
    Init_Wireframe_Circle();
    Init_Wireframe_Square();
    Init_Wireframe_Cylinder();
    Init_Wireframe_Cube();
    // faces
    Init_Face_Circle();
    Init_Face_Square();
    // bodies
    Init_Body_Cylinder();
    Init_Body_Cube();
    Init_Body_Sphere();
} 


// wireframes GL_LINES
void Shapes::Init_Wireframe_Circle()
{        
    for (float th = 0.0f; th <= 360.0f; th += m_ArcAngle) {
        float th2 = th + m_ArcAngle;
        m_Wireframe_Circle.AddVertex({ 0.5f * Cos(th),   -0.5f * Sin(th),    0.0f });
        m_Wireframe_Circle.AddVertex({ 0.5f * Cos(th2),  -0.5f * Sin(th2),   0.0f });
    }
}

void Shapes::Init_Wireframe_Square()
{
    m_Wireframe_Square.AddVertex({ -0.5f, -0.5f, 0.0f });
    m_Wireframe_Square.AddVertex({  0.5f, -0.5f, 0.0f });
    
    m_Wireframe_Square.AddVertex({  0.5f, -0.5f, 0.0f });
    m_Wireframe_Square.AddVertex({  0.5f,  0.5f, 0.0f });
    
    m_Wireframe_Square.AddVertex({  0.5f,  0.5f, 0.0f });
    m_Wireframe_Square.AddVertex({ -0.5f,  0.5f, 0.0f });
    
    m_Wireframe_Square.AddVertex({ -0.5f,  0.5f, 0.0f });
    m_Wireframe_Square.AddVertex({ -0.5f, -0.5f, 0.0f });
}

void Shapes::Init_Wireframe_Cylinder()
{
    // outline bottom
    m_Wireframe_Cylinder.Append(m_Wireframe_Circle.Transform({ 0.0f, 0.0f, -0.5f }));
    // outline top
    m_Wireframe_Cylinder.Append(m_Wireframe_Circle.Transform({ 0.0f, 0.0f, 0.5f }));
}
    
void Shapes::Init_Wireframe_Cube()
{
    // bottom
    m_Wireframe_Cube.Append(m_Wireframe_Square.Transform({ 0.0f, 0.0f, -0.5f }));
    // top
    m_Wireframe_Cube.Append(m_Wireframe_Square.Transform({ 0.0f, 0.0f, 0.5f }));
    // join between bottom and top
    m_Wireframe_Cube.AddVertex({ -0.5f, -0.5f, -0.5f });
    m_Wireframe_Cube.AddVertex({ -0.5f, -0.5f,  0.5f });
    
    m_Wireframe_Cube.AddVertex({  0.5f, -0.5f, -0.5f });
    m_Wireframe_Cube.AddVertex({  0.5f, -0.5f,  0.5f });
    
    m_Wireframe_Cube.AddVertex({  0.5f,  0.5f, -0.5f });
    m_Wireframe_Cube.AddVertex({  0.5f,  0.5f,  0.5f });
    
    m_Wireframe_Cube.AddVertex({ -0.5f,  0.5f, -0.5f });
    m_Wireframe_Cube.AddVertex({ -0.5f,  0.5f,  0.5f });
}

// faces GL_TRIANGLES
void Shapes::Init_Face_Circle()
{
    // circle facing up
    for (float th = 0.0f; th <= 360.0f; th += m_ArcAngle) {
        float th2 = th + m_ArcAngle;
        m_Face_Circle.AddVertex({ 0.5f * Cos(th),   -0.5f * Sin(th),    0.0f });
        m_Face_Circle.AddVertex({ 0.0f,              0.0f,              0.0f });
        m_Face_Circle.AddVertex({ 0.5f * Cos(th2),  -0.5f * Sin(th2),   0.0f });
    }
    
}
void Shapes::Init_Face_Square()
{
    m_Face_Square.AddVertex({ -0.5f, -0.5f, 0.0f });
    m_Face_Square.AddVertex({  0.5f,  0.5f, 0.0f });
    m_Face_Square.AddVertex({ -0.5f,  0.5f, 0.0f });
                                           
    m_Face_Square.AddVertex({ -0.5f, -0.5f, 0.0f });
    m_Face_Square.AddVertex({  0.5f, -0.5f, 0.0f });
    m_Face_Square.AddVertex({  0.5f,  0.5f, 0.0f });
}
/*
uint cubeIndices[] = 
{
    // face 1 (xy)
    0, 1, 2,
    2, 3, 0,
*/

// bodies GL_TRIANGLES 
void Shapes::Init_Body_Cylinder() 
{
    // top face - circle facing up
    m_Body_Cylinder.Append(m_Face_Circle.Transform({ 0.0f, 0.0f, 0.5f }));
    // bottom face - circle facing down 
    m_Body_Cylinder.Append(m_Face_Circle.Transform({ 0.0f, 0.0f, -0.5f }, glm::vec3(1.0f), { 180.0f, 0.0f }));
    
    // walls
    for (float th = 0.0f; th <= 360.0f; th += m_ArcAngle) {
        float th2 = th + m_ArcAngle;
        m_Body_Cylinder.AddVertex({ 0.5f * Cos(th),  -0.5f * Sin(th),  -0.5f });
        m_Body_Cylinder.AddVertex({ 0.5f * Cos(th),  -0.5f * Sin(th),   0.5f });
        m_Body_Cylinder.AddVertex({ 0.5f * Cos(th2), -0.5f * Sin(th2), -0.5f });
        m_Body_Cylinder.AddVertex({ 0.5f * Cos(th),  -0.5f * Sin(th),   0.5f });
        m_Body_Cylinder.AddVertex({ 0.5f * Cos(th2), -0.5f * Sin(th2),  0.5f });
        m_Body_Cylinder.AddVertex({ 0.5f * Cos(th2), -0.5f * Sin(th2), -0.5f });
    }
}

void Shapes::Init_Body_Cube()
{
    // top and bottom faces
    m_Body_Cube.Append(m_Face_Square.Transform({  0.0f,  0.0f,  0.5f }));
    m_Body_Cube.Append(m_Face_Square.Transform({  0.0f,  0.0f, -0.5f }, glm::vec3(1.0f), { 180.0f, 0.0f }));
    // front and back faces                       
    m_Body_Cube.Append(m_Face_Square.Transform({  0.0f, -0.5f,  0.0f }, glm::vec3(1.0f), { 90.0f, 0.0f })); 
    m_Body_Cube.Append(m_Face_Square.Transform({  0.0f,  0.5f,  0.0f }, glm::vec3(1.0f), { 270.0f, 0.0f }));
    // sides                                      
    m_Body_Cube.Append(m_Face_Square.Transform({  0.5f,  0.0f,  0.0f }, glm::vec3(1.0f), { 90.0f, 90.0f })); 
    m_Body_Cube.Append(m_Face_Square.Transform({ -0.5f,  0.0f,  0.0f }, glm::vec3(1.0f), { 90.0f, 270.0f })); 
}


// build vertices of sphere with smooth shading using parametric equation
// x = r * cos(u) * cos(v)
// y = r * cos(u) * sin(v)
// z = r * sin(u)
// where u: stack(latitude) angle (-90 <= u <= 90)
//       v: sector(longitude) angle (0 <= v <= 360)
void Shapes::Init_Body_Sphere()
{
    /*
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
    float s, t;                                     // texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            m_Body_Sphere.AddVertex({ x, y, z });
    
            // normalized vertex normal
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            m_Body_Sphere.AddNormal({ nx, ny, nz });

            // vertex tex coord between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            m_Body_Sphere.AddTextureCoords({ s, t });
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    uint k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding 1st and last stacks
            if(i != 0) {
                m_Body_Sphere.addIndices(k1, k2, k1+1);   // k1---k2---k1+1
            } 
            if(i != (stackCount-1)) {
                m_Body_Sphere.addIndices(k1+1, k2, k2+1); // k1+1---k2---k2+1
            } 
        }
    }
    */
}
    
   
DynamicBuffer::DynamicBuffer(GLenum primitiveType, int maxVertices, int maxIndices)
    : m_PrimitiveType(primitiveType)
{ 
    Resize(maxVertices, maxIndices);
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
    
    m_Shader.reset(new Shader(Viewer_BasicVertexShader, Viewer_BasicFragmentShader));
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
    m_OutlineVertexCount = 0;
}

void DynamicBuffer::AddVertex(const glm::vec3& position, const glm::vec3& colour, bool isOutline)
{
    m_Vertices.emplace_back(Vertex(position, colour)); 
    m_VertexCount++;
    if(isOutline) {
        m_OutlineVertexCount++;
        assert(m_OutlineVertexCount == m_VertexCount && "Outline vertices should be added first");
    }
} 

void DynamicBuffer::AddCursor(Settings& settings, glm::vec2 pos)
{    
    ParametersList::Sketch::Cursor& cursor = settings.p.sketch.cursor;
    float cursorSize = cursor.Size_Scaled / 2.0f;
    
    AddVertex(glm::vec3(pos.x, pos.y, 0.0f) + glm::vec3(0.0f,         -cursorSize,    0.0f), cursor.Colour);
    AddVertex(glm::vec3(pos.x, pos.y, 0.0f) + glm::vec3(0.0f,         cursorSize,     0.0f), cursor.Colour);
    AddVertex(glm::vec3(pos.x, pos.y, 0.0f) + glm::vec3(-cursorSize,  0.0f,           0.0f), cursor.Colour);
    AddVertex(glm::vec3(pos.x, pos.y, 0.0f) + glm::vec3(cursorSize,   0.0f,           0.0f), cursor.Colour);
}

void DynamicBuffer::AddGrid(Settings& settings)
{   
    ParametersList::Viewer3DParameters::Grid& grid = settings.p.viewer.grid;
    if(grid.Spacing <= 0)
        return;
    glm::vec2 gridOrientation = glm::vec2(Geom::Sign(grid.Size.x), Geom::Sign(grid.Size.y));
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

void DynamicBuffer::AddShapeOutline(const Shape& shape, glm::vec3 colour, const glm::vec3& translate, const glm::vec3& scale, const glm::vec2& rotate) 
{
    AddShape(shape, colour, translate, scale, rotate, true);
}        

void DynamicBuffer::AddShape(const Shape& shape, glm::vec3 colour, const glm::vec3& translate, const glm::vec3& scale, const glm::vec2& rotate, bool isOutline) 
{
    for(size_t i = 0; i < shape.Size(); i++) {
        glm::vec3 v = glm::Transform(shape[i], translate, scale, rotate);
        AddVertex(move(v), colour, isOutline);
    }
} 

void DynamicBuffer::AddDynamicVertexListAsLines(const std::vector<DynamicBuffer::DynamicVertexList>* dynamicVertexLists, const glm::vec3& zeroPosition)
{
    for (size_t i = 0; i < dynamicVertexLists->size(); i++) {
        auto& vertices = (*dynamicVertexLists)[i].position;
        auto& colour = (*dynamicVertexLists)[i].colour;
        // add each vertex as lines
        for (size_t j = 1; j < vertices.size(); j++) {
            AddVertex(zeroPosition + vertices[j-1], colour);
            AddVertex(zeroPosition + vertices[j], colour);
        }
    }
}

void DynamicBuffer::AddDynamicVertexListAsPoints(const std::vector<DynamicBuffer::DynamicVertexList>* dynamicVertexLists, const glm::vec3& zeroPosition)
{
    for (size_t i = 0; i < dynamicVertexLists->size(); i++) {
        auto& vertices = (*dynamicVertexLists)[i].position;
        auto& colour = (*dynamicVertexLists)[i].colour;
        // add each vertex as points
        for (size_t j = 0; j < vertices.size(); j++) {
            AddVertex(zeroPosition + vertices[j], colour);
        }
    }
}


void DynamicBuffer::Update() 
{
    m_VertexBuffer->DynamicUpdate(0, m_Vertices.size() * sizeof(Vertex), m_Vertices.data());
}

void DynamicBuffer::Draw(glm::mat4& proj, glm::mat4& view, bool isDrawOutline) 
{    
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_MVP", proj * view * glm::mat4(1.0f));
    
    if(m_VertexCount > m_MaxIndexCount || m_VertexCount > m_MaxVertexCount) {
        Log::Info("Too many vertices to display, resizing buffer to %d vertices, %d indices", m_MaxVertexCount*2, m_MaxIndexCount*2);
        // double size of buffer
        Resize(m_MaxVertexCount*2, m_MaxIndexCount*2);
    }
    // draw
    Renderer::Draw(m_PrimitiveType, *m_VAO, *m_IndexBuffer, *m_Shader, m_OutlineVertexCount, m_VertexCount - m_OutlineVertexCount);
    // outline
    if(isDrawOutline) {
        Renderer::Draw(GL_LINES, *m_VAO, *m_IndexBuffer, *m_Shader, 0, m_OutlineVertexCount);
    }
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
    auto AddDynamicVertexLists = [&](Event_Viewer_AddLineLists data) {
        m_DynamicLineLists = data.dynamicLineLists;
    };
    auto AddDynamicPointLists = [&](Event_Viewer_AddPointLists data) {
        m_DynamicPointLists = data.dynamicPointLists;
    };
    
    auto Set2DModeEvent = [&](Event_Set2DMode data) {
        m_Camera.Set2DMode(data.isTrue);
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
    event_AddLineLists          = make_unique<EventHandler<Event_Viewer_AddLineLists>>(AddDynamicVertexLists); 
    event_AddPointLists         = make_unique<EventHandler<Event_Viewer_AddPointLists>>(AddDynamicPointLists); 
    event_Set2DMode             = make_unique<EventHandler<Event_Set2DMode>>(Set2DModeEvent);
    //event_UpdateCamera = make_unique<EventHandler<Event_SettingsUpdated>>(UpdateCameraEvent);
      
    // dont draw triangles facing the wrong way 
    glEnable(GL_CULL_FACE);  
    // dont draw vertices outside of our visible depth
    glEnable(GL_DEPTH_TEST);
    
    m_Camera.SetNearFar(0.1f, 5000.0f);
    m_Camera.SetZoomMinMax(1.0f, 3000.0f);
    m_Camera.SetZoom(2000.0f);
    
    // 3D shapes
    // Tool (move upward so that bottom of cylinder is on z0)
    m_Shape_Tool = m_Shapes.Body_Cylinder().Transform({ 0.0f, 0.0f, 0.5f }); 
    // Tool WireFrame
    m_Shape_Tool_Wireframe = m_Shapes.Wireframe_Cylinder().Transform({ 0.0f, 0.0f, 0.5f }); 
      
    // Tool Holder
    float height = 0.0f;
    // spindle dimensions
    glm::vec3 dim_Nut       = { 34.0f, 34.0f, 23.0f };
    glm::vec3 dim_Shank     = { 23.0f, 23.0f, 20.0f };
    glm::vec3 dim_Shoulder  = { 65.0f, 65.0f, 12.0f };
    glm::vec3 dim_Body      = { 80.0f, 80.0f, 228.0f };
    // cylinder moved up, so that z = 0
    Shape cylinder = m_Shapes.Body_Cylinder().Transform({ 0.0f, 0.0f, 0.5f });
    // make spindle shape 
    m_Shape_ToolHolder.Append(cylinder.Transform({ 0.0f, 0.0f, height },                   dim_Nut)); 
    m_Shape_ToolHolder.Append(cylinder.Transform({ 0.0f, 0.0f, height += dim_Nut.z },      dim_Shank)); 
    m_Shape_ToolHolder.Append(cylinder.Transform({ 0.0f, 0.0f, height += dim_Shank.z },    dim_Shoulder)); 
    m_Shape_ToolHolder.Append(cylinder.Transform({ 0.0f, 0.0f, height += dim_Shoulder.z }, dim_Body)); 
    // spindle wireframe
    height = 0.0f;
    // cylinder moved up, so that z = 0
    Shape cylinder_Wireframe = m_Shapes.Wireframe_Cylinder().Transform({ 0.0f, 0.0f, 0.5f });
    m_Shape_ToolHolder_Wireframe.Append(cylinder_Wireframe.Transform({ 0.0f, 0.0f, height },                   dim_Nut)); 
    m_Shape_ToolHolder_Wireframe.Append(cylinder_Wireframe.Transform({ 0.0f, 0.0f, height += dim_Nut.z },      dim_Shank)); 
    m_Shape_ToolHolder_Wireframe.Append(cylinder_Wireframe.Transform({ 0.0f, 0.0f, height += dim_Shank.z },    dim_Shoulder)); 
    m_Shape_ToolHolder_Wireframe.Append(cylinder_Wireframe.Transform({ 0.0f, 0.0f, height += dim_Shoulder.z }, dim_Body)); 
    
    
}
 

glm::vec3 Viewer::GetWorldPosition(glm::vec2 px) 
{
    return m_Camera.GetWorldPosition(px);
}

void Viewer::SetPath(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& colours)
{ 
    // determine number of vertices to draw (e.g. 3 points is 2 lines, so 4 vertices)
    size_t nVertices = 0;
    if(positions.size() > 1) 
        nVertices = (positions.size()-1) * 2;
        
    // make vertices
    vector<Vertex> vertices;
    vertices.reserve(nVertices);
    // add as lines
    for (size_t i = 0; i < positions.size() - 1; i++) {
        vertices.emplace_back( positions[i], colours[i+1] );
        vertices.emplace_back( positions[i+1], colours[i+1] );
    }
    // make indices 
    std::vector<uint> indices;
    indices.reserve(nVertices); // 2 points per line
    for (size_t i = 0; i < nVertices; i++) {
        indices.emplace_back(i);
    }
    
    m_Shader.reset(new Shader(Viewer_BasicVertexShader, Viewer_BasicFragmentShader));
   
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
    m_Shader.reset(new Shader(Viewer_BasicVertexShader, Viewer_BasicFragmentShader));
    m_VertexBuffer.reset(new VertexBuffer(0, nullptr));
    m_VAO.reset(new VertexArray());
    m_IndexBuffer.reset(new IndexBuffer(0, nullptr));
    
    m_DrawCount = m_DrawMax = 0;
    m_Initialised = false;
}

void Viewer::Draw2DText(const char* label, glm::vec3 position)
{
    pair<bool, glm::vec2> labelPos = m_Camera.GetScreenCoords(position);
    // centre letters
    ImVec2 charSize = ImGui::CalcTextSize(label);
    glm::vec2 charOffset = { charSize.x/2, charSize.y/2 };
    
    if(labelPos.first) {
        glm::vec2 pos2D = Window::InvertYCoord(labelPos.second) - charOffset;
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
    m_DynamicPoints.ClearVertices();
    m_DynamicLines.ClearVertices();
    m_DynamicBodies.ClearVertices();
    
// -------------LINES----------------
    // this could be in static buffer...
    m_DynamicLines.AddGrid(settings);
    
    // add shape and offset path
    if(m_DynamicLineLists) {
        m_DynamicLines.AddDynamicVertexListAsLines(m_DynamicLineLists, zeroPos);
    }    
    if(m_DynamicPointLists) {
        m_DynamicPoints.AddDynamicVertexListAsPoints(m_DynamicPointLists, zeroPos);
    }
    // Draw coord system axis
    m_DynamicLines.AddAxes(axisSize, zeroPos);
    // add user cursor
    if(auto cursorPos = settings.p.sketch.cursor.Position_WorldCoords) {
        m_DynamicLines.AddCursor(settings, *cursorPos);
    }
           
// -------------Bodies----------------
    glm::vec3 scaleTool = settings.p.tools.GetToolScale();
    
// Draw Current Position
// Draw Outlines
    // tool 
    m_DynamicBodies.AddShapeOutline(m_Shape_Tool_Wireframe,         settings.p.viewer.spindle.colours.toolOutline,  grblVals.status.MPos, scaleTool);
    // tool holder (above tool)
    if(settings.p.viewer.spindle.visibility) {
        m_DynamicBodies.AddShapeOutline(m_Shape_ToolHolder_Wireframe, settings.p.viewer.spindle.colours.toolHolderOutline, grblVals.status.MPos + glm::vec3(0.0f, 0.0f, scaleTool.z));
    }
// Draw Faces
    // tool
    m_DynamicBodies.AddShape(m_Shape_Tool,       settings.p.viewer.spindle.colours.tool,         grblVals.status.MPos, scaleTool);
    // tool holder (above tool)
    if(settings.p.viewer.spindle.visibility) {
        m_DynamicBodies.AddShape(m_Shape_ToolHolder, settings.p.viewer.spindle.colours.toolHolder, grblVals.status.MPos + glm::vec3(0.0f, 0.0f, scaleTool.z));
    }
    
    m_DynamicLines.Update();
    m_DynamicBodies.Update();
    m_DynamicPoints.Update();
}

void Viewer::Render(Settings& settings)
{    
    m_Proj = m_Camera.GetProjectionMatrix();
    m_View = m_Camera.GetViewMatrix();
 
    Renderer::Clear();
    
    // always draw the latest thing on top, prevents overlapping lines looking jittery
    glDepthFunc(GL_ALWAYS);
    glLineWidth(m_LineWidth_Lines);
    m_DynamicLines.Draw(m_Proj, m_View);
    
    // set depth function for bodies
    if(!m_DepthFunction) { // is wireframe on?
        glDepthFunc(GL_LEQUAL);
    } else {
        glDepthFunc((GLenum)(m_DepthFunction | 0x0200)); // GL_NEVER = 0x0200, GL_LESS = 0x02001...
    }
    glLineWidth(m_LineWidth_Bodies);
    m_DynamicBodies.Draw(m_Proj, m_View, (bool)m_DepthFunction); // draw outline?
    
    glPointSize(settings.p.sketch.point.size);
    m_DynamicPoints.Draw(m_Proj, m_View);
    
    DrawPath();
}

void Viewer::DrawPath()
{ 
    if(!m_Initialised || !m_Show)
        return;
    
    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_MVP", m_Proj * m_View * glm::mat4(1.0f));
    
    Renderer::Draw(GL_LINES, *m_VAO, *m_IndexBuffer, *m_Shader, 0, (uint)m_DrawCount);
}
  
void Viewer::ImGuiRender(Settings& settings)  
{ 
    // begin new imgui window
    static ImGuiCustomModules::ImGuiWindow window(settings, "Viewer"); // default size
    if(!window.Begin(settings)) return;

    ImGui::TextUnformatted("GCode Viewer"); 
    ImGui::Separator();
        
    ImGui::TextUnformatted("General"); ImGui::Indent(); 
        ImGui::SliderFloat("Point Size", &settings.p.sketch.point.size, 0.0f, 100.0f);

        ImGui::SliderFloat("Line Width of Lines", &m_LineWidth_Lines, 0.0f, 20.0f);
        ImGui::SliderFloat("Line Width of Bodies", &m_LineWidth_Bodies, 0.0f, 20.0f);
        //ImGui::Combo("Depth Function", &m_DepthFunction, "Never\0<\0=\0<=\0>\0!=\0>=\0Always\0\0");
        static int imgui_wireframe = 0;
        if(ImGui::Combo("Edges", &imgui_wireframe, "Show Edges\0Show Hidden Edges\0Hide Edges\0\0")) { // <=  Always
            if(imgui_wireframe == 0) { m_DepthFunction = 3; } // <=
            if(imgui_wireframe == 1) { m_DepthFunction = 7; } // Always
            if(imgui_wireframe == 2) { m_DepthFunction = 0; } // Never
        }
        ImGui::Separator();
        
        static int toolShape = 2;
        if(ImGui::Combo("Tool Shape", &toolShape, "Circle\0Square\0Cylinder\0Cube\0Sphere\0\0")) {
            if (toolShape == 0) { 
                m_Shape_Tool = m_Shapes.Face_Circle(); 
                m_Shape_Tool_Wireframe = m_Shapes.Wireframe_Circle();
            }
            else if (toolShape == 1) { 
                m_Shape_Tool = m_Shapes.Face_Square(); 
                m_Shape_Tool_Wireframe = m_Shapes.Wireframe_Square();
            }
            else if (toolShape == 2) { 
                m_Shape_Tool = m_Shapes.Body_Cylinder().Transform({ 0.0f, 0.0f, 0.5f }); 
                m_Shape_Tool_Wireframe = m_Shapes.Wireframe_Cylinder().Transform({ 0.0f, 0.0f, 0.5f }); 
            }
            else if (toolShape == 3) { 
                m_Shape_Tool = m_Shapes.Body_Cube().Transform({ 0.0f, 0.0f, 0.5f }); 
                m_Shape_Tool_Wireframe = m_Shapes.Wireframe_Cube().Transform({ 0.0f, 0.0f, 0.5f });
            }
            else if (toolShape == 4) { 
                m_Shape_Tool = m_Shapes.Body_Sphere().Transform({ 0.0f, 0.0f, 0.5f }); 
            }
        } 
    ImGui::Unindent();  ImGui::Separator();
        
    ImGui::TextUnformatted("Cursor"); ImGui::Indent(); 
        ImGuiCustomModules::Text("2D Raw Position", settings.p.sketch.cursor.Position_Raw);
        ImGuiCustomModules::Text("2D World Position", settings.p.sketch.cursor.Position_WorldCoords);
        ImGuiCustomModules::Text("2D Snapped Position", settings.p.sketch.cursor.Position_Snapped);
        ImGuiCustomModules::Text("2D Clicked Position", settings.p.sketch.cursor.Position_Clicked);
        
        if(ImGui::SliderFloat("Cursor Size", &settings.p.sketch.cursor.Size, 0.0f, 100.0f))  {
            settings.p.sketch.cursor.Size_Scaled = ScaleToPx(settings.p.sketch.cursor.Size);
        }
        ImGui::SameLine();
        ImGui::Text("%g Scaled", settings.p.sketch.cursor.Size_Scaled);
        
        if(ImGui::InputFloat("Selection Tolerance", &settings.p.sketch.cursor.SelectionTolerance)) {
             settings.p.sketch.cursor.SelectionTolerance_Scaled = ScaleToPx(settings.p.sketch.cursor.SelectionTolerance);
        }
        
        ImGui::SliderFloat("Cursor Snap Distance", &settings.p.sketch.cursor.SnapDistance, 0.1f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
        ImGui::SameLine();
        ImGui::Text("%g Scaled", settings.p.sketch.cursor.SnapDistance_Scaled);
    ImGui::Unindent();  ImGui::Separator();
         
    ImGui::TextUnformatted("Tool Path"); ImGui::Indent(); 
        ImGui::SliderInt("Vertices", &m_DrawCount, 0, m_DrawMax); 
        ImGui::Checkbox("Show##toolpath", &m_Show);
        ImGui::SameLine();
        if(ImGui::Button("Clear")) { Clear(); }
    ImGui::Unindent();  ImGui::Separator();
        
    ImGui::TextUnformatted("Axis"); ImGui::Indent();  
        ImGui::SliderFloat("Axis Size", &settings.p.viewer.axis.Size, 0.0f, 500.0f);
    ImGui::Unindent();  ImGui::Separator();
    
    ImGui::TextUnformatted("Grid"); ImGui::Indent();    
        ImGui::SliderFloat3("Position", &settings.p.viewer.grid.Position[0], -3000.0f, 3000.0f);
        ImGui::SameLine();
        ImGuiCustomModules::HereButton(settings.grblVals, settings.p.viewer.grid.Position);
        ImGui::SliderFloat2("Size", &settings.p.viewer.grid.Size[0], -3000.0f, 3000.0f);
        ImGui::SliderFloat("Spacing", &settings.p.viewer.grid.Spacing, 0.0f, 1000.0f);
    ImGui::Unindent();  ImGui::Separator();
    
    ImGui::TextUnformatted("Tool Holder"); ImGui::Indent();  
        ImGui::Checkbox("Show##toolholder", &settings.p.viewer.spindle.visibility);
    ImGui::Unindent();  ImGui::Separator();
    
    ImGui::TextUnformatted("Colours"); ImGui::Indent();
        ImGuiColorEditFlags colourFlags = ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs;
        
        ImGui::TextUnformatted("General"); ImGui::Indent();
            ImGui::ColorEdit3("Background", &settings.p.viewer.general.BackgroundColour[0], colourFlags);
            ImGui::ColorEdit3("Grid", &settings.p.viewer.grid.Colour[0], colourFlags);
            ImGui::ColorEdit3("Cursor", &settings.p.sketch.cursor.Colour[0], colourFlags);
            ImGui::Unindent();            
        
        ImGui::TextUnformatted("Tool / Holder"); ImGui::Indent();
            ImGui::ColorEdit3("Tool", &settings.p.viewer.spindle.colours.tool[0], colourFlags);
            ImGui::ColorEdit3("Tool Outline", &settings.p.viewer.spindle.colours.toolOutline[0], colourFlags);
            ImGui::ColorEdit3("Tool Holder", &settings.p.viewer.spindle.colours.toolHolder[0], colourFlags);
            ImGui::ColorEdit3("Tool Holder Outline", &settings.p.viewer.spindle.colours.toolHolderOutline[0], colourFlags);
            ImGui::Unindent();            
            
        ImGui::TextUnformatted("Toolpath"); ImGui::Indent();
            ImGui::ColorEdit3("Feed", &settings.p.viewer.toolpath.Colour_Feed[0], colourFlags);
            ImGui::ColorEdit3("Feed Z", &settings.p.viewer.toolpath.Colour_FeedZ[0], colourFlags);
            ImGui::ColorEdit3("Rapid", &settings.p.viewer.toolpath.Colour_Rapid[0], colourFlags);
            ImGui::ColorEdit3("Home", &settings.p.viewer.toolpath.Colour_Home[0], colourFlags);
            ImGui::Unindent();            
        
        ImGui::TextUnformatted("Sketch"); ImGui::Indent();
            ImGui::ColorEdit3("Points", &settings.p.sketch.point.colour[0], colourFlags);
            ImGui::ColorEdit3("Active Point", &settings.p.sketch.point.colourActive[0], colourFlags);
            ImGui::ColorEdit3("Lines", &settings.p.sketch.line.colour[0], colourFlags);
            ImGui::ColorEdit3("Lines (Disabled)", &settings.p.sketch.line.colourDisabled[0], colourFlags);
            ImGui::Unindent();            
        
    ImGui::Unindent();  ImGui::Separator();

    
    
    window.End();
}


} // end namespace Sqeak

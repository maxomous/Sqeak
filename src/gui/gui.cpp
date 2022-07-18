
#include <iostream>
#include <string>

#include "gui.h"

using namespace std;
using namespace MaxLib;
using namespace MaxLib::Geom;


namespace Sqeak { 

/*
        
        TODO: 
        
        Viewer Dynamic Buffer:
            The raw value Viewer needs is a Vertex (glm::vec3 position; glm::vec3 colour; )
            currently we are using a ColouredVertexList (std::vector<glm::vec3> positions; glm::vec3 colour; )
            We want to tell the dynamic buffer, this is where the data is.. e.g:
            - allow a custom vector as a parameter like imgui does
            - AddVertexPtr() Callback function to show where the point is to render and how to render it. emplace_back(glm::vec3 pos, glm::vec3 colour)
            - maybe a template so you can pass anything?


*/            
    
    
void imgui_Settings(Settings& settings)
{ 
    GUISettings& s = settings.guiSettings;
    ImGuiIO& io = ImGui::GetIO();
       
    // Load Fonts (primary first)
    s.font_medium = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 17.0f);
    if(!s.font_medium)
        cout << "Error: Could not find font: Geomanist 17" << endl;
       
    s.font_small = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 13.0f);
    if(!s.font_small)
        cout << "Error: Could not find font: Geomanist 13" << endl;
      
    s.font_large = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 24.0f);
    if(!s.font_large)
        cout << "Error: Could not find font: Geomanist 24" << endl;
        
    // Images
    s.img_Restart.Init(File::ThisDir("img/img_restart.png").c_str());
    s.img_Pause.Init(File::ThisDir("img/img_pause.png").c_str());
    s.img_Settings.Init(File::ThisDir("img/img_settings.png").c_str());
    s.img_Edit.Init(File::ThisDir("img/img_edit.png").c_str());
    s.img_Add.Init(File::ThisDir("img/img_add.png").c_str());
    s.img_Open.Init(File::ThisDir("img/img_open.png").c_str());
    s.img_Connect.Init(File::ThisDir("img/img_connect.png").c_str());
    s.img_ArrowUp.Init(File::ThisDir("img/img_arrowup0.png").c_str());
    s.img_ArrowDown.Init(File::ThisDir("img/img_arrowdown0.png").c_str());
    s.img_ArrowLeft.Init(File::ThisDir("img/img_arrowleft0.png").c_str());
    s.img_ArrowRight.Init(File::ThisDir("img/img_arrowright0.png").c_str());
    // sketch images
    s.img_Sketch.Init(File::ThisDir("img/img_sketch.png").c_str());
    s.img_Sketch_Draw.Init(File::ThisDir("img/img_sketch_draw.png").c_str());
    s.img_Sketch_Measure.Init(File::ThisDir("img/img_sketch_measure.png").c_str());
    s.img_Sketch_Select.Init(File::ThisDir("img/img_sketch_select.png").c_str());
    s.img_Sketch_Line.Init(File::ThisDir("img/img_sketch_line.png").c_str());
    s.img_Sketch_Arc.Init(File::ThisDir("img/img_sketch_arc.png").c_str());
    
    ImGui::GetStyle().Colors[ImGuiCol_Text] = settings.guiSettings.colour[Colour::Text];
}

// Updates the dynamic vertex buffer in viewer
class Updater
{
public:

    void HandleUpdateFlag(Settings& settings, Sketch::Sketcher& sketcher) 
    {
        // return if no update required
        if(settings.GetUpdateFlag() == ViewerUpdate::None) { return; }
        
        // clear overrides other bits in flag
        if((int)settings.GetUpdateFlag() & (int)ViewerUpdate::Clear) { 
            std::cout << "Clearing Ciewer" << std::endl;  
            ClearViewer(); 
        } 
        else {     
            std::cout << "Updating Viewer" << std::endl;  
            // update 
            UpdateViewer(settings, sketcher);
        }
        
        // reset the update flag
        settings.ResetUpdateFlag();
    }
    
private:
    std::vector<DynamicBuffer::ColouredVertexList> m_ViewerLineLists;
    std::vector<DynamicBuffer::ColouredVertexList> m_ViewerPointLists;
    
    
    void UpdateViewer(Settings& settings, Sketch::Sketcher& sketcher)
    { 
        // make a list of points / lines which is sent to viewer
        m_ViewerPointLists.clear();
        m_ViewerLineLists.clear();

        RenderSketcher(settings, sketcher);
        
        // dispatch points / lines
        Event<Event_Viewer_AddPointLists>::Dispatch( { &m_ViewerPointLists } );
        Event<Event_Viewer_AddLineLists>::Dispatch( { &m_ViewerLineLists } );
    }
    

        
    void RenderSketcher(Settings& settings, Sketch::Sketcher& sketcher) 
    {    
        using namespace Sketch;
        
        auto CopyVertices = [](std::vector<DynamicBuffer::ColouredVertexList>& list, const vector<Geometry>& data, const glm::vec3& colour) {
            
            for(const Geometry& geometry : data) {
                
                DynamicBuffer::ColouredVertexList vertices(colour);
                
                for(const Vec2& p : geometry) {
                    vertices.position.emplace_back(p.x, p.y, 0.0f);
                }
                list.push_back(std::move(vertices));     
            }
        };
        
        auto CopyData = [&](const Sketch::RenderData::Data& data, const glm::vec3& colourPoints, const glm::vec3& colourLines) {
            CopyVertices(m_ViewerPointLists, data.points, colourPoints);
            CopyVertices(m_ViewerLineLists, data.linestrings, colourLines);
        };
        
        const Sketch::RenderData& renderData = sketcher.Renderer().GetRenderData();
        
        CopyData(renderData.elements.unselected,        settings.p.sketch.point.colour,     settings.p.sketch.line.colour);
        CopyData(renderData.elements.selected,          { 1.0f, 0.0f, 0.0f },               { 1.0f, 0.0f, 0.0f });
        CopyData(renderData.elements.hovered,           { 0.568f, 0.019f, 0.940f },         { 0.6f, 0.8f, 0.8f });
                
        CopyData(renderData.constraints.unselected,     { 0.0f, 0.0f, 0.0f },               { 0.0f, 0.0f, 0.0f });
        CopyData(renderData.constraints.selected,       { 0.0f, 0.0f, 0.0f },               { 0.0f, 0.0f, 0.0f });
        CopyData(renderData.constraints.hovered,        { 0.0f, 0.0f, 0.0f },               { 0.0f, 0.0f, 0.0f });
            
        CopyData(renderData.preview,                    settings.p.sketch.point.colour,     settings.p.sketch.line.colour);
        const Sketch::RenderData::Data& cursor = renderData.cursor;
        // Change colour of selection box if left of click position
        if(cursor.points.size() == 1) {
            if(cursor.points[0].size() == 4) {
                glm::vec3 selectionBoxColour = ((cursor.points[0][2].x - cursor.points[0][0].x) < 0) ? glm::vec3(0.1f, 0.3f, 0.8f) : glm::vec3(0.306f, 0.959f, 0.109f);
                CopyData(cursor,                     { 1.0f, 1.0f, 0.0f },               selectionBoxColour);
            }
        }
        
    }
    
    void ClearViewer()
    {
        // line lists
        Event<Event_Viewer_AddLineLists>::Dispatch( { nullptr } );
        // points lists
        Event<Event_Viewer_AddPointLists>::Dispatch( { nullptr } );
        //
        Event<Event_Update3DModelFromVector>::Dispatch( { vector<string>(/*empty*/) } );
    }
    
};
      

int gui(GRBL& grbl, Settings& settings)
{
    // GLFW Config Version
    auto cb_GLFW_ConfigVersion = []() {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        /*
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
        */ 
    };
    // GLFW Config General
    auto cb_GLFW_Config = [](GLFWwindow* window) {
        // set minimum window size
        glfwSetWindowSizeLimits(window, GUI_WINDOW_WMIN, GUI_WINDOW_HMIN, GLFW_DONT_CARE, GLFW_DONT_CARE); 
    };

    // ImGui Config
    auto cb_imgui_Config = [](GLFWwindow* window) {
        // Style
        ImGui::StyleColorsDark();
        // Get IO
        ImGuiIO& io = ImGui::GetIO();
        
        // ImGui ini File
        static string iniFile = File::ThisDir(GUI_CONFIG_FILE);
        io.IniFilename = iniFile.c_str();
        
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.ConfigDockingAlwaysTabBar = true;
        
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowMenuButtonPosition  = ImGuiDir_Right;   // icon in menu of window
        //style.ColorButtonPosition       = ImGuiDir_Left;    // colour icon side for changing colours 
        style.ScrollbarRounding         = 3.0f;             // scroll bars
        style.FrameRounding             = 2.0f;             // frames i.e. buttons, textboxes etc.
        
        // Load icon
        static string iconLocation = File::ThisDir(GUI_IMG_ICON);
        if(!LoadIconFromFile(window, iconLocation.c_str()))
            Log::Error(string("Could not find icon: ") + iconLocation);
    };
    
    GLSystem glsys(GUI_WINDOW_W, GUI_WINDOW_H, GUI_WINDOW_NAME, "#version 300 es", cb_GLFW_ConfigVersion, cb_GLFW_Config, cb_imgui_Config); // glsl version   
	// blending (allows translucence)
	GLCall(glEnable(GL_BLEND));
	// what to do with source (overlapping item) / what to do with destination (item we are overlapping)
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    // set images / fonts
    imgui_Settings(settings);
    
    Timer timer;
    GCodeReader gcReader(settings);
    Viewer viewer;
    sketch::SketchOld sketcher;
    Sketch::Sketcher sketcherNew;
    Frames frames(settings);
    Updater updater;
        
    auto updateGRBL = [&grbl]() {
         // update status (this is probably already done from status thread)
        grbl.sendRT(GRBL_RT_STATUS_QUERY);
        // update modal values
        grbl.sendUpdateSettings();
        delay(100);
    }; 
        
    Event<Event_Update3DModelFromFile>::RegisterHandler([&updateGRBL, &gcReader, &viewer](Event_Update3DModelFromFile data) {
        updateGRBL();
        gcReader.OpenFile(data.filename);
        viewer.SetPath(gcReader.GetVertices(), gcReader.GetColours());
    });
    
    Event<Event_Update3DModelFromVector>::RegisterHandler([&updateGRBL, &gcReader, &viewer](Event_Update3DModelFromVector data) {
        if(data.gcodes.size() > 0) {
            updateGRBL();
            gcReader.OpenVector(data.gcodes);
            viewer.SetPath(gcReader.GetVertices(), gcReader.GetColours());
        } else {
            viewer.Clear();
        }
    });
    
    
    Event<Event_KeyInput>::RegisterHandler([&settings, &sketcherNew](Event_KeyInput data) {
            
        
        sketcherNew.Events().Event_Keyboard(data.Key, (Sketch::SketchEvents::KeyAction)data.Action, (Sketch::SketchEvents::KeyModifier)data.Modifier);
            
        // TODO THIS SHOULDNT BE HERER
        settings.SetUpdateFlag(ViewerUpdate::Full);
    
    
    
    });
    
    Event<Event_MouseButton>::RegisterHandler([&settings, &sketcher, &sketcherNew](Event_MouseButton data) {
        if((data.Button != GLFW_MOUSE_BUTTON_LEFT) && (data.Button != GLFW_MOUSE_BUTTON_RIGHT) && (data.Button != GLFW_MOUSE_BUTTON_MIDDLE))
            return; 
        auto& cursor = settings.p.sketch.cursor;
        // ignore if a ImGui window is hovered over
        if(ImGui::GetIO().WantCaptureMouse /*|| !sketchNew.IsActive()*/) {
            cursor.Position_Clicked = {};
            return;
        }
        // update 2d cursor position
        cursor.Position_Clicked = cursor.Position_Snapped;
        
        // issue event to sketch
        InputEvent inputEvent; 
        Event_MouseButton mouseClick = data;
        inputEvent.mouseClick = &mouseClick;
     //   sketcher.HandleEvents(settings, inputEvent);
        
        if(cursor.Position_Snapped) {  
             
            if(sketcherNew.Events().Mouse_Button((Sketch::SketchEvents::MouseButton)data.Button, (Sketch::SketchEvents::MouseAction)data.Action, (Sketch::SketchEvents::KeyModifier)data.Modifier)) {
                
                settings.SetUpdateFlag(ViewerUpdate::Full);
            }
            
            
            //if(data.Button == GLFW_MOUSE_BUTTON_LEFT && data.Action == GLFW_PRESS) {
            //    sketcherNew.Events().Event_Click({ (*cursor.Position_Snapped).x, (*cursor.Position_Snapped).y });                
            //}
            //if(data.Button == GLFW_MOUSE_BUTTON_LEFT && data.Action == GLFW_RELEASE) {
            //    sketcherNew.Events().Event_MouseRelease();                
            //}        
            
        }
    });
            
    Event<Event_MouseMove>::RegisterHandler([&settings, &viewer, &sketcher, &sketcherNew](Event_MouseMove data) {
        
        auto& cursor = settings.p.sketch.cursor;
         // ignore if a ImGui window is hovered over or sketch is not active
        if(ImGui::GetIO().WantCaptureMouse /*|| !sketchNew.IsActive()*/) {
            cursor.Position_Snapped = {};
            cursor.Position_Raw = {};
            cursor.Position_WorldCoords = {};
            return;
        }
        glm::vec2 screenCoords = Window::InvertYCoord({ data.PosX, data.PosY }) + glm::vec2(0.0f, 1.0f);
        glm::vec3 returnCoords = viewer.GetWorldPosition(screenCoords) - settings.grblVals.status.WCO;

        // update 2d cursor positions
        cursor.Position_Raw = glm::vec2(returnCoords);
        // snap cursor or snap to raw point
        std::optional<Vec2> closestPoint = sketcher.RawPoint_GetClosest({ returnCoords.x, returnCoords.y }, cursor.SelectionTolerance_Scaled);
        cursor.Position_Snapped = (closestPoint) ? glm::vec2({ (*closestPoint).x, (*closestPoint).y }) : cursor.SnapCursor({ returnCoords.x, returnCoords.y });
        cursor.Position_WorldCoords = *(cursor.Position_Snapped) + glm::vec2(settings.grblVals.ActiveCoordSys());

        // issue event to sketch
        InputEvent inputEvent;  
        Event_MouseMove mouseMove = data;
        inputEvent.mouseMove = &mouseMove;
     //   sketcher.HandleEvents(settings, inputEvent);
           
        if(cursor.Position_Snapped) {       
            
            if(sketcherNew.Events().Mouse_Move({ (*cursor.Position_Snapped).x, (*cursor.Position_Snapped).y })) {
                settings.SetUpdateFlag(ViewerUpdate::Full);                
            }            
        }
    });
    
    auto ScaleMouseData = [](Settings& settings, Viewer& viewer) {
        // scale the cursor size
        settings.p.sketch.cursor.Size_Scaled                = viewer.ScaleToPx(settings.p.sketch.cursor.Size);
        
        settings.p.sketch.cursor.SelectionTolerance_Scaled  = viewer.ScaleToPx(settings.p.sketch.cursor.SelectionTolerance);
        // scale the cursor snap distance
        float snapDistance                                  = viewer.ScaleToPx(settings.p.sketch.cursor.SnapDistance);
        // make 0.01, 0.1, 1, 10 or 100
        if(snapDistance <= 0.01f)      { snapDistance = 0.01f; }
        else if(snapDistance <= 0.02f) { snapDistance = 0.02f; }
        else if(snapDistance <= 0.05f) { snapDistance = 0.05f; }
        else if(snapDistance <= 0.1f)  { snapDistance = 0.1f; } 
        else if(snapDistance <= 0.2f)  { snapDistance = 0.2f; } 
        else if(snapDistance <= 0.5f)  { snapDistance = 0.5f; } 
        else if(snapDistance <= 1.0f)  { snapDistance = 1.0f; }
        else if(snapDistance <= 2.0f)  { snapDistance = 2.0f; }
        else if(snapDistance <= 5.0f)  { snapDistance = 5.0f; }
        else if(snapDistance <= 10.0f) { snapDistance = 10.0f; }
        else if(snapDistance <= 20.0f) { snapDistance = 20.0f; }
        else if(snapDistance <= 50.0f) { snapDistance = 50.0f; }
        else                           { snapDistance = 100.0f; }
        
        settings.p.sketch.cursor.SnapDistance_Scaled = snapDistance;        
    };
    
    // scale cursor based on mouse scroll
    Event<Event_MouseScroll>::RegisterHandler([&settings, &viewer, &ScaleMouseData](Event_MouseScroll data) {
        (void)data;
        ScaleMouseData(settings, viewer);
    });
    
    // initialise mouse scale data
    ScaleMouseData(settings, viewer);
    
    
    Event<Event_Set2DMode>::Dispatch( { true } );
    
    
    
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(glsys.GetWindow()))
    {    
        // update timer
        timer.UpdateInterval();
        // Poll for and process events 
        GLCall(glfwPollEvents());
        // set background colour
		GLCall(glClearColor(settings.p.viewer.general.BackgroundColour.r, settings.p.viewer.general.BackgroundColour.g, settings.p.viewer.general.BackgroundColour.b, 1.0f));
       
        // grbl updates
        grbl.Update(settings.grblVals);
        
        // Draw ImGui
        glsys.imgui_NewFrame();
		{
            // Updates the skether when needed
            if(sketcherNew.Update()) { settings.SetUpdateFlag(ViewerUpdate::Sketch); }
            
            // make updates for viewer
            updater.HandleUpdateFlag(settings, sketcherNew);
            
            // update and render 3D Viewer
            viewer.Update(settings, timer.dt()); 
            viewer.Render(settings);
            
            
            // draw imgui frames
            frames.Draw(grbl, settings, viewer, sketcher, sketcherNew, timer.dt());
            
            
    
            
            
                
            // end dockspace
            ImGui::End();
		}
		glsys.imgui_Render();
        // Swap front and back buffers 
        GLCall(glfwSwapBuffers(glsys.GetWindow()));
    }
    
    // save settings on close
    Event<Event_SaveSettings>::Dispatch({ }); 
    return 0;
}

} // end namespace Sqeak

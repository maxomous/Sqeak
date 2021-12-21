
#include <iostream>
#include <string>
using namespace std;

#include "../common.h"
#include "gui.h"

void GLSystem::glfw_ConfigVersion()
{
   	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    /*
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
	*/ 
}
void GLSystem::glfw_Config()
{
    // set minimum window size
    glfwSetWindowSizeLimits(m_Window, GUI_WINDOW_WMIN, GUI_WINDOW_HMIN, GLFW_DONT_CARE, GLFW_DONT_CARE); 
}


void GLSystem::imgui_Config()
{
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
    style.FrameRounding             = 1.0f;             // frames i.e. buttons, textboxes etc.
    
    // Load icon
    static string iconLocation = File::ThisDir(GUI_IMG_ICON);
    if(!LoadIconFromFile(m_Window, iconLocation.c_str()))
        Log::Error(string("Could not find icon: ") + iconLocation);
}
      
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
    
    ImGui::GetStyle().Colors[ImGuiCol_Text] = settings.guiSettings.colour[Colour::Text];
}
    

int gui(GRBL& grbl, Settings& settings)
{
    GLSystem glsys(GUI_WINDOW_W, GUI_WINDOW_H, GUI_WINDOW_NAME, "#version 300 es"); // glsl version   
	// blending (allows translucence)
	GLCall(glEnable(GL_BLEND));
	// what to do with source (overlapping item) / what to do with destination (item we are overlapping)
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    // set images / fonts
    imgui_Settings(settings);
    
    Timer timer;
    GCodeReader gcReader(settings.grblVals);
    Viewer viewer;
    Sketch sketch;
        
    auto updateGRBL = [&grbl]() {
         // update status (this is probably already done from status thread)
        grbl.sendRT(GRBL_RT_STATUS_QUERY);
        // update modal values
        grbl.sendUpdateSettings();
        delay(100);
    };
    
    Event<Event_Update3DModelFromFile>::RegisterHandler([&updateGRBL, &gcReader, &viewer, &settings](Event_Update3DModelFromFile data) {
        updateGRBL();
        gcReader.OpenFile(data.filename);
        viewer.SetPath(settings, gcReader.GetVertices(), gcReader.GetIndices());
        // clear the offset shape buffer
        //Event<Event_DisplayShapeOffset>::Dispatch( { std::vector<glm::vec2>(/*empty*/), std::vector<glm::vec2>(/*empty*/), false } );
    });
    
    Event<Event_Update3DModelFromVector>::RegisterHandler([&updateGRBL, &gcReader, &viewer, &settings](Event_Update3DModelFromVector data) {
        if(data.gcodes.size() > 0) {
            updateGRBL();
            gcReader.OpenVector(data.gcodes);
            viewer.SetPath(settings, gcReader.GetVertices(), gcReader.GetIndices());
        } else {
            viewer.Clear();
        }
    });
    
    Event<Event_MouseButton>::RegisterHandler([&settings, &viewer, &sketch](Event_MouseButton data) {
        if((data.Button != GLFW_MOUSE_BUTTON_LEFT) && (data.Button != GLFW_MOUSE_BUTTON_RIGHT) && (data.Button != GLFW_MOUSE_BUTTON_MIDDLE))
            return; 
            
        if(data.Action == GLFW_PRESS && data.Button == GLFW_MOUSE_BUTTON_LEFT)
        {
            // ignore if a ImGui window is hovered over
            if(ImGui::GetIO().WantCaptureMouse)
                return;
            glm::vec2 screenCoords = Window::InvertYCoord(Mouse::GetPositionClicked()) + glm::vec2(0.0f, 1.0f);
            glm::vec3 returnCoords = viewer.GetWorldPosition(screenCoords) - settings.grblVals.status.WCO;
            
            InputEvent inputEvent; 
            inputEvent.screenCoords = Vec2(returnCoords);
            inputEvent.mouse = data;
            inputEvent.mouseHasChanged = true;
            sketch.HandleEvents(settings, inputEvent);
        }
    });

    Event<Event_MouseMove>::RegisterHandler([&settings, &viewer](Event_MouseMove data) {
         // ignore if a ImGui window is hovered over
        if(ImGui::GetIO().WantCaptureMouse) {
            viewer.SetCursor(false, {});
            return;
        }
        glm::vec2 screenCoords = Window::InvertYCoord({ data.PosX, data.PosY }) + glm::vec2(0.0f, 1.0f);
        std::cout << "Setting cursor before:" << viewer.GetWorldPosition(screenCoords) << std::endl;
        glm::vec3 returnCoords = viewer.GetWorldPosition(screenCoords);// - settings.grblVals.ActiveCoordSys();
        
        glm::vec2 pointRounded = settings.p.viewer.cursor.SnapCursor(Vec2(returnCoords));
        std::cout << "Setting cursor after:" << pointRounded << std::endl;
        
        viewer.SetCursor(true, pointRounded);
    });
    
    auto ScaleMouseData = [](Settings& settings, Viewer& viewer) {
        // scale the cursor size
        settings.p.viewer.cursor.Size_Scaled = viewer.ScaleToPx(settings.p.viewer.cursor.Size);
        // scale the cursor snap distance
        float snapDistance = viewer.ScaleToPx(settings.p.viewer.cursor.SnapDistance);
        // make 0.01, 0.1, 1, 10 or 100
        if(snapDistance <= 0.01)      { snapDistance = 0.01f; }
        if(snapDistance <= 0.05)      { snapDistance = 0.05f; }
        else if(snapDistance <= 0.1)  { snapDistance = 0.1f; } 
        else if(snapDistance <= 0.5)  { snapDistance = 0.5f; } 
        else if(snapDistance <= 1.0)  { snapDistance = 1.0f; }
        else if(snapDistance <= 5.0)  { snapDistance = 5.0f; }
        else if(snapDistance <= 10.0) { snapDistance = 10.0f; }
        else if(snapDistance <= 50.0) { snapDistance = 50.0f; }
        else                         { snapDistance = 100.0f; }
        
        settings.p.viewer.cursor.SnapDistance_Scaled = snapDistance;
            
        //settings.p.viewer.cursor.SnapDistance_Scaled = viewer.ScaleToPx(settings.p.viewer.cursor.SnapDistance);
        
    };
    
    // scale cursor based on mouse scroll
    Event<Event_MouseScroll>::RegisterHandler([&settings, &viewer, &ScaleMouseData](Event_MouseScroll data) {
        (void)data;
        ScaleMouseData(settings, viewer);
    });
    
    // initialise mouse scale data
    ScaleMouseData(settings, viewer);
    
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(glsys.GetWindow()))
    {    
        // Poll for and process events 
        GLCall(glfwPollEvents());
        // set background colour
		GLCall(glClearColor(settings.p.viewer.BackgroundColour.r, settings.p.viewer.BackgroundColour.g, settings.p.viewer.BackgroundColour.b, 1.0f));
        
        grbl.systemChecks();
        // build a structure of all values so we dont have to constantly be locking mutexes throughout program
        grbl.UpdateGRBLVals(settings.grblVals);
        
        timer.UpdateInterval();
        
        // ImGui
        glsys.imgui_NewFrame();
		{
            viewer.Update(settings, timer.dt()); 
            viewer.Render();
            // draw ImGui windows
            drawDockSpace(settings);
            ImGui::ShowDemoWindow(NULL);
            viewer.ImGuiRender(settings);
            sketch.DrawImGui(grbl, settings);
            drawFrames(grbl, settings, timer.dt());
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

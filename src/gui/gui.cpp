
#include <iostream>
#include <string>
using namespace std;

#include "../common.h"
#include "gui.h"

#define GUI_WINDOW_NAME     "Sqeak"
#define GUI_WINDOW_W        1280
#define GUI_WINDOW_H        720
#define GUI_WINDOW_WMIN     200
#define GUI_WINDOW_HMIN     200

#define GUI_CONFIG_FILE     "uiconfig.ini"    // created in the directory of the executable

#define GUI_IMG_ICON        "/img/img_restart.png"

void GLSystem::glfw_ConfigVersion()
{
   	/*
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    
    // Ini File
    static string iniFile = File::ThisDir(GUI_CONFIG_FILE);
    io.IniFilename = iniFile.c_str();
    
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::GetStyle().ScrollbarRounding = 3.0f; // scroll bars
    
    // Load icon
    static string filename = File::ThisDir(GUI_IMG_ICON);
    if(!LoadIconFromFile(m_Window, filename.c_str()))
        Log::Error(string("Could not find icon: ") + filename);
    
    // Load Fonts
    ImFont* font_geomanist = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 17.0f);
    if(!font_geomanist)
        cout << "Error: Could not find font: Geomanist" << endl;
}

    // get all grbl values (this is much quicker than getting
    // as required as it does require lots of mutexes)
void UpdateGRBLVals(GRBL& grbl, GRBLVals& grblVals)
{
    grblVals.coords = grbl.sys.coords.getVals();
    grblVals.modal = grbl.sys.modal.getVals();
    grblVals.status = grbl.sys.status.getVals();
    grblVals.settings = grbl.sys.settings.getVals();

    grblVals.isConnected = grbl.isConnected();
    grblVals.isCheckMode = (grblVals.status.state == GRBLState::Status_Check);
    grblVals.isFileRunning = grbl.isFileRunning();
    grbl.getFilePos(grblVals.curLine, grblVals.totalLines);
}


//void globalEvents()


int gui(GRBL& grbl)
{
    GLSystem glsys(GUI_WINDOW_W, GUI_WINDOW_H, GUI_WINDOW_NAME, "#version 140"); // glsl version   

	// blending (allows translucence)
	GLCall(glEnable(GL_BLEND));
	// what to do with source (overlapping item) / what to do with destination (item we are overlapping)
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    // set background colour
    GLCall(glClearColor(0.45f, 0.55f, 0.60f, 1.00f));
    
    Timer timer;
        
    GRBLVals grblVals;
    GCodeReader gcReader(grblVals);
    Viewer viewer;
    
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
        viewer.SetPath(gcReader.GetVertices(), gcReader.GetIndices());
    });
    
    Event<Event_Update3DModelFromVector>::RegisterHandler([&updateGRBL, &gcReader, &viewer](Event_Update3DModelFromVector data) {
        updateGRBL();
        gcReader.OpenVector(data.gcodes);
        viewer.SetPath(gcReader.GetVertices(), gcReader.GetIndices());
    });
    
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(glsys.GetWindow()))
    {    
        grbl.systemChecks();
        UpdateGRBLVals(grbl, grblVals);
        
        timer.Update();
        
        // ImGui
        glsys.imgui_NewFrame();
		{
            viewer.Update(timer.dt(), grblVals.status.MPos, grblVals.ActiveCoordSys()); 
            viewer.Render();
            viewer.ImGuiRender(grblVals);
            drawFrames(grbl, grblVals);
            ImGui::ShowDemoWindow(NULL);
		}
		glsys.imgui_Render();
        // Swap front and back buffers 
        GLCall(glfwSwapBuffers(glsys.GetWindow()));
        // Poll for and process events 
        GLCall(glfwPollEvents());
    }
    return 0;
}

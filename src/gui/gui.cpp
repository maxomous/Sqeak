

#include "../common.h"
using namespace std;


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int gui(GRBL& grbl)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
   
                                                                             
    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(GUI_WINDOW_W, GUI_WINDOW_H, GUI_WINDOW_NAME, NULL, NULL);
    if (window == NULL)
        return 1;
    // set minimum window size
    glfwSetWindowSizeLimits(window, GUI_WINDOW_WMIN, GUI_WINDOW_HMIN, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
    cout << glGetString(GL_VERSION) << endl;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    string iniFile = File::ThisDir(GUI_CONFIG_FILE);
    io.IniFilename = iniFile.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    
    ImGui::GetStyle().ScrollbarRounding = 3.0f; // scroll bars
    //ImGui::GetStyle().GrabRounding = 3.0f;    // slider grabs
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load icon
    string filename = File::ThisDir(GUI_IMG_ICON);
    if(!LoadIconFromFile(window, filename.c_str()))
        Log::Error(string("Could not find icon: ") + filename);
    
    // Load Fonts
    // io.Fonts->AddFontDefault();
    // Load Geomanist
    ImFont* font_geomanist = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 17.0f);
    if(!font_geomanist)
        Log::Error("Could not find font: Geomanist");




    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    


    
    
    
 {   // add gcodes to the stream
    
    // KILL ALARM LOCK
    //Grbl.Send("$X");
    
    // GRBL SETTINGS
    //Grbl.Send("$$");
    
    // GCODE PARAMETERS (coord systems)
    //Grbl.Send("$#");
    
    // MODAL STATES
    //Grbl.Send("$G");
    
    // BUILD INFO
    //Grbl.Send("$I");
    
    // STARTUP BLOCKS
    //Grbl.Send("$N");
    
    
    // CHECK MODE (send again to cancel)
    //Grbl.Send("$C");
    
    // RUN HOMING CYCLE
    //Grbl.Send("$H");
    
    // JOG
    //Grbl.Send("$J=G91 X10 F1000");
    //Grbl.SendRT(GRBL_RT_JOG_CANCEL);
    
    // RESET SETTINGS ( USE WITH CATION ) 
    // "$RST=$", "$RST=#", and "$RST=*"
    
    // SLEEP
    //Grbl.Send("$SLP");
    
    // Coordinate systems
    // Settings should not be streamed with the character-counting streaming protocols. Only the simple send-response protocol works. This is because during the EEPROM write, the AVR CPU also shuts-down the serial RX interrupt, which means data can get corrupted or lost. This is safe with the send-response protocol, because it's not sending data after commanding Grbl to save data.
    //  G10 L2, G10 L20, G28.1, G30.1
    
    /* PROBE 
    Grbl.Send("G91 G38.2 Z-200 F100\n");
    Grbl.Send("G91 G38.4 Z1 F100\n");
    */
    /*
    string file = "/home/pi/Desktop/New.nc";
    Grbl.FileRun(file);
*/
    
}

        // Our state
    bool show_demo_window = true;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // this should be called in main loop
        grbl.systemChecks();
        
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    
        drawFrames(grbl);
        
        // Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
     
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

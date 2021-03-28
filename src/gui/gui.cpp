
#include "../common.hpp"
using namespace std;

  

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int gui(const string& workingDir, GRBL* Grbl)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only


    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "XYZ Table", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    bool err = glewInit() != GLEW_OK;
    
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    string iniFileName = workingDir + "XYZTable.ini";
    io.IniFilename = iniFileName.c_str();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    ImFont* font_geomanist = io.Fonts->AddFontFromFileTTF("/home/pi/Desktop/Projects/grbl/fonts/geomanist-regular-webfont.ttf", 17.0f);
    IM_ASSERT(font_geomanist != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    
    
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
    ImGuiTextBuffer* consoleLog = new ImGuiTextBuffer();
    string grblReponse;
    
    Grbl->Connect();
    Grbl->SetStatusInterval(100);
    
    
 {   // add gcodes to the stream
    
    // KILL ALARM LOCK
    //Grbl->Send("$X");
    
    // GRBL SETTINGS
    //Grbl->Send("$$");
    
    // GCODE PARAMETERS (coord systems)
    //Grbl->Send("$#");
    
    // MODAL STATES
    //Grbl->Send("$G");
    
    // BUILD INFO
    //Grbl->Send("$I");
    
    // STARTUP BLOCKS
    //Grbl->Send("$N");
    
    
    // CHECK MODE (send again to cancel)
    //Grbl->Send("$C");
    
    // RUN HOMING CYCLE
    //Grbl->Send("$H");
    
    // JOG
    //Grbl->Send("$J=G91 X10 F1000");
    //Grbl->SendRT(GRBL_RT_JOG_CANCEL);
    
    // RESET SETTINGS ( USE WITH CATION ) 
    // "$RST=$", "$RST=#", and "$RST=*"
    
    // SLEEP
    //Grbl->Send("$SLP");
    
    
    
    /* PROBE 
    Grbl->Send("G91 G38.2 Z-200 F100\n");
    Grbl->Send("G91 G38.4 Z1 F100\n");
    */
    /*
    string file = "/home/pi/Desktop/New.nc";
    Grbl->FileRun(file);
*/
    


    
}
    


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        
        Grbl->Write();
        if(Grbl->Read(grblReponse)) {
            consoleLog->append(grblReponse.c_str());
            grblReponse.erase();
        }
        Grbl->RequestStatus();
     
        
        
        
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

        drawFrames(workingDir, Grbl, consoleLog);
        

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

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

    delete(consoleLog);
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

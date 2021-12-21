/*
 * frames.cpp
 *  Max Peglar-Willis 2021
 */ 

using namespace std;

#include "../common.h"

#define MAX_CUSTOM_GCODES 12 // should be divisible by 3
#define MAX_HISTORY 100
 

//******************************************************************************//
//**********************************FRAMES**************************************//

ImGuiWindowFlags general_window_flags = ImGuiWindowFlags_AlwaysAutoResize;


struct Console {
    bool log_Tab_Open = true;
    bool reclaim_focus;    
    bool autoScroll = true;
    
    vector<string> history;
    int historyPos = 0;

    Console() {
        Event<Event_ConsoleScrollToBottom>::RegisterHandler([this](Event_ConsoleScrollToBottom data) {
            (void)data;
            autoScroll = true;
        });
    }
    void DrawLogTab() {
        if (ImGui::BeginTabItem("Log")) {
            log_Tab_Open = true;
            // Console
            float h = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            ImGui::BeginChild("Log", ImVec2(0, -h), false,  ImGuiWindowFlags_AlwaysHorizontalScrollbar);
            // Clip only visible lines
            ImGuiListClipper clipper;
            clipper.Begin(Log::GetConsoleLogSize());
            while (clipper.Step()) {
                for (int line_no = clipper.DisplayStart;
                    line_no < clipper.DisplayEnd; line_no++) {
                    ImGui::TextUnformatted(Log::GetConsoleLog(line_no).c_str());
                }
            }
            clipper.End();

            // scroll to bottom
            float lineHeight = ImGui::GetTextLineHeightWithSpacing();
            
            float autoScroll_AvoidFlickeringDistance = lineHeight;
            float autoScroll_DetatchDistance = 2.0f * lineHeight;
            float autoScroll_JoinDistance = lineHeight - 1.0f;
             
            // scroll to bottom
            float yScrollDif = ImGui::GetScrollMaxY() - ImGui::GetScrollY();
            //cout << "lineHeight = " << lineHeight << "  AutoScroll = " << autoScroll << "  yScrollDif = " << yScrollDif << endl;
            // there is a frame lag from where the y scroll changes which causes a flickering of scroll position, this is a workaround
            if(!ImGui::GetIO().MouseWheel) {
                if(autoScroll && (yScrollDif > autoScroll_AvoidFlickeringDistance)) {
                    ImGui::SetScrollY(ImGui::GetScrollMaxY());
                } 
            }
            // if user scroll above, turn off autoscroll
            else if (yScrollDif > autoScroll_DetatchDistance) { // make line height * mult
                autoScroll = false;
            }
            // if user scrolled to bottom, set to autoscroll
            else if (yScrollDif < autoScroll_JoinDistance) { // make line height
                autoScroll = true;
            }
    
    
            ImGui::EndChild();
            //ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndTabItem();
        } 
    }

    void DrawCommandsTab(GRBL &grbl, GRBLVals& grblVals) {
        
        if (ImGui::BeginTabItem("Commands")) {
        
            log_Tab_Open = false;
            // Commands
            float h = ImGui::GetStyle().ItemSpacing.y +
                      ImGui::GetFrameHeightWithSpacing();
            ImGui::BeginChild("Commands", ImVec2(0, -h));
            ImGuiTableFlags flags =
                ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_Resizable |
                ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                ImGuiTableFlags_BordersV;
            if (ImGui::BeginTable("Commands", 3, flags, ImVec2(0, 0))) { // Make top row always visible
                ImGui::TableSetupScrollFreeze(0, 1);
                // Set up headers
                ImGui::TableSetupColumn("Sent", ImGuiTableColumnFlags_None,
                                        165.0f);
                ImGui::TableSetupColumn("Reponse", ImGuiTableColumnFlags_None,
                                        55.0f);
                ImGui::TableSetupColumn("Description",
                                        ImGuiTableColumnFlags_None, 600.0f);
                ImGui::TableHeadersRow();

                ImGuiListClipper clipper;
                // this sets the size
                clipper.Begin(grbl.getGCListSize());

                while (clipper.Step()) {
                    for (int row_n = clipper.DisplayStart;
                         row_n < clipper.DisplayEnd; row_n++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        // GCItem_t gcItem = grbl.gcList.GetItem(row_n);
                        const GCItem &gcItem = grbl.getGCItem(row_n);

                        ImGui::TextUnformatted(gcItem.str.c_str());
                        ImGui::TableNextColumn();
                        switch (gcItem.status) {
                        case STATUS_UNSENT:
                            ImGui::TextUnformatted("");
                            break;
                        case STATUS_SENT:
                            ImGui::TextUnformatted("Pending...");
                            break;
                        case STATUS_OK:
                            ImGui::TextUnformatted("Ok");
                            break;
                        default: // error
                            string errName, errDesc;
                            if (getErrMsg(gcItem.status, &errName, &errDesc)) {
                                ImGui::Text("Error %d: Can't find error code",
                                            gcItem.status);
                            } else {
                                ImGui::Text("Error %d", gcItem.status);
                                ImGui::TableNextColumn();
                                ImGui::Text("%s: %s", errName.c_str(),
                                            errDesc.c_str());
                            }
                        }
                    } 
                }
                
                if(grblVals.isFileRunning && autoScroll) {
                    // go down 2 cells further than the most recently acknowledged in gclist
                    uint index = grblVals.curLineIndex < 2 ? 0 : grblVals.curLineIndex - 2;
                    float curPos = index * ImGui::GetTextLineHeightWithSpacing();
                    ImGui::SetScrollY(curPos);
                }
                else if (autoScroll || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                    ImGui::SetScrollHereY(1.0f);

                ImGui::EndTable(); 
            }
            ImGui::EndChild();
            
                
            ImGui::EndTabItem();
        }
    }  
      

    void Draw(GRBL &grbl, Settings& settings) {
        GRBLVals& grblVals = settings.grblVals;
        
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float padding = settings.guiSettings.dockPadding;
        
        
        /*
        ImGui::SetNextWindowPos(viewport->WorkPos + ImVec2(padding, padding));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - 2.0f * padding, settings.guiSettings.toolbarHeight));
         
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse 
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        if (!ImGui::Begin("Run", NULL, general_window_flags | window_flags)) {
            ImGui::End();
            return;
        }*/
        static float windowHeight = 270.0f;
        // initialise
        ImGui::SetNextWindowSize(ImVec2(570, windowHeight), ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(ImVec2(500.0f, viewport->WorkSize.y - windowHeight - padding), ImGuiCond_Appearing);
        
        if (!ImGui::Begin("Console", NULL, ImGuiWindowFlags_None)) {
            ImGui::End();
            return;
        }
        windowHeight = ImGui::GetWindowHeight();
        
        
        //ImGuiModules::KeepWindowInsideViewport();
        
        // Disable all widgets when not connected to grbl
        ImGuiModules::BeginDisableWidgets(grblVals);

        if (ImGui::BeginTabBar("Console", ImGuiTabBarFlags_None)) {
            DrawLogTab();
            DrawCommandsTab(grbl, grblVals);

            if (!log_Tab_Open)
                autoScroll = false;
            reclaim_focus = false;

            ImGui::EndTabBar();
        }

        ImGui::Separator();

        static char strConsole[MAX_GRBL_BUFFER - 1] = "";

        if (ImGui::Button("Send"))
            sendToConsole(grbl, strConsole);

        ImGui::SameLine();

        ImGui::PushItemWidth(200); // use -ve for distance from right size

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                    ImGuiInputTextFlags_CallbackEdit |
                                    ImGuiInputTextFlags_CallbackHistory;
        // Console text input
        if (ImGui::InputText("Input GCode Here", strConsole,
                             IM_ARRAYSIZE(strConsole), flags,
                             &TextEditCallbackStub, (void *)this))
            sendToConsole(grbl, strConsole);

        ImGui::PopItemWidth();

        // Auto-focus
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::SameLine();

        if (log_Tab_Open) {
            if (ImGui::Button("Clear Log")) {
                Log::ClearConsoleLog();
            }
        } else {
            if (ImGui::Button("Clear Commands")) {
                grbl.clearCompleted();
            }
        }

        // Disable all widgets when not connected to grbl
        ImGuiModules::EndDisableWidgets(grblVals);
        ImGui::End();
    }

    void updateHistory(char *str) {
        history.push_back(str);
        if (history.size() > MAX_HISTORY) {
            history.erase(history.begin());
        }
        historyPos = history.size();
    }

    void sendToConsole(GRBL &grbl, char *str) {
        if (str[0]) {
            updateHistory(str);
            grbl.send(str);
            strncpy(str, "", 128);
        }
        reclaim_focus = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding
    // callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData *data) {
        Console *console = (Console *)data->UserData;
        return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiInputTextCallbackData *data) {
        // reset history position if user types anything
        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
            historyPos = history.size();
        // select through history of previous sent commands
        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
            if (history.size() == 0)
                return 0;

            if (data->EventKey == ImGuiKey_UpArrow) {
                if (historyPos > 0)
                    historyPos--;
                else
                    return 0;
            } else if (data->EventKey == ImGuiKey_DownArrow) {
                if (historyPos < (int)history.size() - 1)
                    historyPos++;
                else
                    return 0;
            }
            const char *history_str =
                (history.size() > 0) ? history[historyPos].c_str() : "";
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, history_str);
        }
        return 0;
    }
};

struct Toolbar {
        
    //enum ToolbarCommand { None, OpenFile, Function };
    enum Export         { False, Pending, True };
          
    std::unique_ptr<FileBrowser> fileBrowser;
    Timer timer;


    Toolbar(Settings& settings) 
    {
        fileBrowser = std::make_unique<FileBrowser>(&settings.p.system.curDir);
        
        Event<Event_ResetFileTimer>::RegisterHandler([&](Event_ResetFileTimer data) {
            (void)data;
            timer.Reset();
        });
        
    }
     
    bool EditButton(Settings& settings, const char* id) 
    {
        ImVec2& buttonImgSize = settings.guiSettings.button[ButtonType::Edit].ImageSize;
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f,0.0f,0.0f,0.0f));
            ImGui::PushID(id);
                bool clicked = ImGui::ImageButton(settings.guiSettings.img_Edit, buttonImgSize);
            ImGui::PopID();
        ImGui::PopStyleColor();
        return clicked;
    }
    void DrawTitle(Settings& settings, const std::string& name) 
    {        
        ImGui::PushFont(settings.guiSettings.font_small);
            ImGui::PushStyleColor(ImGuiCol_Text, settings.guiSettings.colour[Colour::HeaderText]);
                ImGui::TextUnformatted(name.c_str());
            ImGui::PopStyleColor();
        ImGui::PopFont();
    }
    bool DrawTitleWithEdit(Settings& settings, const std::string& name) 
    {
        DrawTitle(settings, name);
        ImGui::SameLine();
        return EditButton(settings, name.c_str());
    }

    //void DrawTableSeperator(Settings& settings) 
    //{
    //    // draw vertical seperator line
    //    float spacerWidth = settings.guiSettings.toolbarSpacer;
    //    ImVec2 p0 = ImGui::GetCursorScreenPos() + ImVec2(spacerWidth / 2.0f, GImGui->Style.ItemSpacing.y / 2.0f);
    //    ImVec2 p1 = p0 + ImVec2(1.0f /*thickness*/, 2.0f*ImGui::GetFrameHeight() /*length*/);
    //    ImGui::GetWindowDrawList()->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_Separator));
    //};
    
    void Draw(GRBL &grbl, Settings& settings) 
    {        
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float padding = settings.guiSettings.dockPadding;
        
        ImGui::SetNextWindowPos(viewport->WorkPos + ImVec2(padding, padding));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - 2.0f * padding, settings.guiSettings.toolbarHeight));
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse 
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        if (!ImGui::Begin("Toolbar", NULL, general_window_flags | window_flags)) {
            ImGui::End();
            return;
        }
        
        static Functions functions(settings);
        static ToolSettings toolSettings;
                        
        //static ToolbarCommand toolbarCommand = ToolbarCommand::None;
        bool openPopup_ConnectSettings  = false;
        bool openPopup_FileBrowser      = false;
        bool openPopup_Tools            = false;
        
        
        ImGuiModules::BeginDisableWidgets(settings.grblVals);
        
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(settings.guiSettings.toolbarSpacer / 2.0f, 2.0f));
        
        if (ImGui::BeginTable("Toolbar", 5, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_BordersInnerV)) 
        {           
            // Draw titles
            ImGui::TableNextRow(ImGuiTableRowFlags_None, settings.guiSettings.button[ButtonType::Edit].ImageSize.y + 5.0f);
                
            if(ImGui::TableNextColumn()) { openPopup_ConnectSettings =  DrawTitleWithEdit(settings, "Connect"); }
            if(ImGui::TableNextColumn()) {                              DrawTitle(settings, "Open File"); }
            if(ImGui::TableNextColumn()) {                              DrawTitle(settings, "Functions"); }
            if(ImGui::TableNextColumn()) { openPopup_Tools           =  DrawTitleWithEdit(settings, "Tools"); }
         
            ImGui::TableNextRow();
            
                // Connect
                if(ImGui::TableNextColumn()) {
                    ImGuiModules::EndDisableWidgets(settings.grblVals);
                    
                    ImGui::BeginGroup();
                        DrawConnect(grbl, settings);
                    ImGui::EndGroup();
                    
                    ImGuiModules::BeginDisableWidgets(settings.grblVals);
                }
                
                // Open File
                if(ImGui::TableNextColumn()) {

                    ImGui::BeginGroup();
                        if(DrawOpenFile(settings, !functions.IsActiveFunctionSelected(false))) {
                            functions.DeselectActive();
                            // clear screen
                            Event<Event_Update3DModelFromVector>::Dispatch({ std::vector<std::string>(/*empty*/) }); 
                            //Event<Event_DisplayShapeOffset>::Dispatch( { std::vector<glm::vec2>(/*empty*/), std::vector<glm::vec2>(/*empty*/), false } );
                            openPopup_FileBrowser = true;
                        }
                    ImGui::EndGroup();
                }
                
                // Functions
                if(ImGui::TableNextColumn()) {
                                       
                        ImGui::BeginGroup();
                            functions.Draw_Functions(settings); 
                            ImGui::SameLine();
                            functions.Draw_ActiveFunctions(settings); 
                        ImGui::EndGroup();
                }
                    
                // Tools
                if(ImGui::TableNextColumn()) {

                    ImGui::BeginGroup();
                        // updates viewer if tool or material is changed
                        if(toolSettings.Draw(settings)) {
                            functions.Update3DViewOfActiveFunction(settings);
                        }
                        
                    ImGui::EndGroup();
                }    
                /*
                // Sketch
                if(ImGui::TableNextColumn()) {

                    ImGui::BeginGroup();
                        functions.DrawSketch();
                    ImGui::EndGroup();
                }    */
                
            ImGui::EndTable();
        } 
    
        ImGui::PopStyleVar();
        
        
        
        
        
        
        
        /*
        
        if (ImGui::BeginTabBar("ToolbarTabs"))
        {
            if (ImGui::BeginTabItem("Connect")) {
                toolbarCommand = ToolbarCommand::None;
                // Connect / disconnect button
                ImGui::BeginGroup();
                    DrawConnect(grbl, settings);
                    DrawTitle(settings, "Connect");
                    ImGui::SameLine();
                    ImVec2& buttonSize      = settings.guiSettings.button[ButtonType::Edit].Size;
                    ImVec2& buttonImgSize   = settings.guiSettings.button[ButtonType::Edit].ImageSize;
                    if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_Edit)) {
                        
                    }
                ImGui::EndGroup();
                ImGui::EndTabItem();
            }            
            // Disable all widgets when not connected to grbl
            ImGuiModules::BeginDisableWidgets(settings.grblVals);
            
            if (ImGui::BeginTabItem("Open File")) {
                // if tab has been clicked (first time only)
                if(toolbarCommand != ToolbarCommand::OpenFile) {
                    Update3DViewOfFile(settings);
                }             
                toolbarCommand = ToolbarCommand::OpenFile;
                
                if(DrawOpenFile(settings, fileBrowser->CurrentFilePath())) {
                    openPopup_FileBrowser = true;
                }
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Functions")) {
                // if tab has been clicked (first time only)
                if(toolbarCommand != ToolbarCommand::Function) {
                    functions.Update3DViewOfActiveFunction(settings);
                }
                toolbarCommand = ToolbarCommand::Function;
                
                functions.Draw(grbl, settings);
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }*/
        
        ImGui::Separator();
        
        // allows us to determine whether file should be exported (creates popup if file is to be overwritten)
        static pair<Export, string> exportFileName = make_pair(Export::False, "");
        
        //if(toolbarCommand == ToolbarCommand::OpenFile) {
        if(!functions.IsActiveFunctionSelected(false)) {
            DrawTitle(settings, va_str("Commands (%s)", fileBrowser->CurrentFile().c_str())); 
            //DrawCurrentFile(settings, fileBrowser->CurrentFile());
            DrawPlayButtons(grbl, settings, [&]() {
                RunFile(grbl, settings);
            });
        } else {
        //else if(toolbarCommand == ToolbarCommand::Function) {
            DrawTitle(settings, va_str("Commands (%s)", functions.ActiveFunctionName().c_str())); 
            DrawPlayButtons(grbl, settings, [&]() {
                functions.RunActiveFunction(grbl, settings);
            });
            
            ImGui::SameLine(); ImGui::Dummy(ImVec2(50, 0)); ImGui::SameLine();
        
            ImGui::BeginGroup();
                ImVec2& buttonSize      = settings.guiSettings.button[ButtonType::Secondary].Size;
                
                ImGuiModules::CentreItemVertically(2, buttonSize.y);

                // add GCodes to file
                if(ImGui::Button("Export", buttonSize)) {
                    if(functions.IsActiveFunctionSelected()) {
                        std::string filepath = functions.GetActiveFunctionFilepath(settings.p.system.saveFileDirectory);
                        exportFileName = make_pair(Export::Pending, filepath);
                    } 
                }
                ImGui::SameLine();
                
                if(ImGui::Button("Delete", buttonSize)) {
                    functions.DeleteActiveFunction(settings);
                }
            ImGui::EndGroup();
        }
        
        ImGui::SameLine(); ImGui::Dummy(ImVec2(50, 0)); ImGui::SameLine();
        
        ImGui::BeginGroup();
            DrawTimeElapsed(settings.grblVals);
        ImGui::EndGroup(); 

        // handle file export
        DoesFileNeedExport(settings, functions, exportFileName);
        
        // show file browser if visible
        if(openPopup_FileBrowser) { fileBrowser->Open(); }
        fileBrowser->Draw();
        
        // edit tools popup (update viewer if setting is changed)
        if(openPopup_Tools) { ImGui::OpenPopup("Edit Tools"); }
        if(toolSettings.DrawPopup_Tools(settings)) {    
            functions.Update3DViewOfActiveFunction(settings);
        }
        
        // Disable all widgets when not connected to grbl
        ImGuiModules::EndDisableWidgets(settings.grblVals);
        
        // Connect Settings        
        if(openPopup_ConnectSettings) { ImGui::OpenPopup("Edit Connect Popup"); }
        DrawPopup_ConnectSettings(settings);
        
        ImGui::End();
    }
    
    void DrawConnect(GRBL &grbl, Settings& settings) 
    {
        GRBLVals& grblVals = settings.grblVals;
        ImVec2& buttonSize      = settings.guiSettings.button[ButtonType::Primary].Size;
        
        ImGui::BeginGroup();
            ImGuiModules::CentreItemVertically(2, buttonSize.y);
            if (!grblVals.isConnected) {
                if (ImGui::Button("Connect", buttonSize)) {
                    grbl.connect(settings.p.system.serialDevice, stoi(settings.p.system.serialBaudrate));
                }
            } 
            else { //if (grblVals.isConnected)
                if (ImGui::Button("Disconnect", buttonSize)) {
                    grbl.disconnect();
                }
            }
        ImGui::EndGroup();           
    }
    
    void DrawPopup_ConnectSettings(Settings& settings) 
    {
        static bool popupOpen = false;
        if (ImGui::BeginPopup("Edit Connect Popup")) {
            popupOpen = true;
            
            ImGui::SetNextItemWidth(80.0f);
            ImGui::InputText("Device", &settings.p.system.serialDevice);
            ImGui::SetNextItemWidth(80.0f);
            ImGui::InputText("Baudrate", &settings.p.system.serialBaudrate, ImGuiInputTextFlags_CharsDecimal);
            
            ImGui::EndPopup();
        } else { // if popup has just been closed
            if (popupOpen == true) {
                Event<Event_SaveSettings>::Dispatch({ }); 
                popupOpen = false;
            }
        }
    }
    
    bool DrawOpenFile(Settings& settings, bool isSelected)
    {
        bool openHasBeenPressed = false;
        ImVec2& buttonSize = settings.guiSettings.button[ButtonType::Primary].Size;
            
        ImGui::BeginGroup();
            ImGuiModules::CentreItemVertically(2, buttonSize.y);
             
            if(isSelected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
                if (ImGui::Button("Open", buttonSize)) {
                    if(!settings.grblVals.isFileRunning) {
                        openHasBeenPressed = true;
                    } else {
                        Log::Error("A file is running. This must finish before opening another");
                    }
                }
            if(isSelected) ImGui::PopStyleColor();
        ImGui::EndGroup();
        return openHasBeenPressed;
    }
    
    void DrawCurrentFile(Settings& settings, const std::string& currentFilePath)
    {
        ImGui::BeginGroup();
            // shortens file path to "...path/at/place.gc" if length > max_FilePathDisplay
            int cutOff = 0;
            if(currentFilePath.size() > settings.guiSettings.max_FilePathDisplay) {
                cutOff = currentFilePath.size() - settings.guiSettings.max_FilePathDisplay;     
            }                 
            const char* shortenedPath = currentFilePath.c_str() + cutOff;
            ImGui::Text((cutOff ? "Current File: %s" : "Current File: ..%s"), shortenedPath);
        ImGui::EndGroup();
    }
    
    void Update3DViewOfFile(Settings& settings) {
        // check we have selected a file
        if (fileBrowser->CurrentFile() == "") {
            return;
        }
        // get filepath of file
        string filepath = File::CombineDirPath(settings.p.system.curDir, fileBrowser->CurrentFile());
        Event<Event_Update3DModelFromFile>::Dispatch({filepath}); 
    }
    
    void RunFile(GRBL& grbl, Settings& settings) {
        // check we have selected a file
        if (fileBrowser->CurrentFile() == "") {
            Log::Error("No file has been selected");
            return;
        }
        // get filepath of file
        string filepath = File::CombineDirPath(settings.p.system.curDir, fileBrowser->CurrentFile());
        // add to log
        Log::Info(string("Sending File: ") + filepath);
        
        // start file timer
        Event<Event_ResetFileTimer>::Dispatch({});
         
        // send file to grbl
        if (grbl.sendFile(filepath)) {
            // couldn't open file
            Log::Error("Couldn't send file to grbl");
        }
    }
    
    void DrawPlayButtons(GRBL& grbl, Settings& settings, std::function<void()> callback)
    {
        ImVec2& buttonSize      = settings.guiSettings.button[ButtonType::Secondary].Size;
        ImVec2& buttonImgSize   = settings.guiSettings.button[ButtonType::Secondary].ImageSize;
        
        ImGui::BeginGroup();
            ImGuiModules::CentreItemVertically(2, buttonSize.y);

            if (ImGui::Button("Run", buttonSize)) {
                callback();
            }
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", buttonSize)) {
                if (settings.grblVals.isFileRunning) {
                    Log::Info("Cancelling... Note: Any commands remaining in grbl's buffer will still execute.");
                    grbl.cancel();
                }
            }
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(50, 0));
            ImGui::SameLine();

            ImGui::SameLine();
            if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_Pause)) {
                Log::Info("Pausing...");
                grbl.sendRT(GRBL_RT_HOLD);
            }

            ImGui::SameLine();
            if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_Restart)) {
                Log::Info("Resuming...");
                grbl.sendRT(GRBL_RT_RESUME);
            }
        ImGui::EndGroup(); 
    } 
    
    void DrawTimeElapsed(GRBLVals& grblVals)
    {
        
        if (grblVals.isFileRunning) 
        {
            // update time
            timer.UpdateCurrentTime();
            // normalise timer seconds to hours/mins/secs
            Time timeElapsedNorm(timer.dt());
            // how far through file we are
            float percComplete = (float)grblVals.curLine / (float)grblVals.totalLines;
            // estimate time remaining
            uint timeExpected = timer.dt() / percComplete;
            Time timeRemainingNorm(timeExpected - timer.dt());
            
            // draw ImGui widgets
            float cursorPos_FrameTop = ImGui::GetCursorPosY();
            
            // progress bar
            ImGuiModules::CentreItemVertically(2);
            ImGui::ProgressBar(percComplete, ImVec2(0.0f, 0.0f), va_str("%d/%d (%.2f%%)", grblVals.curLine, grblVals.totalLines, 100.0f * percComplete).c_str());
            
            ImGui::SameLine();
            
            ImGui::SetCursorPosY(cursorPos_FrameTop);
            ImGui::BeginGroup();
                ImGui::TextUnformatted("Time Elapsed:");
                ImGui::TextUnformatted("Time Remaining:");
            ImGui::EndGroup();
            
            ImGui::SameLine();
            
            ImGui::BeginGroup();
                ImGui::Text("%s", timeElapsedNorm.TimeString().c_str());
                ImGui::Text("%s", timeRemainingNorm.TimeString().c_str());
            ImGui::EndGroup();
                        
        }
    }
    
    
    void DoesFileNeedExport(Settings& settings, Functions& functions, std::pair<Export, std::string>& exportFileName)
    {   // if file doesn't exist, set flag to export. 
        // If file does exist, show popup to confirm overwrite
        if(exportFileName.first == Export::Pending) {
            if(!File::Exists(exportFileName.second)) {
                exportFileName.first = Export::True;
            } else {
                ImGui::OpenPopup("Overwrite File Popup");
            }
        }
        // draw the popup if visible
        DrawPopup_OverwriteFile(exportFileName);
        // export file when ready and reset the flag
        if(exportFileName.first == Export::True) {
            Log::Info("Exporting file: %s", exportFileName.second.c_str());
            functions.SaveActiveFunction(settings, exportFileName.second);
            exportFileName = make_pair(Export::False, "");
        }
    }
            
    void DrawPopup_OverwriteFile(std::pair<Export, std::string>& exportFileName)
    {       
        // Always center this window when appearing
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Overwrite File Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("The following file already exists:\n\n%s\n\nAre you sure you want to overwrite it?", exportFileName.second.c_str());
            ImGui::Separator();

            if (ImGui::Button("Overwrite", ImVec2(120, 0))) { 
                exportFileName.first = Export::True;
                cout << "Overwrite" << endl;
                ImGui::CloseCurrentPopup(); 
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
                exportFileName = make_pair(Export::False, "");
                cout << "Cancel" << endl;
                ImGui::CloseCurrentPopup(); 
            }
            ImGui::EndPopup();
        }
    }
};

/*
x   std::string state;
    // either of these are given
    // WPos = MPos - WCO
    glm::vec3 MPos;
x    glm::vec3 WPos;
    // this is given every 10 or 30 status messages
    glm::vec3 WCO;
    int lineNum;
    float feedRate;    // mm/min or inches/min
    int spindleSpeed; // rpm

    bool inputPin_LimX;
    bool inputPin_LimY;
    bool inputPin_LimZ;
    bool inputPin_Probe;
    bool inputPin_Door;
    bool inputPin_Hold;
    bool inputPin_SoftReset;
    bool inputPin_CycleStart;

    int override_Feedrate;
    int override_RapidFeed;
    int override_SpindleSpeed;

    int accessory_SpindleDir;
    int accessory_FloodCoolant;
    int accessory_MistCoolant;

*/

struct Stats {

    // sizes of buttons
    ImVec2 sizeL = {80.0f, 60.0f};
    ImVec2 sizeS = {55.0f, 30.0f};
    ImVec2 posS = (sizeL - sizeS) / 2;

    ImVec2 sizeCust = sizeL;
    ImVec2 sizeCustNew = {25.0, 25.0f};
    ImVec2 posCustNew = (sizeCust - sizeCustNew) / 2;

    //Stats() { importCustomGCs(); }

//#define IMPORT_TYPE_NONE 0
//#define IMPORT_TYPE_CUSTOMGC 1
/*
    void importCustomGCs(Settings& settings) {
        
        if(settings.UpdateFromFile()) {
            Log::Error("Could not update settings from file");
        }
        
        return;
        
        
        
        
        vector<string> fileBuf;

        auto executeLine = [&fileBuf](string &str) {
            if (str != "")
                fileBuf.push_back(str);
            return 0;
        };

        string filename = File::ThisDir("config.ini");
        if (File::Read(filename, executeLine)) {
            Log::Error("No config file found");
            return;
        }

        // clear existing values
        customGC.clear();

        // initialise type of import
        int importType = IMPORT_TYPE_NONE;

        // create buffer
        customGC_t gcBuf;

        for (string line : fileBuf) {
            // If heading, set import type
            if (line.substr(0, 13) == "[CustomGCode]") {
                importType = IMPORT_TYPE_CUSTOMGC;
                continue;
            }

            // If value...
            switch (importType) {
            case IMPORT_TYPE_CUSTOMGC:
                if (line.substr(0, 5) == "Name=")
                    gcBuf.name = line.substr(5);
                else if (line.substr(0, 6) == "GCode=") {
                    gcBuf.gcode = line.substr(6);
                    customGC.push_back(gcBuf);
                    // clear
                    gcBuf.name = gcBuf.gcode = "";
                }
                break;
            default:
                Log::Error("Unknown import type");
                break;
            }
        }
    }   
*/


   

    void DrawStatus(GRBL &grbl, Settings& settings, float dt) 
    {
        GRBLVals& grblVals = settings.grblVals;
        // current state colour
        ImVec4 colour;
        if (grblVals.status.state <= GRBLState::Status_Sleep) // idle
            colour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        else if (grblVals.status.state <= GRBLState::Status_Home) // motion
            colour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        else { // alert
            // timer used to flash alarm state
            static float time = 0.0f;
            static float flashTime = 1.0f; // seconds
            time += dt;
            colour = (time < flashTime) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            if(time >= 2.0f * flashTime) {
                time = 0.0f;
            }
        }

        // current state
        ImGui::PushFont(settings.guiSettings.font_large);
            ImGui::PushStyleColor(ImGuiCol_Text, colour);
                ImGuiModules::TextUnformattedCentredHorizontallyInTable(grbl.sys.status.stateStr(grblVals.status.state).c_str());
            ImGui::PopStyleColor();
        ImGui::PopFont();
        
        static bool showBufferState = false;
        showBufferState = ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
        
        if(showBufferState) {
            ImGui::SameLine();
            ImGui::Text("%d, %d", grblVals.status.bufferPlannerAvail, grblVals.status.bufferSerialAvail);
        }
         
    } 
    
    void DrawPosition(Settings& settings) {
        GRBLVals& grblVals = settings.grblVals;
        static bool showMachineCoords = false;
        
        if (ImGui::BeginTable("PosiMAX_CUSTOM_GCODEStion", 3, ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); 
            ImGui::PushStyleColor(ImGuiCol_Text, settings.guiSettings.colour[Colour::HeaderText]);
                ImGuiModules::TextUnformattedCentredHorizontallyInTable("X");
                ImGui::TableSetColumnIndex(1);
                ImGuiModules::TextUnformattedCentredHorizontallyInTable("Y");
                ImGui::TableSetColumnIndex(2);
                ImGuiModules::TextUnformattedCentredHorizontallyInTable("Z"); 
            ImGui::PopStyleColor();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGuiModules::TextCentredHorizontallyInTable("%.3f", showMachineCoords ? grblVals.status.MPos.x : grblVals.status.WPos.x);
            ImGui::TableSetColumnIndex(1);
            ImGuiModules::TextCentredHorizontallyInTable("%.3f", showMachineCoords ? grblVals.status.MPos.y : grblVals.status.WPos.y);
            ImGui::TableSetColumnIndex(2);
            ImGuiModules::TextCentredHorizontallyInTable("%.3f", showMachineCoords ? grblVals.status.MPos.z : grblVals.status.WPos.z);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGuiModules::TextUnformattedCentredHorizontallyInTable( grblVals.settings.units_Distance.c_str()); // mm
            ImGui::TableSetColumnIndex(1);
            ImGuiModules::TextUnformattedCentredHorizontallyInTable(grblVals.settings.units_Distance.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGuiModules::TextUnformattedCentredHorizontallyInTable(grblVals.settings.units_Distance.c_str());

            ImGui::EndTable();
        }
        showMachineCoords = ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
        if(showMachineCoords) {
            ImGui::TextUnformatted("MPos");
        }
    }
    
    /*
    struct GRBLModal_vals{
        std::string StartupBlock[2];
        float MotionMode            = 0.0f;     // *G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
        uint CoordinateSystem       = 0;        // *G54, G55, G56, G57, G58, G59
        uint Plane                  = 0;        // *G17, G18, G19
        uint DistanceMode           = 0;        // *G90, G91
        float ArcIJKDistanceMode    = 91.1f;    // *G91.1
        uint FeedRateMode           = 0;        // G93, *G94
        uint UnitsMode              = 0;        // G20, *G21
        uint CutterRadCompensation  = 40;       // *G40
        float ToolLengthOffset      = 49.0f;    // G43.1, *G49
        uint ProgramMode            = 0;        // *M0, M1, M2, M30
        uint SpindleState           = 0;        // M3, M4, *M5 
        uint CoolantState           = 0;        // M7, M8, *M9

        uint toolNumber             = 0;
        float spindleSpeed          = 0.0f;
        float feedRate              = 0.0f;    
    };
     */  
      
    void DrawCurrentMode(Settings& settings) {
                
        GRBLVals& grblVals = settings.grblVals;
        if (ImGui::BeginTable("Modals", 4, ImGuiTableFlags_NoSavedSettings)) {

            ImGui::TableNextRow();
            
            ImGui::PushStyleColor(ImGuiCol_Text, settings.guiSettings.colour[Colour::HeaderText]);
                ImGui::TableNextColumn();
                ImGuiModules::TextCentredHorizontallyInTable("Motion");
                ImGui::TableNextColumn();
                ImGuiModules::TextCentredHorizontallyInTable("Coord");
                ImGui::TableNextColumn();
                ImGuiModules::TextCentredHorizontallyInTable("Plane");
                ImGui::TableNextColumn();
                ImGuiModules::TextCentredHorizontallyInTable("Mode");
            ImGui::PopStyleColor();
            
            
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGuiModules::TextCentredHorizontallyInTable("G%g", grblVals.modal.MotionMode);
            ImGui::TableNextColumn();
            ImGuiModules::TextCentredHorizontallyInTable("G%u", grblVals.modal.CoordinateSystem);
            ImGui::TableNextColumn();
            ImGuiModules::TextCentredHorizontallyInTable("G%u", grblVals.modal.Plane);
            ImGui::TableNextColumn();
            ImGuiModules::TextCentredHorizontallyInTable("G%u", grblVals.modal.DistanceMode);
                
            ImGui::EndTable();
        }
    }
    
    void DrawMotion(Settings& settings) {
        GRBLVals& grblVals = settings.grblVals;
        GRBLStatus_vals &status = grblVals.status;
        GRBLSettings_vals &grblSettings = grblVals.settings;

        //static float f[] = {0.0f};
        static float s[] = {0.0f};

        if (ImGui::BeginTable("Motion", 4, ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            // ImGui::Dummy(ImVec2(10.0f,0));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
            
            // feed rate
            static float feedValues[50] = {};
            static int f_offset = 0;
            //f[0] = status.feedRate;
            feedValues[f_offset] = status.feedRate;
            f_offset = (f_offset + 1) % IM_ARRAYSIZE(feedValues);
            
            //ImGui::PlotHistogram("", feedValues, IM_ARRAYSIZE(feedValues), 0, NULL, 0.0f, settings.max_FeedRate, ImVec2(20.0f, ImGui::GetFrameHeightWithSpacing() * 3));
            
            ImGui::PlotLines("", feedValues, IM_ARRAYSIZE(feedValues), f_offset, NULL, 0.0f, grblSettings.max_FeedRate, ImVec2(0.0f, 80.0f));
            
            
            
            
            ImGui::TableSetColumnIndex(1);
            ImGui::TextColored(settings.guiSettings.colour[Colour::HeaderText], "Feed Rate");
            ImGui::Text("%.0f", status.feedRate);
            ImGui::TextUnformatted(grblSettings.units_Feed.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);
            s[0] = (float)status.spindleSpeed;
            ImGui::PlotHistogram("", s, IM_ARRAYSIZE(s), 0, NULL, (float)grblSettings.min_SpindleSpeed, (float)grblSettings.max_SpindleSpeed, ImVec2(20.0f, ImGui::GetFrameHeightWithSpacing() * 3));

            ImGui::TableSetColumnIndex(3);
            ImGui::TextColored(settings.guiSettings.colour[Colour::HeaderText], "Spindle");
            ImGui::Text("%.0d", status.spindleSpeed);
            ImGui::TextUnformatted("RPM");

            ImGui::EndTable();
        }
    }

    void DrawInputPins(GRBLVals& grblVals) {
        if (ImGui::BeginTable("InputPins", 4, ImGuiTableFlags_NoSavedSettings, ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 2))) {
            GRBLStatus_vals &status = grblVals.status;

            // orange
            ImVec4 colour = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (status.inputPin_LimX)
                ImGui::TextColored(colour, "Limit X");
            ImGui::TableNextColumn();

            if (status.inputPin_LimY)
                ImGui::TextColored(colour, "Limit Y");
            ImGui::TableNextColumn();

            if (status.inputPin_LimZ)
                ImGui::TextColored(colour, "Limit Z");
            ImGui::TableNextColumn();

            if (status.inputPin_Probe)
                ImGui::TextColored(colour, "Probe");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (status.inputPin_Door)
                ImGui::TextColored(colour, "Door");
            ImGui::TableNextColumn();

            if (status.inputPin_Hold)
                ImGui::TextColored(colour, "Hold");
            ImGui::TableNextColumn();

            if (status.inputPin_SoftReset)
                ImGui::TextColored(colour, "Reset");
            ImGui::TableNextColumn();

            if (status.inputPin_CycleStart)
                ImGui::TextColored(colour, "Start");

            ImGui::EndTable();
        }
    }

    void DrawZeroing(GRBL &grbl) {
        posS.y = 0;

        if (ImGui::BeginTable("XYZ Zeroing", 3,
                              ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
            if (ImGui::Button("Zero X", sizeS)) {
                Log::Info(
                    "Setting current X position to 0 for this coord system");
                grbl.send("G10 L20 P0 X0", PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle);
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
            if (ImGui::Button("Zero Y", sizeS)) {
                Log::Info(
                    "Setting current Y position to 0 for this coord system");
                grbl.send("G10 L20 P0 Y0", PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle);
            }

            ImGui::TableSetColumnIndex(2);
            ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
            if (ImGui::Button("Zero Z", sizeS)) {
                Log::Info(
                    "Setting current Z position to 0 for this coord system");
                grbl.send("G10 L20 P0 Z0", PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle);
            }
            // position cursor to bottom corner of cell so it doesnt clip
            // ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);

            ImGui::EndTable();
        }
    }

    void DrawCommands(GRBL &grbl, GRBLVals& grblVals) {

        if (ImGui::BeginTable("Commands", 3, ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 0.6f)));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                ImGui::GetColorU32(ImVec4(1.0f, 0.8f, 0.8f, 0.6f)));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                ImGui::GetColorU32(ImVec4(1.0f, 0.6f, 0.6f, 0.6f)));
            if (ImGui::Button("Reset", sizeL))
                grbl.softReset();
            ImGui::PopStyleColor(3);

            ImGui::TableSetColumnIndex(1);

            if (grblVals.status.state >= GRBLState::Status_Alarm) { // alert
                ImGui::PushStyleColor(
                    ImGuiCol_Button,
                    ImGui::GetColorU32(ImVec4(1.0f, 0.5f, 0.0f, 0.6f)));
                ImGui::PushStyleColor(
                    ImGuiCol_ButtonHovered,
                    ImGui::GetColorU32(ImVec4(1.0f, 0.8f, 0.8f, 0.6f)));
                ImGui::PushStyleColor(
                    ImGuiCol_ButtonActive,
                    ImGui::GetColorU32(ImVec4(1.0f, 0.8f, 0.6f, 0.6f)));
            }
            if (ImGui::Button(" Clear\nAlarms", sizeL))
                grbl.send("$X");
            if (grblVals.status.state >= GRBLState::Status_Alarm) // alert
                ImGui::PopStyleColor(3);

            ImGui::TableSetColumnIndex(2);
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImGui::GetColorU32(ImVec4(0.6f, 0.8f, 0.6f, 0.6f)));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                ImGui::GetColorU32(ImVec4(0.8f, 0.9f, 0.8f, 0.6f)));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                ImGui::GetColorU32(ImVec4(0.6f, 0.8f, 0.6f, 0.6f)));
            if (ImGui::Button("Home", sizeL))
                grbl.send("$H");
            ImGui::PopStyleColor(3);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(1);

            if (grblVals.isCheckMode) {
                static ImVec4 activeCol =
                    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      ImGui::GetColorU32(activeCol));
            }
            if (ImGui::Button("Check\nMode", sizeL))
                grbl.send("$C");
            if (grblVals.isCheckMode)
                ImGui::PopStyleColor(1);

            ImGui::EndTable();
        }
    }

    void DrawCustomGCodes(GRBL &grbl, Settings& settings) { // splits string using
        auto sendCustomGCode = [&grbl](string gcodestr) {
            istringstream s(gcodestr);
            string segment;
            for (string segment; getline(s, segment, ';');) {
                if (segment != "")
                    grbl.send(segment);
            }
        };

        static int customGCIndex = 0;

        if (ImGui::BeginTable("Commands", 3, ImGuiTableFlags_NoSavedSettings)) 
        {
            size_t gcodeCount = settings.p.customGCodes.size();
            
            for (size_t i = 0; i <= gcodeCount; i++) {
                if (i < MAX_CUSTOM_GCODES) {
                    int rowIndex = i % 3;
                    // New row for every 3 custom gcodes
                    if (rowIndex == 0)
                        ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(rowIndex);
                    // ImGui::SetCursorPos(posM);
                    // Add new button
                    if (i == gcodeCount) {
                        // centre button in cell
                        ImGui::SetCursorPos(ImGui::GetCursorPos() + posCustNew);
                        if (ImGui::ImageButton(sizeCustNew, ImVec2(16.0f,16.0f), settings.guiSettings.img_Add)) {
                        //if (ImGui::Button("+", sizeCustNew)) {
                            if (gcodeCount < MAX_CUSTOM_GCODES)
                                settings.p.customGCodes.push_back({ "", "" });
                        }  
                        // Custom GCode button
                    } else {

                        if (ImGui::Button(settings.p.customGCodes[i].name.c_str(), sizeCust))
                            sendCustomGCode(settings.p.customGCodes[i].gcode);

                        if (ImGuiModules::RightClickedLastItem()) {
                            customGCIndex = i;
                            ImGui::OpenPopup("Custom GCode");
                        }
                    }
                }
            }
            static bool popupOpen = false;
            if (ImGui::BeginPopup("Custom GCode")) {
                popupOpen = true;
                ImGui::Text("Custom GCode #%d", customGCIndex + 1);
                ImGui::SameLine();
                ImGuiModules::HelpMarker("GCode lines should be seperated with a semi-colon ("
                           ";"
                           ")");
                ImGui::SameLine();

                int delButtonW = 60;
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() -
                                     (delButtonW + 10));
                if (ImGui::Button("Delete", ImVec2(delButtonW, 0))) {
                    settings.p.customGCodes.erase(settings.p.customGCodes.begin() + customGCIndex);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::InputText("Name", &settings.p.customGCodes[customGCIndex].name);
                ImGui::InputTextMultiline("GCode", &settings.p.customGCodes[customGCIndex].gcode);

                ImGui::EndPopup();
            } else { // if popup has just been closed
                if (popupOpen == true) {
                    Event<Event_SaveSettings>::Dispatch({ }); 
                    popupOpen = false;
                }
            }
            ImGui::EndTable();
        }
    }

    void Draw(GRBL &grbl, Settings& settings, float dt) {
        GRBLVals& grblVals = settings.grblVals;
        // Initialise
        // ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
        if (!ImGui::Begin("Stats", NULL, general_window_flags)) {
            ImGui::End();
            return;
        }
        ImGuiModules::KeepWindowInsideViewport();
        // Disable all widgets when not connected to grbl
        ImGuiModules::BeginDisableWidgets(grblVals);
        // grbl State
        DrawStatus(grbl, settings, dt);
        ImGui::Separator();
        // Current x y z location
        DrawPosition(settings);
        ImGui::Separator();
        //Current modal state
        if(grblVals.isConnected) {
            DrawCurrentMode(settings);
            ImGui::Separator();
        }
        // Current feedrate & spindle speed
        DrawMotion(settings);
        ImGui::Separator();
        // Limit switches / probe
        DrawInputPins(grblVals);
        ImGui::Separator();
        // zeroing xyz
        DrawZeroing(grbl);
        ImGui::Separator();

        DrawCommands(grbl, grblVals);
        ImGui::Separator();

        DrawCustomGCodes(grbl, settings);
        ImGui::Separator();

        static bool viewStatus = grbl.getViewStatusReport();
        if (ImGui::Checkbox("Status Report", &(viewStatus))) {
            grbl.setViewStatusReport(viewStatus);
        }
        // Disable all widgets when not connected to grbl
        ImGuiModules::EndDisableWidgets(grblVals);

        ImGui::End();
    }
};

struct JogController {
    bool allow_Keyb_Jogging = false;
    bool currently_Jogging = false;
    float jogDistance = 10;
    int feedRate;

    JogController(GRBLVals& grblVals) {
        (void)grblVals;
        feedRate = 6000;//(int)grblVals.settings.max_FeedRate; // intialise to max feed
    }
    /*
      float calcuateJogDistance(float feedrate, float acc)
      {
          float v = feedrate / 60; // mm/s
          int N = 15; // number blocks in planner buffer

          float dt = (v*v) / (2*acc*(N-1));

          cout << "@ " << feedrate << "mm/min" << endl;
          cout << "dt = " << dt << endl;

          float smin = v*dt; // mm (smallest jog distance

          cout << "sMin = " << smin << endl << endl;
          return smin;
      }
  */
    void KeyboardJogging(GRBL &grbl) {
        // - This is a rather messy piece of code, but for now it solves the
        // issue.
        // - When an arrow key is held, we want to repeatedly send jog commands
        // to grbl.
        // - The first issue is that we dont want to send more jog commands than
        // the number of
        //     planner blocks (15) in grbl, otherwise we have to remove any
        //     commands that haven't
        //    been acknowledged. So we wait for an ok to be received before
        //    sending the
        //     next jog (this ensures a max. of 15 acknowledged + 1 pending jogs
        //     are sent)
        // - When we release an arrow key, we want to send a 'realtime jog
        // cancel' cmd. When all jogs
        //    have recieved an 'ok', this works fine, but if there is a pending
        //    jog in the queue, the cancel cmd executes first, clearing grbl's
        //    buffer, which then allows the pending jog to execute.
        //    - Option 1 was to wait for the 'ok' to arrive before sending the
        //    cancel command
        //        But this meant that we would have to wait for the last jog to
        //        execute which could be a sizable distance.
        //     - Option 2 was to repeatedly send cancel commands until we
        //     recieve and 'ok' for that
        //        pending jog - not this most elegant fix but it seems to work
        //        for now.
        //    - Option 3 was to not allow too many jogs to be sent to grbl to
        //    fill the planner
        //        blocks, but the only way to know this information was from the
        //        status response (Bf:15,128). This just seemed messy, as we are
        //        relying on a delayed response from grbl (or the status
        //        responses may not even be switched on)
        //    - Option 4 - send one long jog to end of table

        int KEY_LEFT = ImGui::GetKeyIndex(ImGuiKey_LeftArrow);
        int KEY_UP = ImGui::GetKeyIndex(ImGuiKey_UpArrow);
        int KEY_RIGHT = ImGui::GetKeyIndex(ImGuiKey_RightArrow);
        int KEY_DOWN = ImGui::GetKeyIndex(ImGuiKey_DownArrow);

        // option 4 - one long jog (must be > than the extents of the machine
        static int jogLongDistance = 10000;
        // on key release, send cancel
        if ((ImGui::IsKeyReleased(KEY_LEFT) ||
             ImGui::IsKeyReleased(KEY_RIGHT) || ImGui::IsKeyReleased(KEY_UP) ||
             ImGui::IsKeyReleased(KEY_DOWN)) &&
            (!ImGui::IsKeyPressed(KEY_LEFT) &&
             !ImGui::IsKeyPressed(KEY_RIGHT) && !ImGui::IsKeyPressed(KEY_UP) &&
             !ImGui::IsKeyPressed(KEY_DOWN))) {
            grbl.sendRT(GRBL_RT_JOG_CANCEL);
            currently_Jogging = false;
        } else if (!currently_Jogging) { // only allow combination of 2 buttons
            if (ImGui::IsKeyPressed(KEY_LEFT) + ImGui::IsKeyPressed(KEY_UP) +
                    ImGui::IsKeyPressed(KEY_RIGHT) +
                    ImGui::IsKeyPressed(KEY_DOWN) <=
                2) {
                if (ImGui::IsKeyPressed(KEY_LEFT) &&
                    ImGui::IsKeyPressed(KEY_UP)) {
                    grbl.sendJog(glm::vec3(-jogLongDistance, jogLongDistance, 0),
                                 feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_UP) &&
                           ImGui::IsKeyPressed(KEY_RIGHT)) {
                    grbl.sendJog(glm::vec3(jogLongDistance, jogLongDistance, 0),
                                 feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_RIGHT) &&
                           ImGui::IsKeyPressed(KEY_DOWN)) {
                    grbl.sendJog(glm::vec3(jogLongDistance, -jogLongDistance, 0),
                                 feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_DOWN) &&
                           ImGui::IsKeyPressed(KEY_LEFT)) {
                    grbl.sendJog(glm::vec3(-jogLongDistance, -jogLongDistance, 0),
                                 feedRate);
                    currently_Jogging = true;
                }

                else if (ImGui::IsKeyPressed(KEY_LEFT)) {
                    grbl.sendJog(glm::vec3(-jogLongDistance, 0, 0), feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_RIGHT)) {
                    grbl.sendJog(glm::vec3(jogLongDistance, 0, 0), feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_UP)) {
                    grbl.sendJog(glm::vec3(0, jogLongDistance, 0), feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_DOWN)) {
                    grbl.sendJog(glm::vec3(0, -jogLongDistance, 0), feedRate);
                    currently_Jogging = true;
                }
            }
        }
        /*    option 1: wait to recieve ok before sending cancel
        // flag for when arrow key is released
        static bool send_Jog_Cancel = false;
        // have we recieved an 'ok' for every jog command we have sent?
        bool synced_With_grbl = grbl.gcList.status.size() == grbl.gcList.read;
        // on key release, send cancel
        if((ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) ||
        ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_RightArrow)) ||
            ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) ||
        ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))) {
            send_Jog_Cancel = true;
        }
        else if(synced_With_grbl && !send_Jog_Cancel)
        {
            if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
            grbl.sendJog(X_AXIS, BACKWARD, jogDistance, feedRate);
            else
        if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
            grbl.sendJog(X_AXIS, FORWARD, jogDistance, feedRate);
            else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
            grbl.sendJog(Y_AXIS, FORWARD, jogDistance, feedRate);
            else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
            grbl.sendJog(Y_AXIS, BACKWARD, jogDistance, feedRate);
        }
        // if synced, (last jog recieved 'ok') send cancel
        if(synced_With_grbl && send_Jog_Cancel) {
            grbl.SendRT(GRBL_RT_JOG_CANCEL);
            send_Jog_Cancel = false;
        }
        */
        /* repeatedly send cancels
        // flag for when arrow key is released
        static bool send_Jog_Cancel = false;
        // have we recieved an 'ok' for every jog command we have sent?
        bool synced_With_grbl = grbl.gcList.status.size() == grbl.gcList.read;
        // on key release, set flag to true
        if((ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)) ||
        ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_RightArrow)) ||
            ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_UpArrow)) ||
        ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))) {
            send_Jog_Cancel = true;
        } // only send jog if an ok for the last jog is received & we are not
        waiting to cancel jog else if(synced_With_grbl && !send_Jog_Cancel)
        {
            if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
            grbl.sendJog(X_AXIS, BACKWARD, jogDistance, feedRate);
            else
        if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
            grbl.sendJog(X_AXIS, FORWARD, jogDistance, feedRate);
            else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
            grbl.sendJog(Y_AXIS, FORWARD, jogDistance, feedRate);
            else if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
            grbl.sendJog(Y_AXIS, BACKWARD, jogDistance, feedRate);
        }
        // repeatedly send cancels until the unacknowledged jog has been
        cancelled if(send_Jog_Cancel) { grbl.SendRT(GRBL_RT_JOG_CANCEL); if
        (synced_With_grbl) send_Jog_Cancel = false;
        }*/
    }

    void DrawJogController(GRBL &grbl) {
        ImGui::Checkbox("Control with arrow keys", &allow_Keyb_Jogging);
        if (allow_Keyb_Jogging)
            KeyboardJogging(grbl);

        int w = 40, h = 40;
        // Draw Jog XY
        ImGui::BeginGroup();

        ImGui::Dummy(ImVec2(w, h));
        ImGui::SameLine();

        if (ImGui::Button("Y+", ImVec2(w, h))) {
            if (!currently_Jogging)
                grbl.sendJog(glm::vec3(0, jogDistance, 0), feedRate);
        }

        if (ImGui::Button("X-", ImVec2(w, h))) {
            if (!currently_Jogging)
                grbl.sendJog(glm::vec3(-jogDistance, 0, 0), feedRate);
        }
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(w, h));
        ImGui::SameLine();

        if (ImGui::Button("X+", ImVec2(w, h))) {
            if (!currently_Jogging)
                grbl.sendJog(glm::vec3(jogDistance, 0, 0), feedRate);
        }
        ImGui::Dummy(ImVec2(w, h));
        ImGui::SameLine();

        if (ImGui::Button("Y-", ImVec2(w, h))) {
            if (!currently_Jogging)
                grbl.sendJog(glm::vec3(0, -jogDistance, 0), feedRate);
        }

        ImGui::EndGroup();

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 15);

        // Draw Jog Z
        ImGui::BeginGroup();

        if (ImGui::Button("Z+", ImVec2(w, h)))
            grbl.sendJog(glm::vec3(0, 0, jogDistance), feedRate);

        ImGui::Dummy(ImVec2(w, h));

        if (ImGui::Button("Z-", ImVec2(w, h)))
            grbl.sendJog(glm::vec3(0, 0, -jogDistance), feedRate);

        ImGui::EndGroup();
    }
    void DrawJogSetting(GRBLVals& grblVals) {
        ImGui::PushItemWidth(100);
        ImGui::Indent();
        ImGui::InputFloat("Jog Distance", &jogDistance);
        ImGui::Unindent();

        ImGuiModules::Incrementer("Jog0.1", "0.1", 0.1f, jogDistance, false);
        ImGui::SameLine();
        ImGuiModules::Incrementer("Jog1", "1", 1.0f, jogDistance, false);
        ImGui::SameLine();
        ImGuiModules::Incrementer("Jog10", "10", 10.0f, jogDistance, false);

        ImGui::Separator();

        ImGui::Indent();
        ImGui::SliderInt("Feed Rate", &feedRate, 0,
                         (int)grblVals.settings.max_FeedRate);
        ImGui::Unindent();
        ImGui::PopItemWidth();
    }

    void Draw(GRBL &grbl, GRBLVals& grblVals) { // initialise
                
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
        if (!ImGui::Begin("Jog Controller", NULL, general_window_flags)) {
            ImGui::End();
            return;
        }

        ImGuiModules::KeepWindowInsideViewport();
        // Disable all widgets when not connected to grbl
        ImGuiModules::BeginDisableWidgets(grblVals);

        ImGui::Separator();

        DrawJogController(grbl);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));

        ImGui::Separator();
        // feedrate / distance
        DrawJogSetting(grblVals);

        ImGui::PopStyleVar();

        // Disable all widgets when not connected to grbl
        ImGuiModules::EndDisableWidgets(grblVals);
        ImGui::End();
    }
};

struct Overrides {
    /*    Other Realtime commands similar but currently unused:
        grbl.SendRT(GRBL_RT_SPINDLE_STOP);
        grbl.SendRT(GRBL_RT_FLOOD_COOLANT);
        grbl.SendRT(GRBL_RT_MIST_COOLANT);
    */
    void Draw(GRBL &grbl, GRBLVals& grblVals) {
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
        if (!ImGui::Begin("Overrides", NULL, general_window_flags)) {
            ImGui::End();
            return;  
        }            
        ImGuiModules::KeepWindowInsideViewport();
        // Disable all widgets when not connected to grbl  
        ImGuiModules::BeginDisableWidgets(grblVals);
  
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));
  
        GRBLStatus_vals status = grblVals.status; 
        // Override Spindle Speed
        ImGui::Text("Override Spindle Speed: %d%%",
                    status.override_SpindleSpeed);
        // unused parameter, we read directly from status reports instead
        static int spindleSpeed = 0; 
        
        int buttonPress =
            ImGuiModules::Incrementer("ORSpindle1", "1%", 1, spindleSpeed, false);
        if (buttonPress == -1)
            grbl.sendRT(GRBL_RT_OVERRIDE_SPINDLE_MINUS_1PERCENT);
        else if (buttonPress == 1)
            grbl.sendRT(GRBL_RT_OVERRIDE_SPINDLE_ADD_1PERCENT);
  
        ImGui::SameLine();
        buttonPress = 
            ImGuiModules::Incrementer("ORSpindle10", "10%", 10, spindleSpeed, false);
        if (buttonPress == -1)
            grbl.sendRT(GRBL_RT_OVERRIDE_SPINDLE_MINUS_10PERCENT);
        else if (buttonPress == 1)
            grbl.sendRT(GRBL_RT_OVERRIDE_SPINDLE_ADD_10PERCENT);
 
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##ORSpindleReset"))
            grbl.sendRT(GRBL_RT_OVERRIDE_SPINDLE_100PERCENT);

        ImGui::Separator();

        // Override Feed Rate
        ImGui::Text("Override Feed Rate: %d%%", status.override_Feedrate);
        // Unused parameter, we read directly from status reports instead
        static int feedRate = 0;

        buttonPress = ImGuiModules::Incrementer("ORFeed1", "1%", 1, feedRate, false);
        if (buttonPress == -1)
            grbl.sendRT(GRBL_RT_OVERRIDE_FEED_MINUS_1PERCENT);
        else if (buttonPress == 1)
            grbl.sendRT(GRBL_RT_OVERRIDE_FEED_ADD_1PERCENT);

        ImGui::SameLine();
        buttonPress = ImGuiModules::Incrementer("ORFeed10", "10%", 10, feedRate, false);
        if (buttonPress == -1)
            grbl.sendRT(GRBL_RT_OVERRIDE_FEED_MINUS_10PERCENT);
        else if (buttonPress == 1)
            grbl.sendRT(GRBL_RT_OVERRIDE_FEED_ADD_10PERCENT);

        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##ORFeedReset"))
            grbl.sendRT(GRBL_RT_OVERRIDE_FEED_100PERCENT);

        ImGui::Separator();

        // override rapid feed rate
        ImGui::Text("Override Rapid Feed Rate: %d%%",
                    status.override_RapidFeed);

        if (ImGui::SmallButton("25%"))
            grbl.sendRT(GRBL_RT_OVERRIDE_RAPIDFEED_25PERCENT);
        ImGui::SameLine();
        if (ImGui::SmallButton("50%"))
            grbl.sendRT(GRBL_RT_OVERRIDE_RAPIDFEED_50PERCENT);
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset##ORRapidFeed100"))
            grbl.sendRT(GRBL_RT_OVERRIDE_RAPIDFEED_100PERCENT);

        ImGui::PopStyleVar();

        // Disable all widgets when not connected to grbl
        ImGuiModules::EndDisableWidgets(grblVals);
        ImGui::End();
    }
};

struct Debug {
    void showName(const char *name) {
        ImGui::TextUnformatted(name);
        ImGui::SameLine();
    };
    void addEntry(const char *name, bool val) {
        showName(name);
        ImGui::TextUnformatted(val ? "True" : "False");
    };
    auto addEntry(const char *name, uint val) {
        showName(name);
        ImGui::Text("%u", val);
    };
    auto addEntry(const char *name, int val) {
        showName(name);
        ImGui::Text("%d", val);
    };
    auto addEntry(const char *name, float val) {
        showName(name);
        ImGui::Text("%f", val);
    };
    auto addEntry(const char *name, glm::vec3 val) {
        showName(name);
        ImGui::Text("(%f, %f, %f)", val.x, val.y, val.z);
    };
    auto addEntry(const char *name, string val) {
        showName(name);
        ImGui::TextUnformatted(val.c_str());
    };
    auto addEntry(const char *name, char *val) {
        showName(name);
        ImGui::TextUnformatted(val);
    };
    auto addEntry(const char *name, const char *format, ...) {
        showName(name);
        va_list arglist;
        va_start(arglist, format);
        ImGui::TextV(format, arglist);
        va_end(arglist);
    };

    void Draw(GRBL &grbl, Settings& settings) {
        GRBLVals& grblVals = settings.grblVals;
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
        if (!ImGui::Begin("Debug", NULL, general_window_flags)) {
            ImGui::End();
            return;
        }

        ImGuiModules::KeepWindowInsideViewport();
        GRBLVals &v = grblVals;

        if (ImGui::TreeNode("Settings")) {
            ImGui::InputText("Save File Location", &settings.p.system.saveFileDirectory);
            
            ImGui::SliderFloat("Point Size", &settings.p.viewer.point.size, 0.0f, 100.0f);
            ImGui::ColorEdit3("Point Colour", &settings.p.viewer.point.colour[0]);
            ImGui::ColorEdit3("Line Colour", &settings.p.viewer.line.colour[0]);
            ImGui::ColorEdit3("Line Colour Disabled", &settings.p.viewer.line.colourDisabled[0]);
            
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("GUI Settings")) {
            
            for (int i = 0; i < IM_ARRAYSIZE(settings.guiSettings.button); i++) {
                ImGui::SliderFloat2(va_str("Button Size %d", i).c_str(), &settings.guiSettings.button[i].Size.x, 0.0f, 100.0f);
            }
            for (int i = 0; i < IM_ARRAYSIZE(settings.guiSettings.button); i++) {
                ImGui::SliderFloat2(va_str("Button Image Size %d", i).c_str(), &settings.guiSettings.button[i].ImageSize.x, 0.0f, 100.0f);
            }
            
            ImGui::SliderFloat("Dock Padding", &settings.guiSettings.dockPadding, 0.0f, 100.0f);
            ImGui::SliderFloat("Toolbar Height", &settings.guiSettings.toolbarHeight, 0.0f, 500.0f);
            ImGui::SliderFloat("Toolbar Spacer", &settings.guiSettings.toolbarSpacer, 0.0f, 100.0f);
            ImGui::SliderFloat("Toolbar ComboBox Width", &settings.guiSettings.toolbarComboBoxWidth, 0.0f, 500.0f);
            ImGui::SliderFloat("Input Box Width", &settings.guiSettings.inputBoxWidth, 0.0f, 500.0f);
 
            if(ImGui::ColorEdit3("Text Colour", &settings.guiSettings.colour[Colour::Text].x)) {
                ImGui::GetStyle().Colors[ImGuiCol_Text] = settings.guiSettings.colour[Colour::Text];
            }
            ImGui::ColorEdit3("Header Text Colour", &settings.guiSettings.colour[Colour::HeaderText].x);
            
            ImGui::TreePop();
        }


        if (ImGui::TreeNode("System")) {
            addEntry("OpenGl Version", (char *)glGetString(GL_VERSION));
            addEntry("Frame Rate", "%.3f ms/frame (%.1f FPS)",
                     1000.0f / ImGui::GetIO().Framerate,
                     ImGui::GetIO().Framerate);

            static bool debugGCListBuild = false;
            static bool debugCharacterCounting = false;
            static bool debugGCList = false;
            static bool debugSerial = false;
            static bool debugThreadBlocking = false;
            static bool debugGCReader = false;
            static bool debugSketchRefs = false;
            bool needUpdate = false;

            static bool showDebugMenu = false;
            ImGui::Checkbox("Show Debug Menu", &showDebugMenu);
            if(showDebugMenu) {
                ImGui::TextUnformatted("Debug");
                ImGui::Indent();
                needUpdate |= ImGui::Checkbox("GCode List Build", &(debugGCListBuild));
                needUpdate |= ImGui::Checkbox("Character Counting", &(debugCharacterCounting));
                needUpdate |= ImGui::Checkbox("GCode List", &(debugGCList));
                needUpdate |= ImGui::Checkbox("Serial", &(debugSerial));
                needUpdate |= ImGui::Checkbox("Thread Blocking", &(debugThreadBlocking));
                needUpdate |= ImGui::Checkbox("GCode Reader (For 3D Viewer)", &(debugGCReader));
                needUpdate |= ImGui::Checkbox("Sketch References", &(debugSketchRefs));
                ImGui::Unindent();
            
                if (needUpdate) {
                    int flags = DEBUG_NONE;

                    if (debugGCListBuild)
                        flags |= DEBUG_GCLIST_BUILD;
                    if (debugCharacterCounting)
                        flags |= DEBUG_CHAR_COUNTING;
                    if (debugGCList)
                        flags |= DEBUG_GCLIST;
                    if (debugSerial)
                        flags |= DEBUG_SERIAL;
                    if (debugThreadBlocking)
                        flags |= DEBUG_THREAD_BLOCKING;
                    if (debugGCReader)
                        flags |= DEBUG_GCREADER;
                    if (debugSketchRefs)
                        flags |= DEBUG_SKETCH_REFERENCES;

                    Log::SetDebugFlags(flags);
                }
            }
            ImGui::TreePop();
        }        
        if (ImGui::TreeNode("General")) {
            addEntry("Connected", v.isConnected);
            addEntry("Check Mode", v.isCheckMode);
            addEntry("File: Running", v.isFileRunning);
            addEntry("File: Current Line", (int)v.curLine);
            addEntry("File: Total Lines", (int)v.totalLines);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Coordinates")) {
            if (ImGui::Button("Update '$#'"))
                grbl.send("$#");
            addEntry("Work Coordinates (G54)", v.coords.workCoords[0]);
            addEntry("Work Coordinates (G55)", v.coords.workCoords[1]);
            addEntry("Work Coordinates (G56)", v.coords.workCoords[2]);
            addEntry("Work Coordinates (G57)", v.coords.workCoords[3]);
            addEntry("Work Coordinates (G58)", v.coords.workCoords[4]);
            addEntry("Work Coordinates (G59)", v.coords.workCoords[5]);

            addEntry("Home Coordinates (G28)", v.coords.homeCoords[0]);
            addEntry("Home Coordinates (G30)", v.coords.homeCoords[1]);

            addEntry("Offset Coordinates (G92)", v.coords.offsetCoords);
            addEntry("Tool Length Offset (G43.1)", v.coords.toolLengthOffset);
            addEntry("Probe Offset", v.coords.probeOffset);
            addEntry("Probe Success", v.coords.probeSuccess);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Modal")) {

            if (ImGui::Button("Update '$N'"))
                grbl.send("$N");
            addEntry("Startup Block 1", v.modal.StartupBlock[0]);
            addEntry("Startup Block 2", v.modal.StartupBlock[1]);

            if (ImGui::Button("Update '$G'"))
                grbl.send("$G");
            addEntry("MotionMode", "G%g", v.modal.MotionMode);

            addEntry("Coordinate System", "G%u", v.modal.CoordinateSystem);
            addEntry("Plane", "G%u", v.modal.Plane);
            addEntry("Distance Mode", "G%u", v.modal.DistanceMode);
            addEntry("Arc IJK DistanceMode", "G%g", v.modal.ArcIJKDistanceMode);
            addEntry("Feedrate Mode", "G%u", v.modal.FeedRateMode);
            addEntry("Units Mode", "G%u", v.modal.UnitsMode);
            addEntry("Cutter Radius Compensation", "G%u", v.modal.CutterRadCompensation);
            addEntry("Tool Length Offset", "G%g", v.modal.ToolLengthOffset);
            addEntry("Program Mode", "M%u", v.modal.ProgramMode);
            addEntry("Spindle State", "M%u", v.modal.SpindleState);
            addEntry("Coolant State", "M%u", v.modal.CoolantState);
            addEntry("Tool Number", v.modal.toolNumber);
            addEntry("Spindle Speed", v.modal.spindleSpeed);
            addEntry("Feed Rate", v.modal.feedRate);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Status")) {
            if (ImGui::Button("Update '?'"))
                grbl.send("?");
            addEntry("State", grbl.sys.status.stateStr(v.status.state));
            addEntry("Machine Position", v.status.MPos);
            addEntry("Work Position", v.status.WPos);
            addEntry("Work Coordinate Offset", v.status.WCO);

            addEntry("Line Number", v.status.lineNum);
            addEntry("Feedrate", v.status.feedRate);
            addEntry("Spindle Speed", v.status.spindleSpeed);
            addEntry("Input Pin: Limit X", v.status.inputPin_LimX);
            addEntry("Input Pin: Limit Y", v.status.inputPin_LimY);
            addEntry("Input Pin: Limit Z", v.status.inputPin_LimZ);
            addEntry("Input Pin: Probe", v.status.inputPin_Probe);
            addEntry("Input Pin: Door", v.status.inputPin_Door);
            addEntry("Input Pin: Hold", v.status.inputPin_Hold);
            addEntry("Input Pin: SoftReset", v.status.inputPin_SoftReset);
            addEntry("Input Pin: CycleStart", v.status.inputPin_CycleStart);
            addEntry("Override Feedrate", v.status.override_Feedrate);
            addEntry("Override Rapid Feed", v.status.override_RapidFeed);
            addEntry("Override Spindle Speed", v.status.override_SpindleSpeed);
            addEntry("Accessory Spindle Direction",
                     v.status.accessory_SpindleDir);
            addEntry("Accessory Flood Coolant",
                     v.status.accessory_FloodCoolant);
            addEntry("Accessory Mist Coolant", v.status.accessory_MistCoolant);
            addEntry("Planner Buffer Available", v.status.bufferPlannerAvail);
            addEntry("Serial Buffer Available", v.status.bufferSerialAvail);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("GRBL Settings")) {
            if (ImGui::Button("Update '$$'"))
                grbl.send("$$");
                
            ImGuiTableFlags flags =
                ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_Resizable |
                ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                ImGuiTableFlags_BordersV;
            if (ImGui::BeginTable("GRBLSettings", 5, flags, ImVec2(0, 32 * ImGui::GetFrameHeight()))) { 
                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                // Set up headers
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_None, 55.0f);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_None, 55.0f);
                ImGui::TableSetupColumn("Unit", ImGuiTableColumnFlags_None, 55.0f);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 95.0f);
                ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_None, 300.0f);
                ImGui::TableHeadersRow();

                for(const auto& [id, value] : grblVals.settings.RawValues) {
                    // retrieve name, desc & unit of setting
                    std::string name, unit, desc;
                    if(getSettingsMsg(id, &name, &unit, &desc)) {    
                        Log::Error("Error %d: Can't find settings code", id);
                    }
                    
                    char valueStr[MAX_STRING];
                    if(unit == "mask") {
                        snprintf(valueStr, MAX_STRING, "%g (%s)", value, std::bitset<8>(value).to_string().c_str());
                    } else {
                        snprintf(valueStr, MAX_STRING, "%g", value);
                    }
                    
                    ImGui::TableNextRow();
                    // id
                    ImGui::TableNextColumn();
                    string textBuffer = string("$ ") + std::to_string(id);
                    float idTextWidth = ImGui::CalcTextSize(textBuffer.c_str()).x;
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth()/2 - idTextWidth/2);
                    ImGui::TextUnformatted(textBuffer.c_str());

                    // value
                    ImGui::TableNextColumn();
                    float valueTextWidth = ImGui::CalcTextSize(valueStr).x;
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth()/2 - valueTextWidth/2); //  - ImGui::CalcTextSize(valueStr).x / 2 - ImGui::GetStyle().CellPadding.x
                    ImGui::Text(valueStr);
                    
                    // unit
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(unit.c_str());
                    // name
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(name.c_str());
                    // description
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(desc.c_str());
                }
                ImGui::EndTable();
            }
          
            addEntry("Min Spindle Speed", v.settings.min_SpindleSpeed);
            addEntry("Max Spindle Speed", v.settings.max_SpindleSpeed);
            addEntry("Max FeedRate X", v.settings.max_FeedRateX);
            addEntry("Max FeedRate Y", v.settings.max_FeedRateY);
            addEntry("Max FeedRate Z", v.settings.max_FeedRateZ);
            addEntry("Max FeedRate", v.settings.max_FeedRate);
            addEntry("Units: Distance", v.settings.units_Distance);
            addEntry("Units: Feedrate", v.settings.units_Feed);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Sqeak Settings")) {
            if(ImGui::Button("Save Config")) {
                Event<Event_SaveSettings>::Dispatch({ }); 
                //settings.SaveToFile();
            }
            if(ImGui::Button("Load Config")) {
                Event<Event_UpdateSettingsFromFile>::Dispatch({ }); 
                //settings.UpdateFromFile(); 
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }
};
 
     
void drawDockSpace(Settings& settings)
{
    // fullscreen dockspace
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    float padding = settings.guiSettings.dockPadding;
    float toolbarHeight = settings.guiSettings.toolbarHeight;
    // create the dockspace below the main tool bar
    ImGui::SetNextWindowPos(viewport->WorkPos + ImVec2(padding, toolbarHeight + 2.0f*padding));
    ImGui::SetNextWindowSize(viewport->WorkSize - ImVec2(padding*2.0f, toolbarHeight + 3.0f*padding));
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse 
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    // remove window padding as this is controlled by position + size
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", NULL, window_flags);
    ImGui::PopStyleVar();
    
    /*
    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    
    if (ImGui::DockBuilderGetNode(dockspace_id) == NULL) 
    {
        ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None); // Add empty node
        //ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(200.0f, 400.0f));

        ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);

        ImGui::DockBuilderDockWindow("Stats", dock_id_left);
        ImGui::DockBuilderDockWindow("Jog Controller", dock_id_right);
        ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
        
        ImGui::DockBuilderFinish(dockspace_id);
    }
    //DockBuilderDockWindow() or SetNextWindowDockId()
    */
    
    // Submit the DockSpace
    ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);
    
}    
        
void drawFrames(GRBL& grbl, Settings& settings, float dt) 
{    
    
    static Toolbar toolbar(settings);
    toolbar.Draw(grbl, settings);
    
    static Debug debug;
    debug.Draw(grbl, settings);

    static Stats stats;
    stats.Draw(grbl, settings, dt);

    static Console console;
    console.Draw(grbl, settings);

    static JogController jogController(settings.grblVals);
    jogController.Draw(grbl, settings.grblVals);

    static Overrides overrides;
    overrides.Draw(grbl, settings.grblVals);

}

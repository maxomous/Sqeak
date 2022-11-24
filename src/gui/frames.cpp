/*
 * frames.cpp
 *  Max Peglar-Willis 2021
 */ 


#include "frames.h"

using namespace std;
using namespace MaxLib;
using namespace MaxLib::String;


namespace Sqeak { 


#define MAX_CUSTOM_GCODES 12 // should be divisible by 3
#define MAX_HISTORY 100
 
#define MAX_CONSOLE_SIZE 1000

//******************************************************************************//
//**********************************FRAMES**************************************//


struct Console 
{     
    Console() {
      /*  Event<Event_ConsoleScrollToBottom>::RegisterHandler([this](Event_ConsoleScrollToBottom data) {
            (void)data;
            m_AutoScroll = true;
        });
        */
        // Log Handler for console
        Log::RegisterHandler([&](const char* date, Log::LogLevel level, const std::string& message) {
            (void)date;
            // make string
            std::string str = Log::LevelPrefix(level) + message;
            if(m_ConsoleLog.size() > MAX_CONSOLE_SIZE) {
                m_ConsoleLog.pop_front();
            }
            // print to console
            m_ConsoleLog.emplace_back(str);
            // move cursor to bottom
            m_AutoScroll = true;
            
            // Print to message popup (warning or higher)
            if (level >= Log::LogLevel::LevelWarning) {
                Event<Event_PopupMessage>::Dispatch({ str });
            }
        });
    }

    void Draw(GRBL &grbl, Settings& settings) {
        
        // begin new imgui window
        static ImGuiCustomModules::ImGuiWindow window(settings, "Console", ImVec2(570.0f, 270.0f)); // default size
        if(window.Begin(settings, ImGuiWindowFlags_None)) 
        {
            DrawLog();
                
            ImGui::Separator();

            DrawInputTextBox(grbl);
            
            ImGui::SameLine();

            if (ImGui::Button("Clear Log")) {
                m_ConsoleLog.clear();
            }
            window.End();
        }
    }

    void DrawLog() 
    {
        // set log to stretch window height minus space for the input text box / send button 
        float h = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("Log", ImVec2(0, -h), false,  ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        // Clip only visible lines
        ImGuiListClipper clipper;
        clipper.Begin(m_ConsoleLog.size());
        while (clipper.Step()) {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
                ImGui::TextUnformatted(m_ConsoleLog[line_no].c_str());
            }
        }
        clipper.End();

        AutoScroll();

        ImGui::EndChild();
        //ImGui::Checkbox("Auto-scroll", &AutoScroll);
    }

    void AutoScroll()
    {
        // scroll to bottom if needed
        float lineHeight = ImGui::GetTextLineHeightWithSpacing();
        
        float autoScroll_AvoidFlickeringDistance = lineHeight;
        float autoScroll_DetatchDistance = 2.0f * lineHeight;
        float autoScroll_JoinDistance = lineHeight - 1.0f;
         
        // scroll to bottom
        float yScrollDif = ImGui::GetScrollMaxY() - ImGui::GetScrollY();
        //cout << "lineHeight = " << lineHeight << "  AutoScroll = " << autoScroll << "  yScrollDif = " << yScrollDif << endl;
        // there is a frame lag from where the y scroll changes which causes a flickering of scroll position, this is a workaround
        if(!ImGui::GetIO().MouseWheel) {
            if(m_AutoScroll && (yScrollDif > autoScroll_AvoidFlickeringDistance)) {
                ImGui::SetScrollY(ImGui::GetScrollMaxY());
            } 
        }
        // if user scroll above, turn off autoscroll
        else if (yScrollDif > autoScroll_DetatchDistance) { // make line height * mult
            m_AutoScroll = false;
        }
        // if user scrolled to bottom, set to autoscroll
        else if (yScrollDif < autoScroll_JoinDistance) { // make line height
            m_AutoScroll = true;
        }
    }
    
    void DrawInputTextBox(GRBL& grbl)
    {
        static string userInputText;
        bool reclaim_focus = false;  

        if (ImGui::Button("Send")) {
            SendToGRBL(grbl, userInputText);
            reclaim_focus = true;
        }

        ImGui::SameLine();

        ImGui::PushItemWidth(200); // use -ve for distance from right size

            ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackHistory;
            // Console text input
            if (ImGui::InputText("Input GCode Here", &userInputText, flags, &TextEditCallbackStub, (void *)this)) {
                // on enter key
                SendToGRBL(grbl, userInputText);
            }

        ImGui::PopItemWidth();
        
        // Auto-focus
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus) {
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
        }
    }
    
    void UpdateHistory(const string& text) 
    {
        m_History.push_back(text);
        if (m_History.size() > MAX_HISTORY) {
            m_History.erase(m_History.begin());
        }
        m_HistoryPos = m_History.size();
    }

    void SendToGRBL(GRBL& grbl, string& text) 
    {
        if(text.empty()) {
            return;
        }
        UpdateHistory(text);
        grbl.send(text);
        //clear text
        text = "";
    }


    static int TextEditCallbackStub(ImGuiInputTextCallbackData *data) {
        Console *console = (Console *)data->UserData;
        return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiInputTextCallbackData *data) {
        // reset history position if user types anything
        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
            m_HistoryPos = m_History.size();
        // select through history of previous sent commands
        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
            if (m_History.size() == 0)
                return 0;

            if (data->EventKey == ImGuiKey_UpArrow) {
                if (m_HistoryPos > 0)
                    m_HistoryPos--;
                else
                    return 0;
            } else if (data->EventKey == ImGuiKey_DownArrow) {
                if (m_HistoryPos < (int)m_History.size() - 1)
                    m_HistoryPos++;
                else
                    return 0;
            }
            const char* history_str = (m_History.size() > 0) ? m_History[m_HistoryPos].c_str() : "";
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, history_str);
        }
        return 0;
    }

private:
    // to auto scroll to bottom
    bool m_AutoScroll = true;
    // input textbox history
    vector<string> m_History;
    int m_HistoryPos = 0;
    
    std::deque<std::string> m_ConsoleLog;
        
};


struct Commands {
    
    Commands() {
        // TODO Autoscroll needs to be set to true
       /* Event<Event_ConsoleScrollToBottom>::RegisterHandler([this](Event_ConsoleScrollToBottom data) {
            (void)data;
            m_AutoScroll = true;
        });*/
    }

    void Draw(GRBL &grbl, Settings& settings) 
    {
        // begin new imgui window
        static ImGuiCustomModules::ImGuiWindow window(settings, "Commands", ImVec2(570.0f, 270.0f)); // default size
        if(window.Begin(settings, ImGuiWindowFlags_None)) 
        {        
            DrawCommands(grbl, settings.grblVals);

            ImGui::Separator();
            
            if (ImGui::Button("Clear Commands")) {
                grbl.clearCompleted();
            }
            window.End();
        }
    }
    
    void DrawCommands(GRBL &grbl, GRBLVals& grblVals) 
    {       
        // Commands
        float h = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("Commands", ImVec2(0, -h));
            ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | 
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
            // draw commands table
            if (ImGui::BeginTable("Commands", 3, flags, ImVec2(0, 0))) 
            { 
                // Make top row always visible
                ImGui::TableSetupScrollFreeze(0, 1);
                // Set up headers
                ImGui::TableSetupColumn("Sent", ImGuiTableColumnFlags_None, 165.0f);
                ImGui::TableSetupColumn("Reponse", ImGuiTableColumnFlags_None, 55.0f);
                ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_None, 600.0f);
                ImGui::TableHeadersRow();

                ImGuiListClipper clipper;
                // this sets the size
                clipper.Begin(grbl.getGCodeListSize());

                while (clipper.Step()) {
                    for (int row_n = clipper.DisplayStart;
                         row_n < clipper.DisplayEnd; row_n++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        // GCodeItem_t gcItem = grbl.gcList.GetItem(row_n);
                        const GCodeItem& gcItem = grbl.getGCodeItem(row_n);

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
                                ImGui::Text("Error %d: Can't find error code", gcItem.status);
                            } else {
                                ImGui::Text("Error %d", gcItem.status);
                                ImGui::TableNextColumn();
                                ImGui::Text("%s: %s", errName.c_str(),  errDesc.c_str());
                            }
                        }
                    } 
                }
                
                AutoScroll(grblVals);

                ImGui::EndTable(); 
            }
        ImGui::EndChild();
    }  

    void AutoScroll(GRBLVals& grblVals)
    {
        if(grblVals.isFileRunning && m_AutoScroll) {
            // go down 2 cells further than the most recently acknowledged in gclist
            uint index = grblVals.curLineIndex < 2 ? 0 : grblVals.curLineIndex - 2;
            float curPos = index * ImGui::GetTextLineHeightWithSpacing();
            ImGui::SetScrollY(curPos);
        }
        else if (m_AutoScroll || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
            
        m_AutoScroll = false;
    }

private:
    bool m_AutoScroll = true;

};
 

struct PopupMessages {
    
    // first = time message has been displayed for  /  second = text
    vector<pair<float, string>> messages;
    
    PopupMessages() 
    {
        Event<Event_PopupMessage>::RegisterHandler([&](Event_PopupMessage data) {
            messages.push_back(make_pair(0.0f, data.msg));
        });
    }
    
    void Draw(Settings& settings, float dt) 
    {
        // only allow a maximum number of popup messages
        if(messages.size() > settings.guiSettings.popupMessage_MaxCount) {
            messages.erase(messages.begin());
        }
        
        {   // subtract time from messages and remove if timed out
            size_t i = 0;
            while(i < messages.size()) {
                messages[i].first += dt;
                // remove messages if timed out
                if(messages[i].first > settings.guiSettings.popupMessage_Time) {
                    messages.erase(messages.begin() + i);
                } else {
                    i++;
                }
            }
        }
        
        for(size_t i = 0; i < messages.size(); i++)
        {
            float fadeTime = 1.0f; // seconds
            
            float timeLeft = settings.guiSettings.popupMessage_Time - messages[i].first;
            // fade out
            if(timeLeft < fadeTime)  {                
                ImVec4 fade = { 1.0f, 1.0f, 1.0f, timeLeft / fadeTime };
                ImGui::PushStyleColor(ImGuiCol_Text,     fade * ImGui::GetStyleColorVec4(ImGuiCol_Text));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, fade * ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
            }
            
            float yPos = i * settings.guiSettings.popupMessage_YSpacing;
            float padding = settings.guiSettings.dockPadding;
            ImGui::SetNextWindowPos(ImVec2(padding, settings.guiSettings.toolbarHeight + 2.0f * padding + yPos));
            //ImGui::SetNextWindowSize(ImVec2(200.0f, 30.0f));
            
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs;
            
            if(ImGui::Begin(va_str("PopupMessage%d", i).c_str(), NULL, ImGuiCustomModules::ImGuiWindow::generalWindowFlags | window_flags)) {
                ImGui::TextUnformatted(messages[i].second.c_str());
            }
            
            // fade out
            if(timeLeft < fadeTime) {
                ImGui::PopStyleColor(2);
            }
            
            ImGui::End();
        }
    }
    
};

struct JogController {
    bool allow_Keyb_Jogging = false;
    bool currently_Jogging = false;
    float jogDistance = 10;
    // TODO DELETE THIS, READ DIRECTLY FROM SETTINGS
    int feedRate = 6000;

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
                    grbl.sendJog(Vec3(-jogLongDistance, jogLongDistance, 0),
                                 feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_UP) &&
                           ImGui::IsKeyPressed(KEY_RIGHT)) {
                    grbl.sendJog(Vec3(jogLongDistance, jogLongDistance, 0),
                                 feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_RIGHT) &&
                           ImGui::IsKeyPressed(KEY_DOWN)) {
                    grbl.sendJog(Vec3(jogLongDistance, -jogLongDistance, 0),
                                 feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_DOWN) &&
                           ImGui::IsKeyPressed(KEY_LEFT)) {
                    grbl.sendJog(Vec3(-jogLongDistance, -jogLongDistance, 0),
                                 feedRate);
                    currently_Jogging = true;
                }

                else if (ImGui::IsKeyPressed(KEY_LEFT)) {
                    grbl.sendJog(Vec3(-jogLongDistance, 0, 0), feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_RIGHT)) {
                    grbl.sendJog(Vec3(jogLongDistance, 0, 0), feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_UP)) {
                    grbl.sendJog(Vec3(0, jogLongDistance, 0), feedRate);
                    currently_Jogging = true;
                } else if (ImGui::IsKeyPressed(KEY_DOWN)) {
                    grbl.sendJog(Vec3(0, -jogLongDistance, 0), feedRate);
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

    void DrawJogController(GRBL &grbl, Settings& settings) 
    {        
        if (allow_Keyb_Jogging)
            KeyboardJogging(grbl);
            
        ImVec2& buttonSize = settings.guiSettings.imageButton_Toolbar_Jog->buttonSize;
         
         
        
        //float tableHeight = settings.guiSettings.toolbarItemHeight
        
        ImGui::BeginGroup();
        // ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX,
        if (ImGui::BeginTable("JogController",  5, ImGuiTableFlags_NoSavedSettings  | ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_PadOuterX, ImVec2(buttonSize.x * 6.0f, 0.0f))) 
        {    
            // first row   
            ImGui::TableNextRow();//ImGuiTableRowFlags_None, buttonSize.y);
                
                // Y +
                if(ImGui::TableSetColumnIndex(1)) {
                    if (ImGuiModules::ImageButtonWithText("##JogY+", settings.guiSettings.img_ArrowUp, settings.guiSettings.imageButton_Toolbar_Jog)) {
                        if (!currently_Jogging)
                            grbl.sendJog(Vec3(0, jogDistance, 0), feedRate);
                    }
                }
                
                // Z+
                if(ImGui::TableSetColumnIndex(4)) {
                    if (ImGuiModules::ImageButtonWithText("##JogZ+", settings.guiSettings.img_ArrowUp, settings.guiSettings.imageButton_Toolbar_Jog)) {
                        if (!currently_Jogging)
                            grbl.sendJog(Vec3(0, 0, jogDistance), feedRate);
                    }
                } 
            // next row
            ImGui::TableNextRow();//ImGuiTableRowFlags_None, buttonSize.y);
                
                // X -
                if(ImGui::TableSetColumnIndex(0)) {
                    if (ImGuiModules::ImageButtonWithText("##JogX-", settings.guiSettings.img_ArrowLeft, settings.guiSettings.imageButton_Toolbar_Jog)) {
                        if (!currently_Jogging)
                            grbl.sendJog(Vec3(-jogDistance, 0, 0), feedRate);
                    }
                }
                // "X Y" text
                if(ImGui::TableSetColumnIndex(1)) {
                    ImGuiModules::CentreItemVerticallyAboutItem(buttonSize.y, settings.guiSettings.font_small->FontSize + 1.0f);
                    ImGuiCustomModules::Heading(settings, "X Y", buttonSize.x);
                }
                // X +
                if(ImGui::TableSetColumnIndex(2)) {
                    if (ImGuiModules::ImageButtonWithText("##JogX+", settings.guiSettings.img_ArrowRight, settings.guiSettings.imageButton_Toolbar_Jog)) {
                        if (!currently_Jogging)
                            grbl.sendJog(Vec3(jogDistance, 0, 0), feedRate);
                    }
                }
                // "Z" text
                if(ImGui::TableSetColumnIndex(4)) {                    
                    ImGuiModules::CentreItemVerticallyAboutItem(buttonSize.y, settings.guiSettings.font_small->FontSize + 2.0f);
                    ImGuiCustomModules::Heading(settings, "Z", buttonSize.x);
                }
                  
            // next row
            ImGui::TableNextRow();//ImGuiTableRowFlags_None, buttonSize.y);
                            
                // Y -
                if(ImGui::TableSetColumnIndex(1)) {
                    if (ImGuiModules::ImageButtonWithText("##JogY-", settings.guiSettings.img_ArrowDown, settings.guiSettings.imageButton_Toolbar_Jog)) {
                        if (!currently_Jogging)
                            grbl.sendJog(Vec3(0, -jogDistance, 0), feedRate);
                    }
                }
                // Z-
                if(ImGui::TableSetColumnIndex(4)) {
                    if (ImGuiModules::ImageButtonWithText("##JogZ-", settings.guiSettings.img_ArrowDown, settings.guiSettings.imageButton_Toolbar_Jog)) {
                        if (!currently_Jogging)
                            grbl.sendJog(Vec3(0, 0, -jogDistance), feedRate);
                    }
                }
            
            ImGui::EndTable();
        } 
    
        
        ImGui::EndGroup();
        
        
        
        
        
        /*
        
        
        // Draw Jog XY
        ImGui::BeginGroup();
        ImGui::PushID("JogXY");

            ImGui::Dummy(buttonSize);
            ImGui::SameLine();
            // Y +
            if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_ArrowUp)) {
                if (!currently_Jogging)
                    grbl.sendJog(glm::vec3(0, jogDistance, 0), feedRate);
            }
            // X -
            if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_ArrowLeft)) {
                if (!currently_Jogging)
                    grbl.sendJog(glm::vec3(-jogDistance, 0, 0), feedRate);
            }
            ImGui::SameLine();
            
            // "X/Y" text
            ImGuiModules::MoveCursorPosY(yMove);
            ImGuiCustomModules::Heading(settings, "X Y", buttonSize.x);
            ImGuiModules::MoveCursorPosY(-yMove);
            
            ImGui::SameLine();
            // X +
            if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_ArrowRight)) {
                if (!currently_Jogging)
                    grbl.sendJog(glm::vec3(jogDistance, 0, 0), feedRate);
            }
            ImGui::Dummy(buttonSize);
            ImGui::SameLine();
            // Y -
            if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_ArrowDown)) {
                if (!currently_Jogging)
                    grbl.sendJog(glm::vec3(0, -jogDistance, 0), feedRate);
            }
            
        ImGui::PopID();
        ImGui::EndGroup();

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 15);

        // Draw Jog Z
        ImGui::BeginGroup();
        ImGui::PushID("JogZ");
            // Z+
            if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_ArrowUp)) {
                if (!currently_Jogging)
                    grbl.sendJog(glm::vec3(0, 0, jogDistance), feedRate);
            }
            // "Z" text
            ImGuiModules::MoveCursorPosY(yMove);
            ImGuiCustomModules::Heading(settings, "Z", buttonSize.x);
            ImGuiModules::MoveCursorPosY(yMove);
            // Z-
            if (ImGui::ImageButton(buttonSize, buttonImgSize, settings.guiSettings.img_ArrowDown)) {
                if (!currently_Jogging)
                    grbl.sendJog(glm::vec3(0, 0, -jogDistance), feedRate);
            }

        ImGui::PopID();
        ImGui::EndGroup(); 
        */
    }
    void DrawJogSetting(GRBLVals& grblVals) {
        
        ImGui::Checkbox("Control with arrow keys", &allow_Keyb_Jogging);
        
        ImGui::InputFloat("Jog Distance", &jogDistance);

        ImGuiModules::Incrementer("Jog0.1", "0.1", 0.1f, jogDistance, false);
        ImGui::SameLine();
        ImGuiModules::Incrementer("Jog1", "1", 1.0f, jogDistance, false);
        ImGui::SameLine();
        ImGuiModules::Incrementer("Jog10", "10", 10.0f, jogDistance, false);

        ImGui::Separator();

        ImGui::SliderInt("Feed Rate", &feedRate, 0, grblVals.settings.max_FeedRate);
    }

};


// for making button invisible
const bool BUTTON_IS_INVISIBLE  = true;

// For when a toolbar item is always enabled
const bool EDIT_BUTTON_ENABLED  = true;
const bool EDIT_BUTTON_DISABLED = false; 


// All methods handle their opwn disabling (i.e. if grbl is diconnected)
class ToolbarItem 
{
public:
    enum class DisabledFlag { WhenDisconnected, Always, Never};
    // Only one callback can be added and should be used for either the edit button or with a custom button
    ToolbarItem(const std::string& name, DisabledFlag disabledFlag = DisabledFlag::WhenDisconnected, std::function<void()> cbDraw = []() {}, bool hasEditButton = EDIT_BUTTON_DISABLED, std::function<void()> cbDrawPopup = nullptr, ImGuiTableColumnFlags flags = 0) 
      : m_Name(name), 
        m_DisabledFlag(disabledFlag), 
        cb_Draw(cbDraw), 
        m_HasEditButton(hasEditButton), 
        cb_DrawPopup(cbDrawPopup), 
        m_Popup(std::string("Popup ") + name), 
        m_TableColumnFlags(flags)
    {}
    
    void DrawSetupColumn() {
        // Ensure the item is visible
        if(!m_IsVisible) { return; }
        ImGui::TableSetupColumn(m_Name.c_str(), m_TableColumnFlags);
    }
    
    
    
    void Draw(Settings* settings) 
    { 
        // Ensure the item is visible
        if(!m_IsVisible) { return; }
        
        DisableIfRequired(settings, [&]() {
            cb_Draw();
            return false;
        });
    }
    bool DrawEdit(Settings* settings) 
    {
        // Ensure the item is visible and check if there is an edit callback
        if(!m_IsVisible || !m_HasEditButton) { return false; }
        
        // returns true then edit button is clicked
        return DisableIfRequired(settings, [&]() {
            // Draw Edit Button, if clicked, open popup
            return DrawEditButton(settings); 
        });
    }
    // Must be called after everything else
    bool DrawPopup(Settings* settings) 
    {
        // Ensure the item is visible and check if there is an edit callback
        if(!m_IsVisible || !cb_DrawPopup) { return false; }
        // trigger open popup if edit button was pressed
        if(m_OpenPopup) { 
            m_Popup.Open(); 
            m_OpenPopup = false;
        }
        
        // returns true on close (next frame)
        return DisableIfRequired(settings, [&]() {
            // draw popup widgets (given by callback) 
            return m_Popup.Draw([&]() {
                cb_DrawPopup();
                return false;
            });
        });
    }
    // to manually call open popup when no edit button
    // if CallbackType is EditButtonWithCallback, the edit button will open the popup
    void OpenPopup() { m_OpenPopup = true; }
    void SetVisible(bool value) { m_IsVisible = value; }
    bool IsVisible() { return m_IsVisible; }
    
    std::string m_Name;
    DisabledFlag m_DisabledFlag;
    
private:
    bool m_IsVisible = false;
    bool m_OpenPopup = false;
    std::function<void()> cb_Draw;
    bool m_HasEditButton = false;
    std::function<void()> cb_DrawPopup;
    ImGuiModules::ImGuiPopup m_Popup;
    ImGuiTableColumnFlags m_TableColumnFlags;
    
    bool DrawEditButton(Settings* settings) {
        // align right: ImGui::GetContentRegionAvail().x - width
        ImGui::SameLine(ImGui::CalcTextSize(m_Name.c_str()).x + 5.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        //ImGui::SameLine(13.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            bool isClicked = ImGuiCustomModules::EditButton(*settings, m_Name.c_str()); // name as id
        ImGui::PopStyleVar();
        return isClicked;
    }
    
    bool DisableIfRequired(Settings* settings, std::function<bool()> cb)
    {
        bool isDisabled;
        if(m_DisabledFlag == DisabledFlag::Always)                  { isDisabled = true; }
        else if(m_DisabledFlag == DisabledFlag::Never)              { isDisabled = false; }
        else if(m_DisabledFlag == DisabledFlag::WhenDisconnected)   { isDisabled = !settings->grblVals.isConnected; }
        else { Log::Critical("Unknown disabled flag"); }
        
        // Disable widgets if needed
        if(isDisabled) { ImGui::BeginDisabled(); }
            // Draw Edit Button, if clicked, open popup
            bool isClicked = cb();
        // Enable widgets if needed
        if(isDisabled) { ImGui::EndDisabled(); }
        return isClicked;
    }
};

      


    // Always centre this window when appearing
    //ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    //ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));


struct Toolbar {
        

    //  TOP LEVEL
    //   -> Draw
    //      DRAWING LEVEL
    //       ->New/Open/Save Drawing
    //       ->New Sketch
    //          SKETCH LEVEL
    //           ->Select 
    //           ->Point
    //           ->Line
    //           ->Arc
    //           ->Circle
    //       ->Tool / Material
    //       ->Cut Path
    //          FUNCTION LEVEL
    //       ->Drill
    //          FUNCTION LEVEL
    //  -> Run
    //      RUN LEVEL
    //      -> Connect
    //      -> Open
    //      -> Set X0 Y0 Z0, reset, home etc.
    //      -> Custom GCodes
    //      -> Overrides(collapsable)
    //      -> Jog(collapsable)
    

    // Defines which widgets to show
    enum class CurrentLevel { Run, Drawing, Settings, Sketch, Function_CutPath };
    
    CurrentLevel Level() { return m_CurrentLevel; };

//private: TODO:
    
    // states what Toolbar level we are in
    CurrentLevel m_CurrentLevel; // initialised in constructor
    bool levelUpdateRequired = true;
    

    // The toolbar items
    Vector_Ptrs<ToolbarItem> toolbarItems;
    // Indexes for toolbar items
    struct ToolbarIndex {
        ToolbarItem* Header_Navigation;
        ToolbarItem* Header_Back;
        ToolbarItem* Header_Run;
        ToolbarItem* Header_Draw;
        ToolbarItem* Header_Sketch;
        ToolbarItem* Header_Settings;
        ToolbarItem* Connect;
        ToolbarItem* OpenFile;
        ToolbarItem* Tools;
        ToolbarItem* Spacer;
        ToolbarItem* Jog;
        ToolbarItem* Sketch;
        ToolbarItem* SketchTools;
        ToolbarItem* SketchConstraints;
        ToolbarItem* Function_CutPath;
        ToolbarItem* SwitchView;
    } toolbarIndex;
    
    bool openPopup_FileBrowser = false;
    
    // Gets count of the visible toolbar items
    size_t VisibleToolbarItemCount() {
        size_t counter = 0;
        for(size_t i = 0; i < toolbarItems.Size(); i++) {
            if(toolbarItems[i].IsVisible()) {
                counter++;
            }
        }
        return counter;
    }
    
    // jog controller + keyboard input
    JogController jogController;
    
    // For timing the file
    Timer timer;

    // these are stored because otherwise references are lost in lambdas
    Settings* m_Settings;
    Sketch::Sketcher* m_Sketcher; 
    FileBrowser* m_FileBrowser;

    // shorthand
    typedef ToolbarItem::DisabledFlag DisabledFlag;

    Toolbar(GRBL& grbl, Settings* settings, Sketch::Sketcher* sketcherNew, FileBrowser* fileBrowser) 
        : m_Settings(settings), m_Sketcher(sketcherNew), m_FileBrowser(fileBrowser)
    {
        Event<Event_ResetFileTimer>::RegisterHandler([&](Event_ResetFileTimer data) {
            (void)data;
            timer.Reset();
        });
      
// Current LEVEL Header (simplifed greyed out out button)
            
        // Run / Draw Switch
        toolbarIndex.Header_Navigation = toolbarItems.Addp("Navigation##SwitchHeaderColumn", DisabledFlag::Never, [&]() {
               
            GUISettings& s = m_Settings->guiSettings;
          
        // DRAW / RUN BUTTON
            
            bool isDrawActive = (m_CurrentLevel == CurrentLevel::Drawing) || (m_CurrentLevel == CurrentLevel::Sketch) || (m_CurrentLevel == CurrentLevel::Function_CutPath);
            bool isRunActive = (m_CurrentLevel == CurrentLevel::Run);
            bool isSettingsActive = (m_CurrentLevel == CurrentLevel::Settings);
            
            
            
            // remove spacing between items
            ImGui::BeginGroup();
                // Draw image button
                if(ImGuiModules::ImageButtonWithText("Run##ToolbarNavigation", s.img_Play, s.imageButton_Toolbar_LevelToggler, isRunActive)) {
                    // Set Toolbar level
                    SetToolbarLevel(CurrentLevel::Run);
                    m_Settings->SetUpdateFlag(ViewerUpdate::Full); 
                }
                // on top of each other
                if (ImGuiModules::ImageButtonWithText("Draw##ToolbarNavigation", s.img_Sketch_Draw, s.imageButton_Toolbar_LevelToggler, isDrawActive)) {
                    // Set Toolbar level
                    SetToolbarLevel(CurrentLevel::Drawing);
                    m_Settings->SetUpdateFlag(ViewerUpdate::Full); 
                }
            ImGui::EndGroup();
            
            ImGui::SameLine();
        // Settings BUTTON
            
            if (ImGuiCustomModules::ImageButtonWithText_CentredVertically("##SettingsToolbarNavigation", s.img_Settings, s.imageButton_Toolbar_Settings, isSettingsActive, s.toolbarItemHeight)) {
                // Set Toolbar level
                SetToolbarLevel(CurrentLevel::Settings);
                m_Settings->SetUpdateFlag(ViewerUpdate::Full);   
            }
            
        });
        // Toolbar Header Item
        // draws text and an image by drawing an image button with text and turning off button background colour
        auto DrawHeaderButton = [](Settings* settings, const std::string& name, ImageTexture& image) {
            // Make button graphics invisible (make same colour as background)
            ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.0f, 0.0f, 0.0f, 0.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 0.0f });
            // Draw ImGui Image button with text
            bool isClicked = ImGuiModules::ImageButtonWithText(name, image, settings->guiSettings.imageButton_Toolbar_Header);
            ImGui::PopStyleColor(3);
            return isClicked;
        };
            
        // Run Header
        toolbarIndex.Header_Run = toolbarItems.Addp("##RunHeaderColumn", DisabledFlag::Never, [&]() {
            DrawHeaderButton(m_Settings, "Run##RunHeaderToolbarButton", m_Settings->guiSettings.img_Play);
        });
        // Draw Header
        toolbarIndex.Header_Draw = toolbarItems.Addp("##DrawHeaderColumn", DisabledFlag::Never, [&]() {
            DrawHeaderButton(m_Settings, "Draw##DrawHeaderToolbarButton", m_Settings->guiSettings.img_Sketch_Draw);
        });
        // Sketch Header
        toolbarIndex.Header_Sketch = toolbarItems.Addp("##SketchHeaderColumn", DisabledFlag::Never, [&]() {
            DrawHeaderButton(m_Settings, "Sketch##SketchHeaderToolbarButton", m_Settings->guiSettings.img_Sketch);
        });
        // Sketch Header
        toolbarIndex.Header_Settings = toolbarItems.Addp("##SettingsHeaderColumn", DisabledFlag::Never, [&]() {
            DrawHeaderButton(m_Settings, "Settings##SettingsHeaderToolbarButton", m_Settings->guiSettings.img_Settings);
        });
        

            
         //  ImGui::SameLine();
         //      
         //  // BACK BUTTON
         //  // we disable the back button in the top level (save this value as it may change)
         //  bool isDisabled = !(m_CurrentLevel == CurrentLevel::Sketch || m_CurrentLevel == CurrentLevel::Function);
         //  // Disable back button in the top level
         //  if(isDisabled) { ImGui::BeginDisabled(); } 
         //  
         //      // Draw ImGui Widgets
         //      if (ImGuiCustomModules::ImageButtonWithText_CentredVertically("##BackToolbarNavigation", s.img_ArrowLeft, s.imageButton_Toolbar_Back, false, s.toolbarItemHeight)) {
         //          // Move up a level
         //          //if(m_CurrentLevel == CurrentLevel::TopLevel)        { } // do nothing
         //          if(m_CurrentLevel == CurrentLevel::Run)             { } // do nothing
         //          else if(m_CurrentLevel == CurrentLevel::Drawing)    { } // do nothing
         //          else if(m_CurrentLevel == CurrentLevel::Settings)   { } // do nothing
         //          else if(m_CurrentLevel == CurrentLevel::Sketch)     { SetToolbarLevel(CurrentLevel::Drawing); }
         //          else if(m_CurrentLevel == CurrentLevel::Function)   { SetToolbarLevel(CurrentLevel::Drawing); }
         //          else { Log::Critical("Current toolbar level unknown"); }
         //           // Update viewer
         //          m_Settings->SetUpdateFlag(ViewerUpdate::Full);   
         //      }
         //
         //  if(isDisabled) { ImGui::EndDisabled(); } 
            
        // BACK BUTTON

        toolbarIndex.Header_Back = toolbarItems.Addp("##BackHeaderColumn", DisabledFlag::Never, [&]() {
               
            GUISettings& s = m_Settings->guiSettings;
            // Draw ImGui Widgets
            if (ImGuiCustomModules::ImageButtonWithText_CentredVertically("##BackToolbarNavigation", s.img_ArrowLeft, s.imageButton_Toolbar_Back, false, s.toolbarItemHeight)) {
                // Move up a level
                if(m_CurrentLevel == CurrentLevel::Run)                     { } // do nothing
                else if(m_CurrentLevel == CurrentLevel::Drawing)            { } // do nothing
                else if(m_CurrentLevel == CurrentLevel::Settings)           { } // do nothing
                else if(m_CurrentLevel == CurrentLevel::Sketch)             { SetToolbarLevel(CurrentLevel::Drawing); }
                else if(m_CurrentLevel == CurrentLevel::Function_CutPath)   { SetToolbarLevel(CurrentLevel::Drawing); }
                else { Log::Critical("Current toolbar level unknown"); }
                 // Update viewer
                m_Settings->SetUpdateFlag(ViewerUpdate::Full);   
            }
        });
        
        
// Run LEVEL
        // Connect Button
        toolbarIndex.Connect = toolbarItems.Addp("Connect", DisabledFlag::Never, [&]() {
            GUISettings& s = m_Settings->guiSettings;
            // Draw ImGui Widgets
            ImGui::BeginGroup();
                std::string name = (m_Settings->grblVals.isConnected) ? "Connected##ToolbarButton" : "Connect##ToolbarButton";
                // Draw button
                if (ImGuiCustomModules::ImageButtonWithText_CentredVertically(name, s.img_Connect, s.imageButton_Toolbar_Connect, m_Settings->grblVals.isConnected, s.toolbarItemHeight)) { 
                    // Connect / disconnect from GRBL
                    (m_Settings->grblVals.isConnected) ? grbl.disconnect() : grbl.connect(m_Settings->p.system.serialDevice, stoi(m_Settings->p.system.serialBaudrate));
                }
            ImGui::EndGroup();   
            
        }, EDIT_BUTTON_ENABLED, [&]() { 
            // Draw edit popup
            ImGui::SetNextItemWidth(80.0f);
            ImGui::InputText("Device", &m_Settings->p.system.serialDevice);
            ImGui::SetNextItemWidth(80.0f);
            ImGui::InputText("Baudrate", &m_Settings->p.system.serialBaudrate, ImGuiInputTextFlags_CharsDecimal);
        });
        
        // Open File Button
        toolbarIndex.OpenFile = toolbarItems.Addp("Open File", DisabledFlag::WhenDisconnected, [&]() {
            GUISettings& s = m_Settings->guiSettings;
            // Make active if file selected
            bool isActive = (m_FileBrowser->CurrentFile() != "");
            if (ImGuiCustomModules::ImageButtonWithText_CentredVertically("Open##ToolbarButton", s.img_Open, s.imageButton_Toolbar_ButtonPrimary, isActive, s.toolbarItemHeight)) { 
                // Check file is not running
                if(m_Settings->grblVals.isFileRunning) { Log::Error("A file is running. This must finish before opening another"); } 
                // open popup when draw open file clicked
                toolbarIndex.OpenFile->OpenPopup();
                m_Settings->SetUpdateFlag(ViewerUpdate::Clear);
            }
        }, EDIT_BUTTON_DISABLED, [&]() { 
            // just draw the widgets for the filebrowser as we are handling the popup ourselves
            // if an error occurs of a file is selected, manually close popup
            if(m_FileBrowser->DrawWidgets()) {
                ImGui::CloseCurrentPopup();
            }
        });

        
      
// Drawing LEVEL  

        // New Sketch Button
        toolbarIndex.Sketch = toolbarItems.Addp("Sketch##Column", DisabledFlag::Never, [&]() {
            GUISettings& s = m_Settings->guiSettings;
            // Draw ImGui Widgets
            if (ImGuiCustomModules::ImageButtonWithText_CentredVertically("Sketch##ToolbarButton", s.img_Sketch, s.imageButton_Toolbar_ButtonPrimary, false, s.toolbarItemHeight)) { 
                // Set Toolbar level
                SetToolbarLevel(CurrentLevel::Sketch);
                m_Settings->SetUpdateFlag(ViewerUpdate::Full);   
            }
        });

        // New Function Button
        toolbarIndex.Function_CutPath = toolbarItems.Addp("Functions##Column", DisabledFlag::Never, [&]() {
            GUISettings& s = m_Settings->guiSettings;
            bool isActive = (m_CurrentLevel == CurrentLevel::Function_CutPath);
            // Draw ImGui Widgets
            if (ImGuiCustomModules::ImageButtonWithText_CentredVertically("Cut Path##ToolbarButton", s.img_Function_CutPath, s.imageButton_Toolbar_Button, isActive, s.toolbarItemHeight)) { 
                // Set Toolbar level
                SetToolbarLevel(CurrentLevel::Function_CutPath);
                m_Settings->SetUpdateFlag(ViewerUpdate::Full);   
            }
        });

// Sketch LEVEL  
        // SketchTools
        toolbarIndex.SketchTools = toolbarItems.Addp("Sketch Tools", DisabledFlag::Never, [&]() {
            // Draw ImGui Widgets
            typedef Sketch::SketchEvents::CommandType CommandType;
            GUISettings& s = m_Settings->guiSettings;
            // Select Button
            if (ImGuiCustomModules::ImageButtonWithText_CentredVertically("Select##ToolbarButton", s.img_Sketch_Select, s.imageButton_Toolbar_SketchPrimary, m_Sketcher->Events().GetCommandType() == CommandType::Select, s.toolbarItemHeight)) { 
                 m_Sketcher->Events().SetCommandType(CommandType::Select);
            }
            
            ImGui::SameLine();
            
            ImGui::BeginGroup();
                ImGuiModules::CentreItemVerticallyAboutItem(s.toolbarItemHeight, s.imageButton_Toolbar_Sketch->buttonSize.y);
                
                // Add Point Button
                if (ImGuiModules::ImageButtonWithText("Point##ToolbarButton", s.img_Sketch_Point, s.imageButton_Toolbar_Sketch, m_Sketcher->Events().GetCommandType() == CommandType::Add_Point)) {  
                     m_Sketcher->Events().SetCommandType(CommandType::Add_Point);
                }
                ImGui::SameLine();
                // Add Line Button
                if (ImGuiModules::ImageButtonWithText("Line##ToolbarButton", s.img_Sketch_Line, s.imageButton_Toolbar_Sketch, m_Sketcher->Events().GetCommandType() == CommandType::Add_Line)) { 
                     m_Sketcher->Events().SetCommandType(CommandType::Add_Line);
                }
                ImGui::SameLine();
                // Add Arc Button
                if (ImGuiModules::ImageButtonWithText("Arc##ToolbarButton", s.img_Sketch_Arc, s.imageButton_Toolbar_Sketch, m_Sketcher->Events().GetCommandType() == CommandType::Add_Arc)) { 
                     m_Sketcher->Events().SetCommandType(CommandType::Add_Arc);
                }
                ImGui::SameLine();
                // Add Circle Button
                if (ImGuiModules::ImageButtonWithText("Circle##ToolbarButton", s.img_Sketch_Circle, s.imageButton_Toolbar_Sketch, m_Sketcher->Events().GetCommandType() == CommandType::Add_Circle)) {  
                     m_Sketcher->Events().SetCommandType(CommandType::Add_Circle);
                }
            ImGui::EndGroup();
        });
        
        toolbarIndex.SketchConstraints = toolbarItems.Addp("Constraints", DisabledFlag::Never, [&]() {
            
            
            GUISettings& s = m_Settings->guiSettings;
            float frameHeight = m_Settings->guiSettings.toolbarItemHeight;
            ImVec2& buttonSize = s.imageButton_Toolbar_Constraint->buttonSize;
            
            typedef Sketch::RenderData::Image::Type Type;
            
            // Draws an image button for a constraint
            auto cb_ImageButton = [&](const std::string& name, Type imageType) {
                
                ImageTexture* image;
                if(imageType == Type::Coincident)        { image = &s.img_Sketch_Constraint_Coincident; } 
                else if(imageType == Type::Midpoint)     { image = &s.img_Sketch_Constraint_Midpoint; }
                else if(imageType == Type::Vertical)     { image = &s.img_Sketch_Constraint_Vertical; }
                else if(imageType == Type::Horizontal)   { image = &s.img_Sketch_Constraint_Horizontal; }
                else if(imageType == Type::Parallel)     { image = &s.img_Sketch_Constraint_Parallel; }
                else if(imageType == Type::Perpendicular){ image = &s.img_Sketch_Constraint_Perpendicular; }
                else if(imageType == Type::Tangent)      { image = &s.img_Sketch_Constraint_Tangent; }
                else if(imageType == Type::Equal)        { image = &s.img_Sketch_Constraint_Equal; }
                else if(imageType == Type::Distance)     { image = &s.img_Sketch_Constraint_Distance; }
                else if(imageType == Type::Radius)       { image = &s.img_Sketch_Constraint_Radius; }
                else if(imageType == Type::Angle)        { image = &s.img_Sketch_Constraint_Angle; }
                else { Log::Critical("Unknown image type"); }
                 
                return ImGuiModules::ImageButtonWithText(name, *image, s.imageButton_Toolbar_Constraint);
            };
            
            // Draws an inputbox for a constraint (distance, radius, angle etc.)
            auto cb_InputValue = [&](double* value) {
                // Set inputbox width
                ImGui::SetNextItemWidth(m_Settings->guiSettings.toolbarWidgetWidth);
                // Centre align to the constraint buttons
                ImGuiModules::CentreItemVerticallyAboutItem(frameHeight);
                // Draw inputbox
                ImGui::InputDouble(std::string("##" + std::to_string((int)value)).c_str(), value, 0.0, 0.0, "%g"); // use value ptr as ID
            };
            
            
            
            // Add Constraint Buttons
            ImGui::BeginGroup();
                // Centre contraint buttons about main toolbar buttons
                ImGuiModules::CentreItemVerticallyAboutItem(frameHeight, buttonSize.y);
                // TODO: Order of selection needs to be preserved - if circle then arc, circle should jump onto arc... atm it's just default order
                m_Sketcher->Draw_ConstraintButtons(cb_ImageButton, cb_InputValue);
            ImGui::EndGroup();
        });
 
  
  // RIGHT ALIGNED TABLE ITEMS
  
        // Spacer
        toolbarIndex.Spacer = toolbarItems.Addp("##Spacer", DisabledFlag::WhenDisconnected, [&]() {}, EDIT_BUTTON_DISABLED, nullptr, ImGuiTableColumnFlags_WidthStretch);
          
        // Tools
        toolbarIndex.Tools = toolbarItems.Addp("Tools", DisabledFlag::Never, [&]() {
            // centre the 2 dropdown boxes about the main buttons
            ImGui::BeginGroup();
            
                float frameHeight = m_Settings->guiSettings.toolbarItemHeight;
                //ImGuiModules::MoveCursorPosY((itemHeight - thisItemHeight) / 2.0f);
                ImGuiModules::CentreItemVerticallyAboutItem(frameHeight, ImGui::GetFrameHeight() * 2.0f + ImGui::GetStyle().ItemSpacing.y);
                // Draw ImGui Widgets
                ImGui::SetNextItemWidth(m_Settings->guiSettings.toolbarToolMaterialWidth);
                if(m_Settings->p.toolSettings.DrawTool()) {
                    m_Settings->SetUpdateFlag(ViewerUpdate::Full);
                }
                // Draw Material
                ImGui::SetNextItemWidth(m_Settings->guiSettings.toolbarToolMaterialWidth);
                if(m_Settings->p.toolSettings.DrawMaterial()) {
                    m_Settings->SetUpdateFlag(ViewerUpdate::Full);
                }
            ImGui::EndGroup();
        }, EDIT_BUTTON_ENABLED, [&]() { 
            // Draw edit popup
            if(m_Settings->p.toolSettings.DrawPopup()) {
                ImGui::CloseCurrentPopup(); 
            }
        });
        
        
        // Jog
        toolbarIndex.Jog = toolbarItems.Addp("Jog", DisabledFlag::WhenDisconnected, [&]() {
            // Draw ImGui Widgets
            jogController.DrawJogController(grbl, *m_Settings);
        }, EDIT_BUTTON_ENABLED, [&]() { 
            // Draw edit popup
            jogController.DrawJogSetting(m_Settings->grblVals);
        });
        
        // 2D / 3D
        toolbarIndex.SwitchView = toolbarItems.Addp("Switch View", DisabledFlag::Never, [&]() {
            // Draw ImGui Widgets
            GUISettings& s = m_Settings->guiSettings;
            // Draw ImGui Widgets
            ImGui::BeginGroup();
            
                bool is2DMode;
                Event<Event_Get2DMode>::Dispatch( { is2DMode } );
                
                std::string name = (is2DMode) ? "3D##ToolbarButton" : "2D##ToolbarButton";
                
                // Draw button
                if (ImGuiCustomModules::ImageButtonWithText_CentredVertically(name, s.img_Connect, s.imageButton_Toolbar_Connect, is2DMode, s.toolbarItemHeight)) { 
                    Event<Event_Set2DMode>::Dispatch( { !is2DMode } );
                }
            ImGui::EndGroup();  
        });

// Function LEVEL  
        // ...
        
        
        // initialise toolbar level (show top level items)
        SetToolbarLevel(CurrentLevel::Run);
        UpdateLevel();
    }
     
    
    //void DrawTableSeperator(Settings& settings) 
    //{
    //    // draw vertical seperator line
    //    float spacerWidth = settings.guiSettings.toolbarSpacer;
    //    ImVec2 p0 = ImGui::GetCursorScreenPos() + ImVec2(spacerWidth / 2.0f, GImGui->Style.ItemSpacing.y / 2.0f);
    //    ImVec2 p1 = p0 + ImVec2(1.0f /*thickness*/, 2.0f*ImGui::GetFrameHeight() /*length*/);
    //    ImGui::GetWindowDrawList()->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_Separator));
    //
    
    
    //  TOP LEVEL
    //   -> Draw
    //      DRAWING LEVEL
    //       ->New/Open/Save Drawing
    //       ->New Sketch
    //          SKETCH LEVEL
    //           ->Select 
    //           ->Point
    //           ->Line
    //           ->Arc
    //           ->Circle
    //       ->Tool / Material
    //       ->Cut Path
    //          FUNCTION LEVEL
    //       ->Drill
    //          FUNCTION LEVEL
    //  -> Run
    //      RUN LEVEL
    //      -> Connect
    //      -> Open
    //      -> Set X0 Y0 Z0, reset, home etc.
    //      -> Custom GCodes
    //      -> Overrides(collapsable)
    //      -> Jog(collapsable)
    
    
    void SetToolbarLevel(CurrentLevel level) {
        
        m_CurrentLevel = level;
        levelUpdateRequired = true;
    }
private:
    // updates based on current level set
    void UpdateLevel() {
        
        if(!levelUpdateRequired) { return; }
        levelUpdateRequired = false;
        // Set all items invisible
        for(size_t i = 0; i < toolbarItems.Size(); i++) {
            toolbarItems[i].SetVisible(false);
        }
        
        // Set item visible 
        auto SetItemVisible = [&](ToolbarItem* item) {
            item->SetVisible(true); 
        };
        
        
        // Navigation - visible to all
        SetItemVisible(toolbarIndex.Header_Navigation);
        // Run
        if(m_CurrentLevel == CurrentLevel::Run) {
            // Set visible toolbar items
            SetItemVisible(toolbarIndex.Header_Run); // as header
            SetItemVisible(toolbarIndex.Connect);
            SetItemVisible(toolbarIndex.OpenFile);
            SetItemVisible(toolbarIndex.Spacer);
            SetItemVisible(toolbarIndex.Jog);
        } else {
            // Clear the open file
            //fileBrowser->ClearCurrentFile();
        }
        
        // Drawing
        if(m_CurrentLevel == CurrentLevel::Drawing) {
            // Set visiable toolbar items
            SetItemVisible(toolbarIndex.Header_Draw); // as header
            // SetItemVisible(Drawing (New/Open/Save))  ***MAYBE THIS SHOULD BE A DROPDOWN? 
            SetItemVisible(toolbarIndex.Sketch);
            SetItemVisible(toolbarIndex.Function_CutPath);
            SetItemVisible(toolbarIndex.Spacer);
            SetItemVisible(toolbarIndex.Tools);
            SetItemVisible(toolbarIndex.SwitchView);
        } 
        
        // Settings
        if(m_CurrentLevel == CurrentLevel::Settings) {
            // Set visiable toolbar items 
            SetItemVisible(toolbarIndex.Header_Settings); // as header
        } 
        
        // Sketch
        if(m_CurrentLevel == CurrentLevel::Sketch) {
            // Set visiable toolbar items
            SetItemVisible(toolbarIndex.Header_Back);
            SetItemVisible(toolbarIndex.Header_Sketch); // as header
            SetItemVisible(toolbarIndex.SketchTools);
            SetItemVisible(toolbarIndex.SketchConstraints);
            // Set sketch command type
            m_Sketcher->Events().SetCommandType(Sketch::SketchEvents::CommandType::Select);
        } else {
            // Reset sketch command type
            m_Sketcher->Events().SetCommandType(Sketch::SketchEvents::CommandType::None);
        }
        
        // Functions
        if(m_CurrentLevel == CurrentLevel::Function_CutPath) {
            SetItemVisible(toolbarIndex.Header_Back);
            SetItemVisible(toolbarIndex.Spacer);
            SetItemVisible(toolbarIndex.Tools);
            SetItemVisible(toolbarIndex.SwitchView);
        } else {
            // Reset sketch command type
            m_Sketcher->Events().SetCommandType(Sketch::SketchEvents::CommandType::None);
        }

        
        // Set 2D mode
        if(m_CurrentLevel == CurrentLevel::Sketch || m_CurrentLevel == CurrentLevel::Function_CutPath) {
            // Set 2D Mode
            Event<Event_Set2DMode>::Dispatch( { true } );
        } else {
            // Unset 2D Mode
            Event<Event_Set2DMode>::Dispatch( { false } );
        }
    }
        
        
public:
    
    void Draw(GRBL &grbl, sketch::SketchOld& sketcherOld) 
    {                
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float padding = m_Settings->guiSettings.dockPadding;
        // Set window size and position
        ImGui::SetNextWindowPos(viewport->WorkPos + ImVec2(padding, padding));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - 2.0f * padding, m_Settings->guiSettings.toolbarHeight));
        // flags for window
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse 
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        // Create Toolbar Window
        if (!ImGui::Begin("Toolbar", NULL, ImGuiCustomModules::ImGuiWindow::generalWindowFlags | window_flags)) {
            ImGui::End();
            return;
        }
        // push the general style widget, normally this would be done in ImGuiCustomModules::ImGuiWindow::Begin 
        ImGuiCustomModules::ImGuiWindow::PushWidgetStyle(*m_Settings);
        
    // DRAW THE TOOLBAR TABLE
    
        // Format the table
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(m_Settings->guiSettings.toolbarSpacer / 2.0f, 2.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, m_Settings->guiSettings.toolbarTableScrollbarSize);
        // flags for table
        ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollX;
                
        // Table handles it's own ImGui::BeginDisabled()
        // Draw Toolbar Table
        if (ImGui::BeginTable("Toolbar",  VisibleToolbarItemCount(), flags, ImVec2(0.0f, m_Settings->guiSettings.toolbarTableHeight))) 
        {           
            // Style the header & edit button
            ImGui::PushFont(m_Settings->guiSettings.font_small);
            ImGui::PushStyleColor(ImGuiCol_Text, m_Settings->guiSettings.colour[Colour::HeaderText]);
            // Set the background colour of the heading row to invisible
            ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, { 0.0f, 0.0f, 0.0f, 0.0f });
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.0f, 0.0f, 0.0f, 0.0f });
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 0.0f, 0.0f, 0.0f, 0.0f });
                        
                // Setup Table columns (will only add visible columns)
                for(size_t i = 0; i < toolbarItems.Size(); i++) {
                    // only draw if visible
                    if(toolbarItems[i].IsVisible()) {
                        toolbarItems[i].DrawSetupColumn();
                    }
                } 
                
                // Set up Header Row with Edit buttons 
                // Instead of calling TableHeadersRow() we'll submit custom headers ourselves so that we can add an Edit Button
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                
                size_t counter = 0;
                // Draw Table Headers (with Edit Button)
                for(size_t i = 0; i < toolbarItems.Size(); i++) 
                {
                    // only draw if visible
                    if(toolbarItems[i].IsVisible()) {
                        // Set Column Index
                        ImGui::TableSetColumnIndex(counter++);
                        ImGui::TableHeader(toolbarItems[i].m_Name.c_str());
                        
                        ImGui::PushID(i);
                           // draw the edit button
                            if(toolbarItems[i].DrawEdit(m_Settings)) {
                                toolbarItems[i].OpenPopup();
                            }
                        ImGui::PopID();
                    }
                }
                
            ImGui::PopStyleColor(4);
            ImGui::PopFont();      
            
            // Draw toolbar items (Widgets etc)
            ImGui::TableNextRow();
        
            for(size_t i = 0; i < toolbarItems.Size(); i++) {
                // only draw if visible
                if(toolbarItems[i].IsVisible()) {
                    ImGui::TableNextColumn();
                    ImGui::BeginGroup();
                        toolbarItems[i].Draw(m_Settings);
                    ImGui::EndGroup();
                }
            }
            
            ImGui::EndTable();
        } 


    // DRAW THE PLAY BUTTONS AND DISPLAY CURRENT FILE

        // We only need to draw with run commands
        if(m_CurrentLevel == CurrentLevel::Run) {
            // Save the state as this could change midway
            bool isDisconnected = !m_Settings->grblVals.isConnected;
            // Start disable again in function scope
            if(isDisconnected) { ImGui::BeginDisabled(); }
                
                ImGui::Separator();
                
                flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_ScrollX;
                
                if (ImGui::BeginTable("ControlButtons", 3, flags))
                {
                    ImGui::TableNextRow();
                    // Run, Cancel, Pause & Restart
                    ImGui::TableNextColumn();
                    DrawPlayButtons(grbl, sketcherOld);
                    // Current file name
                    ImGui::TableNextColumn();
                    DrawCurrentFile();
                    // Time elapsed
                    ImGui::TableNextColumn();
                    if(m_Settings->grblVals.isFileRunning) 
                        DrawTimeElapsed(); 
                    
                    ImGui::EndTable();
                }
                
            
            // end disable all widgets
            if(isDisconnected) { ImGui::EndDisabled(); }
        }
       
        ImGui::PopStyleVar(2);
        
        
        
        /*
        
        
        
        
        
        ImGui::SameLine();
        
        ImGui::BeginGroup();
            // Time elapsed
            if(m_Settings->grblVals.isFileRunning) {
                //ImGui::SameLine(); ImGui::Dummy(ImVec2(50, 0)); ImGui::SameLine();
                DrawTimeElapsed(m_Settings->grblVals);
            }
            ImGui::SameLine(); //ImGui::Dummy(ImVec2(50, 0)); ImGui::SameLine();
            
            // Current file name
            DrawCurrentFile(m_Settings, fileBrowser);
        ImGui::EndGroup();
*/
 



           
        
        // popups handle their own ImGui::BeginDisabled()
            
        // Draw edit button Popups if visible
        for(size_t i = 0; i < toolbarItems.Size(); i++) {
            // Draw Popup
            if(toolbarItems[i].DrawPopup(m_Settings)) {
                // update if windows was just closed
                m_Settings->SetUpdateFlag(ViewerUpdate::Full); 
            }
        }
        
        
      
             
        // end
        ImGuiCustomModules::ImGuiWindow::PopWidgetStyle();
        ImGui::End();
        
        // performs update based on current level set if required
        UpdateLevel();
    }

    

    
    void DrawCurrentFile()
    {
        if(m_FileBrowser->CurrentFile() == "") { 
            return; 
        }
        ImGui::BeginGroup();
            ImGuiModules::CentreItemVertically(2);
            
            // shortens file path to "...path/at/place.gc" if length > max_FilePathDisplay
            /*int cutOff = 0;
            if(currentFilePath.size() > m_Settings->guiSettings.max_FilePathDisplay) {
                cutOff = currentFilePath.size() - m_Settings->guiSettings.max_FilePathDisplay;     
            }                 
            const char* shortenedPath = currentFilePath.c_str() + cutOff;
            ImGui::Text((cutOff ? "..%s" : "%s"), shortenedPath);*/
            ImGuiModules::TextCentredHorizontallyInTable("%s", m_FileBrowser->CurrentFile().c_str());
            ImGuiModules::ToolTip_IfItemHovered(m_FileBrowser->CurrentFilePath().c_str());
        ImGui::EndGroup();
    }

    
    void RunFile(GRBL& grbl) {
        // check we have selected a file
        if (m_FileBrowser->CurrentFile() == "") {
            Log::Error("No file has been selected");
            return;
        }
        // get filepath of file
        string filepath = File::CombineDirPath(m_Settings->p.system.curDir, m_FileBrowser->CurrentFile());
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
    
    void DrawPlayButtons(GRBL& grbl, sketch::SketchOld& sketcher)
    {
        auto SameLineSpacer = [&]() {
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(m_Settings->guiSettings.toolbarSpacer, 0.0f));
            ImGui::SameLine();
        };
        
        GUISettings& s = m_Settings->guiSettings;
        
        ImGui::BeginGroup();
            ImGuiModules::CentreItemVertically(2, s.toolbarItemHeight);

            // Run Button
            if (ImGuiModules::ImageButtonWithText("Run##SubToolbar", s.img_Play, s.imageButton_SubToolbar_Button)) {
                if(sketcher.IsActive()) { 
                    // run sketch function
                    sketcher.ActiveA_Function_Run(grbl, *m_Settings);
                } else { 
                    // run file
                    RunFile(grbl);
                }
            }
            
            ImGui::SameLine();
            if (ImGuiModules::ImageButtonWithText("Cancel##SubToolbar", s.img_Add, s.imageButton_SubToolbar_Button)) {
                if (m_Settings->grblVals.isFileRunning) {
                    Log::Info("Cancelling... Note: Any commands remaining in grbl's buffer will still execute.");
                    grbl.cancel();
                }
            }
            
            SameLineSpacer();
            if (ImGuiModules::ImageButtonWithText("Pause##SubToolbar", s.img_Pause, s.imageButton_SubToolbar_Button)) {
                Log::Info("Pausing...");
                grbl.sendRT(GRBL_RT_HOLD);
            }

            ImGui::SameLine();
            if (ImGuiModules::ImageButtonWithText("Pause##SubToolbar", s.img_Restart, s.imageButton_SubToolbar_Button)) {
                Log::Info("Resuming...");
                grbl.sendRT(GRBL_RT_RESUME);
            }
            SameLineSpacer();
            sketcher.ActiveA_Function_Export(*m_Settings);
            
            ImGui::SameLine();
            sketcher.ActiveA_Function_Delete(*m_Settings);
            
        ImGui::EndGroup(); 
    } 
    
    void DrawTimeElapsed()
    {
        GRBLVals& grblVals = m_Settings->grblVals;
        ImGui::BeginGroup();
            // update time
            timer.UpdateCurrentTime();
            // normalise timer seconds to hours/mins/secs
            Time timeElapsedNorm(timer.dt());
            // how far through file we arefunctions
            float percComplete = (float)grblVals.curLine / (float)grblVals.totalLines;
            // estimate time remaining
            uint timeExpected = timer.dt() / percComplete;
            Time timeRemainingNorm(timeExpected - timer.dt());
            
            // draw ImGui widgets
            float cursorPos_FrameTop = ImGui::GetCursorPosY();
            
            // progress bar
            ImGuiModules::CentreItemVertically(2);
            // stretch
            ImGui::SetNextItemWidth(-155.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
                ImGui::ProgressBar(percComplete, ImVec2(0.0f, 0.0f), va_str("%d/%d (%.2f%%)", grblVals.curLine, grblVals.totalLines, 100.0f * percComplete).c_str());
            ImGui::PopStyleVar();
        
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
                  
        ImGui::EndGroup();
    }
    
    /*
     * 
     * 
    enum class Export         { False, Pending, True };
     * 
        // allows us to determine whether file should be exported (creates popup if file is to be overwritten)
        static pair<Export, string> exportFileName = make_pair(Export::False, "");
        // handle file export
 //       DoesFileNeedExport(settings, functions, functionsexportFileName);
             
             
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
*/ 
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

    void DrawZeroing(GRBL &grbl, Settings& settings) {
        posS.y = 0;

        if (ImGui::BeginTable("XYZ Zeroing", 3, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
            if (ImGui::Button("Zero X", sizeS)) {
                Log::Info( "Setting current X position to 0 for this coord system");
                grbl.send("G10 L20 P0 X0", PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle);
                settings.SetUpdateFlag(ViewerUpdate::Full);
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
            if (ImGui::Button("Zero Y", sizeS)) {
                Log::Info("Setting current Y position to 0 for this coord system");
                grbl.send("G10 L20 P0 Y0", PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle);
                settings.SetUpdateFlag(ViewerUpdate::Full);
            }

            ImGui::TableSetColumnIndex(2);
            ImGui::SetCursorPos(ImGui::GetCursorPos() + posS);
            if (ImGui::Button("Zero Z", sizeS)) {
                Log::Info("Setting current Z position to 0 for this coord system");
                grbl.send("G10 L20 P0 Z0", PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle);
                settings.SetUpdateFlag(ViewerUpdate::Full);
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

                        if (ImGuiModules::WasLastItemRightClicked()) {
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
            
        // begin new imgui window
        static ImGuiCustomModules::ImGuiWindow window(settings, "Stats");
        if(window.Begin(settings)) 
        {    
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
            DrawZeroing(grbl, settings);
            ImGui::Separator();

            DrawCommands(grbl, grblVals);
            ImGui::Separator();

            DrawCustomGCodes(grbl, settings);
            ImGui::Separator();

            static bool viewStatus = grbl.getViewStatusReport();
            if (ImGui::Checkbox("Status Report", &(viewStatus))) {
                grbl.setViewStatusReport(viewStatus);
            }
            
            window.End();
        }
    }
};

    
struct Overrides {
    /*    Other Realtime commands similar but currently unused:
        grbl.SendRT(GRBL_RT_SPINDLE_STOP);
        grbl.SendRT(GRBL_RT_FLOOD_COOLANT);
        grbl.SendRT(GRBL_RT_MIST_COOLANT);
    */
    void Draw(GRBL &grbl, Settings& settings) 
    {  
        // begin new imgui window
        static ImGuiCustomModules::ImGuiWindow window(settings, "Overrides");
        if(window.Begin(settings)) 
        {    
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));
      
            GRBLStatus_vals status = settings.grblVals.status; 
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
            
            window.End();
        }
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
    auto addEntry(const char *name, Vec3 val) {
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
  
    
    void Draw(GRBL &grbl, Settings& settings) 
    {
        GRBLVals& grblVals = settings.grblVals;
        GRBLVals& v = grblVals;
     
         
        // begin new imgui window
        static ImGuiCustomModules::ImGuiWindow window(settings, "Debug");
        if(!window.Begin(settings)) return;
             

        if (ImGui::TreeNode("Settings")) {
            ImGui::InputText("Save File Location", &settings.p.system.saveFileDirectory);
            
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("GUI Settings")) {
            
            
            if (ImGui::CollapsingHeader("Image Buttons")) {
                Vector_Ptrs<ImGuiModules::ImageButtonStyle>& v = settings.guiSettings.imageButtons;
                for(size_t i = 0; i < v.Size(); i++)
                {
                    // draw each image button as a treenode
                    if (ImGui::TreeNode(v[i].name.c_str())) {
                        ImGui::SliderFloat2(string("Button Size##" + v[i].name).c_str(), &v[i].buttonSize.x, 0.0f, 100.0f);
                        ImGui::SliderFloat2(string("Image Size##" + v[i].name).c_str(), &v[i].imageSize.x, 0.0f, 100.0f);
                        ImGui::SliderFloat2(string("Text Offset##" + v[i].name).c_str(), &v[i].textOffset.x, -100.0f, 100.0f);
                        ImGui::SliderFloat2(string("Image Offset##" + v[i].name).c_str(), &v[i].imageOffset.x, -100.0f, 100.0f);
                        ImGuiModules::ImageButtonWithText("Text", settings.guiSettings.img_Icon, v.CastItem<ImGuiModules::ImageButtonStyle>(i));
                        ImGui::TreePop();
                    }
                }
            }
            
            
            ImGui::SliderFloat("Dock Padding", &settings.guiSettings.dockPadding, 0.0f, 100.0f);
            ImGui::SliderFloat("Toolbar Height", &settings.guiSettings.toolbarHeight, 0.0f, 500.0f);
            ImGui::SliderFloat("Toolbar TableHeight", &settings.guiSettings.toolbarTableHeight, 0.0f, 300.0f);
            ImGui::SliderFloat("Toolbar Table Scrollbar Size", &settings.guiSettings.toolbarTableScrollbarSize, 0.0f, 50.0f);
            ImGui::SliderFloat("Toolbar Spacer", &settings.guiSettings.toolbarSpacer, 0.0f, 100.0f);
            ImGui::SliderFloat("Toolbar Item Height", &settings.guiSettings.toolbarItemHeight, 0.0f, 100.0f);
            ImGui::SliderFloat("Toolbar Widget Width", &settings.guiSettings.toolbarWidgetWidth, 0.0f, 500.0f);
            ImGui::SliderFloat("Toolbar Tool/Material Width", &settings.guiSettings.toolbarToolMaterialWidth, 0.0f, 500.0f);
            
            ImGui::SliderFloat("Widget Width", &settings.guiSettings.widgetWidth, 0.0f, 500.0f);
 
            if(ImGui::ColorEdit3("Text Colour", &settings.guiSettings.colour[Colour::Text].x)) {
                ImGui::GetStyle().Colors[ImGuiCol_Text] = settings.guiSettings.colour[Colour::Text];
            }
            ImGui::ColorEdit3("Header Text Colour", &settings.guiSettings.colour[Colour::HeaderText].x);
            
            ImGui::SliderFloat("Position Popup Opacity", &settings.guiSettings.popupPosition_alpha, 0.0f, 1.0f);
            ImGui::SliderFloat("Position Popup Offset", &settings.guiSettings.popupPosition_offsetPos, 0.0f, 100.0f);
 
            if(ImGui::SliderFloat("Message Popup Y Spacing", &settings.guiSettings.popupMessage_YSpacing, 0.0f, 100.0f)) {
                Log::Info("Message!");
            }
            if(ImGui::SliderFloat("Message Popup Timeout", &settings.guiSettings.popupMessage_Time, 0.0f, 100.0f)) {
                Log::Info("Message!");
            }
            
            ImGui::TreePop();
        }


        if (ImGui::TreeNode("System")) {
            addEntry("OpenGl Version", (char *)glGetString(GL_VERSION));
            addEntry("Frame Rate", "%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            static bool debugGCodeListBuild = false;
            static bool debugCharacterCounting = false;
            static bool debugGCodeList = false;
            static bool debugSerial = false;
            static bool debugThreadBlocking = false;
            static bool debugGCReader = false;
            bool needUpdate = false;

            static bool showDebugMenu = false;
            ImGui::Checkbox("Show Debug Menu", &showDebugMenu);
            if(showDebugMenu) {
                ImGui::TextUnformatted("Debug");
                ImGui::Indent();
                needUpdate |= ImGui::Checkbox("GCode List Build", &(debugGCodeListBuild));
                needUpdate |= ImGui::Checkbox("Character Counting", &(debugCharacterCounting));
                needUpdate |= ImGui::Checkbox("GCode List", &(debugGCodeList));
                needUpdate |= ImGui::Checkbox("Serial", &(debugSerial));
                needUpdate |= ImGui::Checkbox("Thread Blocking", &(debugThreadBlocking));
                needUpdate |= ImGui::Checkbox("GCode Reader (For 3D Viewer)", &(debugGCReader));
                ImGui::Unindent();
            
                if (needUpdate) { 
                    int flags = DEBUG_NONE;

                    if (debugGCodeListBuild)
                        flags |= DEBUG_GCLIST_BUILD;
                    if (debugCharacterCounting)
                        flags |= DEBUG_CHAR_COUNTING;
                    if (debugGCodeList)
                        flags |= DEBUG_GCLIST;
                    if (debugSerial)
                        flags |= DEBUG_SERIAL;
                    if (debugThreadBlocking)
                        flags |= DEBUG_THREAD_BLOCKING;
                    if (debugGCReader)
                        flags |= DEBUG_GCREADER;

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
                Log::Info("Saving Config File...");
                Event<Event_SaveSettings>::Dispatch({ }); 
                //settings.SaveToFile();
            }
            if(ImGui::Button("Load Config")) {
                Log::Info("Loading Config File...");
                Event<Event_UpdateSettingsFromFile>::Dispatch({ }); 
                //settings.UpdateFromFile(); 
            }
            ImGui::TreePop();
        }
        window.End();
    }
};
 


void Frames::DrawSketcher(Settings& settings, Sketch::Sketcher& sketcher) 
{
    (void)settings; (void)sketcher;
    /*  
    // Cursor Popup
    static ImGuiModules::ImGuiPopup popup_CursorRightClick("popup_Sketch_CursorRightClick");
    // open
    if(m_Drawings.HasItemSelected()) 
    {
        if(auto id =  m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_GetID()) 
        {
            // set to open
            if(!ImGui::GetIO().WantCaptureMouse && IsActive()) {
                if(trigger(settings.p.sketch.cursor.popup.shouldOpen)) { popup_CursorRightClick.Open(); }
            }
            // draw cursor popup
            popup_CursorRightClick.Draw([&]() {
                ImGui::Text("Point %u", (uint)*id);
                ImGui::Separator();
                // delete
                if(ImGui::Selectable("Delete")) {
                    if(m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_Delete()) {
                        settings.SetUpdateFlag(ViewerUpdate::Full);
                    }
                }
            });
        }
    }
    
    
    static bool isNewDrawing = true;
    
    // display x, y coord on screen if not over imgui window
    if(!ImGui::GetIO().WantCaptureMouse && IsActive()) {
        DrawPopup_Cursor(settings);
    }
    
    */
    // begin new imgui window
 ////  static ImGuiCustomModules::ImGuiWindow window(settings, "Sketcher"); // default size
 ////  if(window.Begin(settings)) 
 ////  {    
 ////      if(ImGui::Button("Update")) {
 ////          sketcher.Update(2, { 100.0f, 200.0f});
 ////      }
 ////      window.End();
 ////  }
        
        
        /*
        if (ImGui::SmallButton("New Drawing")) {
            m_Drawings.Add(A_Drawing("Drawing " + to_string(m_DrawingIDCounter++)));
            isNewDrawing = true;
            settings.SetUpdateFlag(ViewerUpdate::Full);
        } 
        
        for(size_t i = 0; i < m_Drawings.Size(); )
        {
            // set active drawing to be open initially & inactive drawings to be closed
            if(m_Drawings.CurrentIndex() == (int)i) { 
                if(isNewDrawing) {
                    ImGui::SetNextItemOpen(true); 
                    isNewDrawing = false;
                }
            } else { ImGui::SetNextItemOpen(false); }
            // close button flag - set by imgui
            bool closeIsntClicked = true; 
            if (ImGui::CollapsingHeader(m_Drawings[i].Name().c_str(), &closeIsntClicked)) {
                // set the active index to match the open tab
                if(m_Drawings.CurrentIndex() != (int)i) {
                    std::cout << "Setting current drawing index" << std::endl;
                    m_Drawings.SetCurrentIndex(i);
                    settings.SetUpdateFlag(ViewerUpdate::Full);
                }
                // draw the imgui widgets for drawing 
                m_Drawings.CurrentItem().DrawImGui(settings); 
            }
            if(!closeIsntClicked) { // has been closed
                m_Drawings.Remove(i); 
                settings.SetUpdateFlag(ViewerUpdate::Full);
            } else { 
                i++; 
            }                        

        }
        window.End();
    }*/
    

     
}


void Frames::DrawDockSpace(Settings& settings)
{
    // fullscreen dockspace
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    float padding = settings.guiSettings.dockPadding;
    float toolbarHeight = settings.guiSettings.toolbarHeight;
    // create the dockspace below the main tool bar
    ImGui::SetNextWindowPos(viewport->WorkPos + ImVec2(padding, toolbarHeight + 2.0f*padding));
    ImGui::SetNextWindowSize(viewport->WorkSize - ImVec2(padding*2.0f, toolbarHeight + 3.0f*padding));
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
      

      
void Frames::Draw(GRBL& grbl, Settings* settings, Viewer& viewer, sketch::SketchOld& sketcherOld, Sketch::Sketcher* sketcherNew, float dt)
{
    
    // draw ImGui windows
    DrawDockSpace(*settings);
    
        static PopupMessages popupMessages;
        static Toolbar toolbar(grbl, settings, sketcherNew, fileBrowser.get());
        static Debug debug;
        static Stats stats;
        static Console console;
        static Commands commands;
        static Overrides overrides;
        static Functions functions(settings, sketcherNew);
        
        // Console messages overlaying screen
        popupMessages.Draw(*settings, dt);
        
        // Draws the toolbar (handles it's own BeginDisabled())
        toolbar.Draw(grbl, sketcherOld);
        
        
        // TODO: TEMPORARILY MAKING THESE VISIBLE THE ENTIRE TIME
        // Debug settings
        debug.Draw(grbl, *settings);
        // show ImGui Demo 
        ImGui::ShowDemoWindow(NULL);
        
        // Draw frames corrosponding to Run
        if(toolbar.Level() == Toolbar::CurrentLevel::Run) {    
            // These frames are disabled if disconnected and show only if correct Toolbar Level is selected
            // Save the state as this could change midway
            bool isDisconnected = !settings->grblVals.isConnected;
            // Make everything disabled is disconnected
            if(isDisconnected) { ImGui::BeginDisabled(); }
                // draw the Run based windows
                stats.Draw(grbl, *settings, dt);
                console.Draw(grbl, *settings);
                commands.Draw(grbl, *settings);
                overrides.Draw(grbl, *settings);
            // Make everything disabled is disconnected
            if(isDisconnected) { ImGui::EndDisabled(); }
        }
        // Draw frames corrosponding to Drawing
        else if(toolbar.Level() == Toolbar::CurrentLevel::Drawing) {
            
        }
        // Draw frames corrosponding to Settings
        else if(toolbar.Level() == Toolbar::CurrentLevel::Settings) {
            // Draw Viewer Imgui Widgets
            viewer.ImGuiRender(*settings);
         //   // Debug settings
         //   debug.Draw(grbl, *settings);
         //   // show ImGui Demo 
         //   ImGui::ShowDemoWindow(NULL);
        }
        // Draw frames corrosponding to Sketch
        else if(toolbar.Level() == Toolbar::CurrentLevel::Sketch) {
            // Draw Sketch ImGui
            sketcherNew->DrawImGui();
            
        } 
        // Draw frames corrosponding to Function
        else if(toolbar.Level() == Toolbar::CurrentLevel::Function_CutPath) {
            // draw functions side panel
            functions.DrawWindows();
        } 
            
}

} // end namespace Sqeak

/*
 * frames.h
 *  Max Peglar-Willis 2021
 */

#pragma once

// get height of line / frame
//std::cout << "LineHeight = " << ImGui::GetTextLineHeight() << "LineHeightWithSpacing = " << ImGui::GetTextLineHeightWithSpacing() << std::endl;
//std::cout << "FrameHeight = " << ImGui::GetFrameHeight() << "FrameHeightWithSpacing = " << ImGui::GetFrameHeightWithSpacing() << std::endl;

struct ImGuiModules
{
    static void KeepWindowInsideViewport() {
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        
        ImVec2 newPos = windowPos;
        
        if(windowPos.x < 0.0f)
            newPos.x = 0.0f;
        if(windowPos.y < 0.0f)
            newPos.y = 0.0f;
        if(windowPos.x + windowSize.x > screenSize.x)
            newPos.x = screenSize.x - windowSize.x;
        if(windowPos.y + windowSize.y > screenSize.y)
            newPos.y = screenSize.y - windowSize.y;
            
        ImGui::SetWindowPos(newPos);
    }
    
    // Converts hex colours e.g. 0xBFE0E0 to ImVec4
    static ImVec4 ConvertColourHexToVec4(ImU32 hexCol) {
        float s = 1.0f / 255.0f;
        return ImVec4(((hexCol >> 16) & 0xFF) * s, ((hexCol >> 8) & 0xFF) * s, ((hexCol >> 0) & 0xFF) * s, 1.0f);
    }
     // Moves the cursor from current position
    static void MoveCursorPosX(float x) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);
    }
    static void MoveCursorPosY(float y) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
    }
    static void MoveCursorPos(ImVec2 distance) {
        ImGui::SetCursorPos(ImGui::GetCursorPos() + distance);
    }
    // produces n dummy items at standard frame height
    static void DummyItemHeight(uint nItems = 1) {
        ImGui::Dummy(ImVec2(0.0f, ImGui::GetFrameHeight() * (float)nItems + ImGui::GetStyle().ItemSpacing.y));
    }
    // Returns the vertical centre of n items of standard height
    static float GetVerticalCentreOfItems(uint nItems) {
        return ((float)nItems * ImGui::GetFrameHeightWithSpacing() - (float)ImGui::GetStyle().ItemSpacing.y) / 2.0f;
    }
    // Sets the cursor y position such that an item of height 'itemHeight' is centred about 'nItems' of standard height
    static void CentreItemVertically(uint nItems, float thisItemHeight = ImGui::GetFrameHeight()) {
        ImGuiModules::MoveCursorPosY(ImGuiModules::GetVerticalCentreOfItems(nItems) - thisItemHeight / 2.0f);
    }
    // Sets the cursor y position such that an item of height 'itemHeight' is centred about an arbitrary vertical distance
    static void CentreItemVerticallyAboutItem(float itemHeight, float thisItemHeight = ImGui::GetFrameHeight()) {
        ImGuiModules::MoveCursorPosY((itemHeight - thisItemHeight) / 2.0f);
    }
    static void TextUnformattedCentredHorizontally(const char* text, float width = ImGui::GetWindowSize().x) {
        // calculate centre position from length of text
        ImGuiModules::MoveCursorPosX((width - ImGui::CalcTextSize(text).x) / 2.0f);
        // draw text
        ImGui::TextUnformatted(text);
    }
    static void TextUnformattedCentredHorizontallyInTable(const char* text) {
        TextUnformattedCentredHorizontally(text, ImGui::GetColumnWidth());
    }
    
    static void TextCentredHorizontally(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
            char buf[255];
            vsnprintf(buf, sizeof(buf), fmt, args);
            // calculate centre position from length of text
            ImGuiModules::TextUnformattedCentredHorizontally(buf);
        va_end(args);
    }
    static void TextCentredHorizontallyInTable(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
            char buf[255];
            vsnprintf(buf, sizeof(buf), fmt, args);
            // calculate centre position from length of text
            ImGuiModules::TextUnformattedCentredHorizontallyInTable(buf);
        va_end(args);
    }
    static bool RightClickedLastItem() {
        return (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right));
    }
    
    template<typename T>
    static bool ComboBox(const char* name, VectorSelectable<T>& data, std::function<std::string(T& item)> cb_GetItemString, ImGuiComboFlags flags = 0)
    {         
        bool isChanged = false;   
        std::string label = (data.Size() <= 0 || !data.HasItemSelected()) ? "" : cb_GetItemString(data.CurrentItem());
        
        if (ImGui::BeginCombo(name, label.c_str(), flags))
        {
            for (int n = 0; n < (int)data.Size(); n++)
            {
                // Get item name
                std::ostringstream stream;
                stream << cb_GetItemString(data.Item(n)) << "##" << (int)&data.Item(n); // using ptr as unique id
                
                const bool isSelected = (data.CurrentIndex() == n);
                if (ImGui::Selectable(stream.str().c_str(), isSelected)) {
                    data.SetCurrentIndex((int)n);
                    isChanged = true;
                }

                // Set the initial scrolling + keyboard navigation focus
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        return isChanged;
    } 
    
    // returns true if item is clicked
    template<typename T>
    static bool DraggableListBox(const char* listboxName, const ImVec2& dimensions, VectorSelectable<T>& data, std::function<std::string(T& item)> cb_GetItemString)
    {
        static int n_clicked = -1;
        static int n_current = -1;
        static float dragDistance_px = 0.0f;
        static int dragDistance_items = 0;        
        
        int arraySize = (int)data.Size();
        if(arraySize <= 0) return false;
        
        bool isClicked = false;
        if (ImGui::BeginListBox(listboxName, dimensions))
        {
            for (int n = 0; n < arraySize; n++)
            {
                // Get item name
                std::ostringstream stream;
                stream << cb_GetItemString(data.Item(n)) << "##" << (int)&data.Item(n); // using ptr as unique id
                const bool isSelected = (data.CurrentIndex() == n);
                // Draw item
                if (ImGui::Selectable(stream.str().c_str(), isSelected)) {
                    data.SetCurrentIndex(n);
                    isClicked = true;
                }
                // Set the initial scrolling + keyboard navigation focus
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
                // Set values when item is pressed
                if(ImGui::IsItemClicked()) {
                    n_clicked = n;
                    n_current = n;
                    // click position always start in the centre of the item
                    dragDistance_px = ImGui::GetIO().MousePos.y - ImGui::GetCursorScreenPos().y + ImGui::GetTextLineHeightWithSpacing() / 2;
                    ImGui::ResetMouseDragDelta();
                } // Reset values when item is unpressed
                else if(ImGui::IsMouseReleased(0) && (n == n_clicked)) {
                    n_clicked = -1;
                    n_current = -1;
                    dragDistance_px = 0.0f;
                }
            }
            ImGui::EndListBox();
        }
        
        auto updateDragDistanceValue = [&]() {
            dragDistance_px += ImGui::GetMouseDragDelta(0, 0.0f).y;
            // Calculate the number of items mouse has been dragged
            dragDistance_items = (int)round(dragDistance_px / ImGui::GetTextLineHeightWithSpacing());
            ImGui::ResetMouseDragDelta();
        };
        if(n_clicked > -1)
        {
            // Update distance if mouse is pressed and dragged up/down
            updateDragDistanceValue();
            // expected position - current position
            int n_difference = (n_clicked + dragDistance_items) - n_current;
            // move m_current towards n_next
            if(n_difference) 
            {
                int move_to = (n_difference < 0.0f) ? n_current-1.0f : n_current+1.0f;
                // check if out of bounds of array
                move_to = std::clamp(move_to, 0, arraySize-1);
                // set current to next
                data.ItemSwap(n_current, move_to);
                if(data.CurrentIndex() == n_current) {
                   data.SetCurrentIndex(move_to);
                }
                n_current = move_to;
                // Update distance if mouse is pressed and dragged up/down
                updateDragDistanceValue();
            }
        }
        return isClicked;
    }

    template <typename T>
    static int Incrementer(const char *id, const char *str, T increment, T &modifyValue, bool allowNegative = true) {
        int buttonPress = 0;
        ImGui::PushID(id);
        if (ImGui::SmallButton("-")) {
            modifyValue = (allowNegative || modifyValue - increment > 0) ? modifyValue - increment : 0;
            buttonPress = -1;
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(str);
        ImGui::SameLine();
        if (ImGui::SmallButton("+")) {
            modifyValue += increment;
            buttonPress = 1;
        }
        ImGui::PopID();
        return buttonPress;
    };


    // Disable all widgets when not connected to grbl
    static void BeginDisableWidgets(GRBLVals& grblVals) {
        if (!grblVals.isConnected) {
            ImGui::BeginDisabled();
        }
    }
    static void EndDisableWidgets(GRBLVals& grblVals) {
        if (!grblVals.isConnected) {
            ImGui::EndDisabled();
        }
    }

    // Helper to display a little (?) mark which shows a tooltip when hovered.
    static void HelpMarker(const char *desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    static bool HereButton(GRBLVals& grblVals, glm::vec3& p) 
    {
        bool isClicked = false;
        ImGui::SameLine();
        // use pointer as unique id
        ImGui::PushID(&p[0]);
            if(ImGui::SmallButton("Here")) {
                isClicked = true;
                p = grblVals.status.WPos;
            }
        ImGui::PopID();
        return isClicked;
    }

    /*
    static void RecentMessage()
    {
       std::optional<const std::string> log = Log::GetConsoleLogLast();
       if(!log)
       return;

       ImGuiViewportP* viewport = (GImGui)->Viewports[0];
       // initialise
       ImGui::SetNextWindowPos(ImVec2(10, viewport->Size.y - 50));

       ImGui::SetNextWindowSize(ImVec2(0,0), ImGuiCond_None);
       if (!ImGui::Begin("RecentMessage", NULL, general_window_flags | ImGuiWindowFlags_NoDecoration)) {
       ImGui::End();
       return;
       }
       ImGui::Text(log->c_str());

       ImGui::End();

    }*/

};

static inline std::ostream& operator<<(std::ostream& os, const ImVec2& p) { os << "(" << p.x << ", " << p.y << ")"; return os; }

void drawDockSpace(Settings& settings);
void drawFrames(GRBL& grbl, Settings& settings, float dt);
extern ImGuiWindowFlags general_window_flags;

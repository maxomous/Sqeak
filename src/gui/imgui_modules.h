#pragma once    
#include "../common.h"

// get height of line / frame
//std::cout << "LineHeight = " << ImGui::GetTextLineHeight() << "LineHeightWithSpacing = " << ImGui::GetTextLineHeightWithSpacing() << std::endl;
//std::cout << "FrameHeight = " << ImGui::GetFrameHeight() << "FrameHeightWithSpacing = " << ImGui::GetFrameHeightWithSpacing() << std::endl;

// ImVec2 ostream operator
static inline std::ostream& operator<<(std::ostream& os, const ImVec2& p) { os << "(" << p.x << ", " << p.y << ")"; return os; }

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
    
    // draws buttons for a selectable vector
    // returns true if an item was clicked
    template<typename T>
    static bool Buttons(VectorSelectable<T>& items, ImVec2 buttonSize, std::function<std::string(T& item)> cb_GetItemString) 
    {
        bool buttonClicked = false;
        // draw all of the active functions
        for (size_t i = 0; i < items.Size(); i++)  
        {
            bool isCurrentItem = ((int)i == items.CurrentIndex());
            // highlight button if active
            if(isCurrentItem) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
            // get label on button (adding pointer as id for imgui)
            std::string imGuiName = cb_GetItemString(items.Item(i)) + std::string("##");
            imGuiName += std::to_string((int)&items.Item(i));
            // draw active function button
            if(ImGui::Button(imGuiName.c_str(), buttonSize)) {
                items.SetCurrentIndex(i);
                buttonClicked = true;
            }
            if(isCurrentItem) ImGui::PopStyleColor();
        }
        return buttonClicked;
    }
    
    // draws combo box's for a selectable vector
    // returns true if an item was clicked
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
    static bool ListBox_Reorderable(const char* listboxName, const ImVec2& dimensions, VectorSelectable<T>& data, std::function<std::string(T& item)> cb_GetItemString)
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
 
    // returns true if current item has changed
    template<typename T>
    static bool TreeNodes(VectorSelectable<T>& items, bool& isActiveItemInitiallyOpen, std::function<std::string*(T& item)>& cb_GetItemStringPtr, std::function<void(T&)> cb_DrawItemImGui)
    {
        bool isActiveItemChanged = false;
        
        // draw tabs
        for (size_t n = 0; n < items.Size(); n++)
        {   
            /*
            // delete button
            std::string deleteName = "-##";
            deleteName += (int) &items[n]; // id
            if(ImGui::SmallButton(deleteName.c_str())) {
                items.Remove(n); 
                isActiveItemChanged = true;
            }
            
            ImGui::SameLine();
            */
            //ImGui::IsItemClicked(ImGuiMouseButton_Right) {
            
            //}
            
            // set active item to be open initially & inactive items to be closed
            if(items.CurrentIndex() == (int)n) { 
                if(isActiveItemInitiallyOpen) {
                    ImGui::SetNextItemOpen(true); 
                    isActiveItemInitiallyOpen = false;
                }
            } else { ImGui::SetNextItemOpen(false); }
            
            // tree node 
            std::string* name = cb_GetItemStringPtr(items[n]);
            if (ImGui::TreeNode(name->c_str())) {
                assert(items.HasItemSelected() && "No item selected in cb_DrawTabImGui");
                // draw the imgui callback for the specific tab
                cb_DrawItemImGui(items[n]);
                
                // set the active index to match the open tree node
                if(items.CurrentIndex() != (int)n) {
                    items.SetCurrentIndex(n);
                    isActiveItemChanged = true; 
                }
                ImGui::TreePop();
            }

        }
        
        return isActiveItemChanged;
    }
    
 
    // returns true if current item has changed
    template<typename T>
    static bool Tabs(VectorSelectable<T>& items, std::function<std::string(T& item)>& cb_GetItemString, std::function<T(void)>& cb_AddNewItem, std::function<void()> cb_DrawItemImGui)
    {
        bool isModified = false;
        if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown))  // or ImGuiTabBarFlags_FittingPolicyScroll
        {
            // New tab button
            if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
                items.Add(std::move(cb_AddNewItem())); // Add new tab
                isModified = true;
            }

            // draw tabs
            for (size_t n = 0; n < items.Size(); )
            {   
                bool isOpen = true;
                // bool hasBeenClosed = just switched to !isOpen;
                std::string name = cb_GetItemString(items[n]);
                if (ImGui::BeginTabItem(name.c_str(), &isOpen, ImGuiTabItemFlags_None)) {
                    // set the active index to match the open tab
                    if(items.CurrentIndex() != (int)n) {
                        items.SetCurrentIndex(n);
                        isModified = true;
                    }
                    // draw the imgui callback for the specific tab
                    cb_DrawItemImGui();
                    ImGui::EndTabItem();
                }
                // x has been pressed on tab, delete it
                if(!isOpen) { 
                    items.Remove(n); 
                    isModified = true;
                }
                else { n++; }
            }
            ImGui::EndTabBar();
        }
        return isModified;
    }
    
    static bool ImageButtonWithText(std::string name, ImVec2 buttonSize, ImageTexture& buttonImage, ImVec2 buttonImgSize, float imageYOffset, float textYOffset, ImFont* font)
    { 
        ImGui::BeginGroup();
            // get initial cursor position
            ImVec2 p0 = ImGui::GetCursorPos();
            // set small font for text on button
            ImGui::PushFont(font);
                // make text at bottom of button (values need to be normalised between 0-1)
                float yNormalised = textYOffset / (buttonSize.y - 2.0f*ImGui::GetStyle().FramePadding.y);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.5f, yNormalised });
                    // draw button
                    bool isClicked = ImGui::Button(name.c_str(), buttonSize);
                ImGui::PopStyleVar();
            ImGui::PopFont();
            // get cursor end position 
            ImVec2 p1 = ImGui::GetCursorPos();
            
            // position of image
            ImVec2 imagePosition = { (buttonSize.x / 2.0f) - (buttonImgSize.x / 2.0f),    imageYOffset + ImGui::GetStyle().FramePadding.y}; // *** * 2  ??
            // set cursor for image
            ImGui::SetCursorPos(p0 + imagePosition);
            // draw image
            ImGui::Image(buttonImage, buttonImgSize);
            
            // reset cursor position to after button
            ImGui::SetCursorPos(p1);
        
        ImGui::EndGroup();
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
    }

    // Helper to display a little (?) mark which shows a tooltip when hovered.
    static void ToolTip_IfItemHovered(const std::string& text) {
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(text.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    
    // Helper to display a little (?) mark which shows a tooltip when hovered.
    static void HelpMarker(const std::string& text) {
        ImGui::TextDisabled("(?)");
        ToolTip_IfItemHovered(text);
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
       if (!ImGui::Begin("RecentMessage", NULL, ImGuiCustomModules::ImGuiWindow::generalWindowFlags | ImGuiWindowFlags_NoDecoration)) {
       ImGui::End();
       return;
       }
       ImGui::Text(log->c_str());

       ImGui::End();

    }*/

};

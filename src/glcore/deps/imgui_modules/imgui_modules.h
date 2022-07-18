#pragma once    
#include <iostream> 
#include <sstream> 
#include <functional>  
#include <MaxLib/Vector.h>

#define IMGUI_DEFINE_MATH_OPERATORS 
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h" // for vector math overrides
#include "../imgui/imgui_stb_image.h"

// get height of line / frame
//std::cout << "LineHeight = " << ImGui::GetTextLineHeight() << "LineHeightWithSpacing = " << ImGui::GetTextLineHeightWithSpacing() << std::endl;
//std::cout << "FrameHeight = " << ImGui::GetFrameHeight() << "FrameHeightWithSpacing = " << ImGui::GetFrameHeightWithSpacing() << std::endl;

// ImVec2 ostream operator
inline std::ostream& operator<<(std::ostream& os, const ImVec2& p) { os << "(" << p.x << ", " << p.y << ")"; return os; }

namespace ImGuiModules
{
    using namespace MaxLib::Vector;
    
    class ImGuiWindow
    {
    public:        
        ImGuiWindow(std::string name, const ImVec2& position = ImVec2(0.0f, 0.0f), const ImVec2& size = ImVec2(0.0f, 0.0f));
        // returns false if closed
        bool Begin(ImGuiWindowFlags flags = ImGuiWindowFlags_None);
        void End();        
    private:
        std::string m_Name;
        ImVec2 m_Position;
        ImVec2 m_Size;
    };

    class ImGuiPopup
    {
    public:
        ImGuiPopup(const std::string& name);
        
        void Open();
        // returns true on close (next frame)
        bool Draw(std::function<void()> cb_ImGuiWidgets);
        
    private:
        std::string m_Name;
        bool m_IsOpen = false;
    };


    
    void KeepWindowInsideViewport();
    // Converts hex colours e.g. 0xBFE0E0 to ImVec4
    ImVec4 ConvertColourHexToVec4(ImU32 hexCol);
     // Moves the cursor from current position
    void MoveCursorPosX(float x);
    void MoveCursorPosY(float y);
    void MoveCursorPos(ImVec2 distance);
    // produces n dummy items at standard frame height
    void DummyItemHeight(uint nItems = 1);
    // Returns the vertical centre of n items of standard height
    float GetVerticalCentreOfItems(uint nItems);
    // Sets the cursor y position such that an item of height 'itemHeight' is centred about 'nItems' of standard height
    void CentreItemVertically(uint nItems, float thisItemHeight = ImGui::GetFrameHeight());
    // Sets the cursor y position such that an item of height 'itemHeight' is centred about an arbitrary vertical distance
    void CentreItemVerticallyAboutItem(float itemHeight, float thisItemHeight = ImGui::GetFrameHeight());
    void TextUnformattedCentredHorizontally(const char* text, float width = ImGui::GetWindowSize().x);
    void TextUnformattedCentredHorizontallyInTable(const char* text);
    void TextCentredHorizontally(const char* fmt, ...);
    void TextCentredHorizontallyInTable(const char* fmt, ...);
    bool WasLastItemRightClicked();
    
    
    
    bool ImageButtonWithText(std::string name, ImVec2 buttonSize, ImageTexture& buttonImage, ImVec2 buttonImgSize, float imageYOffset, float textYOffset, ImFont* font);



    // Helper to display a little (?) mark which shows a tooltip when hovered.
    void ToolTip_IfItemHovered(const std::string& text);
    
    // Helper to display a little (?) mark which shows a tooltip when hovered.
    void HelpMarker(const std::string& text);
   /*
    void RecentMessage()
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


    template <typename T>
    int Incrementer(const char *id, const char *str, T increment, T &modifyValue, bool allowNegative = true) {
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
// **********************************************************
// ************** SELECTABLE VECTOR WRAPPERS ****************

    // draws buttons for a selectable vector
    // returns true if an item was clicked
    template<typename T>
    bool Buttons(Vector_SelectablePtrs<T>& items, ImVec2 buttonSize, std::function<std::string(T& item)> cb_GetItemString) 
    {
        bool buttonClicked = false;
        // draw all of the active functions
        for (size_t i = 0; i < items.Size(); i++)  
        {
            bool isCurrentItem = ((int)i == items.CurrentIndex());
            // highlight button if active
            if(isCurrentItem) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
            // get label on button (adding pointer as id for imgui)
            std::string imGuiName = cb_GetItemString(items[i]) + std::string("##");
            imGuiName += std::to_string((int)&items[i]);
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
    bool ComboBox(const char* name, Vector_SelectablePtrs<T>& data, std::function<std::string(T& item)> cb_GetItemString, ImGuiComboFlags flags = 0)
    {         
        bool isChanged = false;   
        std::string label = (data.Size() <= 0 || !data.HasItemSelected()) ? "" : cb_GetItemString(data.CurrentItem());
        
        if (ImGui::BeginCombo(name, label.c_str(), flags))
        {
            for (int n = 0; n < (int)data.Size(); n++)
            {
                // Get item name
                std::ostringstream stream;
                stream << cb_GetItemString(data[n]) << "##" << (int)&data[n]; // using ptr as unique id
                
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
    bool ListBox_Reorderable(const char* listboxName, const ImVec2& dimensions, Vector_SelectablePtrs<T>& data, std::function<std::string(T& item)> cb_GetItemString)
    {
        int n_clicked = -1;
        int n_current = -1;
        float dragDistance_px = 0.0f;
        int dragDistance_items = 0;        
        
        int arraySize = (int)data.Size();
        if(arraySize <= 0) return false;
        
        bool isClicked = false;
        if (ImGui::BeginListBox(listboxName, dimensions))
        {
            for (int n = 0; n < arraySize; n++)
            {
                // Get item name
                std::ostringstream stream;
                stream << cb_GetItemString(data[n]) << "##" << (int)&data[n]; // using ptr as unique id
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
                data.SwapItems(n_current, move_to);
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
    bool TreeNodes(Vector_SelectablePtrs<T>& items, bool& isActiveItemInitiallyOpen, std::function<std::string*(T& item)>& cb_GetItemStringPtr, std::function<void(T&)> cb_DrawItemImGui)
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
    bool Tabs(Vector_SelectablePtrs<T>& items, std::function<std::string(T& item)>& cb_GetItemString, std::function<T(void)>& cb_AddNewItem, std::function<void()> cb_DrawItemImGui)
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
    
}
#pragma once    
#include <iostream> 
#include <sstream> 
#include <functional>  
#include <algorithm>  
#include <optional>  
#include <MaxLib/Vector.h>

#define IMGUI_DEFINE_MATH_OPERATORS 
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h" // for vector math overrides
#include "../imgui/imgui_stb_image.h"

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
        bool Draw(std::function<void()> cb_ImGuiWidgets, ImGuiWindowFlags flags = 0);
        // returns true on close (next frame)
        bool DrawModal(std::function<void()> cb_ImGuiWidgets, ImGuiWindowFlags flags = 0);
        
    private:
        std::string m_Name;
        bool m_IsOpen = false;
        // returns true on close (next frame)
        bool Draw(std::function<void()> cb_ImGuiWidgets, bool isPopupVisible);
    };

    
    void KeepWindowInsideViewport();
    // Converts hex colours e.g. 0xBFE0E0 to ImVec4
    ImVec4 ConvertColourHexToVec4(ImU32 hexCol);
    // probes the current ImGui widget and prints info to terminal 
    void ProbeWidget();
    
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
 
    // Image button with text parameters. See: ImageButtonWithText()
    struct ImageButtonStyle
    {
        struct Offsets { 
            ImVec2 text; // offsets the text
            ImVec2 image;// offsets the image
        };
        
        ImageButtonStyle(const std::string& Name, const ImVec2& ButtonSize, const ImVec2& ImageSize, ImFont* Font, const ImVec2& TextOffset, const ImVec2& ImageOffset)
            : name(Name), buttonSize(ButtonSize), imageSize(ImageSize), font(Font), textOffset(TextOffset), imageOffset(ImageOffset) {}
        ImageButtonStyle(const std::string& Name, const ImVec2& ButtonSize, const ImVec2& ImageSize, ImFont* Font, const Offsets& offsets)
            : ImageButtonStyle(Name, ButtonSize, ImageSize, Font, offsets.text, offsets.image) {}
        
        // Name is only used to identify, it is not text displayed
        std::string     name; 
        ImVec2          buttonSize;
        ImVec2          imageSize;
        ImFont*         font;
        // Offsets are between 0 to 1:  Left/Top(0)  Centre (0.5)  Right/Bottom(1)
        ImVec2          textOffset;
        ImVec2          imageOffset;
    };
    // Image Button with text
    bool ImageButtonWithText(const std::string& text, ImageTexture image, const ImVec2& buttonSize, const ImVec2& imageSize, ImFont* font, const ImVec2& textOffset, const ImVec2& imageOffset, bool isActive = false, ImageTexture hoveredImage = {});
    bool ImageButtonWithText(const std::string& text, ImageTexture image, ImageButtonStyle* imageButton, bool isActive = false, ImageTexture hoveredImage = {});
    bool ImageButton(ImageTexture image, ImageButtonStyle* imageButton, bool isActive = false, ImageTexture hoveredImage = {});

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
    // V can be Vector_SelectablePtrs
    template<typename T, template <typename> class V>
    bool Buttons(V<T>& items, ImVec2 buttonSize, std::function<std::string(T& item)> cb_GetItemString) 
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
    template<typename T, template <typename> class V>
    bool ComboBox(const char* name, V<T>& data, std::function<std::string(T& item)> cb_GetItemString, const std::string& labelOverride = "", ImGuiComboFlags flags = 0)
    {         
        bool isChanged = false;  
        
        std::string label; 
        if(labelOverride == "") { // Use selected item
            label = (data.Size() <= 0 || !data.HasItemSelected()) ? "" : cb_GetItemString(data.CurrentItem());
        } else { // override label
            label = labelOverride;
        }
         
        
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
    template<typename T, template <typename> class V>
    bool ListBox_Reorderable(const char* listboxName, const ImVec2& dimensions, V<T>& data, std::function<std::string(T& item)> cb_GetItemString)
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
    template<typename T, template <typename> class V>
    bool TreeNodes(V<T>& items, bool& openCurrentItem, std::function<std::string(T&)>& cb_GetItemString, std::function<void(T&)> cb_DrawItemImGui)
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
            if(items.CurrentIndex() != (int)n) { 
                ImGui::SetNextItemOpen(false); 
            } else if(openCurrentItem) {
                ImGui::SetNextItemOpen(true); 
                openCurrentItem = false;
            }
            
            // tree node 
            const std::string& name = cb_GetItemString(items[n]);
            if (ImGui::TreeNode(name.c_str())) {
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
    template<typename T, template <typename> class V>
    bool Tabs(V<T>& items, std::function<std::string(T& item)>& cb_GetItemString, std::function<T(void)>& cb_AddNewItem, std::function<void()> cb_DrawItemImGui)
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

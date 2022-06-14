#pragma once    
#include "../common.h"

struct ImGuiCustomModules
{    
    class ImGuiWindow
    {
    public:
        static const ImGuiWindowFlags generalWindowFlags = ImGuiWindowFlags_AlwaysAutoResize;
        
        ImGuiWindow(Settings& settings, std::string name, const ImVec2& defaultSize = ImVec2(0.0f, 0.0f));
        bool Begin(Settings& settings, ImGuiWindowFlags flags = generalWindowFlags);
        void End();
        // Standard styling for widgets (called from Begin() / End())
        static void PushWidgetStyle(Settings& settings);
        static void PopWidgetStyle();
        
    private:
        std::string m_Name;
        ImVec2 m_Size;
        ImVec2 m_Pos;
    };

    class ImGuiPopup
    {
    public:
        ImGuiPopup(const std::string& name) : m_Name(name) {}
        
        void Open() { ImGui::OpenPopup(m_Name.c_str()); }
        // returns true on close (next frame)
        bool Draw(std::function<void()> cb_ImGuiWidgets) 
        {
            if (ImGui::BeginPopup(m_Name.c_str())) {
                // callback
                cb_ImGuiWidgets();
                ImGui::EndPopup();
                m_IsOpen = true;
            } else { // if popup has just been closed
                if (m_IsOpen == true) {
                    m_IsOpen = false;
                    return true;
                }
            }
            return false;
        }
    private:
        std::string m_Name;
        bool m_IsOpen = false;
    };



    static void Text(std::string label, std::optional<glm::vec2> position)
    {
        if(position) { 
            ImGui::Text("%s: (%g, %g)", label.c_str(), position->x, position->y);
        } else {
            ImGui::Text("%s: (N/A)", label.c_str());
        }
    }

    // Disable all widgets when not connected to grbl
    static void BeginDisableWidgets(GRBLVals& grblVals);
    static void EndDisableWidgets(GRBLVals& grblVals);
    
    static bool HereButton(GRBLVals& grblVals, glm::vec3& p);
    static bool ImageButtonWithText_Function(Settings& settings, std::string name, ImageTexture& image, bool isActive = false, ButtonType buttonType = ButtonType::FunctionButton);
    // text heading
    static void Heading(Settings& settings, const std::string& text, float centreAboutWidth = 0.0f);
    static void HeadingInTable(Settings& settings, const std::string& text); 
    // text heading with edit button (for toolbar)
    static bool HeadingWithEdit(Settings& settings, const std::string& name);
    // edit button
    static bool EditButton(Settings& settings, const char* id);
};

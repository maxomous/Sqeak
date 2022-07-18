#pragma once    
#include "../common.h"

namespace Sqeak { 
    
struct ImGuiCustomModules
{    
    class ImGuiWindow : public ImGuiModules::ImGuiWindow
    {
    public:
        static const ImGuiWindowFlags generalWindowFlags = ImGuiWindowFlags_AlwaysAutoResize;
        
        ImGuiWindow(Settings& settings, std::string name, const ImVec2& position = ImVec2(0.0f, 0.0f), const ImVec2& size = ImVec2(0.0f, 0.0f));
        // returns false if closed 
        bool Begin(Settings& settings, ImGuiWindowFlags flags = generalWindowFlags);
        void End();
        // Standard styling for widgets (called from Begin() / End())
        static void PushWidgetStyle(Settings& settings);
        static void PopWidgetStyle();
        
    private:
        std::string m_Name;
        ImVec2 m_Pos;
        ImVec2 m_Size;
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

} // end namespace Sqeak
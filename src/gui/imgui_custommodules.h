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
    static bool HereButton(GRBLVals& grblVals, glm::vec3& p);
    
    // text heading
    static void Heading(Settings& settings, const std::string& text, float centreAboutWidth = 0.0f);
    static void HeadingInTable(Settings& settings, const std::string& text); 
    // text heading with edit button (for toolbar)
    static bool HeadingWithEdit(Settings& settings, const std::string& name);
    // edit button
    static bool EditButton(Settings& settings, const char* id);
    
    
    // same as ImGuiModules::ImageButtonWithText() but centres about frame 
    static bool ImageButtonWithText_CentredVertically(const std::string& text, ImageTexture& image, ImGuiModules::ImageButtonStyle* imageButton, bool isActive, float frameHeight)
    { 
        // Centre widget about toolbarItemHeight
        ImGui::BeginGroup();
            ImGuiModules::CentreItemVerticallyAboutItem(frameHeight, imageButton->buttonSize.y);
            bool isClicked = ImGuiModules::ImageButtonWithText(text, image, imageButton->buttonSize, imageButton->imageSize, imageButton->font, imageButton->textOffset, imageButton->imageOffset, isActive);
        ImGui::EndGroup();
        return isClicked;
    }

};

} // end namespace Sqeak

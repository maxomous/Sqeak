#include "imgui_custommodules.h"
using namespace std; 

namespace Sqeak { 
    
// default position is under toolbar
ImGuiCustomModules::ImGuiWindow::ImGuiWindow(Settings& settings, std::string name, const ImVec2& position, const ImVec2& size) 
    : ImGuiModules::ImGuiWindow(name, (position.x == 0 && position.y == 0) ? settings.guiSettings.FramePosition_UnderToolbar() : position, size)
{}

bool ImGuiCustomModules::ImGuiWindow::Begin(Settings& settings, ImGuiWindowFlags flags)
{
    // Begin ImGui Window
    bool isOpen = ImGuiModules::ImGuiWindow::Begin(flags);
    // Keep inside Viewport
    ImGuiModules::KeepWindowInsideViewport();
    // Generic styling for widgets 
    PushWidgetStyle(settings);
    
    return isOpen;
}

void ImGuiCustomModules::ImGuiWindow::End() 
{ 
    ImGuiModules::ImGuiWindow::End();
}

void ImGuiCustomModules::ImGuiWindow::PushWidgetStyle(Settings& settings) 
{
    ImGui::PushItemWidth(settings.guiSettings.widgetWidth);
}

void ImGuiCustomModules::ImGuiWindow::PopWidgetStyle()
{
    ImGui::PopItemWidth();
}



bool ImGuiCustomModules::HereButton(GRBLVals& grblVals, glm::vec3& p) 
{
    bool isClicked = false;
    ImGui::SameLine();
    // use pointer as unique id
    ImGui::PushID(&(p.x));
        if(ImGui::SmallButton("Here")) {
            isClicked = true;
            p = { grblVals.status.WPos.x, grblVals.status.WPos.y, grblVals.status.WPos.z };
        }
    ImGui::PopID();
    return isClicked;
}

        
    

void ImGuiCustomModules::Heading(Settings& settings, const std::string& text, float centreAboutWidth) 
{        
    ImGui::PushFont(settings.guiSettings.font_small);
        ImGui::PushStyleColor(ImGuiCol_Text, settings.guiSettings.colour[Colour::HeaderText]);
            if(centreAboutWidth) {
                ImGuiModules::TextUnformattedCentredHorizontally(text.c_str(), centreAboutWidth);
            } else {
                ImGui::TextUnformatted(text.c_str());
            }
        ImGui::PopStyleColor();
    ImGui::PopFont();
}

void ImGuiCustomModules::HeadingInTable(Settings& settings, const std::string& text) 
{        
    ImGui::PushFont(settings.guiSettings.font_small);
        ImGui::PushStyleColor(ImGuiCol_Text, settings.guiSettings.colour[Colour::HeaderText]);
                ImGuiModules::TextUnformattedCentredHorizontallyInTable(text.c_str());
        ImGui::PopStyleColor();
    ImGui::PopFont();
}

bool ImGuiCustomModules::HeadingWithEdit(Settings& settings, const std::string& name) 
{
    ImGuiCustomModules::Heading(settings, name);
    ImGui::SameLine();
    return EditButton(settings, name.c_str());
}

bool ImGuiCustomModules::EditButton(Settings& settings, const char* id) 
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f,0.0f,0.0f,0.0f));
        ImGui::PushID(id);
        
            bool clicked = ImGuiModules::ImageButtonWithText(string(id) + "##Edit", settings.guiSettings.img_Edit, settings.guiSettings.imageButton_Toolbar_Edit);   
        ImGui::PopID();
    ImGui::PopStyleColor();
    return clicked;
}


} // end namespace Sqeak

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


void ImGuiCustomModules::BeginDisableWidgets(GRBLVals& grblVals) 
{
    if (!grblVals.isConnected) {
        ImGui::BeginDisabled();
    }
}
void ImGuiCustomModules::EndDisableWidgets(GRBLVals& grblVals) 
{
    if (!grblVals.isConnected) {
        ImGui::EndDisabled();
    }
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
bool ImGuiCustomModules::ImageButtonWithText_Function(Settings& settings, std::string name,ImageTexture& image, bool isActive, ButtonType buttonType) 
{
    ImVec2& buttonSize = settings.guiSettings.button[(size_t)buttonType].Size;
    ImVec2& buttonImgSize = settings.guiSettings.button[(size_t)buttonType].ImageSize;
    if(isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
    bool clicked = ImGuiModules::ImageButtonWithText(name, buttonSize, image, buttonImgSize, settings.guiSettings.functionButtonImageOffset, settings.guiSettings.functionButtonTextOffset, settings.guiSettings.font_small);
    if(isActive) ImGui::PopStyleColor();
    return clicked;
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
    ImVec2& buttonImgSize = settings.guiSettings.button[ButtonType::Edit].ImageSize;
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f,0.0f,0.0f,0.0f));
        ImGui::PushID(id);
            bool clicked = ImGui::ImageButton(settings.guiSettings.img_Edit, buttonImgSize);    
        ImGui::PopID();
    ImGui::PopStyleColor();
    return clicked;
}


} // end namespace Sqeak

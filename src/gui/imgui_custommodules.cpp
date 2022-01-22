#include "imgui_custommodules.h"
using namespace std; 

ImGuiCustomModules::ImGuiWindow::ImGuiWindow(Settings& settings, std::string name, const ImVec2& defaultSize) 
    : m_Name(name), m_Size(defaultSize) 
{
    float padding = settings.guiSettings.dockPadding;
    // set default position to under toolbar
    m_Pos = ImVec2(padding, settings.guiSettings.toolbarHeight + padding * 2.0f);
    // set default position to bottom of screen: 
    //m_Pos = ImVec2(padding, ImGui::GetMainViewport()->WorkSize.y - windowSize.y - padding);
}

bool ImGuiCustomModules::ImGuiWindow::Begin()
{
    // set default size / position
    ImGui::SetNextWindowSize(m_Size, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(m_Pos, ImGuiCond_Appearing);
    
    if (!ImGui::Begin(m_Name.c_str(), NULL, ImGuiWindowFlags_None)) {
        // window closed
        return false;
    }
    // update size / position if user changed
    m_Size = ImGui::GetWindowSize();
    m_Pos = ImGui::GetWindowPos();
    return true;
}

void ImGuiCustomModules::ImGuiWindow::End() 
{ 
    ImGui::End(); 
}


void ImGuiCustomModules::BeginDisableWidgets(GRBLVals& grblVals) {
    if (!grblVals.isConnected) {
        ImGui::BeginDisabled();
    }
}
void ImGuiCustomModules::EndDisableWidgets(GRBLVals& grblVals) {
    if (!grblVals.isConnected) {
        ImGui::EndDisabled();
    }
}
bool ImGuiCustomModules::HereButton(GRBLVals& grblVals, glm::vec3& p) 
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
bool ImGuiCustomModules::ImageButtonWithText_Function(Settings& settings, std::string name,ImageTexture& image, bool isActive, ButtonType buttonType) 
{
    ImVec2& buttonSize = settings.guiSettings.button[(size_t)buttonType].Size;
    ImVec2& buttonImgSize = settings.guiSettings.button[(size_t)buttonType].ImageSize;
    if(isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
    bool clicked = ImGuiModules::ImageButtonWithText(name, buttonSize, image, buttonImgSize, settings.guiSettings.functionButtonImageOffset, settings.guiSettings.functionButtonTextOffset, settings.guiSettings.font_small);
    if(isActive) ImGui::PopStyleColor();
    return clicked;
}


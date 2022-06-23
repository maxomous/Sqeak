#include <iostream>
#include "toolsettings.h"
using namespace std;

namespace Sqeak { 
    
bool ToolSettings::Draw(Settings& settings) 
{ 
    auto& tools = settings.p.tools; 
  
    bool isModified = false;
    
    ImGui::BeginGroup();
        ImGui::PushItemWidth(settings.guiSettings.toolbarComboBoxWidth);
            static std::function<std::string(ParametersList::Tools::Tool& item)> cb_GetToolName = [](ParametersList::Tools::Tool& item) { return item.Name; };
            isModified |= ImGuiModules::ComboBox("Tool", tools.toolList, cb_GetToolName);
            
            if(tools.toolList.HasItemSelected()) {
                static std::function<std::string(ParametersList::Tools::Tool::ToolData& item)> cb_GetMaterialName = [](ParametersList::Tools::Tool::ToolData& item) { return item.material; };
                isModified |= ImGuiModules::ComboBox("Material", settings.p.tools.toolList.CurrentItem().Data, cb_GetMaterialName);
            } else {
                static int dummyCombo = 0;
                ImGui::Combo("Material", &dummyCombo, "\0");
            }
        ImGui::PopItemWidth();
    ImGui::EndGroup();
    // calls Export GCode and updates viewer
    return isModified;
}   

bool ToolSettings::DrawPopup_Tools(Settings& settings) 
{
    bool isModified = false;
 
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    static bool popupOpen = false;
    if (ImGui::BeginPopupModal("Edit Tools", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        popupOpen = true;
        ImGui::TextUnformatted("Select Tool");
        ImGui::BeginGroup();
        
            ImGui::BeginGroup(); 
                if(Draw_SelectTool(settings)) {
                    ImGui::EndGroup(); ImGui::EndGroup(); ImGui::EndPopup(); return true; // to prevent reading an element which doesnt exist anymore
                }
            ImGui::EndGroup();
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xSpacer);
            
            ImGui::BeginGroup();
                Draw_ToolData(settings);
            ImGui::EndGroup();
            
        ImGui::EndGroup();
         
        if(settings.p.tools.toolList.HasItemSelected()) 
        {
            ImGui::Separator();
        
            ImGui::TextUnformatted("Select Material");
            ImGui::BeginGroup();
                 
                ImGui::BeginGroup();
                    if(Draw_SelectMaterial(settings)) {
                        ImGui::EndGroup(); ImGui::EndGroup(); ImGui::EndPopup(); return true; // to prevent reading an element which doesnt exist anymore
                    }
                ImGui::EndGroup();
                
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xSpacer);
                  
                ImGui::BeginGroup();
                    Draw_MaterialData(settings);
                ImGui::EndGroup(); 
                
            ImGui::EndGroup();
        }
        
        ImGui::Separator();
            
        ImGui::SetItemDefaultFocus();
        float buttonWidth = 120.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) / 2.0f);
        if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) { 
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::EndPopup();
    } else { // if popup has just been closed
        if (popupOpen == true) {
            isModified = true;
            popupOpen = false;
        }
    }
    return isModified;
}


      
int ToolSettings::Draw_SelectTool(Settings& settings)
{
    auto& tools = settings.p.tools;  
    bool isToolSelected = tools.toolList.HasItemSelected();
     
    static std::function<std::string(ParametersList::Tools::Tool& item)> cb_GetToolName = [](ParametersList::Tools::Tool& item) { return item.Name; };
    ImGuiModules::ListBox_Reorderable("##Tools", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), tools.toolList, cb_GetToolName);
    
    if(ImGui::Button("+##AddTool")) {
        tools.toolList.Add("Tool 1");
        auto& lastTool = tools.toolList[tools.toolList.Size()-1];
        lastTool.Data.Add();
        return -1;
    }
    ImGui::SameLine();
    // grey out if no item selected
    if(!isToolSelected) {
        ImGui::BeginDisabled();
    } 
    if(ImGui::Button("-##RemoveTool")) {
        if(isToolSelected) {
            tools.toolList.RemoveCurrent();
            return -1;
        }
    }
    if(!isToolSelected) {
        ImGui::EndDisabled();
    }
    return 0;
}

void ToolSettings::Draw_ToolData(Settings& settings)
{
    auto& tools = settings.p.tools;  
    bool isToolSelected = tools.toolList.HasItemSelected();
    // if tool selected
    if(isToolSelected) 
    {
        ParametersList::Tools::Tool& currentTool = settings.p.tools.toolList.CurrentItem();
        ImGui::InputText("Name", &currentTool.Name);
        ImGui::InputFloat("Cutter Diameter", &currentTool.Diameter, 0.1f, 1.0f, "%.2f"); 
        ImGui::InputFloat("Tool Stickout", &currentTool.Length, 0.1f, 1.0f, "%.2f"); 
    }
}
 

int ToolSettings::Draw_SelectMaterial(Settings& settings)
{
    auto& tools = settings.p.tools;  
    bool isToolSelected = tools.toolList.HasItemSelected();
    bool isMaterialSelected = tools.toolList.CurrentItem().Data.HasItemSelected();  
    
    if(isToolSelected) 
    { 
        ParametersList::Tools::Tool& currentTool = settings.p.tools.toolList.CurrentItem();
            
        static std::function<std::string(ParametersList::Tools::Tool::ToolData& item)> cb_GetMaterialName = [](ParametersList::Tools::Tool::ToolData& item) { return item.material; };
        ImGuiModules::ListBox_Reorderable("##ToolData", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), currentTool.Data, cb_GetMaterialName);
        
        if(ImGui::Button("+##AddToolData")) { 
            currentTool.Data.Add(ParametersList::Tools::Tool::ToolData());
            return -1;
        }
        ImGui::SameLine();
        
        // grey out if no material selected
        if(!isMaterialSelected)
            ImGui::BeginDisabled();
            
        if(ImGui::Button("-##RemoveToolData")) {
            if(isMaterialSelected) {
                currentTool.Data.RemoveCurrent();
                return -1;
            }
        }
        
        if(!isMaterialSelected) 
            ImGui::EndDisabled();  
    }
    return 0;
}

void ToolSettings::Draw_MaterialData(Settings& settings)
{
    auto& tools = settings.p.tools;   
    bool isToolSelected = tools.toolList.HasItemSelected();
    bool isMaterialSelected = tools.toolList.CurrentItem().Data.HasItemSelected();  

    if(isToolSelected && isMaterialSelected) {
        ParametersList::Tools::Tool& currentTool = settings.p.tools.toolList.CurrentItem();
        auto& data = currentTool.Data.CurrentItem();
        
        ImGui::InputText("Material", &data.material); 
        ImGui::Separator();
        ImGui::InputFloat("Spindle Speed", &data.speed, 100.0f, 1000.0f, "%.0f"); 
        ImGui::Separator();
        ImGui::InputFloat("Cutting Feed Rate", &data.feedCutting, 100.0f, 1000.0f, "%.0f");
        ImGui::InputFloat("Plunge Feed Rate", &data.feedPlunge, 100.0f, 1000.0f, "%.0f");
        ImGui::Separator();
        ImGui::InputFloat("Depth of Cut", &data.cutDepth, 0.1f, 1.0f, "%.3f");
    }
}

} // end namespace Sqeak

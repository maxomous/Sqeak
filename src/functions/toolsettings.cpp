#include <iostream>
#include "toolsettings.h"
using namespace std;

void ToolSettings::Draw(Settings& settings) 
{ 
    auto& tools = settings.p.tools; 
    ImGui::BeginGroup();
        DrawEditTools(settings);
    ImGui::EndGroup();
    
    ImGui::SameLine();
    
    ImGui::BeginGroup();
        ImGui::PushItemWidth(settings.guiSettings.toolbarComboBoxWidth);
            static std::function<std::string(ParametersList::Tools::Tool& item)> cb_GetToolName = [](ParametersList::Tools::Tool& item) { return item.Name; };
            ImGuiModules::ComboBox("Select Tool", tools.toolList, cb_GetToolName);
            
            if(tools.toolList.HasItemSelected()) {
                static std::function<std::string(ParametersList::Tools::Tool::ToolData& item)> cb_GetMaterialName = [](ParametersList::Tools::Tool::ToolData& item) { return item.material; };
                ImGuiModules::ComboBox("Select Material", settings.p.tools.toolList.CurrentItem().Data, cb_GetMaterialName);
            } else {
                static int dummyCombo = 0;
                ImGui::Combo("Select Material", &dummyCombo, "\0");
            }
        ImGui::PopItemWidth();
    ImGui::EndGroup();
}   

void ToolSettings::DrawEditTools(Settings& settings) 
{
    ImVec2 buttonSize = settings.guiSettings.buttonSize[1];
    ImGuiModules::CentreItemVertically(2, buttonSize.y);
    
    if (ImGui::Button("Edit Tools", buttonSize))
        ImGui::OpenPopup("Edit Tools");
    
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Edit Tools", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Select Tool");
        ImGui::BeginGroup();
        
            ImGui::BeginGroup(); 
                if(Draw_SelectTool(settings)) {
                    ImGui::EndGroup(); ImGui::EndGroup(); ImGui::EndPopup(); return; // to prevent reading an element which doesnt exist anymore
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
                        ImGui::EndGroup(); ImGui::EndGroup(); ImGui::EndPopup(); return; // to prevent reading an element which doesnt exist anymore
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
        
        Draw_TabParameters(settings) ;
        
        ImGui::Separator();
            
        ImGui::SetItemDefaultFocus();
        float buttonWidth = 120.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) / 2.0f);
        if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) { 
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::EndPopup();
    }
}


      
int ToolSettings::Draw_SelectTool(Settings& settings)
{
    auto& tools = settings.p.tools;  
    bool isToolSelected = tools.toolList.HasItemSelected();
     
    static std::function<std::string(ParametersList::Tools::Tool& item)> cb_GetToolName = [](ParametersList::Tools::Tool& item) { return item.Name; };
    ImGuiModules::DraggableListBox("##Tools", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), tools.toolList, cb_GetToolName);
    
    if(ImGui::Button("+##AddTool")) {
        tools.toolList.Add(ParametersList::Tools::Tool("Tool 1"));
        auto& lastTool = tools.toolList.Item(tools.toolList.Size()-1);
        lastTool.Data.Add(ParametersList::Tools::Tool::ToolData());
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
        ImGui::InputFloat("Tool Length", &currentTool.Length, 0.1f, 1.0f, "%.2f"); 
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
        ImGuiModules::DraggableListBox("##ToolData", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), currentTool.Data, cb_GetMaterialName);
        
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
        ImGui::InputFloat("Width of Cut", &data.cutWidth, 0.1f, 1.0f, "%.3f");
    }
}

void ToolSettings::Draw_TabParameters(Settings& settings) 
{
    ParametersList::PathCutter& pathCutter = settings.p.pathCutter;
    
    ImGui::Checkbox("Cut Tabs", &pathCutter.CutTabs);
        
    ImGui::Indent();
        if(pathCutter.CutTabs) {
            ImGui::InputFloat("Tab Spacing", &pathCutter.TabSpacing);
            ImGui::InputFloat("Tab Height",  &pathCutter.TabHeight);
            ImGui::InputFloat("Tab Width",   &pathCutter.TabWidth);
        }
    ImGui::Unindent();
}

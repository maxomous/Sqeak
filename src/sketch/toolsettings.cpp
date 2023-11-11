#include <iostream>
#include "toolsettings.h"
using namespace std;

namespace Sqeak { 
    
    
// Specific Tool methods

bool ToolSettings::Tools::IsToolAndMaterialSelected() 
{        
    if(!toolList.HasItemSelected()) {
        Log::Error("No Tool Selected");
        return false;
    }
    if(!toolList.CurrentItem().data.HasItemSelected()) {
        Log::Error("No Material Selected");
        return false; 
    }
    return true;
}  

Vec3 ToolSettings::Tools::GetToolScale()
{
    if(toolList.HasItemSelected()) {
        return toolList.CurrentItem().Dimensions();
    } // else get default tool dimensions
    return Tools::Tool("default").Dimensions();
}   

// General Tool methods
// Draw Imgui Dropdown
bool ToolSettings::DrawTool(const std::string& label)  
{     
    static std::function<std::string(Tools::Tool& item)> cb_GetToolName = [](Tools::Tool& item) { return item.name; };
    return ImGuiModules::ComboBox("Tool", tools.toolList, cb_GetToolName, label);
}   

bool ToolSettings::DrawMaterial(const std::string& label) 
{     
    // if no tool selected, draw dummy combo box
    if(!tools.toolList.HasItemSelected()) {
        static int dummyCombo = 0; 
        ImGui::Combo("Material", &dummyCombo, "\0");
        return false;
    } 
    // Draw Material 
    static std::function<std::string(Tools::Tool::ToolData& item)> cb_GetMaterialName = [](Tools::Tool::ToolData& item) { return item.material; };
    return ImGuiModules::ComboBox("Material", tools.toolList.CurrentItem().data, cb_GetMaterialName, label);
}   


bool ToolSettings::DrawToolSelector(float frameHeight, float frameWidth, const std::string& toolLabel, const std::string& materialLabel)
{
    // centre the 2 dropdown boxes about the main buttons
    ImGui::BeginGroup();
        //ImGuiModules::MoveCursorPosY((itemHeight - thisItemHeight) / 2.0f);
        if (frameHeight > 0) { ImGuiModules::CentreItemVerticallyAboutItem(frameHeight, ImGui::GetFrameHeight() * 2.0f + ImGui::GetStyle().ItemSpacing.y); }
        // Set width
        if (frameHeight > 0) { ImGui::SetNextItemWidth(frameWidth); }
        // Draw tool
        bool update = DrawTool(toolLabel);
        // Set width
        if (frameHeight > 0) { ImGui::SetNextItemWidth(frameWidth); }
        // Draw Material
        update |= DrawMaterial(materialLabel);
    ImGui::EndGroup();
    return update;
}

bool ToolSettings::DrawPopup() 
{
    float itemWidth = 200.0f;

    bool needsUpdate = false;
    
    ImGui::TextUnformatted("Select Tool");
    ImGui::BeginGroup();
        // Tool selector
        ImGui::BeginGroup(); 
            needsUpdate |= Draw_SelectTool();
        ImGui::EndGroup();
        // Tool Data
        if(!needsUpdate) { 
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xSpacer); 
            
            ImGui::BeginGroup();
                ImGui::PushItemWidth(itemWidth);
                    Draw_ToolData();
                ImGui::PopItemWidth(); 
            ImGui::EndGroup();
         }
         
    ImGui::EndGroup();
     
    // return early if no tool selected
    if(needsUpdate) { return true; }
    
    if(tools.toolList.HasItemSelected()) 
    {
        ImGui::Separator();
    
        ImGui::TextUnformatted("Select Material");
        ImGui::BeginGroup();
            // Material selector
            ImGui::BeginGroup();
                needsUpdate |= Draw_SelectMaterial();
            ImGui::EndGroup();
            // Material Data
            if(!needsUpdate) { 
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xSpacer);
                  
                ImGui::BeginGroup();
                    ImGui::PushItemWidth(itemWidth);
                        Draw_MaterialData();
                    ImGui::PopItemWidth(); 
                ImGui::EndGroup(); 
            }
            
        ImGui::EndGroup();
    }
    
    ImGui::Separator();
        
    ImGui::SetItemDefaultFocus();
    float buttonWidth = 100.0f;
    float buttonHeight = 32.0f;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) / 2.0f);
    ImGuiModules::MoveCursorPosY(12.0f);
    
    if(ImGui::Button("Close", ImVec2(buttonWidth, buttonHeight))) {
        needsUpdate = true;
        ImGui::CloseCurrentPopup();        
    }
    return needsUpdate;
}

// private members

      
bool ToolSettings::Draw_SelectTool() 
{ 
    bool isToolSelected = tools.toolList.HasItemSelected();
    bool needsUpdate = false;
    
    static std::function<std::string(Tools::Tool& item)> cb_GetToolName = [](Tools::Tool& item) { return item.name; };
    ImGuiModules::ListBox_Reorderable("##Tools", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), tools.toolList, cb_GetToolName);
    
    if(ImGui::Button("+##AddTool")) {
        tools.toolList.Add("Tool 1");
        auto& lastTool = tools.toolList[tools.toolList.Size()-1];
        lastTool.data.Add();
        needsUpdate = true;
    }
    ImGui::SameLine();
    // grey out if no item selected
    if(!isToolSelected) {
        ImGui::BeginDisabled();
    } 
    if(ImGui::Button("-##RemoveTool")) {
        if(isToolSelected) {
            tools.toolList.RemoveCurrent();
            needsUpdate = true;
        }
    }
    if(!isToolSelected) {
        ImGui::EndDisabled();
    }
    return needsUpdate;
}

void ToolSettings::Draw_ToolData()
{ 
    bool isToolSelected = tools.toolList.HasItemSelected();
    // if tool selected
    if(isToolSelected) 
    {
        Tools::Tool& currentTool = tools.toolList.CurrentItem();
        ImGui::InputText("Name", &currentTool.name);
        ImGui::InputFloat("Cutter Diameter", &currentTool.diameter, 0.1f, 1.0f, "%.2f"); 
        ImGui::InputFloat("Tool Stickout", &currentTool.length, 0.1f, 1.0f, "%.2f"); 
    }
}
 

bool ToolSettings::Draw_SelectMaterial()
{
    bool isToolSelected = tools.toolList.HasItemSelected();
    bool isMaterialSelected = tools.toolList.CurrentItem().data.HasItemSelected();  
    int needsUpdate = false;
    if(isToolSelected) 
    { 
        Tools::Tool& currentTool = tools.toolList.CurrentItem();
            
        static std::function<std::string(Tools::Tool::ToolData& item)> cb_GetMaterialName = [](Tools::Tool::ToolData& item) { return item.material; };
        ImGuiModules::ListBox_Reorderable("##ToolData", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), currentTool.data, cb_GetMaterialName);
        
        if(ImGui::Button("+##AddToolData")) { 
            currentTool.data.Add(Tools::Tool::ToolData());
            needsUpdate = true;
        }
        ImGui::SameLine();
        
        // grey out if no material selected
        if(!isMaterialSelected)
            ImGui::BeginDisabled();
            
        if(ImGui::Button("-##RemoveToolData")) {
            if(isMaterialSelected) {
                currentTool.data.RemoveCurrent();
                needsUpdate = true;
            }
        }
        
        if(!isMaterialSelected) 
            ImGui::EndDisabled();  
    }
    return needsUpdate;
}

void ToolSettings::Draw_MaterialData()
{   
    bool isToolSelected = tools.toolList.HasItemSelected();
    bool isMaterialSelected = tools.toolList.CurrentItem().data.HasItemSelected();  

    if(isToolSelected && isMaterialSelected) {
        Tools::Tool& currentTool = tools.toolList.CurrentItem();
        auto& data = currentTool.data.CurrentItem();
        
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

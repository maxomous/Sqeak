#include <iostream>
#include "toolsettings.h"
using namespace std;

namespace Sqeak { 
    
    
// Specific Tool methods

bool ToolSettings::Tools::IsToolAndMaterialSelected() 
{        
    if(!toolList.HasItemSelected()) {
        Log::Error("No Tool Selected");
        return true;
    }
    if(!toolList.CurrentItem().Data.HasItemSelected()) {
        Log::Error("No Material Selected");
        return true; 
    }
    return false;
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
bool ToolSettings::DrawTool() 
{     
    static std::function<std::string(Tools::Tool& item)> cb_GetToolName = [](Tools::Tool& item) { return item.Name; };
    return ImGuiModules::ComboBox("Tool", tools.toolList, cb_GetToolName);
}   

bool ToolSettings::DrawMaterial() 
{     
    // if no tool selected, draw dummy combo box
    if(!tools.toolList.HasItemSelected()) {
        static int dummyCombo = 0;
        ImGui::Combo("Material", &dummyCombo, "\0");
        return false;
    } 
    // Draw Material 
    static std::function<std::string(Tools::Tool::ToolData& item)> cb_GetMaterialName = [](Tools::Tool::ToolData& item) { return item.material; };
    return ImGuiModules::ComboBox("Material", tools.toolList.CurrentItem().Data, cb_GetMaterialName);
}   


bool ToolSettings::DrawPopup() 
{
    bool needsClose = false;
    ImGui::TextUnformatted("Select Tool");
    ImGui::BeginGroup();
    
        ImGui::BeginGroup(); 
            needsClose |= Draw_SelectTool();
        ImGui::EndGroup();
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xSpacer); 
        
        ImGui::BeginGroup();
            if(!needsClose) { Draw_ToolData(); }
        ImGui::EndGroup();
        
    ImGui::EndGroup();
     
    // return early if no tool selected
    if(needsClose) { return true; }
    
    if(tools.toolList.HasItemSelected()) 
    {
        ImGui::Separator();
    
        ImGui::TextUnformatted("Select Material");
        ImGui::BeginGroup();
             
            ImGui::BeginGroup();
                needsClose |= Draw_SelectMaterial();
            ImGui::EndGroup();
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xSpacer);
              
            ImGui::BeginGroup();
                if(!needsClose) { Draw_MaterialData(); }
            ImGui::EndGroup(); 
            
        ImGui::EndGroup();
    }
    
    ImGui::Separator();
        
    ImGui::SetItemDefaultFocus();
    float buttonWidth = 120.0f;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) / 2.0f);
    if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) { 
        needsClose = true;
    }
    return needsClose;
}

// private members

      
int ToolSettings::Draw_SelectTool() 
{ 
    bool isToolSelected = tools.toolList.HasItemSelected();
     
    static std::function<std::string(Tools::Tool& item)> cb_GetToolName = [](Tools::Tool& item) { return item.Name; };
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

void ToolSettings::Draw_ToolData()
{ 
    bool isToolSelected = tools.toolList.HasItemSelected();
    // if tool selected
    if(isToolSelected) 
    {
        Tools::Tool& currentTool = tools.toolList.CurrentItem();
        ImGui::InputText("Name", &currentTool.Name);
        ImGui::InputFloat("Cutter Diameter", &currentTool.Diameter, 0.1f, 1.0f, "%.2f"); 
        ImGui::InputFloat("Tool Stickout", &currentTool.Length, 0.1f, 1.0f, "%.2f"); 
    }
}
 

int ToolSettings::Draw_SelectMaterial()
{
    bool isToolSelected = tools.toolList.HasItemSelected();
    bool isMaterialSelected = tools.toolList.CurrentItem().Data.HasItemSelected();  
    
    if(isToolSelected) 
    { 
        Tools::Tool& currentTool = tools.toolList.CurrentItem();
            
        static std::function<std::string(Tools::Tool::ToolData& item)> cb_GetMaterialName = [](Tools::Tool::ToolData& item) { return item.material; };
        ImGuiModules::ListBox_Reorderable("##ToolData", ImVec2(listWidth, listHeight * ImGui::GetTextLineHeightWithSpacing()), currentTool.Data, cb_GetMaterialName);
        
        if(ImGui::Button("+##AddToolData")) { 
            currentTool.Data.Add(Tools::Tool::ToolData());
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

void ToolSettings::Draw_MaterialData()
{   
    bool isToolSelected = tools.toolList.HasItemSelected();
    bool isMaterialSelected = tools.toolList.CurrentItem().Data.HasItemSelected();  

    if(isToolSelected && isMaterialSelected) {
        Tools::Tool& currentTool = tools.toolList.CurrentItem();
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

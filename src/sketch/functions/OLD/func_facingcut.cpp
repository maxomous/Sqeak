
#include <iostream>
#include "func_facingcut.h"
using namespace std;   

void FunctionType_FacingCut::DrawPopup(Settings& settings)
{
    ImGui::InputText("Name", &m_Name);
    
    ImGui::Dummy(ImVec2());
    
    bool isChanged = false;
    
    isChanged |= ImGui::Combo("Prioritise Axis", &m_Params.isYFirst, "X Axis\0Y Axis\0\0");
    isChanged |= ImGui::InputFloat3("Start", &m_Params.p0[0]);
    isChanged |= ImGuiModules::HereButton(settings.grblVals, m_Params.p0);
    isChanged |= ImGui::InputFloat3("End", &m_Params.p1[0]); 
    isChanged |= ImGuiModules::HereButton(settings.grblVals, m_Params.p1);
    
    // calls Export GCode and updates viewer
    if(isChanged) {
        Update3DView(settings);
    }
}

bool FunctionType_FacingCut::IsValidInputs(Settings& settings) 
{
    // check tool and material is selected
    if(!settings.p.tools.IsToolAndMaterialSelected())
        return false;
    // start and end point
    glm::vec3 pDif = m_Params.p1 - m_Params.p0;
    if(pDif.x == 0 || pDif.y == 0) {
        Log::Error("Invalid start and end points");
        return false;
    }
    // z top and bottom
    if(m_Params.p1.z > m_Params.p0.z) {
        Log::Error("Z Bottom must be below or equal to Z Top");
        return false;
    }
    // cut depth should have value
    ParametersList::Tools::Tool::ToolData& toolData = settings.p.tools.toolList.CurrentItem().Data.CurrentItem();
    if(toolData.cutWidth <= 0.0f) {
        Log::Error("Cut Width must be positive");
        return false;
    }
    if(toolData.cutDepth <= 0.0f) {
        Log::Error("Cut Depth must be positive");
        return false;
    }
    return true;
}        

std::string FunctionType_FacingCut::HeaderText(Settings& settings) 
{
    FacingCut_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();    
    
    // write header
    std::ostringstream stream;
    stream << "; Function: " << m_Name << '\n';
    stream << "; \tBetween: " << p.p0 << " and " << p.p1 << '\n';
    stream << "; Tool: " << tool.Name << '\n';
    stream << "; \tDiameter: " << tool.Diameter << '\n';
    stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
    stream << "; \tCut Width: " << toolData.cutWidth << '\n';
    stream << '\n';
    
    return stream.str();
}

std::pair<bool, std::vector<std::string>> FunctionType_FacingCut::ExportGCode(Settings& settings) 
{
    auto err = make_pair(false, std::vector<std::string>());;
    // error check
    if(!IsValidInputs(settings)) {
        return err;
    }
    
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();
    FacingCut_Parameters& p = m_Params;
    
    // initalise
    FunctionGCodes gcodes;
    gcodes.Add(HeaderText(settings));
    gcodes.InitCommands(toolData.speed);
    
    // move to initial x & y position
    gcodes.Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", p.p0.x, p.p0.y));
       
    int zDirection = ((p.p1.z - p.p0.z) > 0) ? FORWARD : BACKWARD;
    float zCurrent = p.p0.z;
    
    glm::vec2 p0 = { p.p0.x, p.p0.y };
    glm::vec2 p1 = { p.p1.x, p.p1.y };
    
    do {
        // move to next z
        gcodes.Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", zCurrent, toolData.feedPlunge));
        // cut plane
        gcodes.FacingCutXY(settings, p0, p1, p.isYFirst);
        if(zCurrent == p.p1.z) {
            break;
        }
        zCurrent += zDirection * toolData.cutDepth;
        
        if((zDirection == FORWARD && zCurrent > p.p1.z) || (zDirection == BACKWARD && zCurrent < p.p1.z)) {
            zCurrent = p.p1.z;
        }
    } while(1);

    // move to zPlane
    gcodes.EndCommands();
    
    return make_pair(true, gcodes.Get());
}


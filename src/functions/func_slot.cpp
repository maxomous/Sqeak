
#include <iostream>
#include "func_slot.h"
using namespace std;


void FunctionType_Slot::DrawPopup(Settings& settings)
{
    ImGui::InputText("Name", &m_Name);
    
    ImGui::Dummy(ImVec2());
    
    ImGui::InputFloat3("Start", &m_Params.p0[0]);
    ImGuiModules::HereButton(settings.grblVals, m_Params.p0);
    ImGui::InputFloat3("End", &m_Params.p1[0]); 
    ImGuiModules::HereButton(settings.grblVals, m_Params.p1);
    
}
    
std::pair<bool, std::vector<std::string>> FunctionType_Slot::ExportGCode(Settings& settings) {
    
    Slot_Parameters& p = m_Params;
    
    // error check
    if(p.p1 == p.p0) {
        Log::Error("Start and end points must be different");
        return make_pair(false, std::vector<std::string>());
    }
    
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();
    
    // error check
    if(toolData.cutDepth <= 0.0f) {
        Log::Error("Spacing must have value");
        return make_pair(false, std::vector<std::string>());
    }
    
    FunctionGCodes gcodes;
    // initalise
    
    std::ostringstream stream;
    stream << "; Function: " << m_Name << '\n';
    stream << "; \tBetween: " << p.p0 << " and " << p.p1 << '\n';
    stream << "; Tool: " << tool.Name << '\n';
    stream << "; \tDiameter: " << tool.Diameter << '\n';
    stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
    gcodes.Add(stream.str());
    
    gcodes.InitCommands(toolData.speed);
    // move to initial x & y position
    gcodes.Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", p.p0.x, p.p0.y));
    
    int zDirection = ((p.p1.z - p.p0.z) > 0) ? FORWARD : BACKWARD;
    float zCurrent = p.p0.z;
    glm::vec3 pNext = p.p0; 
        
    do {
        // move to next z
        gcodes.Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", zCurrent, toolData.feedPlunge));
        
        pNext = (pNext == p.p0) ? p.p1 : p.p0;
        // move to next xy point
        gcodes.Add(va_str("G1 X%.3f Y%.3f F%.0f", pNext.x, pNext.y, toolData.feedCutting));
        
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


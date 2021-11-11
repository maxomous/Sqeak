
#include <iostream>
#include "func_facingcut.h"
using namespace std;   

  
void FunctionType_FacingCut::DrawPopup(Settings& settings)
{
    ImGui::InputText("Name", &m_Name);
    
    ImGui::Dummy(ImVec2());
    
    ImGui::InputFloat3("Start", &m_Params.p0[0]);
    ImGuiModules::HereButton(settings.grblVals, m_Params.p0);
    ImGui::InputFloat3("End", &m_Params.p1[0]); 
    ImGuiModules::HereButton(settings.grblVals, m_Params.p1);
}
    
std::pair<bool, std::vector<std::string>> FunctionType_FacingCut::ExportGCode(Settings& settings) {
    
    FacingCut_Parameters& p = m_Params;
    
    glm::vec3 pDif = p.p1 - p.p0;
    
    if(pDif.x == 0 || pDif.y == 0) {
        Log::Error("Invalid start and end points");
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
    stream << "; \tCut Width: " << toolData.cutWidth << '\n';
    stream << '\n';
    gcodes.Add(stream.str());
    
    gcodes.InitCommands(toolData.speed);
    // move to initial x & y position
    gcodes.Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", p.p0.x, p.p0.y));
    
//if x first:
   
    int zDirection = ((p.p1.z - p.p0.z) > 0) ? FORWARD : BACKWARD;
    float zCurrent = p.p0.z;
    
    
    do {
        // move to next z
        gcodes.Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", zCurrent, toolData.feedPlunge));
        // cut plane
        FacingCutXY(settings, p, gcodes);
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

// executes length in one axis and then moves width of cutter in other axis
void FunctionType_FacingCut::FacingCutXY(Settings& settings, FacingCut_Parameters& p, FunctionGCodes& gcodes) 
{
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();
    
    //int xDirection = (pDif.x > 0) ? FORWARD : BACKWARD;
    int yDirection = ((p.p1.y - p.p0.y) > 0) ? FORWARD : BACKWARD;
    bool forward = true;
    
    glm::vec3 pNext = p.p0; 
    
    do {
        // x direction
        pNext.x = (forward) ? p.p1.x : p.p0.x;
        gcodes.Add(va_str("G1 X%.3f F%.0f", pNext.x, toolData.feedCutting));
        
        // y direction
        if(pNext.y == p.p1.y) {
            break;
        }
        
        pNext.y += yDirection * toolData.cutWidth;
        
        if((yDirection == FORWARD && pNext.y > p.p1.y) || (yDirection == BACKWARD && pNext.y < p.p1.y)) {
            pNext.y = p.p1.y;
        }
        gcodes.Add(va_str("G1 Y%.3f F%.0f", pNext.y, toolData.feedCutting));
    
        // 
        forward = !forward;
    } while(1);
    
}
 

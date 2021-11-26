
#include <iostream>
#include "func_draw.h"
using namespace std;
     
     
void FunctionType_Draw::DrawPopup(Settings& settings)
{
    ImGui::InputText("Name", &m_Name);
    
    ImGui::Dummy(ImVec2());
    
    bool isChanged = false;
    isChanged |= ImGui::Combo("Cut Side", &m_Params.cutSide, "None\0Left\0Right\0\0");
    isChanged |= ImGui::InputFloat2("Z Top/Bottom", &m_Params.z[0]);
    
    ImGui::Separator();
    
    for (size_t i = 0; i < m_Params.path.size(); i++) {
        glm::vec2& point = m_Params.path[i];
        isChanged |= ImGui::InputFloat2(va_str("Point %d##%d", i+1, (int)&point[0]).c_str(), &point[0]); // use pointer as hidden ImGui ID
    }

    if(ImGui::Button("+##AddPoint")) {
        m_Params.path.push_back({});
        isChanged = true;
    }
    ImGui::SameLine();
    if(ImGui::Button("-##RemovePoint")) {
        if(m_Params.path.size() >= 1) {
            m_Params.path.pop_back();
            isChanged = true;
        }
    }
    
    // calls Export GCode and updates viewer
    if(isChanged) {
        Update3DView(settings);
    }
}
        
bool FunctionType_Draw::IsValidInputs(Settings& settings) 
{
    // check tool and material is selected
    if(settings.p.tools.IsToolAndMaterialSelected())
        return false;
    // start and end point
    if(m_Params.path.size() < 2) {
        Log::Error("2 or more points required");
        return false;
    }
    // z top and bottom
    if(m_Params.z[1] > m_Params.z[0]) {
        Log::Error("Z Bottom must be below or equal to Z Top");
        return false;
    }
    // cut depth should have value
    ParametersList::Tools::Tool::ToolData& toolData = settings.p.tools.toolList.CurrentItem().Data.CurrentItem();
    if(toolData.cutDepth <= 0.0f) {
        Log::Error("Cut Depth must be positive");
        return false;
    }
    return true;
}   

std::string FunctionType_Draw::HeaderText(Settings& settings) 
{
    Draw_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();    
    
    // write header
    std::ostringstream stream;
    stream << "; Function: " << m_Name << '\n';
    stream << "; \tBetween: " << p.z[0] << " and " << p.z[1] << '\n';
    stream << "; \tPoints:" << '\n';
    for(glm::vec2 point : m_Params.path) {
        stream << "; \t\t " << point << '\n';
    }
    
    if(p.cutSide == CompensateCutter::None) stream << "; \tCompensate: None\n";
    if(p.cutSide == CompensateCutter::Left) stream << "; \tCompensate: Left\n";
    if(p.cutSide == CompensateCutter::Right) stream << "; \tCompensate: Right\n";
    
    stream << "; Tool: " << tool.Name << '\n';
    stream << "; \tDiameter: " << tool.Diameter << '\n';
    stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
    stream << '\n';
    
    return stream.str();
}
    
std::pair<bool, std::vector<std::string>> FunctionType_Draw::ExportGCode(Settings& settings) 
{
    auto err = make_pair(false, std::vector<std::string>());;
    // error check
    if(!IsValidInputs(settings)) {
        return err;
    }
    
    //Draw_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
    
    // initalise
    FunctionGCodes gcodes;
    gcodes.Add(HeaderText(settings));
    gcodes.InitCommands(toolData.speed);
    
    // define offset path parameters
    int compensateCutter = (m_Params.cutSide == CompensateCutter::None || m_Params.cutSide == CompensateCutter::Left) ? m_Params.cutSide : -1; /* if Right */ // 0 = no compensation, 1 = compensate left, -1 = compensate right
    float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
    // define cut path parameters & offset path
    Geos geos;
    FunctionsGeneral::CutPathParams pathParams;
    bool isLoop = m_Params.path[0] == m_Params.path.back();
    // calculate offset
    if(isLoop) {
        pathParams.points = geos.offsetPolygon(m_Params.path, compensateCutter * toolRadius, settings.p.pathCutter.QuadrantSegments);
    } else {
        pathParams.points = geos.offsetLine(m_Params.path, compensateCutter * toolRadius, settings.p.pathCutter.QuadrantSegments);
    }

    // populate parameters
    pathParams.z0 = m_Params.z[0];
    pathParams.z1 = m_Params.z[1];
    pathParams.cutDepth = toolData.cutDepth;
    pathParams.feedPlunge = toolData.feedPlunge;
    pathParams.feedCutting = toolData.feedCutting;
    pathParams.isLoop = isLoop;
    
    // add gcodes for path at depths
    if(FunctionsGeneral::CutPath(settings, gcodes, pathParams)) {
        return err;
    }
    // move to zPlane, end program
    gcodes.EndCommands();
    
    // draw path and offset path in viewer
    Event<Event_DisplayShapeOffset>::Dispatch( { m_Params.path, pathParams.points, pathParams.isLoop } );
    
    return make_pair(true, gcodes.Get());
}

    

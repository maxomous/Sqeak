
#include <iostream>
#include "func_square.h"
using namespace std;
     

void FunctionType_Square::DrawPopup(Settings& settings)
{
    ImGui::InputText("Name", &m_Name);
    
    ImGui::Dummy(ImVec2());
    
    bool isChanged = false;
    
    isChanged |= ImGui::Combo("Cut Side", &m_Params.cutSide, "None\0Inside\0Outside\0\0");
    isChanged |= ImGui::InputFloat3("Start", &m_Params.p0[0]);
    isChanged |= ImGuiModules::HereButton(settings.grblVals, m_Params.p0);
    isChanged |= ImGui::InputFloat3("End", &m_Params.p1[0]); 
    isChanged |= ImGuiModules::HereButton(settings.grblVals, m_Params.p1);
    
    // calls Export GCode and updates viewer
    if(isChanged) {
        Update3DView(settings);
    }
}
        
bool FunctionType_Square::IsValidInputs(Settings& settings) 
{
    // check tool and material is selected
    if(settings.p.tools.IsToolAndMaterialSelected())
        return false;
    // start and end point
    if(m_Params.p0.x == m_Params.p1.x || m_Params.p0.y == m_Params.p1.y) {
        Log::Error("Start and end points invalid");
        return false;
    }
    // z top and bottom
    if(m_Params.p1.z > m_Params.p0.z) {
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

std::string FunctionType_Square::HeaderText(Settings& settings) 
{
    Square_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();    
    
    // write header
    std::ostringstream stream;
    stream << "; Function: " << m_Name << '\n';
    stream << "; \tBetween: " << p.p0 << " and " << p.p1 << '\n';
    
    if(p.cutSide == CompensateCutter::None) stream << "; \tCompensate: None\n";
    if(p.cutSide == CompensateCutter::Left) stream << "; \tCompensate: Left\n";
    if(p.cutSide == CompensateCutter::Right) stream << "; \tCompensate: Right\n";
    
    stream << "; Tool: " << tool.Name << '\n';
    stream << "; \tDiameter: " << tool.Diameter << '\n';
    stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
    stream << '\n';
    
    return stream.str();
}
    
std::pair<bool, std::vector<std::string>> FunctionType_Square::ExportGCode(Settings& settings) 
{
    auto err = make_pair(false, std::vector<std::string>());;
    // error check
    if(!IsValidInputs(settings)) {
        return err;
    }
    
    //Square_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
    
    // initalise
    FunctionGCodes gcodes;
    gcodes.Add(HeaderText(settings));
    gcodes.InitCommands(toolData.speed);
    
    // define initial path
    std::vector<glm::vec2> path;    
    path.push_back({ m_Params.p0.x, m_Params.p0.y });
    path.push_back({ m_Params.p1.x, m_Params.p0.y });
    path.push_back({ m_Params.p1.x, m_Params.p1.y });
    path.push_back({ m_Params.p0.x, m_Params.p1.y });
    path.push_back({ m_Params.p0.x, m_Params.p0.y });
    
    // define offset path parameters
    int compensateCutter = (m_Params.cutSide == CompensateCutter::None || m_Params.cutSide == CompensateCutter::Left) ? m_Params.cutSide : -1; /* if Right */ // 0 = no compensation, 1 = compensate left, -1 = compensate right
    float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
    // define cut path parameters & offset path
    Geos geos;
    FunctionGCodes::CutPathParams pathParams;
    
    auto offset = geos.offsetPolygon(path, compensateCutter * toolRadius, settings.p.pathCutter.QuadrantSegments);
    pathParams.points = offset.second;
    pathParams.z0 =             m_Params.p0.z;
    pathParams.z1 =             m_Params.p1.z;
    pathParams.cutDepth =       toolData.cutDepth;
    pathParams.feedPlunge =     toolData.feedPlunge;
    pathParams.feedCutting =    toolData.feedCutting;
    pathParams.isLoop =         true;
    // add gcodes for path at depths
    if(gcodes.CutPath(settings, pathParams)) {
        return err;
    }
    // move to zPlane, end program
    gcodes.EndCommands();
    
    // draw path and offset path in viewer
    //Event<Event_DisplayShapeOffset>::Dispatch( { path, pathParams.points, pathParams.isLoop } );
    
    return make_pair(true, gcodes.Get());
}

    

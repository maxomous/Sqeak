/*
 * frames.cpp
 *  Max Peglar-Willis 2021
 */ 

#include "functions.h"

using namespace std;
using namespace MaxLib;


namespace Sqeak { 





    
int Function_CutPath::Parameters_CutPath::GetCutSide() 
{
    if(cutSide == (int)CompensateCutter::None)  { 
        return 0; 
    } else if(cutSide == (int)CompensateCutter::Right) { 
        return -1; 
    } else { return 1; } //(cutSide == CompensateCutter::Left || cutSide == CompensateCutter::Pocket) 
}

bool Function_CutPath::DrawWindow() 
{
    bool needsUpdate = false;

    // Select Geometry Button
    DrawSelectButton(&m_Selected_Points, &m_Selected_Elements, &m_Selected_Polygons);
    
    // Set Parameters
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Parameters")) 
    {     
        needsUpdate |= ImGui::InputText("Name", &m_Name);
        ImGui::Dummy(ImVec2());
        needsUpdate |= ImGui::InputFloat("Z Top", &m_Params.zTop);
        needsUpdate |= ImGui::InputFloat("Z Bottom", &m_Params.zBottom);
        needsUpdate |= ImGui::Combo("Cut Side", &m_Params.cutSide, "None\0Left\0Right\0Pocket\0\0");
        needsUpdate |= ImGui::InputFloat("Finishing Pass", &m_Params.finishPass);
        
        ImGui::TreePop();
    }
    
    if(ImGui::Button("Build GCode")) {
        // make an array of points to send to gcodebuilder
        LineString v;
        for(size_t i = 0; i < m_Selected_Elements.size(); i++) {
            // copy the points into an array
            v.push_back(m_Sketcher->Factory().GetPositionBySketchItem(m_Selected_Elements[i]));
        }
        
        GCode_SendToViewer(v);
        needsUpdate = true;
    }
    
    // Draw the selected Geometry
    DrawSelectedItems("Points##CutPath", m_Selected_Points);
    DrawSelectedItems("Elements##CutPath", m_Selected_Elements);
    DrawSelectedItems("Polygons##CutPath", m_Selected_Polygons);
    
    return needsUpdate;
}

std::string Function_CutPath::HeaderText() 
{
    Parameters_CutPath& p = m_Params;
    // write header
    std::ostringstream stream;
    stream << "; Function: " << m_Name << '\n';
    stream << "; \tBetween: " << p.zTop << " and " << p.zBottom << '\n';
    stream << "; \tPoints:" << '\n';
    
    // TODO add header text
    
   // m_Params.drawing.AddElementsHeaderText(stream);
    
    if(p.cutSide == Parameters_CutPath::CompensateCutter::None) stream << "; \tCompensate: None\n";
    if(p.cutSide == Parameters_CutPath::CompensateCutter::Left) stream << "; \tCompensate: Left\n";
    if(p.cutSide == Parameters_CutPath::CompensateCutter::Right) stream << "; \tCompensate: Right\n";
    if(p.cutSide == Parameters_CutPath::CompensateCutter::Pocket) stream << "; \tCompensate: Pocket\n";
    
    if(p.finishPass) stream << "; Finishing Pass: " << p.finishPass << '\n';
    
    // Add tool info
    HeaderText_AddToolInfo(stream);
    
    return stream.str();
}



 
bool Function_CutPath::IsValidInputs(const Geom::LineString& inputPath) 
{  
    // check tool and material is selected
    if(m_Settings->p.toolSettings.tools.IsToolAndMaterialSelected()) {
        Log::Error("Tool and Material must be selected");
        return false;
    }
    // start and end point
    if(inputPath.size() == 0) {
        Log::Error("Path is empty");
        return false;
    }
    
    bool isLoop = (inputPath.front() == inputPath.back());
    if(m_Params.cutSide == Parameters_CutPath::CompensateCutter::Pocket && !isLoop) {
        Log::Error("Pocket requires start and end points to be identical");
        return false;
    }
    // z top and bottom
    if(m_Params.zBottom > m_Params.zTop) {
        Log::Error("Z Bottom must be below or equal to Z Top");
        return false;
    }
    // cut depth should have value
    ToolSettings::Tools::Tool::ToolData& toolData = m_Settings->p.toolSettings.tools.toolList.CurrentItem().Data.CurrentItem();
    if(toolData.cutDepth <= 0.0f) {
        Log::Error("Cut Depth must be positive");
        return false;
    }
    
    return true;
}   


    
// return value is success
bool Function_CutPath::InterpretGCode(const Geom::LineString& inputPath, std::function<void(std::vector<std::string>)> cb)  
{
    // error check  
    if(!IsValidInputs(inputPath)) {
        return true;  
    }  
    ToolSettings::Tools::Tool& tool = m_Settings->p.toolSettings.tools.toolList.CurrentItem();
    ToolSettings::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
         
    // initialise 
    GCodeBuilder gcodes;
    // Add the GCode header text
    gcodes.Add(HeaderText());
    // Resets all the main modal commands + sets spindle speed 
    gcodes.InitCommands(toolData.speed);
    
    // Define offset path parameters:  0 = no compensation, 1 = compensate left / pocket, -1 = compensate right
    int cutSide = m_Params.GetCutSide();
    float toolRadius = fabsf(tool.Diameter / 2.0f);
    float pathOffset = toolRadius + fabsf(m_Params.finishPass);
    // define geos parameters
    GeosBufferParams& geosParameters = m_Settings->p.pathCutter.geosParameters;
    // calculate offset
    GCodeBuilder::CutPathParams pathParams;
    // populate parameters
    pathParams.zTop = m_Params.zTop;  
    pathParams.zBottom = m_Params.zBottom; 
    pathParams.cutDepth = toolData.cutDepth; 
    pathParams.feedPlunge = toolData.feedPlunge;  
    pathParams.feedCutting = toolData.feedCutting; 
     
    // vector for storing the final paths
    std::vector<Geom::LineString> path;
    std::vector<Geom::LineString> enclosingPath;
    bool isPocket = (m_Params.cutSide == Parameters_CutPath::CompensateCutter::Pocket);
     
    // Initialise geos (for geometry offseting)
    Geos geos;
    
    // Simple path
    if(m_Params.cutSide == Parameters_CutPath::CompensateCutter::None) {
        path.push_back(inputPath);
    } 
    // Compensate path by radius
    else {
        // make the inital offset   
        path = geos.Offset(inputPath, cutSide * pathOffset, geosParameters);
        // add pocket path 
        if(isPocket) { 
            // distance to offset per pass
            float boringOffset = 2.0f * toolRadius - m_Settings->p.pathCutter.CutOverlap; 
            // start a recursive loop of offset for boring, if cutting simple offset, this breaks loop after the first iteration
            Geos::RecursiveOffset recursiveOffset = geos.OffsetPolygon_Recursive(path, boringOffset, true , geosParameters); // true is reverse
            path = recursiveOffset.path;
            enclosingPath = recursiveOffset.enclosingPath;
        }

    }
    
    // for each one of the pocketing out line loop, cut depths
    for (size_t i = 0; i < path.size(); i++) {
        
        // Check to see if there is a clear path to the next starting point, if not, we retract fully
        if(isPocket) {
            // sanity check
            if(path.size() != enclosingPath.size()) {
                Log::Error("Path sizes do not match, forcing full retract");
                pathParams.retract = GCodeBuilder::ForceRetract::Full;
            } else {
                // We need to retract z for each depth of pocket
                if(geos.Within({ path[i].front(), path[i].back() }, enclosingPath[i])) {
                    pathParams.retract = GCodeBuilder::ForceRetract::Partial;
                } else {
                    pathParams.retract = GCodeBuilder::ForceRetract::Full;
                }
            }
        }
        // copy the ptr of the path vertex data
        pathParams.points = &(path[i]);
        // add gcodes for path at depths
        if(gcodes.CutPathDepths(*m_Settings, pathParams)) { return false; }
    }         
    // add finishing path
    if(m_Params.finishPass) {
        // Force a single pass for the finishing path
        pathParams.zTop = pathParams.zBottom;
        // Calculate the path offset radius away from inputPath
        std::vector<Geom::LineString> finishPath = geos.Offset(inputPath, cutSide * toolRadius, geosParameters);    
        // Cut all the finishing paths 
        for(size_t i = 0; i < finishPath.size(); i++) {
            // copy the ptr of the path vertex data
            pathParams.points = &(finishPath[i]);
            // add gcodes for path at depths
            if(gcodes.CutPathDepths(*m_Settings, pathParams)) { return false; }
        }
    }
    // move to zPlane, end program
    gcodes.EndCommands();
    
    cb(move(gcodes.Get()));
    return true; 
}





void Functions::DrawWindows() 
{  
    bool needsUpdate = false;
    // For each function
    for(size_t i = 0; i < m_Functions.Size(); i++) {
        // begin new imgui window
        static ImGuiCustomModules::ImGuiWindow window(*m_Settings, m_Functions[i].Name());
        if(window.Begin(*m_Settings)) {  
            // Draw function window
            needsUpdate |= m_Functions[i].DrawWindow();
            window.End();
        }
    }
    
    
    // Update if needed
    if(needsUpdate) {
        m_Settings->SetUpdateFlag(ViewerUpdate::Full);
    }
}

} // end namespace Sqeak

/*
 * frames.cpp
 *  Max Peglar-Willis 2021
 */ 

#include "functions.h"

using namespace std;
using namespace MaxLib;


namespace Sqeak { 

    
bool Function_CutPath::Parameters_CutPath::Draw()
{
    bool needsUpdate = false;
    // Draw the parameters in a tree
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Parameters")) {

        // General Parameters
        ImGui::TextUnformatted("General"); ImGui::Indent();
            needsUpdate |= ImGui::Combo("Cut Side", &cutSide, "None\0Left\0Right\0Pocket\0\0");
            
            needsUpdate |= ImGui::InputFloat("Z Top", &cutPathParameters.depth.zTop);
            needsUpdate |= ImGui::InputFloat("Z Bottom", &cutPathParameters.depth.zBottom);
            needsUpdate |= ImGui::InputFloat("Depth of Cut", &cutPathParameters.depth.cutDepth);
            needsUpdate |= ImGui::InputFloat("Cut Overlap", &cutOverlap);
        ImGui::Unindent();
        
        // Retract Parameters
        ImGui::TextUnformatted("Retract"); ImGui::Indent();
            needsUpdate |= ImGui::Combo("Force Retract", (int*)&cutPathParameters.depth.retract, "None\0Partial\0Full\0\0");
            needsUpdate |= ImGui::InputFloat("Partial Retract Distance", &cutPathParameters.depth.partialRetractDistance);
        ImGui::Unindent();
        
          
        // Tab Parameters
        ImGui::TextUnformatted("Tabs"); ImGui::Indent();
        needsUpdate |= ImGui::Checkbox("Cut Tabs", &cutPathParameters.tabs.isActive);
        if(cutPathParameters.tabs.isActive) {
            ImGui::Indent();
                needsUpdate |= ImGui::InputFloat("Spacing", &cutPathParameters.tabs.spacing);
                needsUpdate |= ImGui::InputFloat("Height",  &cutPathParameters.tabs.height);
                needsUpdate |= ImGui::InputFloat("Width",   &cutPathParameters.tabs.width);
            ImGui::Unindent();
        }
        ImGui::Unindent();
        
        // Finishing Pass Parameters
        ImGui::TextUnformatted("Finishing Pass"); ImGui::Indent();
            needsUpdate |= ImGui::InputFloat("Thickness", &finishPass);
        ImGui::Unindent();
        
        // Interpolation
        ImGui::TextUnformatted("Advanced"); ImGui::Indent();
            needsUpdate |= ImGui::InputInt("Arc Smoothness", &geosParameters.QuadrantSegments, 1, 100);
            /*// cap style / join style
            static int imgui_CapStyle = geosParameters.CapStyle - 1;
            if(ImGui::Combo("Cap Style", &imgui_CapStyle, "Round\0Flat\0Square\0\0")) {
                geosParameters.CapStyle = imgui_CapStyle + 1;
                needsUpdate = true;
            }
            static int imgui_JoinStyle = geosParameters.JoinStyle - 1;
            if(ImGui::Combo("Join Style", &imgui_JoinStyle, "Round\0Mitre\0Bevel\0\0")) {
                geosParameters.JoinStyle = imgui_JoinStyle + 1;
                needsUpdate = true;
            }*/ 
        ImGui::Unindent();
        ImGui::TreePop();
    }
    return needsUpdate;
}

    
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

    needsUpdate |= ImGui::InputText("Name", &m_Name); ImGui::Dummy(ImVec2());
        
    // Select Geometry Button
    DrawSelectButton(&m_Selected_Points, &m_Selected_Elements, &m_Selected_Polygons);
    
    m_Params.Draw();
    
    if(ImGui::Button("Build GCode")) {
        
        if(!m_Selected_Elements.empty() || !m_Selected_Polygons.empty()) {
            // make an array of points to send to gcodebuilder 
            std::vector<Geometry> combinedGeometry;
            // choose either elements of polygons
            if(!m_Selected_Polygons.empty()) { // copy the array
                combinedGeometry = m_Selected_Polygons;
            } 
            else { // get points from sketchitem and copy the points into an array
                std::vector<Geometry> geometry;
                for(auto& element : m_Selected_Elements) {
                    Geometry g = m_Sketcher->Factory().RenderElementBySketchItem(element);
                    geometry.push_back(g); 
                }         
                // Simplify geometry       
                Geos geos;
                combinedGeometry = geos.CombineLineStrings(geometry);
            }
            
            GCode_SendToViewer(combinedGeometry);
            needsUpdate = true; 
        }
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
    stream << "; \tBetween: " << p.cutPathParameters.depth.zTop << " and " << p.cutPathParameters.depth.zBottom << '\n';
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


// Error check each inputpath
bool Function_CutPath::IsValidInputs(const Geom::LineString& inputPath) 
{  
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
    
    return true;
}   

// Error check all inputpath
bool Function_CutPath::IsValidInputs(const std::vector<Geom::LineString>& inputPaths) 
{  
    // ensure input data is available
    if(inputPaths.empty()) {
        Log::Error("Path is empty");
        return false;
    }
    // error check for each linestring in inputpath
    for(size_t i = 0; i < inputPaths.size(); i++) { 
        if(!IsValidInputs(inputPaths[i])) {
            return false;  
        }  
    }
    
    // check tool and material is selected
    if(m_Settings->p.toolSettings.tools.IsToolAndMaterialSelected()) {
        Log::Error("Tool and Material must be selected");
        return false;
    }

    // z top and bottom
    if(m_Params.cutPathParameters.depth.zBottom > m_Params.cutPathParameters.depth.zTop) {
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
bool Function_CutPath::InterpretGCode(const std::vector<Geom::LineString>& inputPaths, std::function<void(std::vector<std::string>&)> cb)  
{
    // Error check inputs
    if(!IsValidInputs(inputPaths)) { return false; }  
    // get tool / material
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
    GeosBufferParams& geosParameters = m_Params.geosParameters;
    
    
    // calculate offset
//    GCodeBuilder::CutPathParams pathParams;
    // populate parameters
//    pathParams.zTop = m_Params.zTop;  
//    pathParams.zBottom = m_Params.zBottom; 
//    pathParams.cutDepth = toolData.cutDepth; 
//    pathParams.feedPlunge = toolData.feedPlunge;  
//    pathParams.feedCutting = toolData.feedCutting; 
     
     
    m_Params.cutPathParameters.tool.diameter = tool.Diameter;
    
    // vector for storing the final paths
    std::vector<Geom::LineString> path;
    std::vector<Geom::LineString> enclosingPath;
    bool isPocket = (m_Params.cutSide == Parameters_CutPath::CompensateCutter::Pocket);
     
    // Initialise geos (for geometry offseting)
    Geos geos;
    
    // for each linestring in inputpath
    for(size_t i = 0; i < inputPaths.size(); i++)
    {  
        // Get input path
        const Geom::LineString& inputPath = inputPaths[i];
        // Just add Simple path
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
                float boringOffset = 2.0f * toolRadius - m_Params.cutOverlap; 
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
                    m_Params.cutPathParameters.depth.retract = GCodeBuilder::ForceRetract::Full;
                } else {
                    // We need to retract z for each depth of pocket
                    if(geos.Within({ path[i].front(), path[i].back() }, enclosingPath[i])) {
                        m_Params.cutPathParameters.depth.retract = GCodeBuilder::ForceRetract::Partial;
                    } else {
                        m_Params.cutPathParameters.depth.retract = GCodeBuilder::ForceRetract::Full;
                    }
                }
            }
            // copy the ptr of the path vertex data
         //   pathParams.points = &(path[i]);
            // add gcodes for path at depths
            if(gcodes.CutPathDepths(path[i], m_Params.cutPathParameters)) { return false; }
        }         
        // add finishing path
        if(m_Params.finishPass) {
            // Force a single pass for the finishing path
            GCodeBuilder::CutPathParams finishPathParams = m_Params.cutPathParameters;
            finishPathParams.depth.zTop = finishPathParams.depth.zBottom;
            finishPathParams.depth.retract = GCodeBuilder::ForceRetract::Full;
            // Calculate the path offset radius away from inputPath
            std::vector<Geom::LineString> finishPath = geos.Offset(inputPath, cutSide * toolRadius, geosParameters);    
            // Cut all the finishing paths 
            for(size_t i = 0; i < finishPath.size(); i++) {
                // copy the ptr of the path vertex data
                //finishPathParams.points = &(finishPath[i]);
                // add gcodes for path at depths
                if(gcodes.CutPathDepths(finishPath[i], finishPathParams)) { return false; }
            }
        }
    }
    // move to zPlane, end program
    gcodes.EndCommands();
    // send points to callback
    cb(gcodes.Output());
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

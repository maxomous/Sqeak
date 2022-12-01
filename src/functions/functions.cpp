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
        ImGui::Unindent();
        
        // Pocketing
        if(cutSide == (int)CompensateCutter::Pocket) {
            ImGui::TextUnformatted("Pocketing"); ImGui::Indent();
                needsUpdate |= ImGui::InputFloat("Cut Overlap", &cutOverlap);
                needsUpdate |= ImGui::InputFloat("Partial Retract Distance", &cutPathParameters.depth.partialRetractDistance);
            ImGui::Unindent();
        }
        else { // Non pocketing parameters
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
        }
        
        // Finishing Pass Parameters
        ImGui::TextUnformatted("Finish Pass"); ImGui::Indent();
            needsUpdate |= ImGui::InputFloat("Thickness", &finishPass);
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
            Geos geos;
            // choose either elements of polygons
            if(!m_Selected_Polygons.empty()) { // copy the array
                // combine adjacent or overlapping polygons
                combinedGeometry = geos.CombinePolygons(m_Selected_Polygons);
            } 
            else { // get points from sketchitem and copy the points into an array
                std::vector<Geometry> geometry;
                for(auto& element : m_Selected_Elements) {
                    Geometry g = m_Sketcher->Renderer().RenderElementBySketchItem(element);
                    geometry.push_back(g); 
                }         
                // Simplify geometry     
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
    Geos::BufferParameters& geosParameters = m_Settings->p.geosParameters;
    
    m_Params.cutPathParameters.tool.diameter = tool.Diameter;
    
    // vector for storing the final paths
    std::vector<Geom::LineString> path;
    std::vector<Geom::LineString> enclosingPath;
    bool isPocket = (m_Params.cutSide == Parameters_CutPath::CompensateCutter::Pocket);
    
    if(isPocket) {
        m_Params.cutPathParameters.tabs.isActive = false;
        m_Params.cutPathParameters.depth.retract = GCodeBuilder::RetractType::Full;
    }
    // Initialise geos (for geometry offseting)
    Geos geos;
    
    // for each linestring in inputpath
    for(size_t i = 0; i < inputPaths.size(); i++)
    {  
        // Get input path
        const Geom::LineString& inputPath = inputPaths[i];
        // offset paths / tabs
        std::vector<Geom::LineString> path;
        std::vector<Geom::LineString> enclosingPath; // **For pocketing only
        std::vector<Geom::LineString> finishPath;
        
        struct TabPositions {
            std::vector<std::pair<size_t, Vec2>> pathTabs;
            std::vector<std::pair<size_t, Vec2>> finishPathTabs;
        } tabPositions;
        
        auto CalculatePathOffset = [&]()
        {
            // Just add Simple path
            if(m_Params.cutSide == Parameters_CutPath::CompensateCutter::None) {
                path = {{ inputPath }};
            }  else { // Compensate path by radius
                path = geos.Offset(inputPath, cutSide * pathOffset, geosParameters);
            }
            // add pocket path 
            if(isPocket) { 
                // distance to offset per pass
                float boringOffset = 2.0f * toolRadius - m_Params.cutOverlap; 
                // start a recursive loop of offsetting path until it fails, if cutting single offset, this breaks loop after the first iteration
                Geos::RecursiveOffset recursiveOffset = geos.OffsetPolygon_Recursive(path, boringOffset, true, geosParameters); // true is reverse
                path = recursiveOffset.path;
                enclosingPath = recursiveOffset.enclosingPath;
            }
            // add finishing path (Calculate the path offset radius away from inputPath)
            if(m_Params.finishPass) {  
                finishPath = geos.Offset(inputPath, cutSide * toolRadius, geosParameters);
            }
        };
        
  //     auto CalculatePathTabs = [&]()
  //     {
  //         
  //          
  //         Vec2 p0 = { 100.0, 100.0 };
  //         Vec2 p1 = { 150.0, 200.0 };
  //         Vec2 pStart = (p0+p1) / 2.0;
  //         
  //         double distance = 50.0;
  //         int direction = 1;
  //         
  //         Vec2 pOut = PointPerpendicularToLine(p0, p1, direction * distance, pStart);
  //         
  //         
  //         
  //         
  //         tabPositions = GetTabPositions(inputPath, m_Params.cutPathParameters);
  //         
  //         
  //         cutSide * pathOffset
  //         cutSide * toolRadius
  //         
  //         
  //         // Calculate Tab positions (where tabs should lie and their indexes within path[])
  //         pathTabs = gcodes.GetTabPositions(path, m_Params.cutPathParameters);
  //         finishPathTabs = gcodes.GetTabPositions(finishPath, m_Params.cutPathParameters);
  //
  //     };
        
        
        CalculatePathOffset();
   //     CalculatePathTabs();
        
        
        
        
        
        // for each one of the pocketing out line loop, cut depths
        for (size_t i = 0; i < path.size(); i++) {
            // default to full retract
            m_Params.cutPathParameters.depth.retract = GCodeBuilder::RetractType::Full;
            // Check to see if there is a clear path to the next starting point, if not, we retract fully
            if(isPocket) {
                // sanity check
                if(path.size() != enclosingPath.size()) {
                    Log::Error("Path sizes do not match, forcing full retract");
                }  // If next line falls within the enclosing path, we can do a partial retract to save time
                else if(geos.Within({ path[i].front(), path[i].back() }, enclosingPath[i])) {
                    m_Params.cutPathParameters.depth.retract = GCodeBuilder::RetractType::Partial;
                }
            }
            // add gcodes for path at depths
            if(gcodes.CutPathDepths(path[i], m_Params.cutPathParameters, tabPositions.pathTabs)) { return false; }
        }         
        // add finishing path
        if(m_Params.finishPass) {  
            // Force a single pass for the finishing path
            GCodeBuilder::CutPathParams finishPathParams = m_Params.cutPathParameters;
            finishPathParams.depth.zTop = finishPathParams.depth.zBottom;
            finishPathParams.depth.retract = GCodeBuilder::RetractType::Full;
            // Cut all the finishing paths 
            for(size_t i = 0; i < finishPath.size(); i++) {
                // add gcodes for path at depths
                if(gcodes.CutPathDepths(finishPath[i], finishPathParams, tabPositions.finishPathTabs)) { return false; }
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

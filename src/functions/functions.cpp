/*
 * frames.cpp
 *  Max Peglar-Willis 2021
 */ 

#include "functions.h"

using namespace std;
using namespace MaxLib;

 
namespace Sqeak { 

  
  
    
// update the tool data in depthCutter to the active tool in Tools
void Function_CutPath::Parameters_CutPath::SetTool(ToolSettings::Tools& tools) 
{
    if(tools.IsToolAndMaterialSelected()) {
        // get tool / material
        ToolSettings::Tools::Tool& tool = tools.toolList.CurrentItem();
        ToolSettings::Tools::Tool::ToolData& toolData = tool.data.CurrentItem(); 
        depthCutter.tool.diameter       = tool.diameter;
        depthCutter.tool.cutDepth       = toolData.cutDepth;
        depthCutter.tool.feedCutting    = toolData.feedCutting;
        depthCutter.tool.feedPlunge     = toolData.feedPlunge;
        spindleSpeed                    = toolData.speed;
    } 
    else 
    {   Log::Error("Tool and Material must be selected");
        depthCutter.tool.diameter       = 0.0f;
        depthCutter.tool.cutDepth       = 0.0f;
        depthCutter.tool.feedCutting    = 0.0f;
        depthCutter.tool.feedPlunge     = 0.0f;
        spindleSpeed                    = 0.0f;
    }
}
         

     
    
bool Function_CutPath::Parameters_CutPath::Draw(Settings& settings)
{
    bool needsUpdate = false;
    // Draw the parameters in a tree
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Parameters")) {

        // Tool
        ImGui::TextUnformatted("Tool"); ImGui::Indent();
            // Select Tool for Depth Cutter
            ImGui::Text("Tool Diameter: %g", depthCutter.tool.diameter);
            ImGui::Text("Cut Depth: %g", depthCutter.tool.cutDepth);
            ImGui::Text("FeedRate Cutting: %g", depthCutter.tool.feedCutting);
            ImGui::Text("FeedRate Plunge: %g", depthCutter.tool.feedPlunge);
            ImGui::Text("Spindle Speed: %g", spindleSpeed);
            
            if(ImGui::Button("Use Current Tool Data")) {
                // Set tool for depth cutter to the active tool   
                SetTool(settings.p.toolSettings.tools);
                needsUpdate = true;
            }
        ImGui::Unindent();

        // General Parameters
        ImGui::TextUnformatted("General"); ImGui::Indent();
            needsUpdate |= ImGui::Combo("Cut Side", reinterpret_cast<int*>(&pathCutter.cutSide), "None\0Left\0Right\0Pocket\0\0");
            
            needsUpdate |= ImGui::InputFloat("Z Top", &depthCutter.depth.zTop);
            needsUpdate |= ImGui::InputFloat("Z Bottom", &depthCutter.depth.zBottom);
        ImGui::Unindent();
        
        // Pocketing
        if(pathCutter.cutSide == PathCutter::CompensateCutter::Pocket) {   
            ImGui::TextUnformatted("Pocketing"); ImGui::Indent();
                needsUpdate |= ImGui::InputFloat("Cut Overlap", &pathCutter.cutOverlap);
                needsUpdate |= ImGui::InputFloat("Partial Retract Distance", &depthCutter.depth.partialRetractDistance);
            ImGui::Unindent();
        }
        else { // Non pocketing parameters
            // Tab Parameters
            ImGui::TextUnformatted("Tabs"); ImGui::Indent();
            needsUpdate |= ImGui::Checkbox("Cut Tabs", &depthCutter.tabs.isActive);
            if(depthCutter.tabs.isActive) {
                ImGui::Indent();
                    needsUpdate |= ImGui::InputFloat("Spacing", &depthCutter.tabs.spacing);
                    needsUpdate |= ImGui::InputFloat("Height",  &depthCutter.tabs.height);
                    needsUpdate |= ImGui::InputFloat("Width",   &depthCutter.tabs.width);
                ImGui::Unindent();
            }
            ImGui::Unindent();
        }
        
        // Finishing Pass Parameters
        ImGui::TextUnformatted("Finish Pass"); ImGui::Indent();
            needsUpdate |= ImGui::InputFloat("Thickness", &pathCutter.finishPass);
        ImGui::Unindent();
        
        
        ImGui::TreePop();
    }
    return needsUpdate;
}

  

bool Function_CutPath::Update() 
{
    if(m_Selected_Elements.empty() && m_Selected_Polygons.empty()) { return false; }
    
    GeosCPP geos;
    // choose either polygons
    if(!m_Selected_Polygons.empty()) { // copy the array
        // combine adjacent or overlapping polygons + send to gcoder
        std::vector<Polygon> simplifiedGeometry = geos.operation.Combine(m_Selected_Polygons);
        GCode_SendToViewer(simplifiedGeometry);
    }
    // or choose linestrings
    else {     
        
        std::vector<LineString> lineStrings = Vector::VectorCopy<Sketch::SketchItem, LineString>(m_Selected_Elements, [&](Sketch::SketchItem& item) {
            return m_Sketcher.Renderer().RenderElementBySketchItem(item);
        });
        
       // std::vector<LineString> lineStrings;
       // // get points from sketchitem and copy the points into an array
       // for(auto& element : m_Selected_Elements) {
       //     LineString l = m_Sketcher.Renderer().RenderElementBySketchItem(element);
       //     lineStrings.push_back(l); 
       // }         
        // Simplify lineStrings     
        std::vector<LineString> simplifiedGeometry = geos.operation.Combine(lineStrings);
        GCode_SendToViewer(simplifiedGeometry);
    }
    
    return true; 
}


bool Function_CutPath::DrawWindow() 
{
    bool needsUpdate = false;

    needsUpdate |= ImGui::InputText("Name", &m_Name); ImGui::Dummy(ImVec2());
        
    // Select Geometry Button
    DrawSelectButton(&m_Selected_Points, &m_Selected_Elements, &m_Selected_Polygons);
    
    m_Params.Draw(m_Settings);
    
    if(ImGui::Button("Build GCode")) {
        needsUpdate |= Update();
    }
    
    if(ImGui::Button("Triangulate")) {
        needsUpdate |= true;
      //  GeosCPP::GeometryCollection triangulated = geos.operation.Triangulation(holes, 0.0); 
      //  GEOSDifference();

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
    stream << "; \tBetween: " << p.depthCutter.depth.zTop << " and " << p.depthCutter.depth.zBottom << '\n';
    stream << "; \tPoints:" << '\n';
    
    // TODO add header text
    
   // m_Params.drawing.AddElementsHeaderText(stream);
    
    if(p.pathCutter.cutSide == PathCutter::CompensateCutter::None) stream << "; \tCompensate: None\n";
    if(p.pathCutter.cutSide == PathCutter::CompensateCutter::Left) stream << "; \tCompensate: Left\n";
    if(p.pathCutter.cutSide == PathCutter::CompensateCutter::Right) stream << "; \tCompensate: Right\n";
    if(p.pathCutter.cutSide == PathCutter::CompensateCutter::Pocket) stream << "; \tCompensate: Pocket\n";
    
    if(p.pathCutter.finishPass) stream << "; Finishing Pass: " << p.pathCutter.finishPass << '\n';
    
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
    if(m_Params.pathCutter.cutSide == PathCutter::CompensateCutter::Pocket && !isLoop) {
        Log::Error("Pocket requires start and end points to be identical");
        return false;
    }
    
    return true;
}   

// Error check each inputpath
bool Function_CutPath::IsValidInputs(const Geom::Polygon& inputPath) 
{  
    // error check shell
    if(!IsValidInputs(inputPath.shell)) { return false; }
    // error check holes
    for(auto& hole : inputPath.holes) {
        if(!IsValidInputs(hole)) { return false; }
    }
    return true;
}   


//// return value is success
//// T can be Geom::LineString or Geom::Polygon
//template<typename T>
//bool Function_CutPath::InterpretGCode(const std::vector<T>& inputPaths, std::function<void(std::vector<std::string>&)> cb)  
//// return value is success
//bool Function_CutPath::InterpretGCode(const std::vector<Geom::LineString>& inputPaths, std::function<void(std::vector<std::string>&)> cb)  
//{
//    // Error check inputs
//    if(!IsValidInputs(inputPaths)) { return false; }  
//    // get tool / material
//    ToolSettings::Tools::Tool& tool = m_Settings.p.toolSettings.tools.toolList.CurrentItem();
//    ToolSettings::Tools::Tool::ToolData& toolData = tool.data.CurrentItem(); 
//         
//    // initialise 
//    GCodeBuilder gcodes;
//    // Add the GCode header text
//    gcodes.Add(HeaderText());
//    // Resets all the main modal commands + sets spindle speed 
//    gcodes.Initialisation(toolData.speed);
//    
//    // Define offset path parameters:  0 = no compensation, 1 = compensate left / pocket, -1 = compensate right
//    int cutSide = m_Params.GetCutSide();
//    float toolRadius = fabsf(tool.Diameter / 2.0f);
// //   float pathOffset = toolRadius + fabsf(m_Params.finishPass);
//    
//    m_Params.cutPathParameters.tool.diameter = tool.Diameter;
//    
//    bool isPocket = (m_Params.cutSide == Parameters_CutPath::CompensateCutter::Pocket);
//    if(isPocket) {
//        m_Params.cutPathParameters.tabs.isActive = false;
//        m_Params.cutPathParameters.depth.retract = GCodeBuilder::RetractType::Full;
//    }
//    // define geos parameters
//    GeosCPP::Operation::OffsetParameters& geosParameters = m_Settings.p.geosParameters;
//    // Initialise geos (for geometry offseting)
//    GeosCPP geos;
//     
//    // Offset each linestring individually so that the finish pass executes after each linestring rather than all at the end
//    for(size_t i = 0; i < inputPaths.size(); i++)
//    {  
//        
//        GeometryCollection offset = pathCutter.CalculatePaths(inputPaths[i], toolRadius);
//        
//        
//        
//        // make an array of 1 with this input path
//        std::vector<Geom::Polygon> inputPath = {{ inputPaths[i] }};
//        // offset paths / tabs
//        std::vector<Geom::Polygon> path;
//        std::vector<Geom::Polygon> enclosingPath; // equiv. to offsetting the polygon by  **For pocketing only
//        std::vector<Geom::Polygon> finishPath;
//        
//        auto CalculatePathOffset = [&]()
//        {
//            // Just add Simple path
//            if(m_Params.cutSide == Parameters_CutPath::CompensateCutter::None) {
//                path = inputPath;//{{ inputPath }};
//            } else { // Compensate path by radius
//                GeometryCollection collection = geos.operation.Offset(inputPath, cutSide * pathOffset, geosParameters);
//                
//                for(auto& lineString : collection.lineStrings) {
//                    path.emplace_back(std::move(lineString));
//                }
//                for(auto& polygon : collection.polygons) {
//                    path.emplace_back(std::move(polygon.shell));
//                   // for(auto& hole : polygon.holes) {
//                   //     path.emplace_back(std::move(hole));
//                   // } 
//                }
//            }
//            // add pocket path 
//            if(isPocket) { 
//                // distance to offset per pass
//                float boringOffset = 2.0f * toolRadius - m_Params.cutOverlap; 
//                // TODO: TEMPORARY WHILST WAITING ON GEOS UPDATE
//              //  Geos geos_c;
//                // start a recursive loop of offsetting path until it fails, if cutting single offset, this breaks loop after the first iteration
//                Geos::BufferParameters params;
//                params.arcTolerance = geosParameters.arcTolerance;
//              //  Geos::RecursiveOffset recursiveOffset = geos_c.OffsetPolygon_Recursive(path, boringOffset, true, params); // true is reverse
//              //  path = recursiveOffset.path;
//              //  enclosingPath = recursiveOffset.enclosingPath;
//            }
//            // add finishing path (Calculate the path offset radius away from inputPath)
//            if(m_Params.finishPass) {                  
//                GeometryCollection finishPathCollection = geos.operation.Offset(inputPath, cutSide * toolRadius, geosParameters);
//                
//                //finishPath = Vector::VectorCopy<GeometryCollection, Geom::LineString>(finishPathCollection.lineStrings);
//                // Add both linestring + polygon to finishpath
//                for(auto& lineString : finishPathCollection.lineStrings) {
//                    finishPath.emplace_back(std::move(lineString));
//                }
//                for(auto& polygon : finishPathCollection.polygons) {
//                    finishPath.emplace_back(std::move(polygon.shell));
//                   // for(auto& hole : polygon.holes) {
//                   //     path.emplace_back(std::move(hole));
//                   // } 
//                }
//            }
//        };
//        
//        struct TabPositions {
//            std::vector<std::pair<size_t, Vec2>> pathTabs;
//            std::vector<std::pair<size_t, Vec2>> finishPathTabs;
//        } tabPositions;
//         
//  //     auto CalculatePathTabs = [&]()
//  //     {
//  //         
//  //          
//  //         Vec2 p0 = { 100.0, 100.0 };
//  //         Vec2 p1 = { 150.0, 200.0 };
//  //         Vec2 pStart = (p0+p1) / 2.0;
//  //         
//  //         double distance = 50.0;
//  //         int direction = 1;
//  //         
//  //         Vec2 pOut = PointPerpendicularToLine(p0, p1, direction * distance, pStart);
//  //         
//  //         
//  //         
//  //         
//  //         tabPositions = GetTabPositions(inputPath, m_Params.cutPathParameters);
//  //         
//  //         
//  //         cutSide * pathOffset
//  //         cutSide * toolRadius
//  //         
//  //         
//  //         // Calculate Tab positions (where tabs should lie and their indexes within path[])
//  //         pathTabs = gcodes.GetTabPositions(path, m_Params.cutPathParameters);
//  //         finishPathTabs = gcodes.GetTabPositions(finishPath, m_Params.cutPathParameters);
//  //
//  //     };
//        
//        
//        CalculatePathOffset();
//   //     CalculatePathTabs();
//        
//        
//        
//        
//        
//        // for each one of the pocketing out line loop, cut depths
//        for (size_t i = 0; i < path.size(); i++) {
//            // default to full retract
//            m_Params.cutPathParameters.depth.retract = GCodeBuilder::RetractType::Full;
//            // Check to see if there is a clear path to the next starting point, if not, we retract fully
//            if(isPocket) {
//                // sanity check
//                if(path.size() != enclosingPath.size()) {
//                    Log::Error("Path sizes do not match, forcing full retract");
//                }  // If next line falls within the enclosing path, we can do a partial retract to save time
//                else if(i > 0) {
//                    if(geos.operation.Within(Geom::LineString({ path[i-1].back(), path[i].front() }), enclosingPath[i])) {  // think we can use {{ inputpath[i] }}
//                        m_Params.cutPathParameters.depth.retract = GCodeBuilder::RetractType::Partial;
//                    }
//                }
//            }
//            // add gcodes for path at depths
//            if(gcodes.CutPathDepths(path[i], m_Params.cutPathParameters, tabPositions.pathTabs)) { return false; }
//        }         
//        // add finishing path
//        if(m_Params.finishPass) {  
//            // Force a single pass for the finishing path
//            GCodeBuilder::CutPathParams finishPathParams = m_Params.cutPathParameters;
//            finishPathParams.depth.zTop = finishPathParams.depth.zBottom;
//            finishPathParams.depth.retract = GCodeBuilder::RetractType::Full;
//            // Cut all the finishing paths 
//            for(size_t i = 0; i < finishPath.size(); i++) {
//                // add gcodes for path at depths
//                if(gcodes.CutPathDepths(finishPath[i], finishPathParams, tabPositions.finishPathTabs)) { return false; }
//            }
//        }
//    }
//    // move to zPlane, end program
//    gcodes.EndCommands();
//    // send points to callback
//    cb(gcodes.Output());
//    return true; 
//}





void Functions::Update() 
{
    // Update each function
    for(size_t i = 0; i < m_Functions.Size(); i++) {
        m_Functions[i].Update();
    }
}

void Functions::DrawWindows() 
{  
    bool needsUpdate = false;
    // For each function
    for(size_t i = 0; i < m_Functions.Size(); i++) {
        // begin new imgui window
        static ImGuiCustomModules::ImGuiWindow window(m_Settings, m_Functions[i].Name());
        if(window.Begin(m_Settings)) {  
            // Draw function window
            needsUpdate |= m_Functions[i].DrawWindow();
            window.End();
        }
    }
    
    
    // Update if needed
    if(needsUpdate) {
        m_Settings.SetUpdateFlag(SqeakUpdate::Full);
    }
}

void Functions::DrawItems() 
{
    
    bool isFunctionChanged = true; // m_IsActiveA_FunctionChanged
     
    
    // draw active function Tree Nodes
    //static std::function<const std::string&(Function& item)> cb_GetItemString = [](Function& function) { 
  //  static auto cb_GetItemString = [](Function& function) { 
  //      return function.Name(); 
  //  };
    static std::function<std::string(Function&)> cb_GetItemString = [](Function& function) { return function.Name(); };
    
    static std::function<void(Function&)> cb_DrawImGui = [](Function& function) {
        (void)function;
        ImGui::Text("Test 1");
    };
    
    
     
    if(ImGuiModules::TreeNodes(m_Functions, isFunctionChanged, cb_GetItemString, cb_DrawImGui)) {
        m_Settings.SetUpdateFlag(SqeakUpdate::Full);
    }

}
} // end namespace Sqeak

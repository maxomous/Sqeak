/*
 * frames.h
 *  Max Peglar-Willis 2022
 */

#pragma once
#include "../common.h"
#include "../sketch/gcodebuilder.h"


namespace Sqeak { 
   
// TODO:
//offset path is now taking the input path rather than the 1st offset path

// For a boring opperation, we offset recursively using the previous offset
// We therefore need to simplify the offset so that the arcs dont exponentially interpolate
// (you may end up with more geometries than your input geometry)



 //     TODO:
 //   when we are about to run function:
 //       void PreProcessGeometry() {
 //           - make lines into linestrings
 //           - make polygons in polygons with holes
 //       }
 //        
 //   on sketch element has changed:
 //       void UpdateFunction() {
 //           // For SketchItem
 //           check it still has the same points
 //           // For Polygons
 //           check if loop matches any in new polygonised?
 //       }
        
  
/*
// sadly, this wont work for lines which angle up or down in z / helical lines

struct ZHeight
{
    std::unique_ptr<geos::geom::Geometry> geosGeometry;
    float z;
};


    ordered list of zHeights

    levels
        sum each level's geometry to produce the geometry to subtract from workpiece
    
    std::unique_ptr<geos::geom::Geometry> combined = workpiece; 
    for(auto& zHeight : zHeights) {
        combined += zHeight.Geometry
        geos::geom::Geometry* geometry
        geosGeometry = geosGeometry->Union(geometry);
    }
    */
  
   
class Function
{
public:
    // Get function name
    std::string Name() { return m_Name; } 
    // returns true if updated succesfully
    virtual bool Update() = 0;
    // Draw Imgui side window widgets
    virtual bool DrawWindow() = 0;
    // return value is success
    template<typename T>
    virtual bool InterpretGCode(const std::vector<T>& inputPaths, std::function<void(std::vector<std::string>&)> cb) = 0;
               
    /*
    // Send strait to GRBL
    // ****** MAYBE WE DONT WANT THIS AND OPEN THE SAME WAY AS NORMAL FILES ****
    void GCode_SendToGRBL(GRBL& grbl, const Geom::LineString& inputPath) 
    {
        Log::Info("Sending GCode to GRBL");
        // build gcode of active function and run it
        InterpretGCode(inputPath, [&](auto gcodes){
            // start file timer
            Event<Event_ResetFileTimer>::Dispatch({});  
            // send gocdes to grbl
            if(grbl.sendArray(gcodes)) { 
                Log::Error("Couldn't send file to grbl");
                return -1;
            }; 
            return 0;
        });
    }  
    * */
    // T can be Geom::LineString or Geom::Polygon
    template<typename T>
    bool GCode_SendToViewer(const std::vector<T>& inputPaths)
    { 
        std::cout << "Updating: GCode_SendToViewer" << std::endl;
        // update the active function 
        bool success = InterpretGCode(inputPaths, [](auto gcodes) {
            Event<Event_Update3DModelFromVector>::Dispatch({ vector<string>(move(gcodes)) }); 
        });
        if(!success) {
            Event<Event_Update3DModelFromVector>::Dispatch({ vector<string>(/*empty*/) });
        }
        return success;
    }
        
    // export the active function as gcode
    bool GCode_Export(std::string saveDirectory, const std::vector<Geom::LineString>& inputPaths) 
    { 
        // make filepath for new GCode file 
        std::string filepath = File::CombineDirPath(saveDirectory, Name() + ".nc"); 
        Log::Info("Exporting GCode to %s", filepath.c_str());
        // build gcode of active function and export it to file 
        return InterpretGCode(inputPaths, [&](auto gcodes) { 
            File::WriteArray(filepath, gcodes);
        }); 
    }
    
protected:
    Function(std::string name, Settings& settings, Sketch::Sketcher& sketcher)
        : m_Name(name), m_Settings(settings), m_Sketcher(sketcher) {}
    
    std::string m_Name;
    Settings& m_Settings;
    Sketch::Sketcher& m_Sketcher;
    
    // Parameters are optional with nullptr
    void DrawSelectButton(std::vector<Sketch::SketchItem>* points, std::vector<Sketch::SketchItem>* elements, std::vector<Polygon>* polygons) 
    {
        GUISettings& s = m_Settings.guiSettings;
        Sketch::SketchEvents& events = m_Sketcher.Events();
        // Button is active if select loop command is selected
        bool isActive = (events.GetCommandType() == Sketch::SketchEvents::CommandType::SelectLoop);
        // Selection Button
        if (ImGuiModules::ImageButtonWithText("Select Geometry##FunctionButton", s.img_Sketch_SelectLoop, s.imageButton_Toolbar_SketchPrimary, isActive)) { 
            // Set command type to select loop
            events.SetCommandType(Sketch::SketchEvents::CommandType::SelectLoop);
        }
        // Show ok / cancel buttons if on the Select command
        if(isActive) {
            ImGui::SameLine();
            if(ImGui::Button("Ok")) {
                // Store a copy of the selected geometry
                if(points)   *points   = events.GetSelectedPoints();
                if(elements) *elements = events.GetSelectedElements();
                if(polygons) *polygons = events.GetSelectedPolygons();
                // Deselect tool
                events.SetCommandType(Sketch::SketchEvents::CommandType::None);
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                // Deselect tool
                events.SetCommandType(Sketch::SketchEvents::CommandType::None);
            }
        }
    }
    
    void DrawSelectedItems(const std::string& label, std::vector<Sketch::SketchItem>& elements) 
    {   // ImGui::CollapsingHeader()
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if(ImGui::TreeNode(label.c_str())) { // use ptr as unique id
            // Display the selected linestrings
            for(size_t i = 0; i < elements.size(); i++) {
                ImGui::TextUnformatted(elements[i].Name().c_str());
            }
            ImGui::TreePop();
        }
    }
    
    void DrawSelectedItems(const std::string& label, std::vector<Polygon>& polygons) 
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if(ImGui::TreeNode(label.c_str())) { // use ptr as unique id
            // Display the selected polygons
            for(size_t i = 0; i < polygons.size(); i++) {
                std::string name = "Polygon " + to_string(i);
                ImGui::TextUnformatted(name.c_str());
            }
            ImGui::TreePop();
        }
    }
    
     
 
    void HeaderText_AddToolInfo(std::ostringstream& stream) {
        
        ToolSettings::Tools::Tool& tool =  m_Settings.p.toolSettings.tools.toolList.CurrentItem();
        ToolSettings::Tools::Tool::ToolData& toolData = tool.data.CurrentItem();    
        
        stream << "; Tool: " << tool.name << '\n';
        stream << "; \tDiameter: " << tool.diameter << '\n';
        stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
        stream << '\n';
    }
    
    

};

        
  
// TODO: Make Parameters_CutPath it's own class
    

class Function_CutPath : public Function
{
public: 
    
    struct Parameters_CutPath 
    {
        // contains the parameters used for converting geometry to a basic, offset or boring path
        PathCutter pathCutter;
        // contains the parameters to convert a line string in to GCode by cutting it at depths and adding tabs
        DepthCutter depthCutter;
        // Tool parameters
        float spindleSpeed = 12000.0f;
        
        void SetTool(ToolSettings::Tools& tools);
        // ImGui input widgets for the parameters
        bool Draw(Settings& settings);
    };
    
    Function_CutPath(std::string name, Settings& settings, Sketch::Sketcher& sketcher)
        : Function(name, settings, sketcher) {}
    // returns true if updated succesfully
    bool Update() override;
    
    bool DrawWindow() override;

    std::string HeaderText();
    
    bool IsValidInputs(const Geom::LineString& inputPath);
    bool IsValidInputs(const Geom::Polygon& inputPath);
        
    // Error check all inputpath
    // T can be either Geom::LineString or Geom::Polygon
    template<typename T>
    bool IsValidInputs(const std::vector<T>& inputPaths) 
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
        if(m_Settings.p.toolSettings.tools.IsToolAndMaterialSelected()) {
            Log::Error("Tool and Material must be selected");
            return false;
        }

        // z top and bottom
        if(m_Params.cutPathParameters.depth.zBottom > m_Params.cutPathParameters.depth.zTop) {
            Log::Error("Z Bottom must be below or equal to Z Top");
            return false;
        }
        // cut depth should have value
        ToolSettings::Tools::Tool::ToolData& toolData = m_Settings.p.toolSettings.tools.toolList.CurrentItem().data.CurrentItem();
        if(toolData.cutDepth <= 0.0f) {
            Log::Error("Cut Depth must be positive");
            return false;
        }
        
        return true;
    }   




    // return value is success
    // T can be Geom::LineString or Geom::Polygon
    template<typename T>
    bool InterpretGCode(const std::vector<T>& inputPaths, std::function<void(std::vector<std::string>&)> cb) override
    {
        // Error check inputs
        if(!IsValidInputs(inputPaths)) { return false; }  
             
        // initialise 
        GCodeBuilder gcodes;
        // Add the GCode header text
        gcodes.Add(HeaderText());
        // Resets all the main modal commands + sets spindle speed 
        gcodes.Initialisation(m_Params.spindleSpeed);

        // Tool Radius
        float toolRadius = fabsf(m_Params.depthCutter.tool.diameter / 2.0f);
        // define geos parameters for finish pass (Same as main pass but force a single depth)
        DepthCutter finishPass = m_Params.depthCutter;
        finishPass.depth.zTop = finishPass.depth.zBottom;
        
        // Offset each linestring individually so that the finish pass executes after each linestring rather than all at the end
        for(size_t i = 0; i < inputPaths.size(); i++)
        {  
            // Offset the path by toolRadius
            PathCutter::PathData pathData = m_Params.pathCutter.CalculatePaths(inputPaths[i], toolRadius, m_Settings.p.geosParameters);
                // Determine tab positions
                // if(!isPocket) { CalculatePathTabs(); }
                // else retract = RetractType::Partial;
            
            // Add gcodes for path at each cut depths
            if(m_Params.depthCutter.CutPathDepths(gcodes, pathData.cutPath, DepthCutter::RetractType::Full)) { return false; }
            
            // Add finishing path
            if(m_Params.pathCutter.finishPass) {  
                // add gcodes for path at depths
                if(finishPass.CutPathDepths(gcodes, pathData.finishPath, DepthCutter::RetractType::Full)) { return false; }
            }
        }
        // move to zPlane, end program
        gcodes.EndCommands();
        // send points to callback
        cb(gcodes.Output());
        return true; 
    }



private:
    // A selection of geometry which links back to the original sketch element so that it can update if sketch is modified 
    std::vector<Sketch::SketchItem> m_Selected_Points;
    std::vector<Sketch::SketchItem> m_Selected_Elements;
    // A selection of raw points so cannot link back to original polygonised points
    std::vector<Polygon> m_Selected_Polygons;
    // Parameters
    Parameters_CutPath m_Params;    
};



struct Functions 
{
    Functions(Settings& settings, Sketch::Sketcher& sketcher)
        : m_Settings(settings), m_Sketcher(sketcher) 
    {
        // Initialise a function temporarily
        m_Functions.Add<Function_CutPath>("Cut Path##INSERT ID", settings, sketcher);
    }
    
    void Update();
    void DrawWindows();
    void DrawItems();

    Settings& m_Settings; 
    Sketch::Sketcher& m_Sketcher;

    Vector_SelectablePtrs<Function> m_Functions;

    
};


} // end namespace Sqeak

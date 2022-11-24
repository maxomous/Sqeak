/*
 * frames.h
 *  Max Peglar-Willis 2022
 */

#pragma once
#include "../common.h"
#include "../sketch/gcodebuilder.h"


namespace Sqeak { 
   
 //     TODO:
 
 //     remove m_Settings from gcodes.CutPathDepths()
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
        
   
class Function
{
public:
    // Get function name
    std::string Name() { return m_Name; } 
    // Draw Imgui side window widgets
    virtual bool DrawWindow() = 0;
    // return value is success
    virtual bool InterpretGCode(const std::vector<Geom::LineString>& inputPaths, std::function<void(std::vector<std::string>&)> cb) = 0;
               
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
    
    bool GCode_SendToViewer(const std::vector<Geom::LineString>& inputPaths)
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
    Function(std::string name, Settings* settings, Sketch::Sketcher* sketcher)
        : m_Name(name), m_Settings(settings), m_Sketcher(sketcher) {}
    
    std::string m_Name;
    Settings* m_Settings;
    Sketch::Sketcher* m_Sketcher;
    
    void DrawSelectButton(std::vector<Sketch::SketchItem>* points, std::vector<Sketch::SketchItem>* elements, std::vector<Geometry>* polygons) 
    {
        GUISettings& s = m_Settings->guiSettings;
        Sketch::SketchEvents& events = m_Sketcher->Events();
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
    
    void DrawSelectedItems(const std::string& label, std::vector<Geometry>& polygons) 
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
        
        ToolSettings::Tools::Tool& tool = m_Settings->p.toolSettings.tools.toolList.CurrentItem();
        ToolSettings::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();    
        
        stream << "; Tool: " << tool.Name << '\n';
        stream << "; \tDiameter: " << tool.Diameter << '\n';
        stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
        stream << '\n';
    }
    

 

};

        
  

    

class Function_CutPath : public Function
{
public: 
    
    
    struct Parameters_CutPath 
    {
        enum CompensateCutter { None, Left, Right, Pocket };
        
        // int allows us to use this with ImGui, but really this is a CompensateCutter
        int cutSide                     = (int)CompensateCutter::None;
        // width of the finsihing pass
        float finishPass                = 1.0f;
        // width of overlap between adjacent passes
        float cutOverlap                = 1.0f; // mm
        // path cut at depth parameters
        GCodeBuilder::CutPathParams cutPathParameters;
        // Geos geometry offset parameters
        GeosBufferParams geosParameters;
        // get cut side as 0 (none), 1 (right) or -1 (left)
        int GetCutSide();
        // ImGui input widgets for the parameters
        bool Draw();
    };
/*
    
    struct Parameters_CutPath 
    {
        enum CompensateCutter { None, Left, Right, Pocket };
        
        struct Tabs {
            bool isActive   = true; 
            float spacing   = 50.0f;  
            float height    = 4.0f;  
            float width     = 8.0f;  
        } tabs;
        // get cut side as 0 (none), 1 (right) or -1 (left)
        int GetCutSide();
        // ImGui input widgets for the parameters
        bool Draw();
        // int allows us to use this with ImGui, but really this is a CompensateCutter
        int cutSide                     = (int)CompensateCutter::None;
        // start and end Z coords
        float zTop                      = 20.0f;
        float zBottom                   = 0.0f;
        // width of the finsihing pass
        float finishPass                = 1.0f;
        // width of overlap between adjacent passes
        float cutOverlap                = 1.0f; // mm
        // distance to retract when not retracting to r plane
        float partialRetractDistance    = 1.0f; // mm
        // Geos geometry offset parameters
        GeosBufferParams geosParameters;
    };
*/
    
    
    Function_CutPath(std::string name, Settings* settings, Sketch::Sketcher* sketcher)
        : Function(name, settings, sketcher) {}
        
    bool DrawWindow() override;
    // return value is success
    bool InterpretGCode(const std::vector<Geom::LineString>& inputPaths, std::function<void(std::vector<std::string>&)> cb) override;
    
    std::string HeaderText();
    
    bool IsValidInputs(const Geom::LineString& inputPath);
    bool IsValidInputs(const std::vector<Geom::LineString>& inputPaths);
        
    
private:
    // A selection of geometry which links back to the original sketch element so that it can update if sketch is modified 
    std::vector<Sketch::SketchItem> m_Selected_Points;
    std::vector<Sketch::SketchItem> m_Selected_Elements;
    // A selection of raw points so cannot link back to original polygonised points
    std::vector<Geometry> m_Selected_Polygons;
    // Parameters
    Parameters_CutPath m_Params;    
};



struct Functions 
{
    Functions(Settings* settings, Sketch::Sketcher* sketcher)
        : m_Settings(settings), m_Sketcher(sketcher) 
    {
        // Initialise a function temporarily
        m_Functions.Add<Function_CutPath>("Cut Path##INSERT ID", settings, sketcher);
    }
    
    void DrawWindows();
    
    Settings* m_Settings; 
    Sketch::Sketcher* m_Sketcher;

    Vector_Ptrs<Function> m_Functions;

    
};


} // end namespace Sqeak

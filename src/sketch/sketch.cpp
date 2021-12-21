#include "sketch.h"
using namespace std;
   


    
 /*    
    sketch requirements:
        - loop button instead ofmaking new last = start point
        - multiple functions in 1 drawing (will do all of same tool)
        * functions take a tool type (can take active but not dynamically)
        - distance constraint between points / lines (incremental checkbox on each element?)
    
    - Offset shapes internally to cut out inside
    
    Constraints:
        - Each point can be constrained to another
            - distance contraint
            - vertical/horizontal constraint
    
    
    Sketch
        - Drawing
            - (Functions)            
            - Slot
            - Square
            - Bore
            - Facing Cut
            - QRCode
            - Raw GCode
                - LineString
                    - line elements
                    - arc element
                        - points

*/  

bool DrawRawPoint(std::string name, RawPoint* p) {
    ImGui::Text("(ID#%d) %d", p->ID(), (int)p);
    ImGui::SameLine();
    return ImGui::InputFloat2(va_str("%s##(ID:%d)", name.c_str(), p->ID()).c_str(), &(p->Vec2().x));
}

 
bool RawPoint::DrawImGui() 
{
    bool requiresUpdate = false;
    ImGui::PushID(this);
        if(ImGui::TreeNode(va_str("Raw Point %d", (int)this).c_str())) {
            requiresUpdate |= DrawRawPoint("Raw Point", this);
            
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Element References:"))
            {
                for (size_t i = 0; i < m_ElementRefs.size(); i++) {
                    ImGui::Text("%d", (int)m_ElementRefs[i]->element);
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    ImGui::PopID();
    return requiresUpdate;
}
 
bool Element_Point::DrawImGui() 
{
    bool requiresUpdate = false;
    ImGui::PushID(this);
        if (ImGui::TreeNode(va_str("Point Element %d", (int)this).c_str())) {
            requiresUpdate |= DrawRawPoint("Raw Point", m_Ref_P0->rawPoint);
            ImGui::TreePop();
        }
    ImGui::PopID();
    return requiresUpdate;
}
  

bool Element_Line::DrawImGui() {
    bool requiresUpdate = false;
    ImGui::PushID(this);
        if (ImGui::TreeNode(va_str("Line Element %d", (int)this).c_str())) {
            requiresUpdate |= DrawRawPoint("Raw Point 0", m_Ref_P0->rawPoint);
            requiresUpdate |= DrawRawPoint("Raw Point 1", m_Ref_P1->rawPoint);
            ImGui::TreePop();
        }
    ImGui::PopID();
    return requiresUpdate;
}
bool Element_Arc::DrawImGui() {
    bool requiresUpdate = false; 
    ImGui::PushID(this); 
        if (ImGui::TreeNode(va_str("Arc Element %d", (int)this).c_str())) {
            requiresUpdate |= DrawRawPoint("Raw Point 0", m_Ref_P0->rawPoint); 
            requiresUpdate |= DrawRawPoint("Raw Point 1", m_Ref_P1->rawPoint); 
            requiresUpdate |= DrawRawPoint("Raw Centre ", m_Ref_Centre->rawPoint);
            ImGui::TreePop(); 
        } 
    ImGui::PopID();  
    return requiresUpdate; 
}
 
bool ElementFactory::LineLoop_DrawImGui(LineLoopID id)  
{ 
    LineLoop& lineLoop = LineLoop_GetByID(id);
    
    bool requiresUpdate = false;
    
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Elements"))
    {
        for (size_t i = 0; i < lineLoop.m_Elements.size(); i++) {
            requiresUpdate |= Element_GetByID(lineLoop.m_Elements[i])->DrawImGui();
            ImGui::SameLine();
            if(ImGui::Button(va_str("Delete##%d",i).c_str())) {
                LineLoop_DeleteElement(id, (size_t)i);
                requiresUpdate = true;
            }
        }
        ImGui::TreePop();
    }
    
    return requiresUpdate;
}

bool ElementFactory::RawPoint_DrawImGui() 
{
    bool requiresUpdate = false;
    // set all items open
    if (ImGui::TreeNode("Raw Points"))
    {
        for (size_t i = 0; i < m_Points.Size(); i++) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            requiresUpdate |= m_Points[i]->DrawImGui();
        }
        ImGui::TreePop();
    }
    return requiresUpdate; 
} 
 
void ElementFactory::RefPointToElement_DrawImGui() 
{
    ImGui::PushID(this);
    // set all items open
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("References"))
    {
        for (size_t i = 0; i < m_References.size(); i++) {
            
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode(va_str("Reference %d", (int)m_References[i].get()).c_str()))
            {
                ImGui::Text("Element:   %d", (int)m_References[i]->element);
                ImGui::SameLine();
                ImGui::Text("Raw Point: %d", (int)m_References[i]->rawPoint);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
} 

bool Function_Draw::DrawImGui(ElementFactory& elementFactory, Settings& settings) 
{
    (void)settings;

    bool updateViewer = false;
    
    updateViewer |= ImGui::InputText("Name", &m_Name);
     
    ImGui::Dummy(ImVec2());
     
    updateViewer |= ImGui::Combo("Cut Side", &m_Params.cutSide, "None\0Left\0Right\0Pocket\0\0");
    updateViewer |= ImGui::InputFloat("Finishing Pass", &m_Params.finishingPass);
    updateViewer |= ImGui::InputFloat2("Z Top/Bottom", &m_Params.z[0]);
    
    ImGui::Separator(); 
    
    updateViewer |= elementFactory.LineLoop_DrawImGui(m_LineLoop);
    
    // calls Export GCode and updates viewer
    return updateViewer; 
}
 
bool A_Drawing::DrawImGui(Settings& settings)
{
    
    ImVec2& buttonSize = settings.guiSettings.button[ButtonType::Primary].Size;
    ImVec2& buttonSizeSmall = settings.guiSettings.button[ButtonType::New].Size;
    bool updateViewer = false;
    
    // new function buttons
    ImGui::Text("Add New Function");
    if(ImGui::Button("Draw", buttonSize)) {
        std::unique_ptr<Function> newFunction = std::make_unique<Function_Draw>(m_ElementFactory, "Draw " + to_string(m_FunctionIDCounter++));
        m_ActiveFunctions.Add(move(newFunction));
        updateViewer = true;
    } 
    
    // active functions
    ImGui::Separator();
    ImGui::Text("Active Functions");
    static std::function<std::string(std::unique_ptr<Function>& item)> callback = [](std::unique_ptr<Function>& item) { return item->Name(); };
    if(ImGuiModules::Buttons(m_ActiveFunctions, buttonSize, callback)) {
        updateViewer = true;
    }
    
    ImGui::SameLine();
        
    // remove active drawing button
    if(m_ActiveFunctions.HasItemSelected()) 
    {   // remove drawing
        if(ImGui::Button("-##Remove Drawing", buttonSizeSmall)) {
            m_ActiveFunctions.RemoveCurrent();
            updateViewer = true;
        }
    }
    
    // Current function
    ImGui::Separator();
    ImGui::Text("Current Function");
    // handle the selected active function
    if(m_ActiveFunctions.HasItemSelected()) {
        updateViewer |= m_ActiveFunctions.CurrentItem()->DrawImGui(m_ElementFactory, settings);
    }
    
    // draw the points list
    ImGui::Separator();
    ImGui::Text("Points List");
    updateViewer |= m_ElementFactory.RawPoint_DrawImGui();
    // draw the references
    m_ElementFactory.RefPointToElement_DrawImGui();
    
    
    
    return updateViewer;
}

void Sketch::DrawImGui(GRBL& grbl, Settings& settings) 
{
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
    if (!ImGui::Begin("Sketch", NULL, general_window_flags)) {
        ImGui::End();
        return;  
    }            

    ImVec2& buttonSize = settings.guiSettings.button[ButtonType::Primary].Size;
    //ImVec2& buttonSizeSmall = settings.guiSettings.button[ButtonType::New].Size;
    bool updateViewer = false;
    
    if(!IsActive()) {
        if(ImGui::Button("Sketch", buttonSize)) {
            Activate();
            updateViewer = true;
        }
        ImGui::End();
        return;
    } 
    
    // sketch is active
    if(ImGui::Button("Exit Sketch", buttonSize)) {
        Deactivate();
        ImGui::End();
        return;
    }
     
    ImGui::Separator();
         
           
         
    static std::function<std::string(A_Drawing& item)> cb_GetItemString = [](A_Drawing& item) { return item.Name(); };
    static std::function<A_Drawing(void)> cb_AddNewItem = [&]() { 
        updateViewer = true;
        return A_Drawing("Drawing " + to_string(m_DrawingIDCounter++)); 
    };
    static std::function<bool()> cb_DrawTabImGui = [&]() 
    {  
        assert(m_Drawings.HasItemSelected() && "No item selected in cb_DrawTabImGui");
        // this matches the open tab item
        A_Drawing& drawing = m_Drawings.CurrentItem(); 
        
        // Run, Save & Delete Buttons
        if(ImGui::Button("Run")) { drawing.ActiveFunction_Run(grbl, settings); }
        ImGui::SameLine();
        if(ImGui::Button("Export GCode")) { 
            if(drawing.ActiveFunction_ExportGCode(settings, settings.p.system.saveFileDirectory)) {
                Log::Error("Unable to save file");
            }
        } // TODO overwrite popup (see frames.h :774)
        ImGui::SameLine();
        if(ImGui::Button("Delete")) { 
            drawing.ActiveFunction_Delete();
            updateViewer = true;
        }
        updateViewer |= drawing.DrawImGui(settings);
        return updateViewer;
        
    };
     
    updateViewer |= ImGuiModules::Tabs(m_Drawings, cb_GetItemString, cb_AddNewItem, cb_DrawTabImGui);
    
    // update viewer if setting has changed
    if(updateViewer) {
        ActiveDrawing_UpdateViewer(settings);
    }
    
    ImGui::End();
}   


// Adds a drawing element reference
void RawPoint::AddReference(Ref_PointToElement* ref) { 
    // prevent duplication of references
    for(Ref_PointToElement* elementRef : m_ElementRefs) {
        if(elementRef == ref) return;
    }
    m_ElementRefs.push_back(ref); 
}
// Removes a drawing element reference (returns true if no references left)
bool RawPoint::RemoveReference(Ref_PointToElement* ref) { 
    
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Removing reference of RawPoint 1/%d references\tRawPoint: %d\tElement: %d", m_ElementRefs.size(), (int)ref->rawPoint, (int)ref->element);
    // find ref inside m_ElementRefs
    for (size_t i = 0; i < m_ElementRefs.size(); i++) {
        if(m_ElementRefs[i] == ref) {
            m_ElementRefs.erase(m_ElementRefs.begin() + i); 
            return m_ElementRefs.empty();
        }
    }
    assert(0 && "The reference somehow doesn't exist in RawPoint...");
    return false; // never reaches
}

 // Removes a rawpoint reference (returns true if no references left)
bool Element_Point::RemoveReference(Ref_PointToElement* ref) 
{ 
    if(m_Ref_P0 == ref) { 
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Removing reference of Element_Point P0\tRawPoint: %d\tElement: %d", (int)ref->rawPoint, (int)ref->element);
        m_Ref_P0 = nullptr; 
    }
    else { assert(0 && "The reference somehow doesn't exist in element Point...");}
    
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Element_Point has references\tP0: %d", (int)m_Ref_P0); 
    return (m_Ref_P0 == nullptr) ;
}

bool Element_Line::RemoveReference(Ref_PointToElement* ref) 
{ 
    if(m_Ref_P0 == ref) { 
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Removing reference of Element_Line P0\tRawPoint: %d\tElement: %d", (int)ref->rawPoint, (int)ref->element);
        m_Ref_P0 = nullptr; 
    }
    else if(m_Ref_P1 == ref) { 
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Removing reference of Element_Line P1\tRawPoint: %d\tElement: %d", (int)ref->rawPoint, (int)ref->element); 
        m_Ref_P1 = nullptr; 
    }
    else { assert(0 && "The reference somehow doesn't exist in element Line...");}
    
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Element_Line has references\tP0: %d\tP1: %d", (int)m_Ref_P0, (int)m_Ref_P1); 
    
    return (m_Ref_P0 == nullptr) && (m_Ref_P1 == nullptr);
}

bool Element_Arc::RemoveReference(Ref_PointToElement* ref) 
{ 
    if(m_Ref_P0 == ref) { 
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Removing reference of Element_Arc P0\tRawPoint: %d\tElement: %d", (int)ref->rawPoint, (int)ref->element);
        m_Ref_P0 = nullptr; 
    }
    else if(m_Ref_P1 == ref) { 
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Removing reference of Element_Arc P1\tRawPoint: %d\tElement: %d", (int)ref->rawPoint, (int)ref->element);
        m_Ref_P1 = nullptr; 
    }
    else if(m_Ref_Centre == ref) { 
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Removing reference of Element_Arc Centre\tRawPoint: %d\tElement: %d", (int)ref->rawPoint, (int)ref->element);
        m_Ref_Centre = nullptr; 
    }
    else { assert(0 && "The reference somehow doesn't exist in element Line...");}
    
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Element_Arc has references\tP0: %d\tP1: %d\tCentre: %d", (int)m_Ref_P0, (int)m_Ref_P1, (int)m_Ref_Centre); 
    return (m_Ref_P0 == nullptr) && (m_Ref_P1 == nullptr) && (m_Ref_Centre == nullptr);
}
            
// removes references from element and rawpoint within ref, deletes the element/rawpoint if they have no references left
void ElementFactory::Element_BreakReference(Ref_PointToElement* ref) 
{
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Breaking Reference %d\t(RawPoint: %d\tElement: %d)", (int)ref, (int)ref->rawPoint, (int)ref->element);
    // remove the reference from point. if no references left on point, delete it
    RawPoint* point = ref->rawPoint;
    if(point->RemoveReference(ref)) {
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Deleting RawPoint %d", (int)point);
        RawPoint_Delete(point);
    }
    // remove the reference from element. if no references left on element, delete it
    A_DrawingElement* element = ref->element;
    if(element->RemoveReference(ref)) {
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Deleting Element %d", (int)element);
        DrawingElement_Delete(element);
    }
}

// removes references from element and rawpoint within ref, deletes the element/rawpoint if they have no references left
void ElementFactory::Element_ReplaceRawPointReference(Ref_PointToElement* ref, RawPoint* pNew) 
{
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Replacing RawPoint Reference");
    // remove the reference from point. if no references left on point, delete it
    RawPoint* point = ref->rawPoint;
    if(point->RemoveReference(ref)) {
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Deleting RawPoint %d", (int)point);
        RawPoint_Delete(point);
    }
    // set reference rawpoint to pNew 
    ref->SetReferences(pNew);
    // add reference to pNew
    pNew->AddReference(ref);
    Log::Debug(DEBUG_SKETCH_REFERENCES, "RawPoint has been replaced in Reference:\t%d\t(RawPoint: %d\tElement: %d)", (int)ref, (int)ref->rawPoint, (int)ref->element);
}

// Creates a basic Line Loop
LineLoopID ElementFactory::LineLoop_Create() 
{
    m_LineLoops.push_back(move(std::make_unique<LineLoop>(m_LineLoopIDCounter++)));
    return m_LineLoops.back()->ID();
}
// Creates a basic Line Loop
LineLoopID ElementFactory::LineLoop_Create(const glm::vec2& startPoint) 
{
    ElementID pointElement = Element_CreatePoint(startPoint);
    m_LineLoops.push_back(move(std::make_unique<LineLoop>(m_LineLoopIDCounter++, pointElement)));
    return m_LineLoops.back()->ID();
}
// Create a lineloop of just lines from vector
LineLoopID ElementFactory::LineLoop_Create(const std::vector<glm::vec2>& points) 
{
    assert(points.size() >= 2 && "Vector requires at least 2 points");
    // create the line loop with a start point 
    ElementID pointElement = Element_CreatePoint(points[0]);
    m_LineLoops.push_back(move(std::make_unique<LineLoop>(m_LineLoopIDCounter++, pointElement)));
    // add points to line loop
    for (size_t i = 1; i < points.size(); i++) {
        LineLoop_AddLine(m_LineLoops.back()->ID(), points[i]);
    }
    return m_LineLoops.back()->ID();
}
// Set start point
void ElementFactory::LineLoop_AddStartPoint(LineLoopID lineLoopID, const glm::vec2& startPoint) 
{
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID);
    ElementID pointElementID = Element_CreatePoint(startPoint);
    lineLoop.SetStartPoint(pointElementID);
}
// Adds a line to the Line Loop
void ElementFactory::LineLoop_AddLine(LineLoopID lineLoopID, const glm::vec2& p1) 
{
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID);
    if(lineLoop.IsEmpty()) {
        LineLoop_AddStartPoint(lineLoopID, p1); 
        return;
    } 
    ElementID lineElementID = Element_CreateLine(LineLoop_LastPoint(lineLoopID), p1);
    lineLoop.AddLine(lineElementID);
}

// Adds an arc to the Line Loop from centre point
void ElementFactory::LineLoop_AddArc(LineLoopID lineLoopID, const glm::vec2& p1, int direction, const glm::vec2& centre) 
{
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID);
    assert(!lineLoop.IsEmpty() && "Line loop is empty");
    ElementID arcElementID = Element_CreateArc(LineLoop_LastPoint(lineLoopID), p1, direction, centre);
    lineLoop.AddArc(arcElementID);
}
// Adds an arc to the Line Loop from radius
void ElementFactory::LineLoop_AddArc(LineLoopID lineLoopID, const glm::vec2& p1, int direction, float radius) 
{
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID); 
    assert(!lineLoop.IsEmpty() && "Line loop is empty"); 
    // calculate centre from radius, then pass on
    RawPoint* p0 = LineLoop_LastPoint(lineLoopID); 
    point2D centre = Geom::ArcCentreFromRadius(point2D(p0->X(), p0->Y()), point2D(p1.x, p1.y), radius, direction);
    LineLoop_AddArc(lineLoopID, p1, direction, glm::vec2(centre.x, centre.y));
}
// Deletes Last element in Line Loop
void ElementFactory::LineLoop_DeleteLast(LineLoopID lineLoopID) 
{
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID);
    assert(!lineLoop.IsEmpty() && "Line loop is empty");
    LineLoop_DeleteElement(lineLoopID, lineLoop.Size()-1);
}

    
int Function::InterpretGCode(Settings& settings, ElementFactory& elementFactory, std::function<int(std::vector<std::string> gcode)> callback)
{
    // error check
    if(settings.p.tools.IsToolAndMaterialSelected())
        return -1;
    // export gcode
    std::pair<bool, vector<string>> gcodes = ExportGCode(settings, elementFactory);
    // check if gcode could be read
    if(!gcodes.first) {
        Log::Error("Could not interpret this GCode");
        return -1;
    }
    // gcode was interpretted successfully, call function provided
    return callback(gcodes.second);
};

Function_Draw::Function_Draw(ElementFactory& elementFactory, std::string name) : Function(name) 
{
    m_LineLoop = elementFactory.LineLoop_Create();
}

bool Function_Draw::IsValidInputs(Settings& settings, ElementFactory& elementFactory) 
{
    // check tool and material is selected
    if(settings.p.tools.IsToolAndMaterialSelected())
        return false;
    size_t size = elementFactory.LineLoop_Size(m_LineLoop);
    // start and end point
    if(size == 0) {
        Log::Error("At least 1 drawing element is required");
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

std::string Function_Draw::HeaderText(Settings& settings, ElementFactory& elementFactory) 
{
    Function_Draw_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();    
    
    // write header
    std::ostringstream stream;
    stream << "; Function: " << m_Name << '\n';
    stream << "; \tBetween: " << p.z[0] << " and " << p.z[1] << '\n';
    stream << "; \tPoints:" << '\n';
    
    // TODO add header text
    (void)elementFactory;
   // m_Params.drawing.AddElementsHeaderText(stream);
    
    if(p.cutSide == CompensateCutter::None) stream << "; \tCompensate: None\n";
    if(p.cutSide == CompensateCutter::Left) stream << "; \tCompensate: Left\n";
    if(p.cutSide == CompensateCutter::Right) stream << "; \tCompensate: Right\n";
    if(p.cutSide == CompensateCutter::Pocket) stream << "; \tCompensate: Pocket\n";
    
    if(p.finishingPass) stream << "; Finishing Pass: " << p.finishingPass << '\n';
    
    stream << "; Tool: " << tool.Name << '\n';
    stream << "; \tDiameter: " << tool.Diameter << '\n';
    stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
    stream << '\n';
    
    return stream.str();
}
    
std::pair<bool, std::vector<std::string>> Function_Draw::ExportGCode(Settings& settings, ElementFactory& elementFactory) 
{
    auto err = make_pair(false, std::vector<std::string>());;
    // error check
    if(!IsValidInputs(settings, elementFactory)) {
        return err;
    }
    
    //Draw_Parameters& p = m_Params;
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
    
    // initialise
    FunctionGCodes gcodes;
    gcodes.Add(HeaderText(settings, elementFactory));
    gcodes.InitCommands(toolData.speed);
     
    // define offset path parameters
    // 0 = no compensation, 1 = compensate left / pocket, -1 = compensate right
    int cutSide;
    if(m_Params.cutSide == CompensateCutter::None)
        cutSide = 0;
    if(m_Params.cutSide == CompensateCutter::Right)
        cutSide = -1;  
    if(m_Params.cutSide == CompensateCutter::Left || m_Params.cutSide == CompensateCutter::Pocket)
        cutSide = 1;
       
    float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
    float offset = fabsf(toolRadius) + m_Params.finishingPass;
    int arcSegments = settings.p.pathCutter.QuadrantSegments;  // define cut path parameters & offset path
    Geos geos; 
    // number of line segments per 90 degrees of arc
     
    // make a path of line segments (arcs are converted to many line segments)    
    std::vector<glm::vec2> path = elementFactory.LineLoop_PointsList(m_LineLoop, arcSegments);
     
    //lineLoop.Path(arcSegments);
    // calculate offset
    FunctionGCodes::CutPathParams pathParams;

    // populate parameters
    pathParams.z0 = m_Params.z[0];
    pathParams.z1 = m_Params.z[1];
    pathParams.cutDepth = toolData.cutDepth;
    pathParams.feedPlunge = toolData.feedPlunge;
    pathParams.feedCutting = toolData.feedCutting;
    pathParams.isLoop = elementFactory.LineLoop_IsLoop(m_LineLoop);
    
    GEOSType geosType;
    
    
    
    // populate offset path
    if(pathParams.isLoop) 
    {  
        geosType = GEOSType::Polygon;
        #define MAX_ITERATIONS 500
        #define CUT_OVERLAP 1 // mm
        // dont treat as loop as we are putting all offsets on same vector
        pathParams.isLoop = false;
        // bore out the internal of a shape
        // cut the shape repeatly whilst increasing the offset until the entire shape is gone
        int nIterations = 0;
        
        while(1) {
            // add offset to points list
            if(!geos.offset_AddToVector(GEOSType::Polygon, pathParams.points, path, cutSide * offset, arcSegments)) {
                // err if failed on first attempt, break otherwise as we have finished boring
                if(nIterations) { break; }
                else { return err; }
            }
                
            // repeat if we're boring out entire shape
            if(m_Params.cutSide != CompensateCutter::Pocket) { break; }
            if(++nIterations > MAX_ITERATIONS) { 
                Log::Error("Iterations is maxed out"); 
                break;
            }
            // move tool in 
            offset += 2.0f * fabsf(toolRadius) - CUT_OVERLAP;
        };
        // add gcodes for path at depths
        if(gcodes.CutPath(settings, pathParams)) {
            return err;
        }
        
    } 
    else 
    {
        geosType = GEOSType::Line;
        if(m_Params.cutSide == CompensateCutter::Pocket) { 
            Log::Error("Pocket must be a loop");
            return err; 
        }
        if(!geos.offset_AddToVector(GEOSType::Line, pathParams.points, path, cutSide * offset, arcSegments)) {
            return err;
        }
        // add gcodes for path at depths
        if(gcodes.CutPath(settings, pathParams)) {
            return err;
        }
    }
    
    // add gcodes for finishing pass
    if(m_Params.finishingPass) {
        // make a new set of parameters where z doesnt chance
        FunctionGCodes::CutPathParams finishPassParams = pathParams;
        // clear the points vector
        finishPassParams.points.clear();
        finishPassParams.z0 = finishPassParams.z1;
        if(!geos.offset_AddToVector(geosType, finishPassParams.points, path, cutSide * toolRadius, arcSegments)) {
            return err; // not sure why it would fail
        }
        // add gcodes for path at depths
        if(gcodes.CutPath(settings, finishPassParams)) {
            return err;
        }
    }
    // move to zPlane, end program
    gcodes.EndCommands();
    
    // draw path and offset path in viewer
    //Event<Event_DisplayShapeOffset>::Dispatch( { path, pathParams.points, pathParams.isLoop } );
    
    return make_pair(true, gcodes.Get());
}




bool Function_Draw::HandleEvents(Settings& settings, InputEvent& inputEvent, ElementFactory& elementFactory)
{
    bool updateViewer = true;
    // handle mouse events
    if(inputEvent.mouseHasChanged) 
    {
        if(inputEvent.mouse.Action == GLFW_PRESS && inputEvent.mouse.Button == GLFW_MOUSE_BUTTON_LEFT) 
        {
            cout << "CLICK" << endl;
            glm::vec2 pointRounded = settings.p.viewer.cursor.SnapCursor(inputEvent.screenCoords);
            elementFactory.LineLoop_AddLine(m_LineLoop, pointRounded);
            updateViewer = true;
        }
    }
    
    // handle keyboard event
    if(inputEvent.keyboardHasChanged) {
        updateViewer = true;
    }
    
    // reset the events
    inputEvent.Reset();
    return updateViewer;
} 

void Function_Draw::UpdateViewer(Settings& settings, ElementFactory& elementFactory, std::vector<DynamicBuffer::DynamicVertexList>* viewerLineLists, bool isDisabled)
{
    DynamicBuffer::DynamicVertexList lineloop_Lines;
    lineloop_Lines.position = elementFactory.LineLoop_PointsList(m_LineLoop, settings.p.pathCutter.QuadrantSegments);
    
    lineloop_Lines.colour = (isDisabled) ? settings.p.viewer.line.colourDisabled : settings.p.viewer.line.colour; 
    
    viewerLineLists->push_back(std::move(lineloop_Lines));
}
            
bool A_Drawing::HandleEvents(Settings& settings, InputEvent& inputEvent)
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        return false;
    }
    // handle mouse and keyboard events and return if update viewer needed
    return m_ActiveFunctions.CurrentItem()->HandleEvents(settings, inputEvent, m_ElementFactory);
}


void A_Drawing::ActiveFunction_Run(GRBL& grbl, Settings& settings) 
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        Log::Error("No active function selected");
        return;
    }
    // build gcode of active function and run it
    m_ActiveFunctions.CurrentItem()->InterpretGCode(settings, m_ElementFactory, [&](auto gcodes){
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
// export the active function as gcode
int A_Drawing::ActiveFunction_ExportGCode(Settings& settings, std::string saveFileDirectory) 
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        Log::Error("No active function selected");
        return -1; 
    }
    // make filepath for new GCode file
    std::string filepath = File::CombineDirPath(saveFileDirectory, m_ActiveFunctions.CurrentItem()->Name() + ".nc"); 
    // build gcode of active function and export it to file
    return m_ActiveFunctions.CurrentItem()->InterpretGCode(settings, m_ElementFactory, [&](auto gcodes){
        File::WriteArray(filepath, gcodes);
        return 0;
    });
}

// update viewer
void A_Drawing::ActiveFunction_UpdateViewer(Settings& settings, std::vector<DynamicBuffer::DynamicVertexList>* viewerLineLists)
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        Log::Error("No active function selected");
        return; 
    }
    std::cout << "Updating Viewer - Draw Function" << std::endl;
    // build gcode of active function and update viewer
    for (size_t i = 0; i < m_ActiveFunctions.Size(); i++) {
        bool isCurrentItem = (m_ActiveFunctions.CurrentIndex() == (int)i);
        m_ActiveFunctions[i]->UpdateViewer(settings, m_ElementFactory, viewerLineLists, !isCurrentItem);
    }
    // add gcode path to viewer
    m_ActiveFunctions.CurrentItem()->InterpretGCode(settings, m_ElementFactory, [&](auto gcodes){
        Event<Event_Update3DModelFromVector>::Dispatch({ gcodes }); 
        return 0;
    });
}   
     
// update viewer
void A_Drawing::RawPoints_UpdateViewer(Settings& settings, std::vector<DynamicBuffer::DynamicVertexList>* viewerPointLists)
{
    // add raw points to m_ViewerPointLists 
    DynamicBuffer::DynamicVertexList rawPoints;
    rawPoints.position = m_ElementFactory.RawPoint_PointsList();
    rawPoints.colour = settings.p.viewer.point.colour;
    viewerPointLists->push_back(std::move(rawPoints));
}

    

// delete active function
void A_Drawing::ActiveFunction_Delete()
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        return; 
    }
    // remove active function
    m_ActiveFunctions.RemoveCurrent();
}


Sketch::Sketch() 
{
}

void Sketch::ActiveDrawing_UpdateViewer(Settings& settings)
{            
    // Line List
    m_ViewerLineLists.clear();
    // add lines to m_ViewerLineLists
    if(m_Drawings.HasItemSelected()) {
        m_Drawings.CurrentItem().ActiveFunction_UpdateViewer(settings, &m_ViewerLineLists);
    }
    // dispatch m_ViewerLineLists
    Event<Event_Viewer_AddLineLists>::Dispatch( { &m_ViewerLineLists } );
    
    // Points List
    m_ViewerPointLists.clear();
    // add points to m_ViewerPointLists
    if(m_Drawings.HasItemSelected()) {
        m_Drawings.CurrentItem().RawPoints_UpdateViewer(settings, &m_ViewerPointLists);
    }
    // dispatch m_ViewerPointLists
    Event<Event_Viewer_AddPointLists>::Dispatch( { &m_ViewerPointLists } );
}
    

void Sketch::HandleEvents(Settings& settings, InputEvent& inputEvent)
{
    if(!IsActive() || !m_Drawings.HasItemSelected()) { 
        return;
    }
    // get active drawing
    if(m_Drawings.CurrentItem().HandleEvents(settings, inputEvent)) {
        // update viewer on mouse click
        ActiveDrawing_UpdateViewer(settings);
    }
}

void Sketch::Activate()
{
    m_IsActive = true;
    Event<Event_Set2DMode>::Dispatch( { true } );
}

void Sketch::Deactivate()
{
    m_IsActive = false;
    Event<Event_Set2DMode>::Dispatch( { false } );
}

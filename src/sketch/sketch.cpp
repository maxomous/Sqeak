#include "sketch.h"
using namespace std; 
using namespace sketch;
    
      
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

std::string NameAndID(const std::string& name, RawPoint* p) {
    return va_str("%s\t%d\t(#%d)", name.c_str(), (int)p, p->ID());
}
std::string NameAndID(const std::string& name, Element* e) {
    return va_str("%s\t%d\t(#%d)", name.c_str(), (int)e, e->ID());
}
std::string NameAndID(const std::string& name, ElementFactory::LineLoop* l) {
    return va_str("%s\t%d\t(#%d)", name.c_str(), (int)l, l->ID());
}
std::string NameAndID(const std::string& name, Ref_PointToElement* r) {
    return va_str("%s\t%d\t(e%d & rp%d)", name.c_str(), (int)r, r->element->ID(), r->rawPoint->ID());
}


bool DrawRawPointPosition(Settings& settings, RawPoint* p) {
    if(ImGui::InputFloat2(va_str("##(ID:%d)", p->ID()).c_str(), &(p->Vec2().x))) {
        settings.SetUpdateFlag(ViewerUpdate::Full);
        return true;
    }
    return false;
}

bool DrawRawPoint(Settings& settings, const std::string& name, RawPoint* p) {
    ImGui::Text(NameAndID(name, p).c_str());
    ImGui::SameLine();
    return DrawRawPointPosition(settings, p);
}

 
void RawPoint::DrawImGui(Settings& settings) 
{
    ImGui::PushID(this);
        if(ImGui::TreeNode(NameAndID("Raw Point", this).c_str())) {
            DrawRawPointPosition(settings, this);
            
            if (ImGui::TreeNode("Element References:")) {
                for (size_t i = 0; i < m_ElementRefs.size(); i++) {
                    m_ElementRefs[i]->element->DrawImGui(settings);
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    ImGui::PopID();
}
 
void Element_Point::DrawImGui(Settings& settings) 
{
    ImGui::PushID(this);
        if (ImGui::TreeNode(NameAndID("Point Element", this).c_str())) {
            DrawRawPoint(settings, "Raw Point", m_Ref_P0->rawPoint);
            ImGui::TreePop();
        }   
    ImGui::PopID();   
}   
    
       
void Element_Line::DrawImGui(Settings& settings) {
    ImGui::PushID(this);
        if (ImGui::TreeNode(NameAndID("Line Element", this).c_str())) {
            DrawRawPoint(settings, "Raw Point 0", m_Ref_P0->rawPoint);
            DrawRawPoint(settings, "Raw Point 1", m_Ref_P1->rawPoint);
            ImGui::TreePop();
        } 
    ImGui::PopID(); 
}
void Element_Arc::DrawImGui(Settings& settings) {
    ImGui::PushID(this); 
        if (ImGui::TreeNode(NameAndID("Arc Element", this).c_str())) {
            DrawRawPoint(settings, "Raw Point 0", m_Ref_P0->rawPoint); 
            DrawRawPoint(settings, "Raw Point 1", m_Ref_P1->rawPoint); 
            if(DrawRawPoint(settings, "Raw Centre", m_Ref_Centre->rawPoint)) {
                SetCentre(m_Ref_Centre->rawPoint->Vec2());
            }
            if(ImGui::InputFloat("Radius", &m_Radius)) {
                SetRadius(m_Radius);
                settings.SetUpdateFlag(ViewerUpdate::Full);
            }
            if(ImGui::Combo("Direction", &m_DirectionImGui, "Clockwise\0Anticlockwise\0\0")) {
                m_Direction = (m_DirectionImGui == 0) ? CLOCKWISE : ANTICLOCKWISE;
                settings.SetUpdateFlag(ViewerUpdate::Full);
            }
            ImGui::Text("tangent radius: %g", m_TangentRadius);
            ImGui::TreePop(); 
        } 
    ImGui::PopID();   
}
 
void ElementFactory::LineLoop_DrawImGui(Settings& settings, Sketch_LineLoop& sketchLineLoop)  
{ 
    LineLoop& lineLoop = LineLoop_GetByID(sketchLineLoop->id);
        
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode(NameAndID("LineLoop", &lineLoop).c_str()))
    {
        for (size_t i = 0; i < lineLoop.m_Elements.size(); i++) {
            Element_GetByID(lineLoop.m_Elements[i]->id)->DrawImGui(settings);
            ImGui::SameLine();
            if(ImGui::Button(va_str("Delete##%d",i).c_str())) {
                LineLoop_DeleteElement(sketchLineLoop->id, (size_t)i);
                settings.SetUpdateFlag(ViewerUpdate::Full);
            } 
        }
        ImGui::TreePop();
    }
}

void ElementFactory::RawPoint_DrawImGui(Settings& settings) 
{
    // set all items open
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Raw Points"))
    {
        for (size_t i = 0; i < m_Points.Size(); i++) {
            m_Points[i]->DrawImGui(settings);
        }
        ImGui::TreePop();
    }
} 
 
void ElementFactory::RefPointToElement_DrawImGui() 
{
    ImGui::PushID(this);
    // set all items open
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("References"))
    {
        for (size_t i = 0; i < m_References.size(); i++) {
            
            if (ImGui::TreeNode(NameAndID("Reference", m_References[i].get()).c_str()))
            {
                ImGui::Text(NameAndID("Element", m_References[i]->element).c_str());
                ImGui::SameLine();
                ImGui::Text(NameAndID("Raw Point", m_References[i]->rawPoint).c_str());
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
} 

void Function_Draw::DrawImGui_Tools(Settings& settings) 
{
    if(ImGuiCustomModules::ImageButtonWithText_Function(settings, "Select", settings.guiSettings.img_Sketch_Select, m_ActiveCommand == Command::Select)) { m_ActiveCommand = Command::Select; }
    ImGui::SameLine();
    if(ImGuiCustomModules::ImageButtonWithText_Function(settings, "Line", settings.guiSettings.img_Sketch_Line, m_ActiveCommand == Command::Line)) { m_ActiveCommand = Command::Line; }
    ImGui::SameLine();
    if(ImGuiCustomModules::ImageButtonWithText_Function(settings, "Arc", settings.guiSettings.img_Sketch_Arc, m_ActiveCommand == Command::Arc)) { m_ActiveCommand = Command::Arc; }
     
}

void Function_Draw::DrawImGui(ElementFactory& elementFactory, Settings& settings) 
{
    (void)settings;

    bool updateViewer = false;
    
    updateViewer |= ImGui::InputText("Name", &m_Name);
     
    ImGui::Dummy(ImVec2());
          
    updateViewer |= ImGui::Combo("Cut Side", &m_Params.cutSide, "None\0Left\0Right\0Pocket\0\0");
    updateViewer |= ImGui::InputFloat("Finishing Pass", &m_Params.finishingPass);
    updateViewer |= ImGui::InputFloat2("Z Top/Bottom", &m_Params.z[0]);
    
    updateViewer |= ImGui::InputInt("Quadrant Segments", &settings.p.pathCutter.geosParameters.QuadrantSegments);

    static int imgui_CapStyle = settings.p.pathCutter.geosParameters.CapStyle - 1;
    if(ImGui::Combo("Cap Style", &imgui_CapStyle, "Round\0Flat\0Square\0\0")) {
        settings.p.pathCutter.geosParameters.CapStyle = imgui_CapStyle + 1;
        updateViewer = true;
    }
    static int imgui_JoinStyle = settings.p.pathCutter.geosParameters.JoinStyle - 1;
    if(ImGui::Combo("Join Style", &imgui_JoinStyle, "Round\0Mitre\0Bevel\0\0")) {
        settings.p.pathCutter.geosParameters.JoinStyle = imgui_JoinStyle + 1;
        updateViewer = true;
    }
    
    // set viewer update flag
    if(updateViewer) { settings.SetUpdateFlag(ViewerUpdate::Full); }
    
    ImGui::Separator(); 
    
    elementFactory.LineLoop_DrawImGui(settings, m_LineLoop);
    
    
}
 


void DrawImGui_PathCutterParameters(Settings& settings) 
{
    ParametersList::PathCutter& pathCutter = settings.p.pathCutter;
    
    ImGui::InputFloat("Cut Overlap", &pathCutter.CutOverlap);
    ImGui::InputInt("Quadrant Segments", &pathCutter.geosParameters.QuadrantSegments);

    ImGui::Checkbox("Cut Tabs", &pathCutter.CutTabs);
        
    ImGui::Indent();
        if(pathCutter.CutTabs) {
            ImGui::InputFloat("Tab Spacing", &pathCutter.TabSpacing);
            ImGui::InputFloat("Tab Height",  &pathCutter.TabHeight);
            ImGui::InputFloat("Tab Width",   &pathCutter.TabWidth);
        }
    ImGui::Unindent();
}
std::string A_Drawing::ActiveFunction_Name()
{   
    if(!m_ActiveFunctions.HasItemSelected()) { return ""; }
    return m_ActiveFunctions.CurrentItem()->Name();
}
std::string Sketch::ActiveFunction_Name()
{    
    if(!m_Drawings.HasItemSelected()) { return ""; }
    return m_Drawings.CurrentItem().ActiveFunction_Name();
}
void A_Drawing::ActiveFunction_DrawImGui_Tools(Settings& settings)
{    
    // handle the selected active function
    if(m_ActiveFunctions.HasItemSelected()) {
        m_ActiveFunctions.CurrentItem()->DrawImGui_Tools(settings);
    } 
}

void A_Drawing::DrawImGui_Functions(Settings& settings)
{    
    // new function buttons
    if(ImGuiCustomModules::ImageButtonWithText_Function(settings, "Draw", settings.guiSettings.img_Sketch_Draw)) {    
        std::unique_ptr<Function> newFunction = std::make_unique<Function_Draw>(m_ElementFactory, "Draw " + to_string(m_FunctionIDCounter++));
        m_ActiveFunctions.Add(move(newFunction));
        settings.SetUpdateFlag(ViewerUpdate::Full);
    }
    ImGui::SameLine();
    
    if(ImGuiCustomModules::ImageButtonWithText_Function(settings, "Measure", settings.guiSettings.img_Sketch_Measure)) {    
        std::cout << "Measure" << std::endl;
    }
}   

void A_Drawing::DrawImGui(Settings& settings)
{
    ImVec2& buttonSize = settings.guiSettings.button[ButtonType::FunctionButton].Size;
    ImVec2& buttonSizeSmall = settings.guiSettings.button[ButtonType::New].Size;
    
      
    ImGui::Text("Parameters");
    DrawImGui_PathCutterParameters(settings);
    
    
     // active function buttons
    ImGui::Separator();
    ImGui::Text("Active Functions");
    static std::function<std::string(std::unique_ptr<Function>& item)> callback = [](std::unique_ptr<Function>& item) { return item->Name(); };
    if(ImGuiModules::Buttons(m_ActiveFunctions, buttonSize, callback)) {
        settings.SetUpdateFlag(ViewerUpdate::Full);
    }
    
    ImGui::SameLine();
        
    // remove active Function button
    if(m_ActiveFunctions.HasItemSelected()) 
    {   // remove Function
        if(ImGui::Button("-##Remove Function", buttonSizeSmall)) {
            m_ActiveFunctions.RemoveCurrent();
            settings.SetUpdateFlag(ViewerUpdate::Full);
        }
    }
    
    // Current function
    ImGui::Separator();
    ImGui::Text("Current Function");
    // handle the selected active function
    if(m_ActiveFunctions.HasItemSelected()) {
        m_ActiveFunctions.CurrentItem()->DrawImGui(m_ElementFactory, settings);
    } 
    
    // draw the points list
    ImGui::Separator();
    ImGui::Text("Points List");
    m_ElementFactory.RawPoint_DrawImGui(settings);
    // draw the references
    m_ElementFactory.RefPointToElement_DrawImGui();
}
    
    
void Sketch::ActiveFunction_DrawImGui_Tools(Settings& settings) 
{
    if(m_Drawings.HasItemSelected()) { 
        m_Drawings.CurrentItem().ActiveFunction_DrawImGui_Tools(settings);
    }
}
void Sketch::ActiveDrawing_DrawImGui_Functions(Settings& settings) 
{
    if(m_Drawings.HasItemSelected()) { 
        m_Drawings.CurrentItem().DrawImGui_Functions(settings);
    }
}

bool Sketch::DrawImGui_StartSketch(Settings& settings) 
{
    //ImVec2& buttonSize = settings.guiSettings.button[ButtonType::Primary].Size;
    //ImGuiModules::CentreItemVerticallyAboutItem(settings.guiSettings.toolbarItemHeight, buttonSize.y);
    
    if(!IsActive()) {
        
        if(ImGuiCustomModules::ImageButtonWithText_Function(settings, "Sketch", settings.guiSettings.img_Sketch)) {  
        //if(ImGui::Button("Sketch", buttonSize)) {
            Activate();
            settings.SetUpdateFlag(ViewerUpdate::Full);
            return true;
        }
    } else {    
        // sketch is active
        if(ImGuiCustomModules::ImageButtonWithText_Function(settings, "Sketch", settings.guiSettings.img_Sketch, true)) {  
        //if(ImGui::Button("Exit Sketch", buttonSize)) {
            Deactivate();
        }
    }
    return false;
}
    



void Sketch::DrawImGui(GRBL& grbl, Settings& settings) 
{
    (void)grbl;
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
    if (!ImGui::Begin("Sketch", NULL, settings.guiSettings.general_window_flags)) {
        ImGui::End(); 
        return;  
    }
     
    if (ImGui::SmallButton("New Drawing")) {
        m_Drawings.Add(A_Drawing("Drawing " + to_string(m_DrawingIDCounter++)));
        settings.SetUpdateFlag(ViewerUpdate::Full);
    }
    
    for(size_t i = 0; i < m_Drawings.Size(); )
    {
        bool isOpen = true;
        if (ImGui::CollapsingHeader(m_Drawings[i].Name().c_str(), &isOpen)) {
            // set the active index to match the open tab
            if(m_Drawings.CurrentIndex() != (int)i) {
                std::cout << "Setting current drawing index" << std::endl;
                m_Drawings.SetCurrentIndex(i);
                settings.SetUpdateFlag(ViewerUpdate::Full);
            }
            // draw the imgui widgets for drawing
            m_Drawings.CurrentItem().DrawImGui(settings);
        }
        if(!isOpen) { // has been closed
            m_Drawings.Remove(i); 
            settings.SetUpdateFlag(ViewerUpdate::Full);
        } else { 
            i++; 
        }                        

    }
    
    // ******* TODO *******: opening 2 drawings causes updating constantly
    
    
    
/*           
    // draw ImGui Tabs
    static std::function<std::string(A_Drawing& item)> cb_GetItemString = [](A_Drawing& item) { 
        return item.Name(); 
    };
    static std::function<A_Drawing(void)> cb_AddNewItem = [&]() { 
        settings.SetUpdateFlag(ViewerUpdate::Full);
        return A_Drawing("Drawing " + to_string(m_DrawingIDCounter++)); 
    };
    static std::function<void()> cb_DrawTabImGui = [&]() {  
         assert(m_Drawings.HasItemSelected() && "No item selected in cb_DrawTabImGui");
         m_Drawings.CurrentItem().DrawImGui(settings);        
    };
     
    if(ImGuiModules::Tabs(m_Drawings, cb_GetItemString, cb_AddNewItem, cb_DrawTabImGui)) {
        settings.SetUpdateFlag(ViewerUpdate::Full);
    }
*/
    ImGui::End();
}   



void RawPoint::GetReferences(std::function<void(Ref_PointToElement*)> callback) 
{ 
    for(Ref_PointToElement* ref : m_ElementRefs) { 
        callback(ref);
    }
} 

void RawPoint::SetThisRawPointFromRefs(const glm::vec2& p) 
{  
    // update element(s) attached to point
    for(Ref_PointToElement* ref : m_ElementRefs) { // GetReferences([&](Ref_PointToElement* ref) { 
        int update = 0; 
        Element* element = ref->element;
                
        // if this point is the same as the active selection point, set position to p
        Ref_PointToElement* p0 = element->P0();
        Ref_PointToElement* p1 = element->P1();
        Ref_PointToElement* pC = element->Centre();
        if(p0) {  
            if(p0->rawPoint->ID() == ID()) {
                // set position to p
                element->SetP0(p);
                update++;
            }
        }
        if(p1) { 
            if(p1->rawPoint->ID() == ID()) {
                // set position to p
                element->SetP1(p);
                update++;
            }
        }
        if(pC) { 
            if(pC->rawPoint->ID() == ID()) {
                // set position to p
                element->SetCentre(p);
                update++;
            }
        }
        // sanity check
        assert((update == 1) && "Update count is not 1");
        // update elements
        element->Update();
    }
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
        RawPoint_DeleteFromFactory(point);
    }
    // remove the reference from element. if no references left on element, delete it
    Element* element = ref->element;
    if(element->RemoveReference(ref)) {
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Deleting Element %d", (int)element);
        Element_DeleteFromFactory(element);
    }
    // delete the reference itself
    Reference_DeleteFromFactory(ref);
}

// removes references from element and rawpoint within ref, deletes the element/rawpoint if they have no references left
void ElementFactory::Element_ReplaceRawPointReference(Ref_PointToElement* ref, RawPoint* pNew) 
{
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Replacing RawPoint Reference");
    // remove the reference from point. if no references left on point, delete it
    RawPoint* point = ref->rawPoint;
    if(point->RemoveReference(ref)) {
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Deleting RawPoint %d", (int)point);
        RawPoint_DeleteFromFactory(point);
    }
    // set reference rawpoint to pNew 
    ref->SetReference(pNew);
    // add reference to pNew
    pNew->AddReference(ref);
    Log::Debug(DEBUG_SKETCH_REFERENCES, "RawPoint has been replaced in Reference:\t%d\t(RawPoint: %d\tElement: %d)", (int)ref, (int)ref->rawPoint, (int)ref->element);
} 

void ElementFactory::Element_DeleteByID(ElementID elementID) {
    for (size_t i = 0; i < m_Elements.size(); i++) {
        if(m_Elements[i]->ID() == elementID) {
            m_Elements.erase(m_Elements.begin() + i);
            return;
        }
    }
    assert(0 && "Couldn't find element ID");
}
Element* ElementFactory::Element_GetByID(ElementID id) 
{
    for (size_t i = 0; i < m_Elements.size(); i++) {
        if(m_Elements[i]->ID() == id) {
            return m_Elements[i].get();
        }
    }
    assert(0 && "Couldn't find element ID");
    return nullptr; // never reaches
} 
// should not be called with nullptr!
RawPoint* ElementFactory::RawPoint_Create(glm::vec2 p, Ref_PointToElement* ref) {
    std::unique_ptr<RawPoint> rawPoint = std::make_unique<RawPoint>(m_PointIDCounter++, p);
    if(ref) { rawPoint->AddReference(ref); }
    m_Points.Add(move(rawPoint));
    return m_Points[m_Points.Size()-1].get();
}   

RawPoint* ElementFactory::RawPoint_GetByID(RawPointID pointID) {
    for (size_t i = 0; i < m_Points.Size(); i++) {
        if(m_Points[i]->ID() == pointID) {
            return m_Points[i].get();
        }
    }
    assert(0 && "Couldn't find point ID");
    return nullptr; // never reaches
}
RawPoint* ElementFactory::RawPoint_GetByPosition(glm::vec2 p, float tolerance) {
    std::cout << "tolerance: " << tolerance << std::endl;
    std::vector<std::pair<size_t, float>> pointsUnderCursor;
    glm::vec2 tol = { tolerance, tolerance };
    for (size_t i = 0; i < m_Points.Size(); i++) {
        std::cout << "point > cursor-tolerance: " << (m_Points[i]->Vec2() > (p-tol)) << std::endl;
        if((m_Points[i]->Vec2() > (p-tol)) && (m_Points[i]->Vec2() < (p+tol))) {
            glm::vec2 dif = m_Points[i]->Vec2() - p;
            pointsUnderCursor.push_back(make_pair(i, hypot(dif)));
            //return m_Points[i].get();
        }
    }
    // no point under cursor
    if(!pointsUnderCursor.size()) { return nullptr; }
    
    size_t closestPoint;
    float  minDistance;
    // find closest point
    for(size_t i = 0; i < pointsUnderCursor.size(); i++) {
        auto& [index, distance] = pointsUnderCursor[i];
        if(distance < minDistance || i == 0) {
            closestPoint = i;
            minDistance = distance;
        }
    }
    size_t pIndex = pointsUnderCursor[closestPoint].first;
    return m_Points[pIndex].get();
}
void ElementFactory::RawPoint_DeleteFromFactory(RawPoint* point) {
    for (size_t i = 0; i < m_Points.Size(); i++) {
        if(m_Points[i].get() == point) {
            if(m_ActiveSelection == point) m_ActiveSelection = nullptr;
            m_Points.Remove(i);
            return;
        }
    }
    assert(0 && "Couldn't find point to delete");
}

void ElementFactory::Element_DeleteFromFactory(Element* element) {
    for (size_t i = 0; i < m_Elements.size(); i++) {
        if(m_Elements[i].get() == element) {
            m_Elements.erase(m_Elements.begin() + i);
            return;
        }
    }
    assert(0 && "Couldn't find element to delete");
}

void ElementFactory::LineLoop_DeleteFromFactory(LineLoopID id) {
    for (size_t i = 0; i < m_LineLoops.size(); i++) {
        if(m_LineLoops[i]->ID() == id) {
            m_LineLoops.erase(m_LineLoops.begin() + i);
            return;
        }
    }
    assert(0 && "Couldn't find lineLoop to delete");
}

void ElementFactory::Reference_DeleteFromFactory(Ref_PointToElement* ref) {
    for (size_t i = 0; i < m_References.size(); i++) {
        if(m_References[i].get() == ref) {
            m_References.erase(m_References.begin() + i);
            return;
        }
    }
    assert(0 && "Couldn't find reference to delete");
} 
 
// Creates a basic Line Loop 
ElementFactory::Sketch_LineLoop ElementFactory::LineLoop_Create() 
{
    m_LineLoops.push_back(move(std::make_unique<LineLoop>(m_LineLoopIDCounter++)));
    return make_unique<Sketch_LineLoop_Identifier>(m_LineLoops.back()->ID(), this);
}
// Set start point  
void ElementFactory::LineLoop_SetStartPoint(LineLoop& lineLoop, const glm::vec2& startPoint) 
{
    Sketch_Element pointElement = Element_CreatePoint(startPoint);
    LineLoop_SetStartPoint(lineLoop, move(pointElement)); 
} 
// Set start point
void ElementFactory::LineLoop_SetStartPoint(LineLoop& lineLoop, Sketch_Element pointElement) 
{
    if(!lineLoop.m_Elements.size()) {
        lineLoop.m_Elements.push_back(move(pointElement));
    } else {
        lineLoop.m_Elements[0] = move(pointElement);
    }
}

// Adds a line to the Line Loop
void ElementFactory::LineLoop_AddLine(ElementFactory::Sketch_LineLoop& sketchLineLoop, const glm::vec2& p1) 
{
    LineLoopID lineLoopID = sketchLineLoop->id;
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID); 
    if(lineLoop.IsEmpty()) { LineLoop_SetStartPoint(lineLoop, p1);  return; } 
    
    Sketch_Element lineElement = Element_CreateLine(LineLoop_LastPoint(lineLoopID), p1);
    lineLoop.m_Elements.push_back(move(lineElement));
} 

// Adds an arc to the Line Loop from centre point
void ElementFactory::LineLoop_AddArc(ElementFactory::Sketch_LineLoop& sketchLineLoop, const glm::vec2& p1, int direction, const glm::vec2& centre) 
{
    LineLoopID lineLoopID = sketchLineLoop->id;
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID);
    if(lineLoop.IsEmpty()) { LineLoop_SetStartPoint(lineLoop, p1);  return; } 
    
    Sketch_Element arcElement = Element_CreateArc(LineLoop_LastPoint(lineLoopID), p1, direction, centre);
    lineLoop.m_Elements.push_back(move(arcElement));
    
}
void ElementFactory::LineLoop_AddArc(ElementFactory::Sketch_LineLoop& sketchLineLoop, const glm::vec2& p1, int direction) 
{
    LineLoopID lineLoopID = sketchLineLoop->id;
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID); 
    if(lineLoop.IsEmpty()) { LineLoop_SetStartPoint(lineLoop, p1);  return; } 
    
    glm::vec2& p0 = LineLoop_LastPoint(sketchLineLoop->id)->Vec2();
    glm::vec2 centre = (p0 + p1) / 2.0f;
    LineLoop_AddArc(sketchLineLoop, p1, direction, centre);
}

// Adds an arc to the Line Loop from radius
void ElementFactory::LineLoop_AddArc(ElementFactory::Sketch_LineLoop& sketchLineLoop, const glm::vec2& p1, int direction, float radius) 
{    
    LineLoopID lineLoopID = sketchLineLoop->id;
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID);
    if(lineLoop.IsEmpty()) { LineLoop_SetStartPoint(lineLoop, p1);  return; } 
    // calculate centre from radius, then pass on
    RawPoint* p0 = LineLoop_LastPoint(lineLoopID); 
    point2D centre = Geom::ArcCentreFromRadius(point2D(p0->X(), p0->Y()), point2D(p1.x, p1.y), radius, direction);
    LineLoop_AddArc(sketchLineLoop, p1, direction, glm::vec2(centre.x, centre.y));
} 
// returns 0 on success 
int Function::InterpretGCode(Settings& settings, ElementFactory& elementFactory, std::function<int(std::vector<std::string> gcode)> callback)
{
    // get gcodes and and if successful, call function provided
    if(auto gcodes = InterpretGCode(settings, elementFactory)) {
        return callback(move(*gcodes));        
    }
    return -1;
};

// bool is success
std::optional<std::vector<std::string>> Function::InterpretGCode(Settings& settings, ElementFactory& elementFactory)
{
    // error check
    if(settings.p.tools.IsToolAndMaterialSelected())
        return {};
    // export gcode
    if(auto gcodes = move(ExportGCode(settings, elementFactory)))
        return move(gcodes);
    // error
    Log::Error("Could not interpret this GCode");
    return {};
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
    
    if(m_Params.cutSide == CompensateCutter::Pocket && !elementFactory.LineLoop_IsLoop(m_LineLoop)) {
        Log::Error("Pocket requires start and end points to be identica");
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
    
// TODO: can we get rid of pathParams.isLoop?
    
std::optional<std::vector<std::string>> Function_Draw::ExportGCode(Settings& settings, ElementFactory& elementFactory)  
{
    // error check  
    if(!IsValidInputs(settings, elementFactory)) {
        return {};  
    }  
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
     
    // initialise 
    GCodeBuilder gcodes;
    gcodes.Add(HeaderText(settings, elementFactory));
    gcodes.InitCommands(toolData.speed);
     
    // define offset path parameters:  0 = no compensation, 1 = compensate left / pocket, -1 = compensate right
    int cutSide = GetCutSide((CompensateCutter)m_Params.cutSide);
    float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
    float offsetDistance = fabsf(toolRadius) + m_Params.finishingPass;
    // define geos parameters
 	GeosBufferParams& geosParameters = settings.p.pathCutter.geosParameters;
    // calculate offset
    GCodeBuilder::CutPathParams pathParams;
    // populate parameters
    pathParams.z0 = m_Params.z[0]; 
    pathParams.z1 = m_Params.z[1];
    pathParams.cutDepth = toolData.cutDepth; 
    pathParams.feedPlunge = toolData.feedPlunge; 
    pathParams.feedCutting = toolData.feedCutting; 
    pathParams.isLoop = elementFactory.LineLoop_IsLoop(m_LineLoop);
    // make a path of line segments (arcs are converted to many line segments)    
    Geos::LineString inputPath = elementFactory.LineLoop_PointsList(m_LineLoop, geosParameters.QuadrantSegments);
    // vector for storing the final paths
    std::vector<Geos::LineString> path; // = BuildPath();
 
    // Simple path
    if(m_Params.cutSide == CompensateCutter::None) {
        path.push_back(inputPath);
    } // Compensate path 
    else {
        Geos geos;  
        // make the inital offset
        path = geos.Offset(inputPath, cutSide * offsetDistance, geosParameters);
        // add pocket path
        if(m_Params.cutSide == CompensateCutter::Pocket) {
            // distance to offset per pass
            float boringOffset = 2.0f * fabsf(toolRadius) - settings.p.pathCutter.CutOverlap;
            // start a recursive loop of offset for boring, if cutting simple offset, this breaks loop after the first iteration
            path = geos.OffsetPolygon_Recursive(path, boringOffset, true /*reverse*/, geosParameters);
        }
        // add finishing path
        if(m_Params.finishingPass) {  
            std::vector<Geos::LineString> finishPath = geos.Offset(inputPath, cutSide * fabsf(toolRadius), geosParameters);     
            for(size_t i = 0; i < finishPath.size(); i++) {
                path.push_back(finishPath[i]);
            }
        }
    }
    // for each one of the pocketing out line loop, cut depths
    for (size_t i = 0; i < path.size(); i++) {
        pathParams.points = &(path[i]);
        // add gcodes for path at depths
        if(gcodes.CutPathDepths(settings, pathParams)) { return {}; }
    } 
    // move to zPlane, end program
    gcodes.EndCommands();
    return gcodes.Get();
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
    Log::Info("Exporting GCode to %s", filepath.c_str());
    // build gcode of active function and export it to file 
    return m_ActiveFunctions.CurrentItem()->InterpretGCode(settings, m_ElementFactory, [&](auto gcodes){ 
        File::WriteArray(filepath, gcodes);
        return 0;
    }); 
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

void Sketch::ActiveFunction_Run(GRBL& grbl, Settings& settings) 
{
    if(!m_Drawings.HasItemSelected()) { return; }
    if(!m_Drawings.CurrentItem().ActiveFunction_HasItemSelected()) { return; }
    
    m_Drawings.CurrentItem().ActiveFunction_Run(grbl, settings);
}
void Sketch::ActiveFunction_Export(Settings& settings) 
{
    if(!m_Drawings.HasItemSelected()) { return; }
    if(!m_Drawings.CurrentItem().ActiveFunction_HasItemSelected()) { return; }
    
    if(ImGui::Button("Export GCode")) { 
        if(m_Drawings.CurrentItem().ActiveFunction_ExportGCode(settings, settings.p.system.saveFileDirectory)) {
            Log::Error("Unable to save file");
        }
    }
}
void Sketch::ActiveFunction_Delete(Settings& settings) 
{    
    if(!m_Drawings.HasItemSelected()) { return; }
    if(!m_Drawings.CurrentItem().ActiveFunction_HasItemSelected()) { return; }
    
    if(ImGui::Button("Delete")) { 
        m_Drawings.CurrentItem().ActiveFunction_Delete();
        settings.SetUpdateFlag(ViewerUpdate::Full);
    }
}

Sketch::Sketch() 
{   
    m_Drawings.Add(A_Drawing("Drawing " + to_string(m_DrawingIDCounter++))); 
}





void Function_Draw::HandleEvents(Settings& settings, InputEvent& inputEvent, ElementFactory& elementFactory) 
{ 
    static bool updateRequiredOnKeyRelease = false;
    
    auto SnapCursor = [&](const glm::vec2 p) { 
        return settings.p.viewer.cursor.SnapCursor(p); 
    }; 
    
    // handle mouse events
    if(inputEvent.mouseClick)  
    {
        Event_MouseButton* mouse = inputEvent.mouseClick;
        if(mouse->Action == GLFW_PRESS  &&  mouse->Button == GLFW_MOUSE_BUTTON_LEFT) 
        {  
            auto snappedCursor = SnapCursor(inputEvent.screenCoords_Click);
            
            if(m_ActiveCommand == Command::Select) {
                elementFactory.ActivePoint_SetByPosition(snappedCursor, settings.p.viewer.cursor.SelectionTolerance_Scaled);
                settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing);
            } else {
                elementFactory.ActivePoint_Unset();
            }
            if(m_ActiveCommand == Command::Line) {
                elementFactory.LineLoop_AddLine(m_LineLoop, snappedCursor);
                settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing | ViewerUpdate::ActiveFunction);
            }
            if(m_ActiveCommand == Command::Arc) {
                elementFactory.LineLoop_AddArc(m_LineLoop, snappedCursor, CLOCKWISE);
                settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing | ViewerUpdate::ActiveFunction);
            }
        }
        // update on key release
        if(mouse->Action == GLFW_RELEASE  &&  mouse->Button == GLFW_MOUSE_BUTTON_LEFT) {  
            if(updateRequiredOnKeyRelease) {
                settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing | ViewerUpdate::ActiveFunction);
                updateRequiredOnKeyRelease = false;
            } 
        }
       
    } 
   
    // handle mouse move events
    if(inputEvent.mouseMove) 
    {
        // if clicked and held, move a point
        if(Mouse::IsLeftClicked()) {//inputEvent.mouseClick.Action == GLFW_REPEAT  &&  inputEvent.mouseClick.Button == GLFW_MOUSE_BUTTON_LEFT) {
            if(elementFactory.ActivePoint_Move(SnapCursor(inputEvent.screenCoords_Move))) {    
                std::cout << "setting update flag to active drawing" << std::endl;            
                settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing);
                updateRequiredOnKeyRelease = true;
            }
        }
    }
    // handle keyboard event
    if(inputEvent.keyboard) {
        // settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing);
    }
} 
            
void A_Drawing::HandleEvents(Settings& settings, InputEvent& inputEvent)
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        return;
    }
    // handle mouse and keyboard events and return if update viewer needed
    m_ActiveFunctions.CurrentItem()->HandleEvents(settings, inputEvent, m_ElementFactory);
}


void Sketch::HandleEvents(Settings& settings, InputEvent& inputEvent)
{
    if(!IsActive() || !m_Drawings.HasItemSelected()) { 
        return;
    }
    // get active drawing
    m_Drawings.CurrentItem().HandleEvents(settings, inputEvent);
}






void Function_Draw::UpdateViewer(Settings& settings, ElementFactory& elementFactory, std::vector<DynamicBuffer::DynamicVertexList>* viewerLineLists, bool isDisabled)
{
    DynamicBuffer::DynamicVertexList lineloop_Lines;
    lineloop_Lines.position = elementFactory.LineLoop_PointsList(m_LineLoop, settings.p.pathCutter.geosParameters.QuadrantSegments);
    lineloop_Lines.colour = (isDisabled) ? settings.p.viewer.line.colourDisabled : settings.p.viewer.line.colour;
    viewerLineLists->push_back(std::move(lineloop_Lines));
} 
// update viewer
void A_Drawing::UpdateViewer(Settings& settings, std::vector<DynamicBuffer::DynamicVertexList>* viewerLineLists)
{
    // update viewer for each active function
    for (size_t i = 0; i < m_ActiveFunctions.Size(); i++) {
        bool isCurrentItem = (m_ActiveFunctions.CurrentIndex() == (int)i);
        m_ActiveFunctions[i]->UpdateViewer(settings, m_ElementFactory, viewerLineLists, !isCurrentItem);
    }
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


void Sketch::ActiveDrawing_UpdateViewer(Settings& settings)
{     
    if(!m_Drawings.HasItemSelected()) { return; }
    
    std::cout << "Updating: Drawing" << std::endl;
    // make a list of lines which is sent to viewer
    m_ViewerLineLists.clear();
    // update the drawing
    m_Drawings.CurrentItem().UpdateViewer(settings, &m_ViewerLineLists);
    // dispatch m_ViewerLineLists
    Event<Event_Viewer_AddLineLists>::Dispatch( { &m_ViewerLineLists } );
    
    
    // make a list of points which is sent to viewer
    m_ViewerPointLists.clear();
    // add points to m_ViewerPointLists
    m_Drawings.CurrentItem().RawPoints_UpdateViewer(settings, &m_ViewerPointLists);
    // dispatch m_ViewerPointLists
    Event<Event_Viewer_AddPointLists>::Dispatch( { &m_ViewerPointLists } );
}


// update viewer
std::optional<std::vector<std::string>> A_Drawing::ActiveFunction_UpdateViewer(Settings& settings)
{
    if(!m_ActiveFunctions.HasItemSelected()) {
        Log::Error("No active function selected");
        return {};  
    }
    // add gcode path of current active function to viewer
    return move(m_ActiveFunctions.CurrentItem()->InterpretGCode(settings, m_ElementFactory));
}   

void Sketch::ActiveFunction_UpdateViewer(Settings& settings)
{ 
    // get current active function's gcodes and update viewer
    if(!m_Drawings.HasItemSelected()) { return; }
    std::cout << "Updating: Draw Function" << std::endl;
    // update the active function 
    if(auto gcodes = m_Drawings.CurrentItem().ActiveFunction_UpdateViewer(settings)) {
        Event<Event_Update3DModelFromVector>::Dispatch({ vector<string>(move(*gcodes)) }); 
    } else {
        Event<Event_Update3DModelFromVector>::Dispatch({ vector<string>(/*empty*/) });
    }
}

void Sketch::UpdateViewer(Settings& settings)
{
    // return if no update required
    if(settings.GetUpdateFlag() == ViewerUpdate::None) { return; }
    
    std::cout << "*** UPDATE FLAG *** : " << (int)settings.GetUpdateFlag() << std::endl;
    
    // clear overrides other bits in flag
    if((int)settings.GetUpdateFlag() & (int)ViewerUpdate::Clear) { 
        std::cout << "Clearing viewer" << std::endl;  
        ClearViewer(); 
    } 
    else {     
        // update active drawing
        if((int)settings.GetUpdateFlag() & (int)ViewerUpdate::ActiveDrawing)  { std::cout << "UPDATING ACTIVE DRAWING" << std::endl;   ActiveDrawing_UpdateViewer(settings); }
        // update active function
        if((int)settings.GetUpdateFlag() & (int)ViewerUpdate::ActiveFunction) { std::cout << "UPDATING ACTIVE FUNCTION" << std::endl;  ActiveFunction_UpdateViewer(settings); }
    }
    
    // reset the update flag
    settings.ResetUpdateFlag();
}

void Sketch::ClearViewer()
{
    // line lists
    Event<Event_Viewer_AddLineLists>::Dispatch( { nullptr } );
    // points lists
    Event<Event_Viewer_AddPointLists>::Dispatch( { nullptr } );
    //
    Event<Event_Update3DModelFromVector>::Dispatch({ vector<string>(/*empty*/) });
}













void Sketch::Activate()
{
    if(m_IsActive == false) {
        m_IsActive = true;
        Event<Event_Set2DMode>::Dispatch( { true } );
    }
}

void Sketch::Deactivate()
{
    if(m_IsActive == true) {
        m_IsActive = false;
        Event<Event_Set2DMode>::Dispatch( { false } );
    }
}

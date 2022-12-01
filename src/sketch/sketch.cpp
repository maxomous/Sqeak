#include "sketch.h"
using namespace std; 
using namespace MaxLib;
using namespace MaxLib::String;
using namespace MaxLib::Geom;

// some unique bit
const int DEBUG_SKETCH_REFERENCES = Log::NextDebugBit();
      
      
namespace Sqeak { 
    
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
    
    
    SketchOld
        - Drawing
            - (A_Functions)            
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
    
    (void)settings;
    (void)p;
   // if(ImGui::InputFloat2(va_str("##(ID:%d)", p->ID()).c_str(), &(p->Vec2().x))) {
   //     settings.SetUpdateFlag(ViewerUpdate::Full);
   //     return true;
   // }
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
                m_Direction = (m_DirectionImGui == 0) ? Geom::Direction::CW : Geom::Direction::CCW;
                settings.SetUpdateFlag(ViewerUpdate::Full);
            }
            ImGui::Text("tangent radius: %g", m_TangentRadius);
            ImGui::TreePop(); 
        } 
    ImGui::PopID();   
}
 
void ElementFactory::LineLoop_DrawImGui(Settings& settings, SketchOld_LineLoop& sketchLineLoop)  
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
 
void A_Function_Draw::DrawImGui_Tools(Settings& settings) 
{
    (void)settings;
   // if(ImGuiModules::ImageButtonWithText_A_Function(settings, "Select##OLD", settings.guiSettings.img_Sketch_Select, m_ActiveCommand == Command::Select)) { m_ActiveCommand = Command::Select; }
   // ImGui::SameLine();
   // if(ImGuiModules::ImageButtonWithText_A_Function(settings, "Line##OLD", settings.guiSettings.img_Sketch_Line, m_ActiveCommand == Command::Line)) { m_ActiveCommand = Command::Line; }
   // ImGui::SameLine();
   // if(ImGuiModules::ImageButtonWithText_A_Function(settings, "Arc##OLD", settings.guiSettings.img_Sketch_Arc, m_ActiveCommand == Command::Arc)) { m_ActiveCommand = Command::Arc; }
     
} 

void A_Function_Draw::DrawImGui(ElementFactory& elementFactory, Settings& settings) 
{
    (void)settings;

    bool updateViewer = false;
    
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Parameters")) 
    {     
        updateViewer |= ImGui::Combo("Polygonise Output", &m_Params.polygoniseOutput, "Input\0Cuts\0Dangles\0Invalid Rings\0\0");
        updateViewer |= ImGui::InputText("Name", &m_Name);
        ImGui::Dummy(ImVec2());
  //      updateViewer |= ImGui::InputFloat2("Z Top/Bottom", &m_Params.z[0]);
        updateViewer |= ImGui::Combo("Cut Side", &m_Params.cutSide, "None\0Left\0Right\0Pocket\0\0");
        updateViewer |= ImGui::InputFloat("Finishing Pass", &m_Params.finishPass);
        
        ImGui::TreePop();
    }
    
    // set viewer update flag
    if(updateViewer) { settings.SetUpdateFlag(ViewerUpdate::Full); }
    
    elementFactory.LineLoop_DrawImGui(settings, m_LineLoop);
    
    
}
 


void DrawImGui_PathCutterParameters(Settings& settings) 
{
    (void)settings;
   // ParametersList::PathCutter& pathCutter = settings.p.pathCutter;
   // bool updateViewer = false;
   // 
   // ImGui::InputFloat("Cut Overlap", &pathCutter.CutOverlap);
   // ImGui::InputFloat("Partial Retract Distance", &pathCutter.PartialRetractDistance);
   // 
   // updateViewer |= ImGui::InputInt("Quadrant Segments", &settings.p.pathCutter.geosParameters.QuadrantSegments);
/* //   // cap style / join style
   // static int imgui_CapStyle = settings.p.pathCutter.geosParameters.CapStyle - 1;
   // if(ImGui::Combo("Cap Style", &imgui_CapStyle, "Round\0Flat\0Square\0\0")) {
   //     settings.p.pathCutter.geosParameters.CapStyle = imgui_CapStyle + 1;
   //     updateViewer = true;
   // }
   // static int imgui_JoinStyle = settings.p.pathCutter.geosParameters.JoinStyle - 1;
   // if(ImGui::Combo("Join Style", &imgui_JoinStyle, "Round\0Mitre\0Bevel\0\0")) {
   //     settings.p.pathCutter.geosParameters.JoinStyle = imgui_JoinStyle + 1;
   //     updateViewer = true;
   // }
*/ //   
   // updateViewer |= ImGui::Checkbox("Cut Tabs", &pathCutter.CutTabs);
   //     
   // ImGui::Indent();
   //     if(pathCutter.CutTabs) {
   //         updateViewer |= ImGui::InputFloat("Tab Spacing", &pathCutter.TabSpacing);
   //         updateViewer |= ImGui::InputFloat("Tab Height",  &pathCutter.TabHeight);
   //         updateViewer |= ImGui::InputFloat("Tab Width",   &pathCutter.TabWidth);
   //     }
   // ImGui::Unindent();
   // if(updateViewer) { settings.SetUpdateFlag(ViewerUpdate::Full); }
}
std::string A_Drawing::ActiveA_Function_Name()
{   
    if(!m_ActiveA_Functions.HasItemSelected()) { return ""; }
    return m_ActiveA_Functions.CurrentItem()->Name();
} 
std::string SketchOld::ActiveA_Function_Name()
{    
    if(!m_Drawings.HasItemSelected()) { return ""; }
    return m_Drawings.CurrentItem().ActiveA_Function_Name();
}
void A_Drawing::ActiveA_Function_DrawImGui_Tools(Settings& settings)
{     
    // handle the selected active function
    if(m_ActiveA_Functions.HasItemSelected()) {
        m_ActiveA_Functions.CurrentItem()->DrawImGui_Tools(settings);
    }  
}

void A_Drawing::DrawImGui_A_Functions(Settings& settings)
{    
    (void)settings;
    // new function buttons
  // if(ImGuiModules::ImageButtonWithText_A_Function(settings, "Draw##OLD", settings.guiSettings.img_Sketch_Draw)) {    
  //     std::unique_ptr<A_Function> newA_Function = std::make_unique<A_Function_Draw>(m_ElementFactory, "Draw " + to_string(m_A_FunctionIDCounter++));
  //     m_ActiveA_Functions.Add(move(newA_Function));
  //     settings.SetUpdateFlag(ViewerUpdate::Full);
  //     // to open tree node
  //     m_IsActiveA_FunctionChanged = true;
  // }
  // ImGui::SameLine();
  // 
  // if(ImGuiModules::ImageButtonWithText_A_Function(settings, "Measure##OLD", settings.guiSettings.img_Sketch_Measure)) {    
  //     std::cout << "Measure" << std::endl;
  // }
}   

void A_Drawing::DrawImGui(Settings& settings)
{

    if (ImGui::TreeNode("General Parameters")) {
        DrawImGui_PathCutterParameters(settings);
        ImGui::TreePop();
    }
    
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    
     // active function imgui widgets
    if (ImGui::TreeNode("Active A_Functions")) 
    { 
        // draw active function Tree Nodes
        static std::function<std::string*(std::unique_ptr<A_Function>& item)> cb_GetItemStringPtr = [](std::unique_ptr<A_Function>& activeA_Function) { 
            return &(activeA_Function->Name()); 
        };
        static std::function<void(std::unique_ptr<A_Function>&)> cb_DrawTabImGui = [&](std::unique_ptr<A_Function>& activeA_Function) { 
            activeA_Function->DrawImGui(m_ElementFactory, settings);
        };
         
        if(ImGuiModules::TreeNodes(m_ActiveA_Functions, m_IsActiveA_FunctionChanged, cb_GetItemStringPtr, cb_DrawTabImGui)) {
            settings.SetUpdateFlag(ViewerUpdate::Full);
        }
        ImGui::TreePop();
    }
        
        /*
        ImVec2& buttonSize = settings.guiSettings.button[ButtonType::Toolbar_Button].Size;
        ImVec2& buttonSizeSmall = settings.guiSettings.button[ButtonType::New].Size;
   
      
        static std::function<std::string(std::unique_ptr<A_Function>& item)> callback = [](std::unique_ptr<A_Function>& item) { return item->Name(); };
        if(ImGuiModules::Buttons(m_ActiveA_Functions, buttonSize, callback)) {
            settings.SetUpdateFlag(ViewerUpdate::Full);
        }
        
        ImGui::SameLine();
            
        // remove active A_Function button
        if(m_ActiveA_Functions.HasItemSelected()) 
        {   // remove A_Function
            if(ImGui::Button("-##Remove A_Function", buttonSizeSmall)) {
                m_ActiveA_Functions.RemoveCurrent();
                settings.SetUpdateFlag(ViewerUpdate::Full);
            }
        }
        */
    
    // draw the points list
    ImGui::Separator();
    if (ImGui::TreeNode("Points List")) {
        m_ElementFactory.RawPoint_DrawImGui(settings);
        // draw the references
        m_ElementFactory.RefPointToElement_DrawImGui();
        ImGui::TreePop();
    }
}
    
    
void SketchOld::ActiveA_Function_DrawImGui_Tools(Settings& settings) 
{
    if(m_Drawings.HasItemSelected()) { 
        m_Drawings.CurrentItem().ActiveA_Function_DrawImGui_Tools(settings);
    }
}
void SketchOld::ActiveDrawing_DrawImGui_A_Functions(Settings& settings) 
{
    if(m_Drawings.HasItemSelected()) { 
        m_Drawings.CurrentItem().DrawImGui_A_Functions(settings);
    }
}

bool SketchOld::DrawImGui_StartSketchOld(Settings& settings) 
{
    (void)settings;
    if(!IsActive()) {
        
  //      if(ImGuiModules::ImageButtonWithText_A_Function(settings, "SketchOld", settings.guiSettings.img_Sketch)) {  
  //          Activate();
  //          settings.SetUpdateFlag(ViewerUpdate::Full);
  //          return true;
  //      }
  //  } else {    
  //      // sketch is active
  //      if(ImGuiModules::ImageButtonWithText_A_Function(settings, "SketchOld", settings.guiSettings.img_Sketch, true)) {  
  //          Deactivate();
  //      }
    }
    return false;
}     

    
/*

        // if the rawpoint has another element attached, use this as it's tangent basis
        RawPoint* RawPointPrev = nullptr;
        m_Ref_P0->rawPoint->GetReferences([&](Ref_PointToElement* ref) { 
            // get the other element attached to this point
            if(ref->element->ID() != ID()) { RawPointPrev = ref->element->P0()->rawPoint; }
        });
        if(!RawPointPrev) { return false; }
        
        
        LineLoopID id
        
        if(ImGui::Button(va_str("Delete##%d",i).c_str())) {
            LineLoop_DeleteElement(id, (size_t)i);
            settings.SetUpdateFlag(ViewerUpdate::Full);
        } 
        
           */ 
            
            
void SketchOld::DrawPopup_Cursor(Settings& settings) 
{   
    /*
    const char* popupName = "openPopup_SketchOld_RightClickMenu";
    // right click menu for point
    if(m_Drawings.HasItemSelected()) { 

        if(trigger(settings.p.sketch.cursor.popup.shouldOpen)) {
            ImGui::OpenPopup(popupName);
        }
        
        if (ImGui::BeginPopup(popupName)) 
        {
            std::cout << "drawing right click popup: " << std::endl;
            if(auto id =  m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_GetID()) {
                std::cout << "active id: " << (uint)*id << std::endl;
                ImGui::Text("Point %u", (uint)*id);
                ImGui::Separator();
                // delete
                if(ImGui::Selectable("Delete")) {
                    if(m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_Delete()) {
                        settings.SetUpdateFlag(ViewerUpdate::Full);
                    }
                }
            }
            ImGui::EndPopup();
        } 
        if(ImGui::IsPopupOpen(popupName)) {
            std::cout << "popup open: " << true << std::endl;
            return;
        }
    
    }*/
    
    if(auto pos = settings.p.sketch.cursor.Position_Snapped) 
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs;
        
        float offsetPos = settings.guiSettings.popupPosition_offsetPos;
        ImGui::SetNextWindowPos(ImGui::GetMousePos() + ImVec2(offsetPos, offsetPos));
        
        //ImGui::PushStyleColor(ImGuiCol_Text,     { 1.0f, 1.0f, 1.0f, 0.6f } * ImGui::GetStyleColorVec4(ImGuiCol_Text));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, settings.guiSettings.popupPosition_alpha) * ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

        if(ImGui::Begin("PopupPosition", NULL, ImGuiCustomModules::ImGuiWindow::generalWindowFlags | window_flags)) {
            ImGui::Text("(%g, %g)", pos->x, pos->y);
        }
        ImGui::PopStyleColor();
        ImGui::End(); 
    }
}

void SketchOld::DrawImGui(Settings& settings) 
{
      
    // Cursor Popup
    static ImGuiModules::ImGuiPopup popup_CursorRightClick("popup_CursorRightClick");
    // open
    if(m_Drawings.HasItemSelected()) 
    {
        if(auto id =  m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_GetID()) 
        {
            // set to open
            if(!ImGui::GetIO().WantCaptureMouse && IsActive()) {
            
                if(settings.p.sketch.cursor.popup.shouldOpen) {        
                    settings.p.sketch.cursor.popup.shouldOpen = false;
                    popup_CursorRightClick.Open();
                }
            }
            // draw cursor popup
            popup_CursorRightClick.Draw([&]() {
                ImGui::Text("Point %u", (uint)*id);
                ImGui::Separator();
                // delete
                if(ImGui::Selectable("Delete")) {
                    if(m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_Delete()) {
                        settings.SetUpdateFlag(ViewerUpdate::Full);
                    }
                }
            });
        }
    }
    
    
    
    static bool isNewDrawing = true;
    
    // display x, y coord on screen if not over imgui window
    if(!ImGui::GetIO().WantCaptureMouse && IsActive()) {
        DrawPopup_Cursor(settings);
    }
    
    // begin new imgui window
    static ImGuiCustomModules::ImGuiWindow window(settings, "SketchOld"); // default size
    if(window.Begin(settings)) 
    {    
        if (ImGui::SmallButton("New Drawing")) {
            m_Drawings.Add("Drawing " + to_string(m_DrawingIDCounter++));
            isNewDrawing = true;
            settings.SetUpdateFlag(ViewerUpdate::Full);
        } 
        
        for(size_t i = 0; i < m_Drawings.Size(); )
        {
            // set active drawing to be open initially & inactive drawings to be closed
            if(m_Drawings.CurrentIndex() == (int)i) { 
                if(isNewDrawing) {
                    ImGui::SetNextItemOpen(true); 
                    isNewDrawing = false;
                }
            } else { ImGui::SetNextItemOpen(false); }
            // close button flag - set by imgui
            bool closeIsntClicked = true; 
            if (ImGui::CollapsingHeader(m_Drawings[i].Name().c_str(), &closeIsntClicked)) {
                // set the active index to match the open tab
                if(m_Drawings.CurrentIndex() != (int)i) {
                    std::cout << "Setting current drawing index" << std::endl;
                    m_Drawings.SetCurrentIndex(i);
                    settings.SetUpdateFlag(ViewerUpdate::Full);
                }
                // draw the imgui widgets for drawing 
                m_Drawings.CurrentItem().DrawImGui(settings); 
            }
            if(!closeIsntClicked) { // has been closed
                m_Drawings.Remove(i); 
                settings.SetUpdateFlag(ViewerUpdate::Full);
            } else { 
                i++; 
            }                        

        }
        window.End();
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
        m_Drawings.CurrentItem().DrawImGui(settings);        
    };
     
    if(ImGuiModules::Tabs(m_Drawings, cb_GetItemString, cb_AddNewItem, cb_DrawTabImGui)) {
        settings.SetUpdateFlag(ViewerUpdate::Full);
    }
*/
}   



void RawPoint::GetReferences(std::function<void(Ref_PointToElement*)> callback) 
{ 
    for(Ref_PointToElement* ref : m_ElementRefs) { 
        callback(ref);
    }
} 

/*

enum ConstraintType { Arc, Equal, Distance, DistanceX, DistanceY, Horizontal, Vertical }; 
    
    
class Constraint
{
protected:
    Constraint(ConstraintType type) : m_Type(type) {}
    
private:
    ConstraintType m_Type;
};


class Constraint_Arc : public Constraint
{
public:
    Constraint_Arc() : Constraint(ConstraintType::Arc) {}
    
private:
    ... Ref_P0;
    ... Ref_P1;
    ... Ref_PC;
};

class Constraint_Equal : public Constraint
{
public:
    Constraint_Equal() : Constraint(ConstraintType::Equal) {}
    
private:
    ... Ref_P0;
    ... Ref_P1;
};



class RawPoint 
{
    void UpdateConstraints(std::vector<UpdatedPoint>& updatedPoints) 
    {
        for(Constraint& constraint : m_Constraints) {
            if(!prevRawPoint) {
                constraint.update();
            }
        }
    }

    std::vector<std::unique_ptr<Constraint>> m_Constraints; 
    m_IsAlreadyUpdated = false;
}


struct UpdatedPoint
{
    UpdatedPoint(RawPoint* point, Vec2 position) : rawPoint(point), pos(position) {}
    
    RawPoint* rawPoint = nullptr;
    Vec2 pos;
    bool isFixed = false;
};

void ElementFactory::RawPoint_Move(RawPoint* rawPoint, Vec2 pos) 
{
    
    std::vector<UpdatedPoint> updatedPoints;
    updatedPoints.push_back(UpdatedPoint(rawPoint, pos));
    
    rawPoint->UpdateConstraints(updatedPoints);
    
    RawPoint->SetPosition(pos);
}




*/


void RawPoint::SetThisRawPointFromRefs(const MaxLib::Geom::Vec2& p) 
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
bool ElementFactory::Reference_Break(Ref_PointToElement* ref) 
{
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Breaking Reference %d\t(RawPoint: %d\tElement: %d)", (int)ref, (int)ref->rawPoint, (int)ref->element);
    int itemsDeleted = 0;
    // remove the reference from point. if no references left on rawpoint, delete it
    RawPoint* point = ref->rawPoint;
    if(point->RemoveReference(ref)) {
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Deleting RawPoint %d", (int)point);
        RawPoint_DeleteFromFactory(point);
        itemsDeleted++;
    }
    // remove the reference from element. if no references left on element, delete it
    Element* element = ref->element;
    if(element->RemoveReference(ref)) {
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Deleting Element %d", (int)element);
        Element_DeleteFromFactory(element);
        itemsDeleted++;
    }
    // delete the reference itself
    Reference_DeleteFromFactory(ref);
    return (itemsDeleted == 2);
}

// removes references from element and rawpoint within ref, deletes the element/rawpoint if they have no references left
bool ElementFactory::Reference_ReplaceRawPoint(Ref_PointToElement* ref, RawPoint* pNew) 
{
    Log::Debug(DEBUG_SKETCH_REFERENCES, "Replacing RawPoint Reference");
    int itemsDeleted = 0;
    // remove the reference from point. if no references left on point, delete it
    RawPoint* point = ref->rawPoint;
    if(point->RemoveReference(ref)) {
        Log::Debug(DEBUG_SKETCH_REFERENCES, "Deleting RawPoint %d", (int)point);
        RawPoint_DeleteFromFactory(point);
        itemsDeleted++;
    }
    // set reference rawpoint to pNew 
    ref->SetReference(pNew);
    // add reference to pNew
    pNew->AddReference(ref);
    Log::Debug(DEBUG_SKETCH_REFERENCES, "RawPoint has been replaced in Reference:\t%d\t(RawPoint: %d\tElement: %d)", (int)ref, (int)ref->rawPoint, (int)ref->element);
    return (itemsDeleted == 1);
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
RawPoint* ElementFactory::RawPoint_Create(Vec2 p, Ref_PointToElement* ref) {
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

RawPoint* ElementFactory::RawPoint_GetByPosition(Vec2 p, float tolerance) 
{
    Vec2 tol = { tolerance, tolerance };
    int closestPoint = -1;
    float minDistance = 0.0f;

    // go through all the points and if it falls within the tolerence frame, work out the one with the shortest distance
    for (size_t i = 0; i < m_Points.Size(); i++) {
        // does point fall within tolerence frame
        if((m_Points[i]->Vec2() > (p-tol)) && (m_Points[i]->Vec2() < (p+tol))) 
        {
            Vec2 dif = m_Points[i]->Vec2() - p;
            float distance = Geom::Hypot({ dif.x, dif.y });
            // if first point found or if this point is closer than previous one found
            if(closestPoint == -1 || distance < minDistance) {
                closestPoint = i;
                minDistance = distance;
            }
        }   
    }
    // return nullptr if no point under cursor
    return (closestPoint > -1) ? m_Points[closestPoint].get() : nullptr;
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
ElementFactory::SketchOld_LineLoop ElementFactory::LineLoop_Create() 
{
    m_LineLoops.push_back(move(std::make_unique<LineLoop>(m_LineLoopIDCounter++)));
    return make_unique<SketchOld_LineLoop_Identifier>(m_LineLoops.back()->ID(), this);
}
// Set start point  
void ElementFactory::LineLoop_SetStartPoint(LineLoop& lineLoop, const Vec2& startPoint) 
{
    SketchOld_Element pointElement = Element_CreatePoint(startPoint);
    LineLoop_SetStartPoint(lineLoop, move(pointElement)); 
} 
// Set start point
void ElementFactory::LineLoop_SetStartPoint(LineLoop& lineLoop, SketchOld_Element pointElement) 
{
    if(!lineLoop.m_Elements.size()) {
        lineLoop.m_Elements.push_back(move(pointElement));
    } else {
        lineLoop.m_Elements[0] = move(pointElement);
    }
}

// Adds a line to the Line Loop
void ElementFactory::LineLoop_AddLine(ElementFactory::SketchOld_LineLoop& sketchLineLoop, const Vec2& p1) 
{
    LineLoopID lineLoopID = sketchLineLoop->id;
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID); 
    if(lineLoop.IsEmpty()) { LineLoop_SetStartPoint(lineLoop, p1);  return; } 
    
    SketchOld_Element lineElement = Element_CreateLine(LineLoop_LastPoint(lineLoopID), p1);
    lineLoop.m_Elements.push_back(move(lineElement));
} 

// Adds an arc to the Line Loop from centre point
void ElementFactory::LineLoop_AddArc(ElementFactory::SketchOld_LineLoop& sketchLineLoop, const Vec2& p1, int direction, const Vec2& centre) 
{
    LineLoopID lineLoopID = sketchLineLoop->id;
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID);
    if(lineLoop.IsEmpty()) { LineLoop_SetStartPoint(lineLoop, p1);  return; } 
    
    SketchOld_Element arcElement = Element_CreateArc(LineLoop_LastPoint(lineLoopID), p1, direction, centre);
    lineLoop.m_Elements.push_back(move(arcElement));
    
}
void ElementFactory::LineLoop_AddArc(ElementFactory::SketchOld_LineLoop& sketchLineLoop, const Vec2& p1, int direction) 
{
    LineLoopID lineLoopID = sketchLineLoop->id;
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID); 
    if(lineLoop.IsEmpty()) { LineLoop_SetStartPoint(lineLoop, p1);  return; } 
    
    Vec2& p0 = LineLoop_LastPoint(sketchLineLoop->id)->Vec2();
    Vec2 centre = (p0 + p1) / 2.0f;
    LineLoop_AddArc(sketchLineLoop, p1, direction, centre);
}

// Adds an arc to the Line Loop from radius
void ElementFactory::LineLoop_AddArc(ElementFactory::SketchOld_LineLoop& sketchLineLoop, const Vec2& p1, int direction, float radius) 
{    
    LineLoopID lineLoopID = sketchLineLoop->id;
    LineLoop& lineLoop = LineLoop_GetByID(lineLoopID);
    if(lineLoop.IsEmpty()) { LineLoop_SetStartPoint(lineLoop, p1);  return; } 
    // calculate centre from radius, then pass on
    RawPoint* p0 = LineLoop_LastPoint(lineLoopID); 
    Geom::Vec2 centre = Geom::ArcCentre(Geom::Vec2(p0->X(), p0->Y()), Geom::Vec2(p1.x, p1.y), radius, (Geom::Direction)direction);
    LineLoop_AddArc(sketchLineLoop, p1, direction, Vec2(centre.x, centre.y));
} 
// returns 0 on success 
int A_Function::InterpretGCode(Settings& settings, ElementFactory& elementFactory, std::function<int(std::vector<std::string> gcode)> callback)
{
    // get gcodes and and if successful, call function provided
    if(auto gcodes = InterpretGCode(settings, elementFactory)) {
        return callback(move(*gcodes));        
    }
    return -1;
}

// bool is success
std::optional<std::vector<std::string>> A_Function::InterpretGCode(Settings& settings, ElementFactory& elementFactory)
{
    // error check
    if(settings.p.toolSettings.tools.IsToolAndMaterialSelected())
        return {};
    // export gcode
    if(auto gcodes = move(ExportGCode(settings, elementFactory)))
        return move(gcodes);
    // error
    Log::Error("Could not interpret this GCode");
    return {};
}
 
A_Function_Draw::A_Function_Draw(ElementFactory& elementFactory, std::string name) : A_Function(name) 
{
    m_LineLoop = elementFactory.LineLoop_Create();
}

bool A_Function_Draw::IsValidInputs(Settings& settings, ElementFactory& elementFactory) 
{
    // check tool and material is selected
    if(settings.p.toolSettings.tools.IsToolAndMaterialSelected())
        return false;
    size_t size = elementFactory.LineLoop_Size(m_LineLoop);
    // start and end point
    if(size == 0) {
        Log::Error("At least 1 drawing element is required");
        return false;
    }
    
    if(m_Params.cutSide == CompensateCutter::Pocket && !elementFactory.LineLoop_IsLoop(m_LineLoop)) {
        Log::Error("Pocket requires start and end points to be identical");
        return false;
    }
    // z top and bottom
    if(m_Params.z.x > m_Params.z.y) {
        Log::Error("Z Bottom must be below or equal to Z Top");
        return false;
    }
    // cut depth should have value
    ToolSettings::Tools::Tool::ToolData& toolData = settings.p.toolSettings.tools.toolList.CurrentItem().Data.CurrentItem();
    if(toolData.cutDepth <= 0.0f) {
        Log::Error("Cut Depth must be positive");
        return false;
    }
    
    
    return true;
}   

std::string A_Function_Draw::HeaderText(Settings& settings, ElementFactory& elementFactory) 
{
    A_Function_Draw_Parameters& p = m_Params;
    ToolSettings::Tools::Tool& tool = settings.p.toolSettings.tools.toolList.CurrentItem();
    ToolSettings::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();    
    
    // write header
    std::ostringstream stream;
    stream << "; A_Function: " << m_Name << '\n';
    stream << "; \tBetween: " << p.z.x << " and " << p.z.y << '\n';
    stream << "; \tPoints:" << '\n';
    
    // TODO add header text
    (void)elementFactory;
   // m_Params.drawing.AddElementsHeaderText(stream);
    
    if(p.cutSide == CompensateCutter::None) stream << "; \tCompensate: None\n";
    if(p.cutSide == CompensateCutter::Left) stream << "; \tCompensate: Left\n";
    if(p.cutSide == CompensateCutter::Right) stream << "; \tCompensate: Right\n";
    if(p.cutSide == CompensateCutter::Pocket) stream << "; \tCompensate: Pocket\n";
    
    if(p.finishPass) stream << "; Finishing Pass: " << p.finishPass << '\n';
    
    stream << "; Tool: " << tool.Name << '\n';
    stream << "; \tDiameter: " << tool.Diameter << '\n';
    stream << "; \tCut Depth: " << toolData.cutDepth << '\n';
    stream << '\n';
    
    return stream.str();
}
    
// TODO: can we get rid of pathParams.isLoop?
    
std::optional<std::vector<std::string>> A_Function_Draw::ExportGCode(Settings& settings, ElementFactory& elementFactory)  
{
    (void)settings; (void)elementFactory;
   // // error check  
   // if(!IsValidInputs(settings, elementFactory)) {
   //     return {};  
   // }  
   // ToolSettings::Tools::Tool& tool = settings.p.toolSettings.tools.toolList.CurrentItem();
   // ToolSettings::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem(); 
   //      
   // // initialise 
   // GCodeBuilder gcodes;
   // gcodes.Add(HeaderText(settings, elementFactory));
   // gcodes.InitCommands(toolData.speed);
   //   
   // // define offset path parameters:  0 = no compensation, 1 = compensate left / pocket, -1 = compensate right
   // int cutSide = GetCutSide((CompensateCutter)m_Params.cutSide);
   // float toolRadius = settings.p.toolSettings.tools.toolList.CurrentItem().Diameter / 2.0f;
   // float offsetDistance = fabsf(toolRadius) + m_Params.finishPass;
   // // define geos parameters
 	//GeosBufferParams& geosParameters = settings.p.pathCutter.geosParameters;
   // // calculate offset
   // GCodeBuilder::CutPathParams pathParams;
   // // populate parameters
   // pathParams.z0 = m_Params.zTop;  
   // pathParams.z1 = m_Params.zBottom; 
   // pathParams.cutDepth = toolData.cutDepth; 
   // pathParams.feedPlunge = toolData.feedPlunge; 
   // pathParams.feedCutting = toolData.feedCutting; 
   //  
   // Geos geos;  
   // // make a path of line segments (arcs are converted to many line segments)    
   // Geom::LineString inputPath = elementFactory.LineLoop_PointsList(m_LineLoop, geosParameters.QuadrantSegments);
   // // vector for storing the final paths
   // std::vector<Geom::LineString> path;
   // bool isPocket = false;
   // std::vector<Geom::LineString> enclosingPath;
   //  
   // // Simple path
   // if(m_Params.cutSide == CompensateCutter::None) {
   //     //path.push_back(inputPath);
   //     //path = geos.Polygonise(inputPath, m_Params.polygoniseOutput);
   // } // Compensate path 
   // else {
   //     // make the inital offset   
   //     path = geos.Offset(inputPath, cutSide * offsetDistance, geosParameters);
   //     // add pocket path 
   //     if(m_Params.cutSide == CompensateCutter::Pocket) { 
   //         isPocket = true;
   //         // distance to offset per pass
   //         float boringOffset = 2.0f * fabsf(toolRadius) - settings.p.pathCutter.CutOverlap; 
   //         // make a copy of the orignal path
   //         // start a recursive loop of offset for boring, if cutting simple offset, this breaks loop after the first iteration
   //         Geos::RecursiveOffset recursiveOffset = geos.OffsetPolygon_Recursive(path, boringOffset, true /*reverse*/, geosParameters);
   //         path = recursiveOffset.path;
   //         enclosingPath = recursiveOffset.enclosingPath;
   //     }
   //
   // }
   //
   // 
   // // for each one of the pocketing out line loop, cut depths
   // for (size_t i = 0; i < path.size(); i++) {
   //     
   //     if(isPocket) {
   //         // sanity check
   //         if(path.size() != enclosingPath.size()) {
   //             Log::Error("Path sizes do not match, forcing full retract");
   //             pathParams.retract = GCodeBuilder::RetractType::Full;
   //         } else {
   //             // do we need to retract z for each depth of pocket
   //             if(geos.Within({ path[i].front(), path[i].back() }, enclosingPath[i])) {
   //                 pathParams.retract = GCodeBuilder::RetractType::Partial;
   //             } else {
   //                 pathParams.retract = GCodeBuilder::RetractType::Full;
   //             }
   //         }
   //     }
   //     
   //     pathParams.points = &(path[i]);
   //     // add gcodes for path at depths
   //     if(gcodes.CutPathDepths(settings, pathParams)) { return {}; }
   // }         
   // // add finishing path
   // if(m_Params.finishPass) {
   //     pathParams.z0 = pathParams.z1;
   //     std::vector<Geom::LineString> finishPath = geos.Offset(inputPath, cutSide * fabsf(toolRadius), geosParameters);     
   //     for(size_t i = 0; i < finishPath.size(); i++) {
   //         pathParams.points = &(finishPath[i]);
   //         // add gcodes for path at depths
   //         if(gcodes.CutPathDepths(settings, pathParams)) { return {}; }
   //     }
   // }
   // // move to zPlane, end program
   // gcodes.EndCommands();
   // return gcodes.Get();
   return {};
}

 
 
 
 
void A_Drawing::ActiveA_Function_Run(GRBL& grbl, Settings& settings) 
{
    if(!m_ActiveA_Functions.HasItemSelected()) {
        Log::Error("No active function selected");
        return;
    }
    // build gcode of active function and run it
    m_ActiveA_Functions.CurrentItem()->InterpretGCode(settings, m_ElementFactory, [&](auto gcodes){
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
int A_Drawing::ActiveA_Function_ExportGCode(Settings& settings, std::string saveFileDirectory) 
{ 
    if(!m_ActiveA_Functions.HasItemSelected()) {
        Log::Error("No active function selected");
        return -1; 
    }
    // make filepath for new GCode file 
    std::string filepath = File::CombineDirPath(saveFileDirectory, m_ActiveA_Functions.CurrentItem()->Name() + ".nc"); 
    Log::Info("Exporting GCode to %s", filepath.c_str());
    // build gcode of active function and export it to file 
    return m_ActiveA_Functions.CurrentItem()->InterpretGCode(settings, m_ElementFactory, [&](auto gcodes){ 
        File::WriteArray(filepath, gcodes);
        return 0;
    }); 
}


// delete active function
void A_Drawing::ActiveA_Function_Delete()
{
    if(!m_ActiveA_Functions.HasItemSelected()) {
        return; 
    }
    // remove active function
    m_ActiveA_Functions.RemoveCurrent();
}

void SketchOld::ActiveA_Function_Run(GRBL& grbl, Settings& settings) 
{
    if(!m_Drawings.HasItemSelected()) { return; }
    if(!m_Drawings.CurrentItem().ActiveA_Function_HasItemSelected()) { return; }
    
    m_Drawings.CurrentItem().ActiveA_Function_Run(grbl, settings);
}
void SketchOld::ActiveA_Function_Export(Settings& settings) 
{
    if(!m_Drawings.HasItemSelected()) { return; }
    if(!m_Drawings.CurrentItem().ActiveA_Function_HasItemSelected()) { return; }
    
    if(ImGui::Button("Save")) { 
        if(m_Drawings.CurrentItem().ActiveA_Function_ExportGCode(settings, settings.p.system.saveFileDirectory)) {
            Log::Error("Unable to save file");
        }
    }
}
void SketchOld::ActiveA_Function_Delete(Settings& settings) 
{    
    if(!m_Drawings.HasItemSelected()) { return; }
    if(!m_Drawings.CurrentItem().ActiveA_Function_HasItemSelected()) { return; }
    
    if(ImGui::Button("Delete")) { 
        m_Drawings.CurrentItem().ActiveA_Function_Delete();
        settings.SetUpdateFlag(ViewerUpdate::Full);
    }
}

std::optional<Vec2> SketchOld::RawPoint_GetClosest(const Vec2& p, float tolerance) 
{
    if(!m_Drawings.HasItemSelected()) { return {}; }
    return m_Drawings.CurrentItem().m_ElementFactory.RawPoint_GetClosest(p, tolerance);
}

SketchOld::SketchOld() 
{   
    m_Drawings.Add("Drawing " + to_string(m_DrawingIDCounter++)); 
}




 
void A_Function_Draw::HandleEvents(Settings& settings, InputEvent& inputEvent, ElementFactory& elementFactory) 
{ 
    auto selectRawPoint = [&]() 
    {
        if(m_ActiveCommand == Command::Select) 
        {   // select point under cursor
            if(auto cursorPos = settings.p.sketch.cursor.Position_Snapped) {
                if(elementFactory.ActivePoint_SetByPosition(Vec2((*cursorPos).x, (*cursorPos).y), settings.p.sketch.cursor.SelectionTolerance_Scaled)) {
                    settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing);
                    return true;
                }
            }
        }
        return false;
    };
    
    static bool updateRequiredOnKeyRelease = false;
    
    // handle mouse events
    if(inputEvent.mouseClick)  
    {
        Event_MouseButton* mouse = inputEvent.mouseClick;
         
        if(mouse->Action == GLFW_PRESS) 
        {  
            // Select point (left / right click)
            if(mouse->Button == GLFW_MOUSE_BUTTON_LEFT || mouse->Button == GLFW_MOUSE_BUTTON_RIGHT) 
            {  
                if(!selectRawPoint()) {
                    elementFactory.ActivePoint_Unset(); 
                }
            }
            // Create New Element (left click)
            if(mouse->Button == GLFW_MOUSE_BUTTON_LEFT) 
            {  
                if(auto cursorPos = settings.p.sketch.cursor.Position_Clicked) {
                    
                    if(m_ActiveCommand == Command::Line) {
                        elementFactory.LineLoop_AddLine(m_LineLoop, { (*cursorPos).x,  (*cursorPos).y });
                        settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing | ViewerUpdate::ActiveFunction);
                    }
                    if(m_ActiveCommand == Command::Arc) {
                        elementFactory.LineLoop_AddArc(m_LineLoop, { (*cursorPos).x,  (*cursorPos).y }, Geom::Direction::CW);
                        settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing | ViewerUpdate::ActiveFunction);
                    }
                }
            }
            // right click
            if(mouse->Button == GLFW_MOUSE_BUTTON_RIGHT) 
            {  
                settings.p.sketch.cursor.popup.shouldOpen = true;
            }
        }
        // update on key release
        if(mouse->Action == GLFW_RELEASE) 
        {
            if(mouse->Button == GLFW_MOUSE_BUTTON_LEFT) 
            {  
                if(updateRequiredOnKeyRelease) {
                    settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing | ViewerUpdate::ActiveFunction);
                    updateRequiredOnKeyRelease = false;
                } 
            }
        }
       
    } 
    // handle mouse move events
    if(inputEvent.mouseMove) 
    {
        if(auto cursorPos = settings.p.sketch.cursor.Position_Snapped) {
            // move a point if dragged
            if(Mouse::IsLeftClicked()) {//inputEvent.mouseClick.Action == GLFW_REPEAT  &&  inputEvent.mouseClick.Button == GLFW_MOUSE_BUTTON_LEFT) {
                if(elementFactory.ActivePoint_Move({ (*cursorPos).x,  (*cursorPos).y })) {    
                    std::cout << "setting update flag to active drawing" << std::endl;            
                    settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing);
                    updateRequiredOnKeyRelease = true;
                }
            } 
            // preview next element based on current mouse position
            settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing);
        }
    }
    // handle keyboard event
    if(inputEvent.keyboard) {
        // settings.SetUpdateFlag(ViewerUpdate::ActiveDrawing);
    }
} 
            
void A_Drawing::HandleEvents(Settings& settings, InputEvent& inputEvent)
{
    if(!m_ActiveA_Functions.HasItemSelected()) {
        return;
    }
    // handle mouse and keyboard events and return if update viewer needed
    m_ActiveA_Functions.CurrentItem()->HandleEvents(settings, inputEvent, m_ElementFactory);
}


void SketchOld::HandleEvents(Settings& settings, InputEvent& inputEvent)
{
    if(!IsActive() || !m_Drawings.HasItemSelected()) { 
        return;
    }
    // get active drawing
    m_Drawings.CurrentItem().HandleEvents(settings, inputEvent);
}


void A_Function_Draw::UpdateViewer(Settings& settings, ElementFactory& elementFactory, std::vector<DynamicBuffer::ColouredVertexList>* viewerLineLists, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists, bool isActive)
{
    // set colours
    DynamicBuffer::ColouredVertexList lines((!isActive) ? settings.p.sketch.line.colourDisabled : settings.p.sketch.line.colour);
    DynamicBuffer::ColouredVertexList points(settings.p.sketch.point.colour);
    
    // get line loop positions (return if there are none)
    std::vector<Vec2> positions = elementFactory.LineLoop_PointsList(m_LineLoop, 15);
    if(positions.empty()) { return; }
    
    lines.position.clear();
        // copy line loop to viewerLineLists at z0
        for(size_t i = 0; i < positions.size(); i++) {
            lines.position.push_back({ positions[i].x, positions[i].y, m_Params.z.x });
        }
        viewerLineLists->push_back(lines);
        
        // copy line loop to viewerLineLists at z1
        lines.position.clear();
        for(size_t i = 0; i < positions.size(); i++) {
            lines.position.push_back({ positions[i].x, positions[i].y, m_Params.z.y }); 
        }
    viewerLineLists->push_back(lines);
     
    if(isActive) {
        if(auto P1 = settings.p.sketch.cursor.Position_Snapped) {
            // draw line / arc to current mouse position
            lines.position.clear();
            points.position.clear();
                Vec2 p0 = positions.back();
                const Vec2& p1 = { (*P1).x,  (*P1).y };

                if(m_ActiveCommand == Command::Line) {
                    points.position.push_back({ p1.x, p1.y, 0.0f });
                    lines.position.push_back({ p0.x, p0.y, 0.0f });
                    lines.position.push_back({ p1.x, p1.y, 0.0f });
                }
                if(m_ActiveCommand == Command::Arc) {  
                    // midpoint
                    Vec2 centre = (p0 + p1) / 2.0f;
                    // add points
                    points.position.push_back({ p1.x, p1.y, 0.0f });
                    points.position.push_back({ centre.x, centre.y, 0.0f });
                    // get path of arc as lines
                    int direction = Geom::Direction::CW;
                    double arcTolerance = 15;
                    std::vector<Vec2> arcPath = elementFactory.Element_GetArcPath(p0, p1, direction, centre, arcTolerance);
                    // add lines
                    for(const Vec2& p : arcPath) {
                        lines.position.push_back({ p.x, p.y, 0.0f });
                    }
                }
            viewerLineLists->push_back(lines);
            viewerPointLists->push_back(points);
        }
    }
} 

// update viewer
void A_Drawing::UpdateViewer(Settings& settings, std::vector<DynamicBuffer::ColouredVertexList>* viewerLineLists, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists)
{
    // update viewer for each active function
    for (size_t i = 0; i < m_ActiveA_Functions.Size(); i++) {
        bool isActiveItem = (m_ActiveA_Functions.CurrentIndex() == (int)i);
        m_ActiveA_Functions[i]->UpdateViewer(settings, m_ElementFactory, viewerLineLists, viewerPointLists, isActiveItem);
    }
}
// update viewer
void A_Drawing::RawPoints_UpdateViewer(Settings& settings, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists)
{
    // add raw points to m_ViewerPointLists 
    DynamicBuffer::ColouredVertexList rawPoints(settings.p.sketch.point.colour);
    
    std::vector<Vec2> positions = m_ElementFactory.RawPoint_PointsList();
    // add positions at z = 0
    for(size_t i = 0; i < positions.size(); i++) {
        rawPoints.position.push_back({ positions[i].x, positions[i].y, 0.0f });
    }
    viewerPointLists->push_back(std::move(rawPoints));
}

// update viewer
void A_Drawing::ActivePoint_UpdateViewer(Settings& settings, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists)
{    
    if(auto position = m_ElementFactory.ActivePoint_GetPosition()) {
        // add raw points to m_ViewerPointLists 
        DynamicBuffer::ColouredVertexList rawPoints(settings.p.sketch.point.colourActive);
        // add position at z = 0
        rawPoints.position.push_back({ position->x, position->y, 0.0f });
        viewerPointLists->push_back(std::move(rawPoints));
    }  
}    

 
void SketchOld::ActiveDrawing_UpdateViewer(Settings& settings)
{     
    if(!m_Drawings.HasItemSelected()) { return; }
    
  //  std::cout << "Updating: Drawing" << std::endl;
    // make a list of points / lines which is sent to viewer
    m_ViewerLineLists.clear();
    m_ViewerPointLists.clear();
    
    // update the drawing
    m_Drawings.CurrentItem().UpdateViewer(settings, &m_ViewerLineLists, &m_ViewerPointLists);
    // add points to m_ViewerPointLists
    m_Drawings.CurrentItem().RawPoints_UpdateViewer(settings, &m_ViewerPointLists);
    // add active point
    m_Drawings.CurrentItem().ActivePoint_UpdateViewer(settings, &m_ViewerPointLists);
    // dispatch points / lines
    Event<Event_Viewer_AddLineLists>::Dispatch( { &m_ViewerLineLists } );
    Event<Event_Viewer_AddPointLists>::Dispatch( { &m_ViewerPointLists } );
}


// update viewer
std::optional<std::vector<std::string>> A_Drawing::ActiveA_Function_UpdateViewer(Settings& settings)
{
    if(!m_ActiveA_Functions.HasItemSelected()) {
        Log::Error("No active function selected");
        return {};  
    }
    // add gcode path of current active function to viewer
    return move(m_ActiveA_Functions.CurrentItem()->InterpretGCode(settings, m_ElementFactory));
}   

void SketchOld::ActiveA_Function_UpdateViewer(Settings& settings)
{ 
    // get current active function's gcodes and update viewer
    if(!m_Drawings.HasItemSelected()) { return; }
    std::cout << "Updating: Draw A_Function" << std::endl;
    // update the active function 
    if(auto gcodes = m_Drawings.CurrentItem().ActiveA_Function_UpdateViewer(settings)) {
        Event<Event_Update3DModelFromVector>::Dispatch({ vector<string>(move(*gcodes)) }); 
    } else {
        Event<Event_Update3DModelFromVector>::Dispatch({ vector<string>(/*empty*/) });
    }
}

void SketchOld::UpdateViewer(Settings& settings)
{
    // return if no update required
    if(settings.GetUpdateFlag() == ViewerUpdate::None) { return; }
    
    //std::cout << "*** UPDATE FLAG *** : " << (int)settings.GetUpdateFlag() << std::endl;
    
    // clear overrides other bits in flag
    if((int)settings.GetUpdateFlag() & (int)ViewerUpdate::Clear) { 
        std::cout << "Clearing viewer" << std::endl;  
        ClearViewer(); 
    } 
    else {     
        // update active drawing
        if((int)settings.GetUpdateFlag() & (int)ViewerUpdate::ActiveDrawing)  { /*8std::cout << "UPDATING ACTIVE DRAWING" << std::endl; */  ActiveDrawing_UpdateViewer(settings); }
        // update active function
        if((int)settings.GetUpdateFlag() & (int)ViewerUpdate::ActiveFunction) { /*std::cout << "UPDATING ACTIVE FUNCTION" << std::endl; */ ActiveA_Function_UpdateViewer(settings); }
    }
    
    // reset the update flag
    settings.ResetUpdateFlag();
}

void SketchOld::ClearViewer()
{
    // line lists
    Event<Event_Viewer_AddLineLists>::Dispatch( { nullptr } );
    // points lists
    Event<Event_Viewer_AddPointLists>::Dispatch( { nullptr } );
    //
    Event<Event_Update3DModelFromVector>::Dispatch({ vector<string>(/*empty*/) });
}








void SketchOld::Activate()
{
    if(m_IsActive == false) {
        m_IsActive = true;
        Event<Event_Set2DMode>::Dispatch( { true } );
    }
}

void SketchOld::Deactivate()
{
    if(m_IsActive == true) {
        m_IsActive = false;
        Event<Event_Set2DMode>::Dispatch( { false } );
    }
}



















































/*


        SketchOld 
        -> EnterSketchOldMode();
        -> DrawElement();





        Element Point/Line/Arc/Circle
            m_P0 = RawPoint
        
        -> RawPoint  
            m_Parent = Element
            m_Constraints


        SketchOld_Element == unique_ptr<Element_Identifier>
            ~SketchOld_Element() { deleteMe(); }
            


        ElementFactory
            vector<RawPoint> m_RawPoints;
            vector<Element> m_Elements;

            

*/







} // end namespace Sqeak








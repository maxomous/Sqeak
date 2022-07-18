
#include <iostream>
#include "../glcore/deps/imgui/imgui.h"
#include "sketch.h"

 
 

        
namespace Sketch {

using namespace MaxLib::String;
using namespace MaxLib::Geom;


PolygonisedGeometry::PolygonisedGeometry(Sketcher* parent, const std::vector<Geometry>& geometries) 
    : m_Parent(parent)
{
    Geos geos;
    // Polygonise the line data
    Geos::PolygonisedData polygonised = geos.Polygonise(geometries);
    // move data into 
    //m_Geometry.clear();
    for(auto& geometry : polygonised.valid)     { m_Geometry.push_back(std::move(geometry)); }
    //for(auto& geometry : polygonised.cuts)      { m_Geometry.push_back({ SelectableGeometry::Type::Cuts, std::move(geometry) }); }
    for(auto& geometry : polygonised.dangles)   { m_Geometry.push_back(std::move(geometry)); }
    //for(auto& geometry : polygonised.invalid)   { m_Geometry.push_back({ SelectableGeometry::Type::Invalid, std::move(geometry) }); }
}

// Clears all of the geometries' hovered flags
void PolygonisedGeometry::ClearHovered() {
    for(auto& geometry : m_Geometry) {
        geometry.m_IsHovered = false;
    }
}
// Clears all of the geometries' seleceted flags
void PolygonisedGeometry::ClearSelected() {
    for(auto& geometry : m_Geometry) {
        geometry.m_IsSelected = false;
    }
}
// Finds geometry within a tolerance to position p and sets their Hovered flag to true
bool PolygonisedGeometry::SetHoveredByPosition(const Vec2& p, double tolerance) {
    return FindIntersects(p, tolerance, [](SelectableGeometry& geometry) {
        geometry.m_IsHovered = true;
    });
}   
// Finds geometry within a tolerance to position p and sets their Selected flag to true
bool PolygonisedGeometry::SetSelectedByPosition(const Vec2& p, double tolerance) {
    return FindIntersects(p, tolerance, [](SelectableGeometry& geometry) {
        geometry.m_IsSelected = true;
    });
}
 
// Calls callback function on each SelectableGeometry item  
void PolygonisedGeometry::ForEachGeometry(std::function<void(SelectableGeometry&)> cb)  {
    for(auto& geometry : m_Geometry) {
        cb(geometry);
    }
}
 
// Finds geometry within a tolerance to position p and sets their hovered flag to true
// l is a polygon which 
bool PolygonisedGeometry::FindIntersects(const Vec2& p, double tolerance, std::function<void(SelectableGeometry&)> cb) 
{
    // draw a tolerence ring around point, and check if intersect
    LineString p_with_tol = m_Parent->Factory().RenderCircle(p, tolerance);
    // create instance of geos for calculating intersect
    Geos geos;
    
    bool itemFound = false;
    ForEachGeometry([&](SelectableGeometry& geometry) 
    {
        if(itemFound) { return; } // skip to end after first item found
        
        // Check whether geometry intersects with p
        if(auto success = geos.Intersect(p_with_tol, geometry.Geometry())) {
            if(*success) {
                cb(geometry);
                // TODO: this just returns first found
                itemFound = true;
            }
        } 
    });
    return itemFound;
}



SketchRenderer::SketchRenderer(Sketcher* parent) 
    : m_Parent(parent) {}
    
    

const RenderData& SketchRenderer::GetRenderData() const { 
    return m_RenderData; 
}
 


bool SketchRenderer::UpdateRenderData()
{

    auto RenderElements = [&](RenderData::Data& data, std::function<bool(const Item&)> cb_Condition = [](const Item& item){ (void)item; return true; }) { 
                
        // Clear the old data
        data.Clear();
        // Draw Element / Points
        m_Parent->Factory().ForEachElement([&](Sketch::Element* element) {
            // Point buffer
            Points points;
            // Go through each point
            element->ForEachItemPoint([&](Sketch::Item_Point& item) {
                // If condition met, add point to buffer
                if(cb_Condition(item)) {
                    points.push_back(item.p);                
                }
            });
            // Add Point(s) to render data
            if(!points.empty()) { data.points.push_back(std::move(points)); }
            
            // Return early if condtion not met for element
            if(!cb_Condition(element->Item_Elem())) { return; }
            
            // Line Data
            LineString l;
            // skip point, as it is added to pointdata above 
            if(auto* point = dynamic_cast<const Sketch::Point*>(element))           { (void)point; return; } 
            // other element are addded with line buffer 
            else if(auto* line = dynamic_cast<const Sketch::Line*>(element))        { l = m_Parent->Factory().RenderLine(line->P0(), line->P1()); }
            else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element))          { l = m_Parent->Factory().RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction()); }
            else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element))    { l = m_Parent->Factory().RenderCircle(circle->PC(), circle->Radius()); }
            else { assert(0 && "Cannot render element, type unknown"); }            // Should never reach
            
            assert(!l.empty() && "No render data in linestring");
            
            // Add Element to renderdata
            data.linestrings.push_back(std::move(l));
        }); 
        
    };
        
    
    auto RenderPolygonisedElements = [&](RenderData::Data& data, std::function<bool(const SelectableGeometry&)> cb_Condition = [](const SelectableGeometry& item){ (void)item; return true; }) { 
                
        // draw polygonised geometry
        m_Parent->Events().m_PolygonisedGeometry.ForEachGeometry([&](SelectableGeometry& geometry) {
                // If condition met, add point to buffer
            if(cb_Condition(geometry)) { 
                data.linestrings.push_back(geometry.Geometry()); 
            }
        });
    };

    // return if no update required
    if(m_Update == UpdateFlag::None) { return false; }
    
    // A full update includes system which will also reset everything, including hovered / selected flags
    if(m_Update & UpdateFlag::System) {
        std::cout << "updating System" << std::endl;
        // reset dragged selection box
        m_Parent->Events().m_IsDragSelectionBox = false;                
        // reset preview data
        m_Parent->Events().m_InputData.clear();
        // clear the selected / hovered flags on elements (note: flags on polygonised geometry are reset with UpdateFlag::Elements)
        m_Parent->Factory().ClearSelected();
        m_Parent->Factory().ClearHovered();
    } 
        
    RenderData::Data_Selectable& elements = m_RenderData.elements;
        
    // Update Elements
    if(m_Update & UpdateFlag::Elements) {
        std::cout << "updating Elements" << std::endl;
    
        // Render all items    
        RenderElements(elements.unselected);
        
        // Update the polygonised geometry (Selectable polygons / dangles geometry from all of the element's linestrings)
        m_Parent->Events().m_PolygonisedGeometry = { m_Parent, elements.unselected.linestrings };
    }
    
    // Update Selected / Hovered
    if(m_Update & UpdateFlag::Selection) {
        std::cout << "updating Selection" << std::endl;
         
        // Render all selected items
        RenderElements(elements.selected, [](const Item& item) { return item.IsSelected(); });
        // Render all hovered items
        RenderElements(elements.hovered,  [](const Item& item) { return item.IsHovered();  });

        // Render all selected polygonised items
        RenderPolygonisedElements(elements.selected, [](const SelectableGeometry& item) { return item.IsSelected(); });
        // Render all hovered polygonised items
        RenderPolygonisedElements(elements.hovered,  [](const SelectableGeometry& item) { return item.IsHovered(); });
    }
    
    
    
/*
    
    // Update Constraints
    if(m_Update & UpdateFlag::Constraints) { 
    
        Data_Selectable& constraints = m_RenderData.constraints;
        constraints.Clear();
        constraints.unselected.points
        constraints.unselected.linestrings
        constraints.selected.points
        constraints.selected.linestrings
        constraints.hovered.points
        constraints.hovered.linestrings

        m_Parent->Factory().ForEachConstraint([&](const Sketch::Constraint* constraint) 
        {
            if(auto* c = dynamic_cast<const Sketch::Constraint_Template_OneItem*>(constraint)) {
                (void)c;
                //points.push_back(c->P());
                return;
            } else if(auto* c = dynamic_cast<const Sketch::Constraint_Template_OneItem*>(constraint)) {
                (void)c;
                //points.push_back(c->P());
                return;
            }
        });
    }
*/
    
     
    // Update Preview
    if(m_Update & UpdateFlag::Preview) {
        std::cout << "updating Preview" << std::endl;
        UpdatePreview();
    }
    
    if(m_Update & UpdateFlag::Cursor) { 
        std::cout << "updating Cursor" << std::endl;

        RenderData::Data& cursor = m_RenderData.cursor;
        // Clear old selection box data
        cursor.Clear();
        
        // Render dragged selection box
        if(m_Parent->Events().m_IsDragSelectionBox) {
        
            Vec2& p0 = m_Parent->Events().m_CursorClickedPos;
            Vec2& p1 = m_Parent->Events().m_CursorPos;
                
            cursor.points.push_back({ p0, { p0.x, p1.y }, p1, { p1.x, p0.y } });
            cursor.linestrings.push_back({ p0, { p0.x, p1.y }, p1, { p1.x, p0.y }, p0 });
        } 
        // if not dragging, render cursor as point
        else { 
            cursor.points.push_back({ m_Parent->Events().m_CursorPos });
        }
    }
    
    // Clear the update flag
    m_Update = UpdateFlag::None;
    std::cout << "Resetting UpdateFlag" << std::endl;
    return true;
}



void SketchRenderer::SetUpdateFlag(UpdateFlag flag) 
{ 
    m_Update = m_Update | flag; 
}



// Render preview element 
void SketchRenderer::UpdatePreview() 
{
    
    RenderData::Data& preview = m_RenderData.preview;
    // Clear old preview data
    preview.Clear();
        
    Points points;
    LineString linestring;
    
    SketchEvents::CommandType command   = m_Parent->Events().m_CommandType;
    const Vec2& p                       = m_Parent->Events().m_CursorPos; 
    std::vector<Vec2>& inputData        = m_Parent->Events().m_InputData;
    Direction inputDirection            = m_Parent->Events().m_InputDirection;
    
    // Render Point     
    if(command == SketchEvents::CommandType::Add_Point) {
        points.push_back(p);
            
    }
    // Render Line
    else if(command == SketchEvents::CommandType::Add_Line) {
        if(inputData.size() > 0) {
            points.push_back(inputData[0]);
            linestring = m_Parent->Factory().RenderLine(p, inputData[0]);
        }
        points.push_back(p); 
    }
    // Render Arc
    else if(command == SketchEvents::CommandType::Add_Arc) {
        if(inputData.size() == 0) {
            points.push_back(p); 
        }
        else if(inputData.size() == 1) {
            points.push_back(inputData[0]); 
            points.push_back(p); 
            Vec2 midPoint = (p + inputData[0]) / 2;
            linestring = m_Parent->Factory().RenderArc(inputData[0], p, midPoint, inputDirection);    
            // TODO: Direction should be settable
        }
        else if(inputData.size() == 2) {
            points.push_back(inputData[0]); 
            points.push_back(inputData[1]); 
            
            // Calculate centre based on p
            Vec2 newCentre = m_Parent->Factory().AdjustCentrePoint(inputData[0], inputData[1], p);
            points.push_back(newCentre); 
                        
            linestring = m_Parent->Factory().RenderArc(inputData[0], inputData[1], newCentre, inputDirection);
        }
    }
    // Render Circle
    else if(command == SketchEvents::CommandType::Add_Circle) {
        if(inputData.size() == 0) {
            points.push_back(p); 
        }
        else if(inputData.size() == 1) {
            points.push_back(inputData[0]); 
            // calculate circle from radius (p to pC)
            linestring = m_Parent->Factory().RenderCircle(inputData[0], Hypot(p-inputData[0]));
        }
    }
    
    if(!points.empty())       { preview.points.push_back(std::move(points)); }
    if(!linestring.empty())   { preview.linestrings.push_back(std::move(linestring)); }
}
    
    





SketchEvents::CommandType SketchEvents::GetCommandType() const { return m_CommandType; }

void SketchEvents::SetCommandType(CommandType commandType) {
    m_CommandType = commandType;  
}




void SketchEvents::Event_Keyboard(int key, KeyAction action, KeyModifier modifier) 
{   
    (void)modifier;
    // GLFW_KEY_DELETE
    if (key == 261 && action == KeyAction::Press) {
        m_Parent->Factory().DeleteSelection();
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
    }
}



// Mouse Button Event
//
//   On Left Click                 
//       -  select item
//       -  if(there is no item), deselect items
//   Left Click w/ Ctrl or Shift
//       -  add to selection
//   On Release
//       -  if(selection box) select these items

bool SketchEvents::Mouse_Button(MouseButton button, MouseAction action, KeyModifier modifier) 
{    
    m_MouseButton = button; 
    m_MouseAction = action;
    // Update cursor clicked pos
    if(button == MouseButton::Left && action == MouseAction::Press) { 
        m_CursorClickedPos = m_CursorPos; 
    }

    // TODO: Work out where point should be (i.e. if snapped etc) 
    
    // Return if no command set
    if(m_CommandType == CommandType::None) { return false; } // do nothing - this assumes we dont need to update if only the cursor position has changed
    // Handle Select Command
    else if(m_CommandType == CommandType::Select) {
        
        if(button == MouseButton::Left && action == MouseAction::Press) {
            // Reset selected item if ctrl or shift is not pressed
            if(modifier != KeyModifier::Ctrl && modifier != KeyModifier::Shift) {
                m_Parent->Factory().ClearSelected();
            }
            
            bool success = m_Parent->Factory().SetSelectedByPosition(m_CursorPos, m_SelectionTolerance);
            // start dragging selection box if no item under cursor
            m_IsDragSelectionBox = !success;
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
            
        }
        else if(button == MouseButton::Left && action == MouseAction::Release) {
            // ensure bounding box hsa a size
            if(m_CursorPos == m_CursorClickedPos)  {
                m_IsDragSelectionBox = false;
            }
            // If we have dragged the cursor whilst button was pressed
            if(m_IsDragSelectionBox) {
                // Reset selected item if ctrl or shift is not pressed
                if(modifier != KeyModifier::Ctrl && modifier != KeyModifier::Shift) {
                    m_Parent->Factory().ClearSelected();
                }
                bool includePartiallyInside = (m_CursorPos.x - m_CursorClickedPos.x) < 0;
                // Find selection inside selection box
                m_Parent->Factory().AddSelectionBetween(m_CursorPos, m_CursorClickedPos, includePartiallyInside);
                m_IsDragSelectionBox = false;                
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection | UpdateFlag::Cursor);
            }
        }    
    }
    
    // Handle Select loop Command
    else if(m_CommandType == CommandType::SelectLoop) {
        
        if(button == MouseButton::Left && action == MouseAction::Press) {
            // Reset selected item if ctrl or shift is not pressed
            if(modifier != KeyModifier::Ctrl && modifier != KeyModifier::Shift) {
                m_Parent->Events().m_PolygonisedGeometry.ClearSelected();
            } 
            // selected item at cursorpos
            m_Parent->Events().m_PolygonisedGeometry.SetSelectedByPosition(m_CursorPos, m_SelectionTolerance);
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
        }  
    }
    // Handle add element commands
    else if(m_CommandType == CommandType::Add_Point || m_CommandType == CommandType::Add_Line || m_CommandType == CommandType::Add_Arc || m_CommandType == CommandType::Add_Circle) 
    {
        if(button == MouseButton::Left && action == MouseAction::Press) {
        
            m_InputData.push_back(m_CursorClickedPos); 
            // Handle Add Point     
            if(m_CommandType == CommandType::Add_Point) {
                m_Parent->Factory().AddPoint(m_InputData[0]);
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
            }
            // Handle Add Line
            else if(m_CommandType == CommandType::Add_Line) {
                if(m_InputData.size() == 2) {
                    m_Parent->Factory().AddLine(m_InputData[0], m_InputData[1]);
                    // Update render data
                    m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
                }
            }
            // Handle Add Arc
            else if(m_CommandType == CommandType::Add_Arc) {
                if(m_InputData.size() == 3) {
                    // make sure points arent the same
                    if(!(m_InputData[0] == m_InputData[1] && m_InputData[1] == m_InputData[2])) {
                        // Calculate centre from point
                        Vec2 newCentre = m_Parent->Factory().AdjustCentrePoint(m_InputData[0], m_InputData[1], m_InputData[2]);
                        m_Parent->Factory().AddArc(m_InputData[0], m_InputData[1], newCentre, m_Parent->Events().m_InputDirection);    
                        // Update render data
                        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);                
                    }
                }
            }
            // Handle Add Circle
            else if(m_CommandType == CommandType::Add_Circle) {
                if(m_InputData.size() == 2) {
                    double radius = Hypot(m_InputData[1] - m_InputData[0]);
                    m_Parent->Factory().AddCircle(m_InputData[0], radius);
                    // Update render data
                    m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
                }
            }
            else {
                assert(0 && "Command doesn't exist");
            }
            
        }
    }
    return true;
}

// Mouse Move Event
//
//  Input should be (x, y) coords in sketch space
//
//  Hover over item -  Highlights it
//  Drag            -  if (clicked)
//                        -  if(Selected) drag items
//                        -  if(!selected) drag selection box

// return true if update required
bool SketchEvents::Mouse_Move(const Vec2& p)
{ 
    
    // Reset the mouse button / action
    if(m_MouseAction == MouseAction::Release) { m_MouseButton = MouseButton::None; m_MouseAction = MouseAction::None; }
    
    // TODO: Work out where point should be (i.e. if snapped etc)
    
    // return early if no change to p, to prevent solving constraints unnecessarily 
    if(p == m_CursorPos) { return false; }
    
            
    Vec2 pDif = p - m_CursorPos;
    // Update cursor
    m_CursorPos = p;
    // Update render data
    m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Cursor);
    
    // Return if no command set
    if(m_CommandType == CommandType::None) { return false;} // do nothing - this assumes we dont need to update if only the cursor position has changed
    // Hover / drag / 
    else if(m_CommandType == CommandType::Select) {
        
        // Highlight item if hovered over
        if(m_MouseButton == MouseButton::None) { 
            m_Parent->Factory().ClearHovered();
            m_Parent->Factory().SetHoveredByPosition(m_CursorPos, m_SelectionTolerance);            
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
        }
        
        if(m_MouseButton == MouseButton::Left && m_MouseAction == MouseAction::Press) {
            // Drag items(s)
            if(m_IsDragSelectionBox) {
                // currently dragging... already updating cursor
            } else {
                std::cout << "p: " << p << std::endl;
                m_Parent->SolveConstraints(pDif);
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::FullNoSystem);
            }
        }
    } 
    // Hover / drag / 
    else if(m_CommandType == CommandType::SelectLoop) {
        
        // Highlight item if hovered over
        if(m_MouseButton == MouseButton::None) { 
            m_Parent->Events().m_PolygonisedGeometry.ClearHovered();
            m_Parent->Events().m_PolygonisedGeometry.SetHoveredByPosition(m_CursorPos, m_SelectionTolerance);
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
        }
    } 
    // Preview New Element
    else if(m_CommandType == CommandType::Add_Point || m_CommandType == CommandType::Add_Line || m_CommandType == CommandType::Add_Arc || m_CommandType == CommandType::Add_Circle) {
        // update the preview render data from the cursorPos
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Preview);
    }
    
    return true;
} 

    
    
    

Sketcher::Sketcher() 
    : m_Events(this), m_Renderer(this)
{
    
    ElementFactory& f = Factory(); // sketcher.Factory();
    
    ElementID l1 = f.AddLine({ 100.0f, 100.0f }, { 200.0f, 523.0f });
    ElementID l2 = f.AddLine({ 200.0f, 523.0f }, { 500.0f, 500.0f });
    ElementID l3 = f.AddLine({ 500.0f, 500.0f }, { 100.0f, 100.0f });
    
    //Circle* circle = m_Factory.AddCircle({ -6.0f, -7.0f }, 200.0f);
    
    //m_Factory.AddConstraint<Coincident_PointToPoint>(l1.p1, p1);
    
    typedef SketchItem::Type Type;
    
        f.AddConstraint<Coincident_PointToPoint>(SketchItem({ Type::Line_P1, l1 }), SketchItem({ Type::Line_P0, l2 }));
        f.AddConstraint<Coincident_PointToPoint>(SketchItem({ Type::Line_P1, l2 }), SketchItem({ Type::Line_P0, l3 }));
        f.AddConstraint<Coincident_PointToPoint>(SketchItem({ Type::Line_P1, l3 }), SketchItem({ Type::Line_P0, l1 }));
        
        f.AddConstraint<Distance_PointToPoint>(SketchItem({ Type::Line, l1 }), 300.0f);
        f.AddConstraint<Distance_PointToPoint>(SketchItem({ Type::Line, l2 }), 400.0f);
        f.AddConstraint<Distance_PointToPoint>(SketchItem({ Type::Line, l3 }), 550.0f);        
    
    
    
    
    
    
}



void Sketcher::SolveConstraints(Vec2 pDif) 
{      
    // Update Solver set dragged point and its position
    if(bool success = m_Factory.UpdateSolver(pDif)) {
        (void)success;
    }
}



bool InputDouble2(std::string label, Vec2* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0) 
{
    bool isModified = false;
        
    // Make unique id for widgets
    std::string id = "##" + std::to_string((int)v);
    float widgetWidth = ImGui::GetWindowWidth() / 3.0f;
    
    // match spacing to inputfloat2 spacing
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { ImGui::GetStyle().ItemInnerSpacing.x, ImGui::GetStyle().ItemSpacing.y });
    
        ImGui::SetNextItemWidth(widgetWidth);
        isModified = ImGui::InputDouble(std::string(id + "x").c_str(), &(v->x), step, step_fast, format, flags);
        ImGui::SameLine();
        
        ImGui::SetNextItemWidth(widgetWidth);
        isModified |= ImGui::InputDouble(std::string(id + "y").c_str(), &(v->y), step, step_fast, format, flags);
        
        ImGui::SameLine();
        
        ImGui::SetNextItemWidth(widgetWidth);
        ImGui::TextUnformatted(label.c_str());
        
    ImGui::PopStyleVar();
                    
    return isModified;
}

//TODO:
//    on inputdouble enter, add the point (inputData.push_back(m_Events.m_CursorPos))
//    may be helpful? IsItemDeactivatedAfterEdit

void Sketcher::DrawImGui()
{
    
      /*
    // Cursor Popup
    static ImGuiModules::ImGuiPopup popup_CursorRightClick("popup_Sketch_CursorRightClick");
    // open
    if(m_Drawings.HasItemSelected()) 
    {
        if(auto id =  m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_GetID()) 
        {
            // set to open
            if(!ImGui::GetIO().WantCaptureMouse && IsActive()) {
                if(trigger(settings.p.sketch.cursor.popup.shouldOpen)) { popup_CursorRightClick.Open(); }
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
    
    */
    
    // set default size / position
    //ImGui::SetNextWindowSize(m_Size, ImGuiCond_Appearing);
    //ImGui::SetNextWindowPos(m_Pos, ImGuiCond_Appearing);
    
    ImGui::SetNextWindowPos(ImVec2(), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    // id of element / contraint to be deleted - must only modify at end / beginning of frame
    ElementID deleteElement = 0;
    ConstraintID deleteConstraint = 0;
        
    // Main body of the Demo window starts here.
    if (ImGui::Begin("SketchApp", NULL))
    {
    
    
    
        if(ImGui::Button("Update")) {
            SolveConstraints();
            // Update render data
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);
        }
        
        ImGui::Separator();
        
        if (ImGui::InputInt("Arc Segments", &m_Factory.arcSegments)) {
            m_Factory.arcSegments = abs(m_Factory.arcSegments); // it is uint
            // Update render data
            m_Renderer.SetUpdateFlag(UpdateFlag::Elements);
        }
        ImGui::InputDouble("Selection Tolerance", &m_Events.m_SelectionTolerance);
        
        ImGui::Separator();
        
        
        static int command = 0;
        if(ImGui::Combo("Command", &command, "None\0Select\0Select Loop\0Add Point\0Add Line\0Add Arc\0Add Circle\0\0")) {
            if(command == 0) { m_Events.SetCommandType(SketchEvents::CommandType::None); }
            if(command == 1) { m_Events.SetCommandType(SketchEvents::CommandType::Select); }
            if(command == 2) { m_Events.SetCommandType(SketchEvents::CommandType::SelectLoop); }
            if(command == 3) { m_Events.SetCommandType(SketchEvents::CommandType::Add_Point); }
            if(command == 4) { m_Events.SetCommandType(SketchEvents::CommandType::Add_Line); }
            if(command == 5) { m_Events.SetCommandType(SketchEvents::CommandType::Add_Arc); }
            if(command == 6) { m_Events.SetCommandType(SketchEvents::CommandType::Add_Circle); }
            
            // Update render data
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);
        }
    
        ImGui::Separator();
        
        // get currently selected points
        std::vector<Vec2>& inputData = m_Events.InputData();
        
        
        if(m_Events.GetCommandType() == SketchEvents::CommandType::None) {
            ImGui::TextUnformatted("Choose a Command");
        }
        else if(m_Events.GetCommandType() == SketchEvents::CommandType::Select) {
            ImGui::TextUnformatted("Select a Point or Element");
        }
        else if(m_Events.GetCommandType() == SketchEvents::CommandType::SelectLoop) {
            ImGui::TextUnformatted("Select Geometry Loop");
        }
        else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Point) {
            ImGui::TextUnformatted("Set Point Position");
                        
            if(InputDouble2("P (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            
            if(ImGui::Button("Add Point")) {
                m_Factory.AddPoint(m_Events.m_CursorPos);
                m_Renderer.SetUpdateFlag(UpdateFlag::Full);
            }
            
        }
        else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Line) {
            
            if(inputData.size() == 0)       { 
                ImGui::TextUnformatted("Set P0 Position");
                // draw p0
                if(InputDouble2("P0 (X, Y)", &m_Events.m_CursorPos)) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                // draw disabled p1
                ImGui::BeginDisabled();
                    static Vec2 p1_disabled = { 0.0f, 0.0f };
                    InputDouble2("P1 (X, Y)", &p1_disabled);
                ImGui::EndDisabled();
                                
            }
            else if(inputData.size() == 1)  { 
                ImGui::TextUnformatted("Set P1 Position"); 
                // draw p0
                if(InputDouble2("P0 (X, Y)", &inputData[0])) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                // draw p1
                if(InputDouble2("P1 (X, Y)", &m_Events.m_CursorPos)) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
            }        
            // Draw Add Line Button (Only enable button if p0 has a value)
            ImGui::BeginDisabled(inputData.size() == 0);
                if(ImGui::Button("Add Line")) {
                    m_Factory.AddLine(inputData[0], m_Events.m_CursorPos);
                    m_Renderer.SetUpdateFlag(UpdateFlag::Full);
                }
            ImGui::EndDisabled();
            
        }
        else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Arc) {
             // get currently selected points
            std::vector<Vec2>& inputData = m_Events.InputData();
            
            if(inputData.size() == 0)       { 
                ImGui::TextUnformatted("Set P0 Position");
                // draw p0
                if(InputDouble2("P0 (X, Y)", &m_Events.m_CursorPos)) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                // draw disabled p1 / pC
                ImGui::BeginDisabled();
                    static Vec2 p1_disabled = { 0.0f, 0.0f };
                    static Vec2 pC_disabled = { 0.0f, 0.0f };
                    InputDouble2("P1 (X, Y)", &p1_disabled);
                    InputDouble2("Centre (X, Y)", &pC_disabled);
                ImGui::EndDisabled();
                                
            }
            else if(inputData.size() == 1)  { 
                ImGui::TextUnformatted("Set P1 Position"); 
                // draw p0
                if(InputDouble2("P0 (X, Y)", &inputData[0])) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                // draw p1
                if(InputDouble2("P1 (X, Y)", &m_Events.m_CursorPos)) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                // draw disabled pC
                ImGui::BeginDisabled();
                    static Vec2 pC_disabled = { 0.0f, 0.0f };
                    InputDouble2("Centre (X, Y)", &pC_disabled);
                ImGui::EndDisabled();
            }   
            else if(inputData.size() == 2)  { 
                ImGui::TextUnformatted("Set PC Position"); 
                // draw p0
                if(InputDouble2("P0 (X, Y)", &inputData[0])) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                // draw p1
                if(InputDouble2("P1 (X, Y)", &inputData[1])) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                // draw p1
                if(InputDouble2("Centre (X, Y)", &m_Events.m_CursorPos)) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
            }        
            
            static int direction = 0;
            static float radius = 0.0f;
            
            radius = (inputData.size() < 2) ? 0.0f : Hypot(m_Events.m_CursorPos - inputData[0]);
            direction = (m_Events.m_InputDirection == Direction::CW) ? 0 : 1;
            
            ImGui::BeginDisabled(inputData.size() < 2);
                // Radius
                if(ImGui::InputFloat("Radius##Arc", &radius)) {
                    // update preview based on radius
                    m_Events.m_CursorPos = ArcCentreFromRadius(inputData[0], inputData[1], radius, m_Events.m_InputDirection);
                    
                    std::cout << "m_Events.m_CursorPos: " << m_Events.m_CursorPos << std::endl;
                    std::cout << "inputData[0]: " << inputData[0] << std::endl;
                    std::cout << "inputData[1]: " << inputData[1] << std::endl;
                    std::cout << "radius: " << radius << std::endl;
                    std::cout << "m_Events.m_InputDirection: " << m_Events.m_InputDirection << std::endl;
                    
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
            ImGui::EndDisabled();
            
            // Direction
            if(ImGui::Combo("Direction", &direction, "Clockwise\0Anti-Clockwise\0\0")) {
                m_Events.m_InputDirection = (direction == 0) ? Direction::CW : Direction::CCW;
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            
            ImGui::BeginDisabled(inputData.size() < 2);
            // Draw Add Line Button (Only enable button if p0 has a value)
                if(ImGui::Button("Add Arc")) {
                    
                    Vec2 new_pC = m_Factory.AdjustCentrePoint(inputData[0], inputData[1], m_Events.m_CursorPos);
                    m_Factory.AddArc(inputData[0], inputData[1], new_pC, m_Events.m_InputDirection);
                    m_Renderer.SetUpdateFlag(UpdateFlag::Full);
                }
            ImGui::EndDisabled();
                    
        }
        else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Circle) {
             // get currently selected points
            std::vector<Vec2>& inputData = m_Events.InputData();
            
            if(inputData.size() == 0)       { 
                ImGui::TextUnformatted("Set Centre Position");
                // draw pC
                if(InputDouble2("Centre (X, Y)", &m_Events.m_CursorPos)) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                                
            }
            else if(inputData.size() == 1)  { 
                ImGui::TextUnformatted("Set Radius"); 
                // draw pC
                if(InputDouble2("Centre (X, Y)", &inputData[0])) {
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
            }   
            
            // Only enable if pC has a value
            ImGui::BeginDisabled(inputData.size() == 0);
                // Direction
                static float radius = 0;
                if(ImGui::InputFloat("Radius##Circle", &radius)) {
                    // update preview based on radius
                    m_Events.m_CursorPos = inputData[0] + Vec2(0.0f, radius);
                    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
                }
                // Add Line Button
                if(ImGui::Button("Add Circle")) {
                    m_Factory.AddCircle(inputData[0], radius);
                    m_Renderer.SetUpdateFlag(UpdateFlag::Full);
                }
            ImGui::EndDisabled();
            
        }
        else { // should never reach
            assert(0 && "Type unknown"); 
        } 
        

    
        ImGui::Separator();
    
        ImGui::TextUnformatted("Input Data");
        for(auto& p : inputData) {
            ImGui::Text("(%g, %g)", p.x, p.y);
        }
    
    
        ImGui::Separator();
        
        // Add Constraint Buttons
        
        ImGui::Text("Add Constraint");
        
        
        // TODO: Order of selection needs to be preserved
        // TODO: Update when selected is changed, not every frame
        
        bool isNewConstraint = false;
        // Make an array of the selected points / elements
        std::vector<SketchItem> points;
        std::vector<SketchItem> elements;
        
        // TODO: Should only do this once per  update ...
        m_Factory.ForEachItemPoint([&points](Item_Point& item) {
            if(item.IsSelected()) { points.push_back(item.Reference()); }
        });
        m_Factory.ForEachItemElement([&elements](Item_Element& item) {
            // ignore points as elements (they are handled as points)
            if(item.Type() == SketchItem::Type::Point) { return; }
            if(item.IsSelected()) { elements.push_back(item.Reference()); }
        });
        
        auto distanceConstraint = [&](SketchItem p0, SketchItem p1) {
            
            // Add Distance constraint between 2 points
            static float distance = 100.0f;
            if(ImGui::Button("Distance")) {
                m_Factory.AddConstraint<Distance_PointToPoint>(p0, p1, distance);
                isNewConstraint = true;
            }
            ImGui::SameLine();
            ImGui::InputFloat("##distance", &distance);
        };
        
        
        
        if(elements.empty()) 
        {
            if(points.size() == 2) {
                // Add Coincident constraint between 2 points
                if(ImGui::Button("Coincident")) {
                    m_Factory.AddConstraint<Coincident_PointToPoint>(points[0], points[1]);
                    isNewConstraint = true;
                }
                // Add Distance constraint between 2 points
                distanceConstraint(points[0], points[1]);
            }
        } 
        else if(elements.size() == 1) {
            
            if(points.size() == 0) {
                if(elements[0].type == SketchItem::Type::Line) {
                    // Add Distance constraint of Line
                    distanceConstraint({ SketchItem::Type::Line_P0, elements[0].element }, { SketchItem::Type::Line_P1, elements[0].element });
                }
            }
        } 
        else if(elements.size() == 2) {
            
        } 
        
        
        
        if(isNewConstraint) {
            // We need to clear the selected items, otherwise when we update, it will fix all selected items to their current position
            m_Factory.ClearSelected();
            SolveConstraints();
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);            
        }
        
    
        
        
        // returns true if requires update
        DrawImGui_Elements(deleteElement);
        DrawImGui_Constraints(deleteConstraint);

    }
    ImGui::End(); 
        
     
        
        
        
        /*
        if (ImGui::SmallButton("New Drawing")) {
            m_Drawings.Add(A_Drawing("Drawing " + to_string(m_DrawingIDCounter++)));
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
    }*/
    
    // modify should be and start / end of frame
    if(deleteElement) { 
        m_Factory.RemoveElement(deleteElement); 
        m_Renderer.SetUpdateFlag(UpdateFlag::Full);
    }
    if(deleteConstraint) { 
        m_Factory.RemoveConstraint(deleteConstraint); 
        m_Renderer.SetUpdateFlag(UpdateFlag::Full);
    }
    
}



bool Sketcher::DrawImGui_Elements(ElementID& deleteElement) 
{    
    bool updateRequired = false;
    
    // Draw ImGui widgets for a point
    auto drawPointStats = [](const std::string& name, Item_Point& item) {
        ImGui::Text("%s: (%.3f, %.3f)", name.c_str(), item.p.x, item.p.y);
        if(item.IsSelected())  { ImGui::SameLine(); ImGui::Text("(Selected)"); }
        if(item.IsHovered())   { ImGui::SameLine(); ImGui::Text("(Hovered)"); }                        
    };
    
    // Draw ImGui Treenode widgets for an element
    auto drawElementTreeNode = [&deleteElement, &updateRequired](const std::string& name, Sketch::Element* element) {
        bool isTreeNodeOpen = ImGui::TreeNode(va_str("%s %d", name.c_str(), element->ID()).c_str());
        if(element->Item_Elem().IsSelected())  { ImGui::SameLine(); ImGui::Text("(Selected)"); }
        if(element->Item_Elem().IsHovered())   { ImGui::SameLine(); ImGui::Text("(Hovered)"); }  
        ImGui::SameLine(); 
        if(ImGui::SmallButton(va_str("Delete##%d", element->ID()).c_str())) { updateRequired = true; deleteElement = element->ID(); }
        return isTreeNodeOpen;                      
    };
    
    
    if (ImGui::CollapsingHeader("Elements"))
    {
        m_Factory.ForEachElement([&](Sketch::Element* element) {
            if(auto* point = dynamic_cast<Sketch::Point*>(element)) {
            
                if(drawElementTreeNode("Point", element)) {
                    drawPointStats("P", point->Item_P());
                    ImGui::TreePop(); ImGui::Separator();
                }
            }
            else if(auto* line = dynamic_cast<Sketch::Line*>(element)) {
                
                if(drawElementTreeNode("Line", element)) {
                    drawPointStats("P0", line->Item_P0());
                    drawPointStats("P1", line->Item_P1());
                    ImGui::TreePop(); ImGui::Separator();
                }
            }
            else if(auto* arc = dynamic_cast<Sketch::Arc*>(element)) {
                
                if(drawElementTreeNode("Arc", element)) {
                    drawPointStats("P0", arc->Item_P0());
                    drawPointStats("P1", arc->Item_P1());
                    drawPointStats("PC", arc->Item_PC());
                    ImGui::TreePop(); ImGui::Separator();
                }
            }
            else if(auto* circle = dynamic_cast<Sketch::Circle*>(element)) {
                
                if(drawElementTreeNode("Circle", element)) {
                    drawPointStats("PC", circle->Item_PC());
                    ImGui::Text("Radius: %.3f", circle->Radius());
                    ImGui::TreePop(); ImGui::Separator();
                }
            }
            else { // Should never reach
                assert(0 && "Cannot draw imgui for element, type unknown");                
            }
        });
    }
    ImGui::Separator();
    return updateRequired;
}

bool Sketcher::DrawImGui_Constraints(ConstraintID& deleteConstraint) 
{ 
    bool updateRequired = false;
    
    auto drawConstraintTreeNode = [&deleteConstraint, &updateRequired](const std::string& name, Sketch::Constraint* constraint) {
        bool isTreeNodeOpen = ImGui::TreeNode(va_str("%s %d", name.c_str(), constraint->ID()).c_str());
        if(constraint->Failed())  { ImGui::SameLine(); ImGui::Text("(Failed)"); }
        ImGui::SameLine(); 
        if(ImGui::SmallButton(va_str("Delete##%d", constraint->ID()).c_str())) { updateRequired = true; deleteConstraint = constraint->ID(); }
        return isTreeNodeOpen;                      
    };
    
    if (ImGui::CollapsingHeader("Constraints"))
    {
        m_Factory.ForEachConstraint([&](Sketch::Constraint* constraint) 
        {
            if(auto* c = dynamic_cast<Sketch::Constraint_Template_OneItem*>(constraint)) {
                
                if(drawConstraintTreeNode("Constraint", c)) {
                    // For each SketchItem in constraint
                    c->ForEachElement([&](SketchItem& ref) {
                        ImGui::Text("%s", ref.Name().c_str());
                    });
                    ImGui::TreePop(); ImGui::Separator();
                }
                
            } else if(auto* c = dynamic_cast<Sketch::Constraint_Template_TwoItems*>(constraint)) {
                
                if(drawConstraintTreeNode("Constraint", c)) {
                    // For each SketchItem in constraint
                    c->ForEachElement([&](SketchItem& ref) {
                        ImGui::Text("%s", ref.Name().c_str());
                    });
                    ImGui::TreePop(); ImGui::Separator();
                }
            }
        });
    }
    ImGui::Separator();
    return updateRequired;
}


} // end namespace Sketch

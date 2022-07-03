
#include <iostream>
#include "../glcore/deps/imgui/imgui.h"
#include "sketch.h"

 
// TODO: Check if point on arc works?


/*
    The following pattern allows us to be in control of all of the members within the factory, the user receives ID's for the members and uses these IDs to call functions.

        class Member
        {
        public:
            Member(Factory* parent) : parent(m_Parent), m_ID(#)
        private:
            Factory* m_Parent
            ID m_ID;
        };
         
        class Factory
        {
        public:
            CreateMember() { m_Members.push_back(...); return m_Members.back().ID(); }
            DoSomething(ID id)  { member = FindMember(id); do something... }
        private:
            vector<Member> m_Members;
            FindMember(ID id) { for each ... }
        };
        
        ** User **
        Factory factory;
        ID memberID = factory.CreateMember();
        factory.DoSomething(memberID)

 */
 
namespace Sketch {

using namespace MaxLib::String;
using namespace MaxLib::Geom;


SketchRenderer::SketchRenderer(Sketcher* parent) 
    : m_Parent(parent),
    m_PreviewPoints(Sketch::RenderData::RenderType::Points, RenderData::DataType::Preview),
    m_PreviewLines(RenderData::RenderType::LineStrings, RenderData::DataType::Preview)
{}
    
    
LineString SketchRenderer::RenderLine(const Vec2& p0, const Vec2& p1) const
{
    return std::move(LineString({ p0, p1 }));
}

LineString SketchRenderer::RenderArc(const Vec2& pC, double radius, Direction direction, double th_Start, double th_End) const
{
    LineString linestring;
    // Clean up angles
    CleanAngles(th_Start, th_End, direction);
    // Calculate increment from n segments in 90 degrees
    double th_Incr = direction * (M_PI / 2.0) / m_ArcSegments;
    // Calculate number of incrments for loop
    int nIncrements = floorf(fabsf((th_End - th_Start) / th_Incr));
    
    // from 'n == 1' because we have already calculated the first angle
    // to 'n == nIncremenets' to ensure last point is added
    for (int n = 0; n <= nIncrements; n++) {
        
        double th = (n == nIncrements) ? th_End : th_Start + n * th_Incr;
        // Calculate position from radius and angle
        Vec2 p = pC + Vec2(fabsf(radius) * sin(th), fabsf(radius) * cos(th));       
        
        // This prevents double inclution of point 
        if(!linestring.empty()) { 
            if(p == linestring.back()) { continue; }
        }
        
        //Add Line to output
        linestring.emplace_back(move(p));
    }
    return std::move(linestring);
}
LineString SketchRenderer::RenderArc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction) const
{
    // get start and end points relative to the centre point
    Vec2 v_Start    = p0 - pC;
    Vec2 v_End      = p1 - pC;
    // get start and end angles
    double th_Start = atan2(v_Start.x, v_Start.y);
    double th_End   = atan2(v_End.x, v_End.y);
    double radius   = hypot(v_End.x, v_End.y);
    // draw arc between angles
    return std::move(RenderArc(pC, radius, direction, th_Start, th_End));
}

LineString SketchRenderer::RenderCircle(const Vec2& pC, double radius) const {
    // draw arc between angles
    return std::move(RenderArc(pC, radius, Direction::CW, 0.0, 2.0 * M_PI));
}

Vec2 SketchRenderer::AdjustCentrePoint(const Vec2& p0, const Vec2& p1, const Vec2& pC) const {
    // Get listance between line and point
    double d = MaxLib::Geom::DistanceBetween(p0, p1, pC);
    
    double th = Polar(p1 - p0).th;
    double thPerpendicular = CleanAngle(th + M_PI_2);
    Polar newCentre = Polar(d, thPerpendicular);
    
    Vec2 pMid = (p0 + p1) / 2.0;
    int flipSide = LeftOfLine(p0, p1, pC) ? -1 : 1;
    return pMid + newCentre.Cartesian() * flipSide;
}

const std::vector<RenderData>& SketchRenderer::GetRenderData() const { 
    return m_RenderData; 
}
 
void SketchRenderer::UpdateRenderData()
{
    //PointsCollection& pointsCollection = m_RenderData.points;   // vector<vector<Vec2>>>
    //LineStrings& linestrings = m_RenderData.linestrings;        // vector<vector<Vec2>>>
    
    //pointsCollection.clear();
    //linestrings.clear();
    
    // Clear any existing data
    m_RenderData.clear();
    
    
    

    RenderData pointData(RenderData::RenderType::Points, RenderData::DataType::Items);  // takes in a vector<LineString> equiv.
    RenderData pointData_Selected(RenderData::RenderType::Points, RenderData::DataType::Selected);

    m_Parent->Factory().ForEachElement([&](Sketch::Element* element) {
        
        Points points;
        Points points_Selected;

        element->ForEachItemPoint([&](const Sketch::Item_Point& item) {
            Points& p = (item.IsSelected()) ? points_Selected : points ;
            p.push_back(item.p);
        });
        if(!points.empty())                 { pointData.Add(std::move(points)); } 
        if(!points_Selected.empty())        { pointData_Selected.Add(std::move(points_Selected)); } 
    });
    
    if(!pointData.Empty())              { m_RenderData.emplace_back(std::move(pointData)); } 
    if(!pointData_Selected.Empty())     { m_RenderData.emplace_back(std::move(pointData_Selected)); } 
    


    RenderData lineData(RenderData::RenderType::LineStrings, RenderData::DataType::Items);
    RenderData lineData_Selected(RenderData::RenderType::LineStrings, RenderData::DataType::Selected);
    
    m_Parent->Factory().ForEachElement([&](Sketch::Element* element) {
        
        LineString linestring;
        LineString linestring_Selected;
        LineString& l = (element->Item_Elem().IsSelected()) ? linestring_Selected : linestring;
        
        if(auto* point = dynamic_cast<const Sketch::Point*>(element))           { (void)point; } // do nothing
        else if(auto* line = dynamic_cast<const Sketch::Line*>(element))        { l = RenderLine(line->P0(), line->P1()); }
        else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element))          { l = RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction()); }
        else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element))    { l = RenderCircle(circle->PC(), circle->Radius()); }
        else { assert(0 && "Cannot render element, type unknown"); }            // Should never reach
        
        // Return values    
        if(!l.empty())             { lineData.Add(std::move(l)); }    
    }); 
    
    // Add Selected Data
    if(!lineData.Empty())               { m_RenderData.emplace_back(std::move(lineData)); }  
    if(!lineData_Selected.Empty())      { m_RenderData.emplace_back(std::move(lineData_Selected)); }  
    
    /*   
    m_Parent->Factory().ForEachElement([&](const Sketch::Element* element) {
        
        Points points;
        LineString linestring;
        
        Points points_Selected;
        LineString linestring_Selected;
        
        
        auto AddPoint = [&points, &points_Selected](bool isSelected, const Vec2& p) {
            (isSelected) ? points_Selected.push_back(p) : points.push_back(p);
        };
        auto AddLineString = [&linestring, linestring_Selected](bool isSelected, const LineString& l) {
            (isSelected) ? linestring_Selected.push_back(l) : linestring.push_back(l);
        };
        
        
        if(auto* point = dynamic_cast<const Sketch::Point*>(element)) {
            AddPoint(point->IsSelected_P(), point->P());
            //points.push_back(point->P());
        }
        else if(auto* line = dynamic_cast<const Sketch::Line*>(element)) {
            AddPoint(point->IsSelected_P0(), point->P0());
            AddPoint(point->IsSelected_P1(), point->P1());
            AddLineString(point->IsSelected(), RenderLine(line->P0(), line->P1()));
            //points.push_back(line->P0());
            //points.push_back(line->P1());
            //linestring = RenderLine(line->P0(), line->P1());
        }
        else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element)) {
            AddPoint(arc->IsSelected_P0(), arc->P0());
            AddPoint(arc->IsSelected_P1(), arc->P1());
            AddPoint(arc->IsSelected_PC(), arc->PC());
            AddLineString(arc->IsSelected(), RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction()));
            //points.push_back(arc->P0());
            //points.push_back(arc->P1());
            //points.push_back(arc->PC());
            //linestring = RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction());
        }
        else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element)) {
            AddPoint(circle->IsSelected_PC(), circle->PC());
            AddLineString(circle->IsSelected(), RenderCircle(circle->PC(), circle->Radius()));
            //points.push_back(circle->PC());
            //linestring = RenderCircle(circle->PC(), circle->Radius());
        }
        else { // Should never reach
            assert(0 && "Cannot render element, type unknown");                
        }
        
        // Return values
        //if(!points.empty())             { pointsCollection.emplace_back(std::move(points)); } 
        //if(!linestring.empty())         { linestrings.emplace_back(std::move(linestring)); }    
        if(!points.empty())                 { pointData.Add(std::move(points)); } 
        if(!linestring.empty())             { lineData.Add(std::move(linestring)); }    
        if(!points_Selected.empty())        { pointData_Selected.Add(std::move(points_Selected)); } 
        if(!linestring_Selected.empty())    { lineData_Selected.Add(std::move(linestring_Selected)); }    
    
    });
    */
    
            
    
    
      
    /*
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
    
    */
    
    
    // Add preview render data
    if(!m_PreviewPoints.Empty())       { m_RenderData.emplace_back(std::move(m_PreviewPoints)); }
    if(!m_PreviewLines.Empty())        { m_RenderData.emplace_back(std::move(m_PreviewLines)); }
    

    
}

SketchEvents::CommandType SketchEvents::GetCommandType() const { return m_CommandType; }

void SketchEvents::GetCommandType(CommandType commandType) {
    m_CommandType = commandType;
    m_InputData.clear();   
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

void SketchEvents::Mouse_Button(MouseButton button, MouseAction action, KeyModifier modifier) 
{    
    m_MouseButton = button;
    m_MouseAction = action;
    // Update cursor clicked pos
    if(button == MouseButton::Left && action == MouseAction::Press) { m_CursorClickedPos = m_CursorPos; }

    // Return if no command set
    if(m_CommandType == CommandType::None) { return; }
    
    // TODO: Work out where point should be (i.e. if snapped etc) 
    
    
    // Handle Select Command
    if(m_CommandType == CommandType::Select) {
        
        if(button == MouseButton::Left && action == MouseAction::Press) {
            // Reset selected item if ctrl or shift is not pressed
            if(!(modifier == KeyModifier::Ctrl || modifier == KeyModifier::Shift)) {
                m_Parent->Factory().ClearSelection();
                //ClearSelected();
            }
            
            m_Parent->Factory().AddSelectionByPosition(m_CursorPos, m_SelectionTolerance);
            
            
           //// Find closest items to p
           //std::vector<std::pair<SketchItem, double>> items = m_Parent->Factory().GetItemsByPosition(m_CursorPos, m_SelectionTolerance);
           //// Deselect items and return if no point found
           //if(!items.empty()) { 
           //    SketchItem selectedItem = items[0].first;
           //    
           //    // Check if it exists inside vector
           //    bool itemExists = std::find_if(m_Selected.begin(), m_Selected.end(), [&](const SketchItem& item) { return ((item.element == selectedItem.element) && (item.type == selectedItem.type)); }) != m_Selected.end();
           //    // Add closest item to selection if it doesn't already exist
           //    if(!itemExists) { m_Selected.push_back(selectedItem); }
           //                  
           //      
           //    // Print items
           //    std::cout << "Selected Items: " << std::endl;
           //    for(SketchItem& item : m_Selected) { 
           //        std::cout << item.Name() << std::endl; 
           //    }
           //    
           //    
           //    
           //    
           //    
           //    
           //    
           //    
           //    
           //    
           //    
           //    
           //    
           //    
           //} else { // Clear selection if no item found at p
           //    ClearSelected();
           //}        
        }
        else if(button == MouseButton::Left && action == MouseAction::Release) {
            // If we have dragged the cursor whilst button was pressed
            if(m_CursorPos != m_CursorClickedPos) {
                // Find selection inside selection box
                
                // TODO: work out only if fully inside or partially inside selection box
                //m_Selected = m_Parent->GetItemsByBetweenPositions(m_CursorPos, m_CursorClickedPos);
            }
        }
        return;     
    }
        
    // Handle other commands
    if(button == MouseButton::Left && action == MouseAction::Press) {
    
        m_InputData.push_back(m_CursorClickedPos); 
        std::cout << "InputData Size: " << m_InputData.size() << std::endl;
        // Handle Add Point     
        if(m_CommandType == CommandType::Add_Point) {
            m_Parent->Factory().AddPoint(m_InputData[0]);                
            m_InputData.clear();
        }
        // Handle Add Line
        else if(m_CommandType == CommandType::Add_Line) {
            if(m_InputData.size() == 2) {
                m_Parent->Factory().AddLine(m_InputData[0], m_InputData[1]);
                m_InputData.clear();
            }
        }
        // Handle Add Arc
        else if(m_CommandType == CommandType::Add_Arc) {
            if(m_InputData.size() == 3) {
                // make sure points arent the same
                if(!(m_InputData[0] == m_InputData[1] && m_InputData[1] == m_InputData[2])) {
                    //TODO Make direction settable
                    Direction direction = Direction::CW;
                    // Calculate centre from point
                    Vec2 newCentre = m_Parent->Renderer().AdjustCentrePoint(m_InputData[0], m_InputData[1], m_InputData[2]);
                    m_Parent->Factory().AddArc(m_InputData[0], m_InputData[1], newCentre, direction);                    
                }
                m_InputData.clear();
            }
        }
        // Handle Add Circle
        else if(m_CommandType == CommandType::Add_Circle) {
            if(m_InputData.size() == 2) {
                double radius = Hypot(m_InputData[1] - m_InputData[0]);
                m_Parent->Factory().AddCircle(m_InputData[0], radius);
                m_InputData.clear();
            }
        }
        else {
            assert(0 && "Command doesn't exist");
        }
    }
    
}

// Mouse Move Event
//
//  Input should be (x, y) coords in sketch space
//
//  Hover over item -  Highlights it
//  Drag            -  if (clicked)
//                        -  if(Selected) drag items
//                        -  if(!selected) drag selection box

void SketchEvents::Mouse_Move(const Vec2& p)
{ 
    // Update cursor
    m_CursorPos = p;
    
    // Reset the mouse button / action
    if(m_MouseAction == MouseAction::Release) { m_MouseButton = MouseButton::None; m_MouseAction = MouseAction::None; }
    
    // TODO: Work out where point should be (i.e. if snapped etc) 
    
     
    // Return if no command set
    if(m_CommandType == CommandType::None) { return; }
    
    if(m_CommandType == CommandType::Select) {
        
        // Highlight item if hovered over
        if(m_MouseButton == MouseButton::None) { 
                
            //if(!items.empty()) {
                //HighlightItem(items[0].first);
            //}
        }
        
        if(m_MouseButton == MouseButton::Left && m_MouseAction == MouseAction::Press) {
            // Drag items(s)
            
            
            
            m_Parent->SolveConstraints(p);
            
            //if(!m_Selected.empty()) {
            //    m_Parent->SolveConstraints(m_Selected[0], p);
            //} 
            // Selection box
            //else { 
                // Draw selection box
            //}
        }
        return;
    }
    
    
    // Preview New Element
     
    
    
    
    const SketchRenderer& renderer = m_Parent->Renderer();
    
    Points preview_Points;
    LineString preview_LineString;
    
    // Handle Add Point     
    if(m_CommandType == CommandType::Add_Point) {
        preview_Points.push_back(p);
            
    }
    // Handle Add Line
    else if(m_CommandType == CommandType::Add_Line) {
        preview_Points.push_back(p); 
        if(m_InputData.size() > 0) {
            preview_Points.push_back(m_InputData[0]);
            preview_LineString = renderer.RenderLine(p, m_InputData[0]);
        }
    }
    // Handle Add Arc
    else if(m_CommandType == CommandType::Add_Arc) {
        if(m_InputData.size() == 0) {
            preview_Points.push_back(p); 
        }
        else if(m_InputData.size() == 1) {
            preview_Points.push_back(m_InputData[0]); 
            preview_Points.push_back(p); 
            Vec2 midPoint = (p + m_InputData[0]) / 2;
            preview_LineString = renderer.RenderArc(m_InputData[0], p, midPoint, Direction::CW);    
            // TODO: Direction should be settable
        }
        else if(m_InputData.size() == 2) {
            preview_Points.push_back(m_InputData[0]); 
            preview_Points.push_back(m_InputData[1]); 
            
            // Calculate centre based on p
            Vec2 newCentre = renderer.AdjustCentrePoint(m_InputData[0], m_InputData[1], p);
            preview_Points.push_back(newCentre); 
                        
            preview_LineString = renderer.RenderArc(m_InputData[0], m_InputData[1], newCentre, Direction::CW);
        }
    }
    // Handle Add Circle
    else if(m_CommandType == CommandType::Add_Circle) {
        if(m_InputData.size() == 0) {
            preview_Points.push_back(p); 
        }
        else if(m_InputData.size() == 1) {
            preview_Points.push_back(m_InputData[0]); 
            // dont show mouse point for circle? preview_Points.push_back(p);
            preview_LineString = renderer.RenderCircle(m_InputData[0], Hypot(p-m_InputData[0]));
        }
    }
    else {
        assert(0 && "Command doesn't exist");
    }
    
    if(!preview_Points.empty())       { m_Parent->Renderer().m_PreviewPoints.Add(std::move(preview_Points)); } 
    if(!preview_LineString.empty())   { m_Parent->Renderer().m_PreviewLines.Add(std::move(preview_LineString)); }   
    
} 


    
    
    
    
    
    
/*
void SketchEvents::Event_Keyboard(int key, int scancode, KeyAction action, KeyModifier modifier) 
{   
    
}
*/


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






void Sketcher::SolveConstraints(Vec2 p) 
{      
    // Update Solver set dragged point and its position
    if(bool success = m_Factory.UpdateSolver(p)) {
        (void)success;
    }
}


bool Sketcher::DrawImGui()
{
    bool updateRequired = false;
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

    // Main body of the Demo window starts here.
    if (ImGui::Begin("SketchApp", NULL))
    {
    
    
    
        if(ImGui::Button("Update")) {
            SolveConstraints();
            updateRequired = true;
        }
        
        
        static int command = 0;
        if(ImGui::Combo("Command", &command, "None\0Select\0Add Point\0Add Line\0Add Arc\0Add Circle\0\0")) {
            if(command == 0) { m_Events.GetCommandType(SketchEvents::CommandType::None); }
            if(command == 1) { m_Events.GetCommandType(SketchEvents::CommandType::Select); }
            if(command == 2) { m_Events.GetCommandType(SketchEvents::CommandType::Add_Point); }
            if(command == 3) { m_Events.GetCommandType(SketchEvents::CommandType::Add_Line); }
            if(command == 4) { m_Events.GetCommandType(SketchEvents::CommandType::Add_Arc); }
            if(command == 5) { m_Events.GetCommandType(SketchEvents::CommandType::Add_Circle); }
            updateRequired = true;
        }
        
        
        
  /*      
        ImGui::Separator();
    
    
        static double dragPoint[2]; 
        ImGui::InputFloat2("Drag Point To (X, Y)", dragPoint);
        if(ImGui::Button("Drag Point")) {
            SolveConstraints(SketchItem ... , { 100.0f, 200.0f});
            updateRequired = true;
        }
    */
    
        ImGui::Separator();
        
        
        static float position[2];
        updateRequired |= ImGui::InputFloat2("Position (X, Y)", position);
        
        if(ImGui::Button("Add Point")) {
            /*Sketch::Point* p0 = */ 
            m_Factory.AddPoint({ position[0], position[1] });
            updateRequired = true;
        }


        ImGui::Separator();
        
        
        static float p0[2]; static float p1[2]; static float pCentre[2];
        updateRequired |= ImGui::InputFloat2("P0", p0);
        updateRequired |= ImGui::InputFloat2("P1", p1);
        updateRequired |= ImGui::InputFloat2("Centre", pCentre);
            
        static int direction = 0;
        updateRequired |= ImGui::Combo("Direction", &direction, "Clockwise\0Anti-Clockwise\0\0");
        
        if(ImGui::Button("Add Arc")) {
            /*Sketch::Point* p0 = */ 
            m_Factory.AddArc({ p0[0], p0[1] }, { p1[0], p1[1] }, { pCentre[0], pCentre[1] }, !direction ? Direction::CW : Direction::CCW);
            updateRequired = true;;
        }
    
    
        ImGui::Separator();
        
        
        updateRequired |= DrawImGui_Elements();
        updateRequired |= DrawImGui_Constraints();

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
    

     return updateRequired;
}


bool Sketcher::DrawImGui_Elements() 
{
    bool updateRequired = false;
    if (ImGui::CollapsingHeader("Elements"))
    {
        m_Factory.ForEachElement([&](Sketch::Element* element) {
            if(auto* point = dynamic_cast<const Sketch::Point*>(element)) {
                
                if (ImGui::TreeNode(va_str("Point %d", point->ID()).c_str())) {
                    ImGui::Text("P: (%.3f, %.3f)", point->P().x, point->P().y);
                    ImGui::TreePop();
                    ImGui::Separator();
                }
            }
            else if(auto* line = dynamic_cast<const Sketch::Line*>(element)) {
                
                if (ImGui::TreeNode(va_str("Line %d", line->ID()).c_str())) {
                    ImGui::Text("P0: (%.3f, %.3f)", line->P0().x, line->P0().y);
                    ImGui::Text("P1: (%.3f, %.3f)", line->P1().x, line->P1().y);
                    ImGui::TreePop();
                    ImGui::Separator();
                }
            }
            else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element)) {
                
                if (ImGui::TreeNode(va_str("Arc %d", arc->ID()).c_str())) {
                    ImGui::Text((arc->Direction() == Direction::CW) ? "Direction: CW" : "Direction: CCW");
                    ImGui::Text("P0: (%.3f, %.3f)", arc->P0().x, arc->P0().y);
                    ImGui::Text("P1: (%.3f, %.3f)", arc->P1().x, arc->P1().y);
                    ImGui::Text("PC: (%.3f, %.3f)", arc->PC().x, arc->PC().y);
                    ImGui::TreePop();
                    ImGui::Separator();
                }
            }
            else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element)) {
                if (ImGui::TreeNode(va_str("Circle %d", circle->ID()).c_str())) {
                    ImGui::Text("PC: (%.3f, %.3f)", circle->PC().x, circle->PC().y);
                    ImGui::Text("Radius: %.3f", circle->Radius());
                    ImGui::TreePop();
                    ImGui::Separator();
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

bool Sketcher::DrawImGui_Constraints() 
{
    bool updateRequired = false;
    
    if (ImGui::CollapsingHeader("Constraints"))
    {
        m_Factory.ForEachConstraint([&](Sketch::Constraint* constraint) 
        {
            if(auto* c = dynamic_cast<Sketch::Constraint_Template_OneItem*>(constraint)) {
                if (ImGui::TreeNode(va_str("Constraint %d", c->ID()).c_str())) {
                    // For each SketchItem in constraint
                    c->ForEachElement([&](SketchItem& ref) {
                        ImGui::Text("%s", ref.Name().c_str());
                    });
                    ImGui::TreePop();
                    ImGui::Separator();
                }
                //points.push_back(c->P());
            } else if(auto* c = dynamic_cast<Sketch::Constraint_Template_TwoItems*>(constraint)) {
                if (ImGui::TreeNode(va_str("Constraint %d", c->ID()).c_str())) {
                    // For each SketchItem in constraint
                    c->ForEachElement([&](SketchItem& ref) {
                        ImGui::Text("%s", ref.Name().c_str());
                    });
                    ImGui::TreePop();
                    ImGui::Separator();
                }
                //points.push_back(c->P());
            }
        });
    }
    ImGui::Separator();
    return updateRequired;
}


} // end namespace Sketch

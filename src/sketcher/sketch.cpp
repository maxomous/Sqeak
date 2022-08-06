
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
    for(auto& geometry : polygonised.valid)     { m_Geometry.push_back(std::move(geometry)); }
    for(auto& geometry : polygonised.dangles)   { m_Geometry.push_back(std::move(geometry)); }
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
        geometry.m_IsSelected = !geometry.m_IsSelected;
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
        if(auto success = geos.Intersects(p_with_tol, geometry.Geometry())) {
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

    auto RenderConstraints = [&](RenderData::Data& data, std::function<bool(const Constraint*)> cb_Condition = [](const Constraint* constraint){ (void)constraint; return true; }) { 
        // Clear the old data
        data.Clear();
        // Draw Element / Points
        m_Parent->Factory().ForEachConstraint([&](Sketch::Constraint* constraint) {
            
            // Return early if condtion not met for constraint
            if(!cb_Condition(constraint)) { return; }
                
            // Points / lines buffer
            Points points;
            LineString l;

            auto imageAtSketchItem = [&](RenderData::Image::Type type, SketchItem item) {
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item);
                data.images.push_back({ type, p0 });
            };
            auto imageAtSketchItems = [&](RenderData::Image::Type type, SketchItem item1, SketchItem item2) {
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item1);
                const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(item2);
                data.images.push_back({ type, p0 });
                data.images.push_back({ type, p1 });
            };
                
            auto distanceArrowBetween = [&](const Vec2& p0, const Vec2& p1, double distance) {
                Vec2 midPoint = (p0 + p1) / 2.0;
                std::string s = MaxLib::String::va_str("%.1f", distance);
                data.texts.push_back({ s, midPoint});
                // TODO: AND ADD LINESTRINGS of double ended arror
            };
                
                
                    
            // Add a coincident constraint image on point of constraint
            if(auto* c = dynamic_cast<Sketch::Coincident_PointToPoint*>(constraint))        { imageAtSketchItem(RenderData::Image::Type::Coincident, c->Ref_1()); } 
            else if(auto* c = dynamic_cast<Sketch::Coincident_PointToLine*>(constraint))    { imageAtSketchItem(RenderData::Image::Type::Coincident, c->Ref_1()); } 
            else if(auto* c = dynamic_cast<Sketch::Coincident_PointToArc*>(constraint))     { imageAtSketchItem(RenderData::Image::Type::Coincident, c->Ref_1()); } 
            else if(auto* c = dynamic_cast<Sketch::Coincident_PointToCircle*>(constraint))  { imageAtSketchItem(RenderData::Image::Type::Coincident, c->Ref_1()); } 
            
            // Add a distance constraint
            else if(auto* c = dynamic_cast<Sketch::Distance_PointToPoint*>(constraint)) {        
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(c->Ref_1());
                const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(c->Ref_2());
                distanceArrowBetween(p0, p1, c->distance);
            } 
            else if(auto* c = dynamic_cast<Sketch::Distance_PointToLine*>(constraint)) {     
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(c->Ref_1());
                const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(c->Ref_2());         //TODO: p1 should actually be perpendicular point  
                distanceArrowBetween(p0, p1, c->distance);
            } 
            
            // Add midpoint constraint
            else if(auto* c = dynamic_cast<Sketch::AddMidPoint_PointToLine*>(constraint))   { imageAtSketchItem(RenderData::Image::Type::Midpoint, c->Ref_1()); }
            
            // Add radius
            else if(auto* c = dynamic_cast<Sketch::AddRadius_Circle*>(constraint)) {     
                // Add a coincident constraint image on each point           
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(c->Ref());
                const Vec2& p1 = p0 + Vec2(0.0, c->radius);         //TODO: p1 should actually be chosen point
                distanceArrowBetween(p0, p1, c->radius);
            } 
            else if(auto* c = dynamic_cast<Sketch::AddRadius_Arc*>(constraint)) {     
                // Add a coincident constraint image on each point           
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(c->Ref());
                const Vec2& p1 = p0 + Vec2(0.0, c->radius);         //TODO: p1 should actually be chosen point
                distanceArrowBetween(p0, p1, c->radius);
            } 
            
            else if(auto* c = dynamic_cast<Sketch::Angle_LineToLine*>(constraint)) {   
                (void)c;  
                // TODO: Draw Angle
            } 
            
            // Add Vertical constraint
            else if(auto* c = dynamic_cast<Sketch::Vertical*>(constraint))                  { imageAtSketchItems(RenderData::Image::Type::Vertical, c->Ref_1(), c->Ref_2()); }
            // Add Horizontal constraint
            else if(auto* c = dynamic_cast<Sketch::Horizontal*>(constraint))                { imageAtSketchItems(RenderData::Image::Type::Horizontal, c->Ref_1(), c->Ref_2()); }
            // Add Parallel constraint
            else if(auto* c = dynamic_cast<Sketch::Parallel*>(constraint))                  { imageAtSketchItems(RenderData::Image::Type::Parallel, c->Ref_1(), c->Ref_2()); }
            // Add Perpendicular constraint
            else if(auto* c = dynamic_cast<Sketch::Perpendicular*>(constraint))             { imageAtSketchItems(RenderData::Image::Type::Perpendicular, c->Ref_1(), c->Ref_2()); }
            // Add Tangent constraint
            else if(auto* c = dynamic_cast<Sketch::Tangent_Arc_Line*>(constraint))          { imageAtSketchItem(RenderData::Image::Type::Tangent, (!c->tangentPoint) ? c->Ref_1() : c->Ref_2()); }
            else if(auto* c = dynamic_cast<Sketch::Tangent_Arc_Arc*>(constraint))           { imageAtSketchItems(RenderData::Image::Type::Tangent, c->Ref_1(), c->Ref_2()); }
            // Add Equal constraint
            else if(auto* c = dynamic_cast<Sketch::EqualLength*>(constraint))               { imageAtSketchItems(RenderData::Image::Type::Equal, c->Ref_1(), c->Ref_2()); }
            else if(auto* c = dynamic_cast<Sketch::EqualRadius_Arc_Circle*>(constraint))    { imageAtSketchItems(RenderData::Image::Type::Equal, c->Ref_1(), c->Ref_2()); }
            else if(auto* c = dynamic_cast<Sketch::EqualRadius_Arc_Arc*>(constraint))       { imageAtSketchItems(RenderData::Image::Type::Equal, c->Ref_1(), c->Ref_2()); }
            else if(auto* c = dynamic_cast<Sketch::EqualRadius_Circle_Circle*>(constraint)) { imageAtSketchItems(RenderData::Image::Type::Equal, c->Ref_1(), c->Ref_2()); }
            
          
          // Add Point(s) to render data
          if(!points.empty()) { data.points.push_back(std::move(points)); }
          if(!l.empty()) { data.linestrings.push_back(std::move(l)); }
          
        }); 

    };
    
    
    // return if no update required
    if(m_Update == UpdateFlag::None) { return false; }
    
    // A full update includes this, clears input data
    if(m_Update & UpdateFlag::ClearInputData) {
        std::cout << "updating Clear Input Data" << std::endl;          
        // reset preview data
        m_Parent->Events().m_InputData.clear();
        
        // setting input data so that it can join to previous element
        if(m_Update & UpdateFlag::DontSetInputDataToLastElement) {
            m_Parent->Events().m_PreviousElement = {};            
        } 
        // Continue element from end of last element (used for lines and arcs)
        else {
            m_Parent->Events().m_InputData.push_back(m_Parent->Events().m_CursorClickedPos); 
        }

    } 
        
    // A full update includes this, clears selection box, selection / hovered items
    if(m_Update & UpdateFlag::ClearSelection) {
        std::cout << "updating Clear Selection" << std::endl;
        // reset dragged selection box
        m_Parent->Events().m_IsDragSelectionBox = false;    
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
        // Render all failed to solve items
        RenderElements(elements.failed, [](const Item& item) { return item.IsFailed(); });
        
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
    
    
    
    
    
    
    
    
    
    
    
    RenderData::Data_Selectable& constraints = m_RenderData.constraints;
    
    // Update Constraints
    if(m_Update & UpdateFlag::Constraints) { 
        std::cout << "updating Constraints" << std::endl;
        // Render all items
        RenderConstraints(constraints.unselected);
        // Render all failed to solve constraints
        RenderConstraints(constraints.failed, [](const Constraint* constraint) { return constraint->Failed(); });
       // // Render all selected constraints
       // RenderConstraints(constraints.selected, [](const Constraint* constraint) { return constraint->IsSelected(); });
       // // Render all hovered constraints
       // RenderConstraints(constraints.hovered,  [](const Constraint* constraint) { return constraint->IsHovered(); });
    }
    
    
  /*
Coincident_PointToPoint
Coincident_PointToLine
Coincident_PointToArc
Coincident_PointToCircle
Distance_PointToPoint(ElementFactory* parent, SketchItem p0, SketchItem p1, double distance)       : Constraint_Template_TwoItems(parent, p0, p1), m_Distance(distance) {}
Distance_PointToLine
AddMidPoint_PointToLine
AddRadius_Circle
AddRadius_Arc
Angle_LineToLine
Vertical(ElementFactory* parent, SketchItem p0, SketchItem p1)       : Constraint_Template_TwoItems(parent, p0, p1) {}
Horizontal(ElementFactory* parent, SketchItem p0, SketchItem p1)       : Constraint_Template_TwoItems(parent, p0, p1) {}
Parallel
Perpendicular
Tangent_Arc_Line
Tangent_Arc_Arc
EqualLength
EqualRadius_Arc_Circle
EqualRadius_Arc_Arc
EqualRadius_Circle_Circle


    C               coincident
    arrow + value   distance
    ==              midpoint
    arc + value     angle
    V               vertical
    H               horizontal
    //              parallel
    |_              perpendicular wants image...
    (_              tangent
    =               equal
  
*/
    
    
  /*
  // Go through each point
  constraint->ForEachItem([&](Sketch::SketchItem& sketchItem) {
      const Vec2& p = GetPositionBySketchItem(sketchItem);
  });*/
    
    

    
     
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
        else {
             
            if(m_Parent->Events().m_PreviousElement.type == SketchItem::Type::Line) 
            {
                // 1 point has already been added
                if(inputData.size() == 1) {
                    // calculate centre point such that arc is tangent to previous line
                    Vec2 pC = Geom::ArcCentreFromTangent(m_Parent->Factory().GetPositionBySketchItem(m_Parent->Events().m_PreviousElement.P0()), inputData[0], p);
                    // add the render data
                    points.push_back(inputData[0]); // p0
                    points.push_back(p);            // p1
                    points.push_back(pC);           // pC
                    linestring = m_Parent->Factory().RenderArc(inputData[0], p, pC, inputDirection);    
                }
            }
            else 
            {
                // 1 point has already been added
                if(inputData.size() == 1) {
                    // calculate centre point as mid point between p0 and p1
                    Vec2 pC = (p + inputData[0]) / 2;
                    // add the render data
                    points.push_back(inputData[0]); // P0
                    points.push_back(p);            // p1
                    points.push_back(pC);           // pC
                    linestring = m_Parent->Factory().RenderArc(inputData[0], p, pC, inputDirection);    
                }
                // 2 points have already been added
                else if(inputData.size() == 2) {
                    // Calculate centre based on p
                    Vec2 pC = Geom::ArcCentre(inputData[0], inputData[1], p);
                    // add the render data
                    points.push_back(inputData[0]); // p0
                    points.push_back(inputData[1]); // p1
                    points.push_back(pC);           // pC
                    linestring = m_Parent->Factory().RenderArc(inputData[0], inputData[1], pC, inputDirection);
                }                
            }
            
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

void SketchEvents::SetCommandType(CommandType commandType) 
{
    CommandType previousCommand = m_CommandType;
    // Set command
    m_CommandType = commandType;
    
    // Update render data
    if(previousCommand == CommandType::Add_Line && m_CommandType == CommandType::Add_Arc) {
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_SetInputDataToLastElement);        
    }
   // else if(previousCommand == CommandType::Add_Arc && m_CommandType == CommandType::Add_Line) {
   //     m_Renderer.SetUpdateFlag(UpdateFlag::Full_SetInputDataToLastElement);        
   // }
   else {
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);       
   }
}




void SketchEvents::Event_Keyboard(int key, KeyAction action, KeyModifier modifier) 
{   
    (void)modifier;
    // GLFW_KEY_DELETE
    if (key == 261 && action == KeyAction::Press) {
        m_Parent->Factory().DeleteSelection();
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
    }
    // GLFW_KEY_ESCAPE 
    if (key == 256 && action == KeyAction::Press) {
        SetCommandType(SketchEvents::CommandType::Select);
        m_Parent->SetSelectCommand();
        // update handled in SetCommandType()
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
            
            bool success = m_Parent->Factory().SetSelectedByPosition(m_CursorPos);
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
            m_Parent->Events().m_PolygonisedGeometry.SetSelectedByPosition(m_CursorPos, m_Parent->Factory().selectionTolerance);
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
        }  
    }
    // Handle add element commands
    else if(m_CommandType == CommandType::Add_Point || m_CommandType == CommandType::Add_Line || m_CommandType == CommandType::Add_Arc || m_CommandType == CommandType::Add_Circle) 
    {
        if(button == MouseButton::Left && action == MouseAction::Press) {
             
        /*            
                       inputdata.Size     Last Point
            point   -       N/A
            line    -   1               if(m_CursorClickedPos == m_InputData[0]) // dont allow p1 on p0
            arc     -   2               if(m_CursorClickedPos == m_InputData[0] || m_CursorClickedPos == m_InputData[1]) // dont allow pC on p0 or p1
            circle  -   1               if(m_CursorClickedPos == m_InputData[0]) // dont allow p on pC (0 radius)
        */
            
            
            m_InputData.push_back(m_CursorClickedPos); 
            // Handle Add Point     
            if(m_CommandType == CommandType::Add_Point) 
            {
                // check that input data is the corrent size
                if(m_InputData.size() != 1) { return false; }
                // Add point
                m_Parent->Factory().AddPoint(m_InputData[0]);
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
            }
            // Handle Add Line
            else if(m_CommandType == CommandType::Add_Line) {
                    
                // check that input data is the corrent size
                if(m_InputData.size() != 2) { return false; }
                // make sure points aren't the same
                if(m_InputData[1] == m_InputData[0]) { m_InputData.pop_back(); return false; }
                
                // allows us to continue element from last element's end points
                SketchItem previousElement = m_PreviousElement;
                
                // create new Line
                m_PreviousElement = m_Parent->Factory().AddLine(m_InputData[0], m_InputData[1]);
                
                // constraint to beggining of our new line
                if(previousElement.type == SketchItem::Type::Line) {
                    // add constraint between end of last line to beginning of this line
                    m_Parent->Factory().AddConstraint<Coincident_PointToPoint>(previousElement.P1(), m_PreviousElement.P0());
                }
                
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_SetInputDataToLastElement);
                 
            }
            // Handle Add Arc
            else if(m_CommandType == CommandType::Add_Arc) {
                
                if(m_PreviousElement.type == SketchItem::Type::Line) {
                    
                    // check that input data is the corrent size
                    if(m_InputData.size() != 2) { return false; }
                    // make sure points aren't the same
                    if(m_InputData[1] == m_InputData[0]) { m_InputData.pop_back(); return false; }
                    // allows us to continue element from last element's end points
                    SketchItem previousElement = m_PreviousElement;
                    // Calculate centre from point
                    Vec2 newCentre = Geom::ArcCentreFromTangent(m_Parent->Factory().GetPositionBySketchItem(previousElement.P0()), m_InputData[0], m_InputData[1]);
                    // Create new Arc
                    m_PreviousElement = m_Parent->Factory().AddArc(m_InputData[0], m_InputData[1], newCentre, m_Parent->Events().m_InputDirection); 
                    // add constraint between end of last line to beginning of this line
                    m_Parent->Factory().AddConstraint<Tangent_Arc_Line>(m_PreviousElement, previousElement, 0);
                }  
                else {
                    // check that input data is the corrent size
                    if(m_InputData.size() != 3) { return false; }
                    // make sure points aren't the same
                    if(m_InputData[2] == m_InputData[0] || m_InputData[2] == m_InputData[1]) { m_InputData.pop_back(); return false; }
                    // Calculate closest possible centre point from input centre point
                    Vec2 newCentre = Geom::ArcCentre(m_InputData[0], m_InputData[1], m_InputData[2]);
                    // Create new Arc
                    m_PreviousElement = m_Parent->Factory().AddArc(m_InputData[0], m_InputData[1], newCentre, m_Parent->Events().m_InputDirection);  
                }
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_SetInputDataToLastElement);
            }
            // Handle Add Circle
            else if(m_CommandType == CommandType::Add_Circle) {
                // check that input data is the corrent size
                if(m_InputData.size() != 2) { return false; }
                
                // make sure points aren't the same
                if(m_InputData[1] == m_InputData[0]) { m_InputData.pop_back(); return false; }
                
                double radius = Hypot(m_InputData[1] - m_InputData[0]);
                m_Parent->Factory().AddCircle(m_InputData[0], radius);
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
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
            m_Parent->Factory().SetHoveredByPosition(m_CursorPos);            
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
        }
        
        if(m_MouseButton == MouseButton::Left && m_MouseAction == MouseAction::Press) {
            // Dragging selection box
            if(m_IsDragSelectionBox) {
                
                m_Parent->Factory().ClearHovered();
                
                bool includePartiallyInside = (m_CursorPos.x - m_CursorClickedPos.x) < 0;
                // Find selection inside selection box
                m_Parent->Factory().AddHoveredBetween(m_CursorPos, m_CursorClickedPos, includePartiallyInside);               
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
                
            } else {
                std::cout << "p: " << p << std::endl;
                m_Parent->SolveConstraints(pDif);
                // Update render data (selection is required for knowing which is dragged)
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_DontClearSelection);
            }
        }
    } 
    // Hover / drag / 
    else if(m_CommandType == CommandType::SelectLoop) {
        
        // Highlight item if hovered over
        if(m_MouseButton == MouseButton::None) { 
            m_Parent->Events().m_PolygonisedGeometry.ClearHovered();
            m_Parent->Events().m_PolygonisedGeometry.SetHoveredByPosition(m_CursorPos, m_Parent->Factory().selectionTolerance);
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
    
    SketchItem l1 = f.AddLine({ 100.0, 100.0 }, { 200.0, 523.0 });
    SketchItem l2 = f.AddLine({ 200.0, 523.0 }, { 500.0, 500.0 });
    SketchItem l3 = f.AddLine({ 500.0, 500.0 }, { 100.0, 100.0 });
    
    //Circle* circle = m_Factory.AddCircle({ -6.0, -7.0 }, 200.0);
    
    //m_Factory.AddConstraint<Coincident_PointToPoint>(l1.p1, p1);
        
        f.AddConstraint<Coincident_PointToPoint>(l1.P1(), l2.P0());
        f.AddConstraint<Coincident_PointToPoint>(l2.P1(), l3.P0());
        f.AddConstraint<Coincident_PointToPoint>(l3.P1(), l1.P0());
        
        f.AddConstraint<Distance_PointToPoint>(l1, 300.0);
        f.AddConstraint<Distance_PointToPoint>(l2, 400.0);
        f.AddConstraint<Distance_PointToPoint>(l3, 550.0);        
     
}



void Sketcher::SolveConstraints(Vec2 pDif) 
{      
    // Update Solver set dragged point and its position
    bool success = m_Factory.UpdateSolver(pDif);
    if(!success) {
        Log::Error("Unable to solve constraints");
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



class ConstraintButtons
{
public:
    ConstraintButtons(ElementFactory* factory) : m_Factory(factory) {}
    
    // Draw ImGui buttons for constraints
    bool DrawImGui() 
    {
        const std::vector<SketchItem>& points = m_Factory->GetSelectedPoints();
        const std::vector<SketchItem>& elements = m_Factory->GetSelectedElements();
        
        // return ealry if nothing selected
        if(points.empty() && elements.empty()) { return false; }
                        
        bool isConstraintAdded = false;
        
        // 0 elements selected
        if(elements.empty()) 
        {
            // 1 points selected
            if(points.size() == 1) {
                // Fix point constraint
            }
            // 2 points selected
            else if(points.size() == 2) {
                ImGui::Text("Add Constraint");
                // Add Distance constraint between 2 points
                isConstraintAdded |= ConstraintButton_WithInput<Distance_PointToPoint>("Distance", &m_Distance, points[0], points[1]);
                // Add Coincident constraint between 2 points
                isConstraintAdded |= ConstraintButton<Coincident_PointToPoint>("Coincident", points[0], points[1]);
                // Add Vertical constraint between 2 points
                isConstraintAdded |= ConstraintButton<Vertical>("Vertical", points[0], points[1]);
                // Add Horizontal constraint between 2 points
                isConstraintAdded |= ConstraintButton<Horizontal>("Horizontal", points[0], points[1]);
            }
        } 
        // 1 element selected
        else if(elements.size() == 1) {
            
            // 0 points and 1 element selected
            if(points.size() == 0) 
            {
                if(elements[0].type == SketchItem::Type::Line) {
                    ImGui::Text("Add Constraint");
                    // Add Distance constraint of Line       
                    isConstraintAdded |= ConstraintButton_WithInput<Distance_PointToPoint>("Distance", &m_Distance, elements[0]);
                    // Add Vertical constraint of Line
                    isConstraintAdded |= ConstraintButton<Vertical>("Vertical", elements[0]);
                    // Add Horizontal constraint of Line
                    isConstraintAdded |= ConstraintButton<Horizontal>("Horizontal", elements[0]);
                }
                else if(elements[0].type == SketchItem::Type::Arc) {
                    ImGui::Text("Add Constraint");
                    // Add Radius constraint of Arc                
                    isConstraintAdded |= ConstraintButton_WithInput<AddRadius_Arc>("Radius", &m_Radius, elements[0]);
                }
                else if(elements[0].type == SketchItem::Type::Circle) {
                    ImGui::Text("Add Constraint");
                    // Add Radius constraint of Circle                   
                    isConstraintAdded |= ConstraintButton_WithInput<AddRadius_Circle>("Radius", &m_Radius, elements[0]);
                }
            }
            // 1 point and 1 element selected
            else if(points.size() == 1) 
            {
                if(elements[0].type == SketchItem::Type::Line) {
                    ImGui::Text("Add Constraint");
                    // Add Distance constraint between Point and Line         
                    isConstraintAdded |= ConstraintButton_WithInput<Distance_PointToLine>("Distance", &m_Distance, points[0], elements[0]);
                    // Add Coincident constraint between Point and Line                    
                    isConstraintAdded |= ConstraintButton<Coincident_PointToLine>("Coincident", points[0], elements[0]);
                    // Add Midpoint constraint between Point and Line                    
                    isConstraintAdded |= ConstraintButton<AddMidPoint_PointToLine>("Midpoint", points[0], elements[0]);
                }
                else if(elements[0].type == SketchItem::Type::Arc) {
                    ImGui::Text("Add Constraint");
                    // Add Coincident constraint between Point and Arc  
                    isConstraintAdded |= ConstraintButton<Coincident_PointToArc>("Coincident", points[0], elements[0]);
                }
                else if(elements[0].type == SketchItem::Type::Circle) {
                    ImGui::Text("Add Constraint");
                    // Add Coincident constraint between Point and Circke  
                    isConstraintAdded |= ConstraintButton<Coincident_PointToCircle>("Coincident", points[0], elements[0]);
                }
            }
        } 
        // 2 element selected
        else if(elements.size() == 2) {
            
            // 0 points and 2 elements selected
            if(points.size() == 0) 
            {
                // Line and Line selected
                if(elements[0].type == SketchItem::Type::Line && elements[1].type == SketchItem::Type::Line) {
                    ImGui::Text("Add Constraint");
                    // Add Angle constraint         
                    isConstraintAdded |= ConstraintButton_WithInput<Angle_LineToLine>("Angle", &m_Angle, elements[0], elements[1]);
                    // Add Parallel constraint
                    isConstraintAdded |= ConstraintButton<Parallel>("Parallel", elements[0], elements[1]);
                    // Add Midpoint constraint            
                    isConstraintAdded |= ConstraintButton<Perpendicular>("Perpendicular", elements[0], elements[1]);
                    // Add Equal Length constraint               
                    isConstraintAdded |= ConstraintButton<EqualLength>("Equal", elements[0], elements[1]);
                }
                // Arc and Arc selected
                else if(elements[0].type == SketchItem::Type::Arc && elements[1].type == SketchItem::Type::Arc) {
                    ImGui::Text("Add Constraint");
                    // Add Equal radius constraint               
                    isConstraintAdded |= ConstraintButton<EqualRadius_Arc_Arc>("Equal", elements[0], elements[1]);
                    // Add Tangent constraint               
                    isConstraintAdded |= ConstraintButton<Tangent_Arc_Arc>("Tangent", elements[0], elements[1]);
                }
                // Circle and Circle selected
                else if(elements[0].type == SketchItem::Type::Circle && elements[1].type == SketchItem::Type::Circle) {
                    ImGui::Text("Add Constraint");
                    // Add Equal radius constraint               
                    isConstraintAdded |= ConstraintButton<EqualRadius_Circle_Circle>("Equal", elements[0], elements[1]);
                }
                
                // Arc and Line selected
                else if(elements[0].type == SketchItem::Type::Arc && elements[1].type == SketchItem::Type::Line) {
                    ImGui::Text("Add Constraint");
                    // Add Tangent constraint               
                    isConstraintAdded |= ConstraintButton_Tangent_ArcLine("Tangent", elements[0], elements[1]);
                }
                // Line and Arc selected (reverse order)
                else if(elements[0].type == SketchItem::Type::Line && elements[1].type == SketchItem::Type::Arc) {
                    ImGui::Text("Add Constraint");
                    // Add Tangent constraint                  
                    isConstraintAdded |= ConstraintButton_Tangent_ArcLine("Tangent", elements[1], elements[0]);
                }
                
                // Arc and Circle selected
                else if(elements[0].type == SketchItem::Type::Arc && elements[1].type == SketchItem::Type::Circle) {
                    ImGui::Text("Add Constraint");
                    // Add Equal Radius constraint                
                    isConstraintAdded |= ConstraintButton<EqualRadius_Arc_Circle>("Equal", elements[0], elements[1]);
                }
                // Circle and Arc selected (reverse order)
                else if(elements[0].type == SketchItem::Type::Circle && elements[1].type == SketchItem::Type::Arc) {
                    ImGui::Text("Add Constraint");
                    // Add Equal Radius constraint                  
                    isConstraintAdded |= ConstraintButton<EqualRadius_Arc_Circle>("Equal", elements[1], elements[0]);
                }
            }
        }
        ImGui::Separator();
        return isConstraintAdded;
    } 
    
private:
    ElementFactory* m_Factory;
    double m_Distance = 100.0;
    double m_Radius = 100.0;
    double m_Angle = 90.0;
    
    // Add constraint between 1 or 2 points, returns true if new constraint was added
    template<typename T>
    bool ConstraintButton(const std::string& label, SketchItem p0) 
    {
        if(ImGui::Button(label.c_str())) {
            // Add Constraint
            m_Factory->AddConstraint<T>(p0);
            return true;
        }
        return false;
    }
    // Add constraint between 1 or 2 points, returns true if new constraint was added
    template<typename T>
    bool ConstraintButton(const std::string& label, SketchItem p0, SketchItem p1) 
    {
        if(ImGui::Button(label.c_str())) {
            // Add Constraint
            m_Factory->AddConstraint<T>(p0, p1);
            return true;
        }
        return false;
    }
    // Add constraint between 1 or 2 points and a value, returns true if new constraint was added
    template<typename T>
    bool ConstraintButton_WithInput(const std::string& label, double* value, SketchItem p0) 
    {
        bool isConstraintAdded = false;
        // Add Distance constraint between 2 points
        if(ImGui::Button(label.c_str())) {
            // Add Constraint
            m_Factory->AddConstraint<T>(p0, *value);
            // Set flag true
            isConstraintAdded = true;
        }
        ImGui::SameLine();
        ImGui::InputDouble(std::string("##" + label).c_str(), value); // use name as ID
        return isConstraintAdded;
    }
    // Add constraint between 1 or 2 points and a value, returns true if new constraint was added
    template<typename T>
    bool ConstraintButton_WithInput(const std::string& label, double* value, SketchItem p0, SketchItem p1) 
    {
        bool isConstraintAdded = false;
        // Add Distance constraint between 2 points
        if(ImGui::Button(label.c_str())) {
            // Add Constraint
            m_Factory->AddConstraint<T>(p0, p1, *value);
            // Set flag true
            isConstraintAdded = true;
        }
        ImGui::SameLine();
        ImGui::InputDouble(std::string("##" + label).c_str(), value); // use name as ID
        return isConstraintAdded;
    }
        // Add constraint between 1 or 2 points, returns true if new constraint was added
    bool ConstraintButton_Tangent_ArcLine(const std::string& label, SketchItem arc, SketchItem line) 
    {
         
        // Draw ImGui Button
        if(ImGui::Button(label.c_str())) 
        {
            SketchItem item_arc_p0  = { SketchItem::Type::Arc_P0, arc.element };
            SketchItem item_arc_p1  = { SketchItem::Type::Arc_P1, arc.element };
            SketchItem item_line_p0 = { SketchItem::Type::Line_P0, line.element };
            SketchItem item_line_p1 = { SketchItem::Type::Line_P1, line.element };
            
             
            // get p0 & p1 from arc and line
            const Vec2& arc_p0  = m_Factory->GetItemPointBySketchItem(item_arc_p0).p;
            const Vec2& arc_p1  = m_Factory->GetItemPointBySketchItem(item_arc_p1).p;
            const Vec2& line_p0 = m_Factory->GetItemPointBySketchItem(item_line_p0).p;
            const Vec2& line_p1 = m_Factory->GetItemPointBySketchItem(item_line_p1).p;
            
            // Check which points are closest together (default to arc p0 to line p0)
            double shortestDistance = Hypot(line_p0 - arc_p0);
            // flags determine which point 
            int closestPoints = 0b00;
            
            auto checkIfCloser = [&shortestDistance, &closestPoints](const Vec2& arc_p, const Vec2& line_p, int closestPointsFlag) {
                double distance = Hypot(line_p - arc_p);
                if(distance < shortestDistance) {
                    shortestDistance = distance;
                    closestPoints = closestPointsFlag;
                } 
            };
            checkIfCloser(arc_p1, line_p0, 0b10);
            checkIfCloser(arc_p1, line_p1, 0b11);
            checkIfCloser(arc_p0, line_p1, 0b01); 
                        
            // Constrain these closest points
            m_Factory->AddConstraint<Coincident_PointToPoint>((closestPoints & 0b10) ? item_arc_p1 : item_arc_p0, (closestPoints & 0b01) ? item_line_p1 : item_line_p0);
            // Add Tangent constraint   
            m_Factory->AddConstraint<Tangent_Arc_Line>(arc, line, (closestPoints & 0b10));
            return true;
        }
        return false;
    }
};


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
                * 
                *  
                    // returns value of input and switches input to false if true 
                    bool trigger(bool& input)
                    {
                        if(!input) {        
                            return false;
                        }
                        input = false;
                        return true;
                    }
                * 
                * 
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
        
        // Settings
        DrawImGui_Settings();
        // Draw Element List (returns element to delete if needed)
        DrawImGui_Elements(deleteElement);
        // Draw Constraint List (returns constraint to delete if needed)
        DrawImGui_Constraints(deleteConstraint);
        // User input values for when creating elements
        DrawImGui_ElementInputValues();
        
        
        // Add Constraint Buttons
        
        // TODO: Order of selection needs to be preserved - if circle then arc, circle should jump onto arc... atm it's just default order

        
        static ConstraintButtons constraintButtons(&m_Factory);
        // Draw Constraints Buttons
        if(constraintButtons.DrawImGui()) {
            // We need to clear the selected items, otherwise when we update, it will fix all selected items to their current position
            m_Factory.ClearSelected();
            SolveConstraints();
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);     
        }
        
    
        


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
        SolveConstraints();
        m_Renderer.SetUpdateFlag(UpdateFlag::Full);
    }
    if(deleteConstraint) { 
        m_Factory.RemoveConstraint(deleteConstraint); 
        SolveConstraints();
        m_Renderer.SetUpdateFlag(UpdateFlag::Full);
    }
    
}



void Sketcher::DrawImGui_Elements(ElementID& deleteElement) 
{        
    // Draw ImGui widgets for a point
    auto drawPointStats = [&](const std::string& name, Item_Point& item) {
        //ImGui::Text("%s: (%.3f, %.3f)", name.c_str(), item.p.x, item.p.y);
        if(InputDouble2(name.c_str(), &item.p)) {
            // select item and solve for the position we just changed
            Factory().SetSelected(item.Reference(), true);
            SolveConstraints({ 0.0, 0.0 });
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);  
        }
        if(item.IsSelected())  { ImGui::SameLine(); ImGui::Text("(Selected)"); }
        if(item.IsHovered())   { ImGui::SameLine(); ImGui::Text("(Hovered)"); }                        
        if(item.IsFailed())   { ImGui::SameLine(); ImGui::Text("(Failed)"); }                        
    };
    
    // Draw ImGui Treenode widgets for an element
    auto drawElementTreeNode = [&deleteElement](const std::string& name, Sketch::Element* element) {
        bool isTreeNodeOpen = ImGui::TreeNode(va_str("%s %d", name.c_str(), element->ID()).c_str());
        if(element->Item_Elem().IsSelected())  { ImGui::SameLine(); ImGui::Text("(Selected)"); }
        if(element->Item_Elem().IsHovered())   { ImGui::SameLine(); ImGui::Text("(Hovered)"); }  
        if(element->Item_Elem().IsFailed())   { ImGui::SameLine(); ImGui::Text("(Failed)"); }  
        ImGui::SameLine(); 
        if(ImGui::SmallButton(va_str("Delete##Element%d", element->ID()).c_str())) { deleteElement = element->ID(); }
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
}


void Sketcher::DrawImGui_Constraints(ConstraintID& deleteConstraint) 
{     
    
    auto drawConstraintTreeNode = [&deleteConstraint](const std::string& name, Sketch::Constraint* c) {
        bool isTreeNodeOpen = ImGui::TreeNode(va_str("%s %d", name.c_str(), c->ID()).c_str());
        //if(c.IsSelected()) { ImGui::SameLine(); ImGui::Text("(Selected)"); }
        //if(c.IsHovered())  { ImGui::SameLine(); ImGui::Text("(Hovered)"); }    
        if(c->Failed())    { ImGui::SameLine(); ImGui::Text("(Failed)"); }
        ImGui::SameLine(); 
        if(ImGui::SmallButton(va_str("Delete##Constraint%d", c->ID()).c_str())) { deleteConstraint = c->ID(); }
        return isTreeNodeOpen;                      
    };
    
    auto drawConstraintItems = [&](Sketch::Constraint* c) {
        // For each SketchItem in constraint
        c->ForEachItem([&](SketchItem& ref) {
            ImGui::Text("%s", ref.Name().c_str());
        });
    };
    
    
    auto drawConstraint = [&](Sketch::Constraint* c, std::function<void()> cb) {
                
        if(drawConstraintTreeNode("Constraint", c)) {                                 //  TODO c->name
            cb();
            drawConstraintItems(c);
            ImGui::TreePop(); ImGui::Separator();
        }
    };
    
    
    
    if (ImGui::CollapsingHeader("Constraints"))
    {
        bool updateRequired = false;
        m_Factory.ForEachConstraint([&](Sketch::Constraint* constraint) 
        {
            if(auto* c = dynamic_cast<Distance_PointToPoint*>(constraint)) {
                drawConstraint(constraint, [&]() {
                    updateRequired |= ImGui::InputDouble(String::va_str("Distance##%d", (int)c).c_str(), &(c->distance));
                });
            }
            else if(auto* c = dynamic_cast<Distance_PointToLine*>(constraint)) {
                drawConstraint(constraint, [&]() {
                    updateRequired |= ImGui::InputDouble(String::va_str("Distance##%d", (int)c).c_str(), &(c->distance));
                });
            }
            else if(auto* c = dynamic_cast<AddRadius_Circle*>(constraint)) {
                drawConstraint(constraint, [&]() {
                    updateRequired |= ImGui::InputDouble(String::va_str("Radius##%d", (int)c).c_str(), &(c->radius));
                });
            }
            else if(auto* c = dynamic_cast<AddRadius_Arc*>(constraint)) {
                drawConstraint(constraint, [&]() {
                    updateRequired |= ImGui::InputDouble(String::va_str("Radius##%d", (int)c).c_str(), &(c->radius));
                });
            }
            else if(auto* c = dynamic_cast<Angle_LineToLine*>(constraint)) {
                drawConstraint(constraint, [&]() {
                    updateRequired |= ImGui::InputDouble(String::va_str("Angle##%d", (int)c).c_str(), &(c->angle));
                });
            }
            else if(auto* c = dynamic_cast<Tangent_Arc_Line*>(constraint)) {
                drawConstraint(constraint, [&]() {
                    updateRequired |= ImGui::Combo(String::va_str("Tangent Point##%d", (int)c).c_str(), &(c->tangentPoint), "P0\0P1\0\0");
                });
            }
            else {
                drawConstraint(constraint, [](){});
            }
        });
        if(updateRequired)  {                        
            SolveConstraints();
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);  
        }
    }
    ImGui::Separator();
}


void Sketcher::DrawImGui_Settings() 
{
    if(ImGui::CollapsingHeader("Settings")) {
    
        if(ImGui::Button("Update Solver")) {
            SolveConstraints();
            // Update render data
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);
        }
        
        // Arc Segments
        if (ImGui::InputInt("Arc Segments", &m_Factory.arcSegments)) {
            m_Factory.arcSegments = abs(m_Factory.arcSegments); // it is uint
            // Update render data
            m_Renderer.SetUpdateFlag(UpdateFlag::Elements);
        }
        // Selection Tolerance
        ImGui::InputDouble("Selection Tolerance", &m_Factory.selectionTolerance);
        
    }
    ImGui::Separator();
}

void Sketcher::DrawImGui_ElementInputValues() 
{
    // get currently selected points
    std::vector<Vec2>& inputData = m_Events.InputData();
    
    if(m_Events.GetCommandType() == SketchEvents::CommandType::None) {
        ImGui::TextUnformatted("Choose a Command");
    }
    else if(m_Events.GetCommandType() == SketchEvents::CommandType::Select) {
        
        const std::vector<SketchItem>& points = m_Factory.GetSelectedPoints();
        const std::vector<SketchItem>& elements = m_Factory.GetSelectedElements();
        
        // Display current selection
        ImGui::TextUnformatted("Current Selection");
        ImGui::Indent();
            if(points.empty() && elements.empty())  { ImGui::Text("None"); }
            for(SketchItem item : points)           { ImGui::Text(item.Name().c_str()); }
            for(SketchItem item : elements)         { ImGui::Text(item.Name().c_str()); }
        ImGui::Unindent();
        
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
                static Vec2 p1_disabled = { 0.0, 0.0 };
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
                static Vec2 p1_disabled = { 0.0, 0.0 };
                static Vec2 pC_disabled = { 0.0, 0.0 };
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
                static Vec2 pC_disabled = { 0.0, 0.0 };
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
        static double radius = 0.0;
        
        radius = (inputData.size() < 2) ? 0.0 : Hypot(m_Events.m_CursorPos - inputData[0]);
        direction = (m_Events.m_InputDirection == Direction::CW) ? 0 : 1;
        
        ImGui::BeginDisabled(inputData.size() < 2);
            // Radius
            if(ImGui::InputDouble("Radius##Arc", &radius)) {
                // update preview based on radius
                m_Events.m_CursorPos = ArcCentre(inputData[0], inputData[1], radius, m_Events.m_InputDirection);
                
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
                
                Vec2 new_pC = Geom::ArcCentre(inputData[0], inputData[1], m_Events.m_CursorPos);
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
            static double radius = 0;
            if(ImGui::InputDouble("Radius##Circle", &radius)) {
                // update preview based on radius
                m_Events.m_CursorPos = inputData[0] + Vec2(0.0, radius);
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
}
} // end namespace Sketch

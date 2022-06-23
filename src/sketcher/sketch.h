#pragma once

#include <MaxLib.h>

#include "sketch_common.h"

//#include "deps/constraintsolver/solver.h"
//#include "elements.h"
//#include "constraints.h"
#include "elementfactory.h"


namespace Sketch {

using namespace MaxLib::Vector;
using namespace MaxLib::Geom;

// forward declare
class Sketcher;




/*




    use std::variant for class with 2 return types
    // TODO: Check if conincident point on arc works?
    TODO: Make SketchItems work for elements...
    TODO: Check if point on arc works on solver?
    TODO: May have an error where it gets index - 1 but if 1 gets deleted, thats not true, so search with id
    TODO: Dont pass ptrs around top level
    TODO: remove unnessesary dynamic_cast, change to static_cast
    TODO: Are we actually using ID?
    TODO: Dynamically allocate memory (or at leasst add up what we need), currently we have hardcoded a number#
    TODO: Change name of Point2D, to Point somehow
    TODO: ensure all radius' are double
    * Rename pointType to more general type
    * 
    * Perhaps Constraints_ could be replaced by Constraints<> and move specific to solver
    * Would SketchItem<...>() work?
    * 
    TODO: make sure circle elememts are also updating radius and distances 
    * remove 
        using namespace MaxLib::;
    * from header files
*/



class SketchRenderer
{
public:
    SketchRenderer(Sketcher* parent) : m_Parent(parent) {}
    
    const RenderData&   GetRenderData() const;
    // Goes through each element and each constraint and updates RenderData accordingly
    void                UpdateRenderData();
    // Render element to linestring

    LineString          RenderLine(const Vec2& p0, const Vec2& p1) const;
    LineString          RenderArc(const Vec2& pC, double radius, Direction direction, double th_Start, double th_End) const;
    LineString          RenderArc(const Vec2& p0, const Vec2& p1, const Vec2& pC, Direction direction) const;
    LineString          RenderCircle(const Vec2& pC, double radius) const;
    Vec2                AdjustCentrePoint(const Vec2& p0, const Vec2& p1, const Vec2& pC) const;
private:
    uint m_ArcSegments = 8;
    // draw list for viewer
    RenderData m_RenderData;
    
    Sketcher* m_Parent = nullptr;
};


class SketchCommands
{
public:
    enum CommandType { None, Select, Add_Point, Add_Line, Add_Arc, Add_Circle, Add_Constraint_1_Item, Add_Constraint_2_Items };
    
    SketchCommands(Sketcher* parent) : m_Parent(parent) {}
    
    const Points&       RenderPreview_Points() const        { return m_Preview_Points; };
    const LineString&   RenderPreview_LineString() const    { return m_Preview_LineString; };
    
    CommandType GetCommandType() const;
    void GetCommandType(CommandType command);

    void Event_MouseRelease();
    void Event_Click(const Vec2& p);
    void Event_Hover(const SketchRenderer& sketchRenderer, const Vec2& p);
    
private:
    SketchItem m_SelectedItem;
    CommandType m_CommandType = CommandType::None;
    std::vector<Vec2> m_InputData;
    
    Points m_Preview_Points;
    LineString m_Preview_LineString;
    
    Sketcher* m_Parent = nullptr;
};



class Sketcher
{
public:
    Sketcher();
    
    ElementFactory& Factory()   { return m_Factory; }
    SketchCommands& Commands()  { return m_Commands; }
    SketchRenderer& Renderer()  { return m_Renderer; }
    
    
    
    // Attempts to solve constraints. 
    // movedPoint can be set for moving a point
    void SolveConstraints(SketchItem movedPoint = {}, Vec2 p = Vec2()); 
    
    std::vector<std::pair<SketchItem, double>>    GetItemByPosition(Vec2 p, double tolerance);
    
    bool DrawImGui();
    bool DrawImGui_Elements();
    bool DrawImGui_Constraints();

private:
    bool m_IsActive = false;
    
    ElementFactory m_Factory;
    SketchCommands m_Commands;  
    SketchRenderer m_Renderer;
};
     

} // end namespace Sketch

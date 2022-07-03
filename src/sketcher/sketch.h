#pragma once

#include <algorithm>
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



    // TODO:    - SketchItem to become ItemRef
    //          - Should SketchItem just be a ref to Item_Element / Item_Point?
    //          - remove comments (old code)





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
    SketchRenderer(Sketcher* parent);
    
    const std::vector<RenderData>&   GetRenderData() const;
    // Goes through each element and each constraint and updates RenderData accordingly
    void                UpdateRenderData();
    // Render element to linestring

    LineString          RenderLine(const Vec2& p0, const Vec2& p1) const;
    LineString          RenderArc(const Vec2& pC, double radius, Direction direction, double th_Start, double th_End) const;
    LineString          RenderArc(const Vec2& p0, const Vec2& p1, const Vec2& pC, Direction direction) const;
    LineString          RenderCircle(const Vec2& pC, double radius) const;
    Vec2                AdjustCentrePoint(const Vec2& p0, const Vec2& p1, const Vec2& pC) const;
private:
    uint m_ArcSegments = 16;
    // draw list for viewer
    std::vector<RenderData> m_RenderData;
    
    Sketcher* m_Parent = nullptr;
    
    RenderData m_PreviewPoints;
    RenderData m_PreviewLines;
    
    
    friend class SketchEvents;
};
    
    
class SketchEvents
{
public:
    enum class CommandType { None, Select, Add_Point, Add_Line, Add_Arc, Add_Circle, Add_Constraint_1_Item, Add_Constraint_2_Items };
    
    // Equivelent to:
    //  GLFW_MOUSE_BUTTON_LEFT      0
    //  GLFW_MOUSE_BUTTON_RIGHT     1
    //  GLFW_MOUSE_BUTTON_MIDDLE    2
    enum class MouseButton { None = -1, Left = 0, Right = 1, Middle = 2 };

    // Equivelent to:
    // GLFW_RELEASE     0
    // GLFW_PRESS       1
    enum class MouseAction { None = -1, Release = 0, Press = 1 };

    // GLFW_REPEAT      2
    enum class KeyAction   { None = -1, Release = 0, Press = 1, Repeat = 2 };

    // Equivelent to:
    // GLFW_MOD_SHIFT      0x0001
    // GLFW_MOD_CONTROL    0x0002
    // GLFW_MOD_ALT        0x0004
    // GLFW_MOD_SUPER      0x0008
    // GLFW_MOD_CAPS_LOCK  0x0010
    // GLFW_MOD_NUM_LOCK   0x0020
    enum class KeyModifier { None = 0x00, Shift = 0x01, Ctrl = 0x02, Alt = 0x04, Super = 0x08, CapsLock = 0x10, NumLock = 0x20 };


    SketchEvents(Sketcher* parent) : m_Parent(parent) {}
    
    CommandType GetCommandType() const;
    void GetCommandType(CommandType command);


    // Mouse Button Event
    //
    //   On Left Click                 
    //       -  select item
    //       -  if(there is no item), deselect items
    //   Left Click w/ Ctrl or Shift
    //       -  add to selection
    //   On Release
    //       -  if(selection box) select these items
    void Mouse_Button(MouseButton button, MouseAction action, KeyModifier modifier);
    
    // Mouse Move Event
    //
    //  Input should be (x, y) coords in sketch space
    //
    //  Hover over item -  Highlights it
    //  Drag            -  if (clicked)
    //                        -  if(Selected) drag items
    //                        -  if(!selected) drag selection box
    void Mouse_Move(const Vec2& p);


        
private:
    CommandType m_CommandType = CommandType::None;
    std::vector<Vec2> m_InputData;
    
    Vec2 m_CursorPos;
    Vec2 m_CursorClickedPos;
    double m_SelectionTolerance = 1.0;

    MouseButton m_MouseButton;
    MouseAction m_MouseAction;

    //KeyModifier m_Modifier;
    
    Sketcher* m_Parent = nullptr;
};



class Sketcher
{
public:
    Sketcher();
    
    ElementFactory& Factory()   { return m_Factory; }
    SketchEvents& Events()  { return m_Events; }
    SketchRenderer& Renderer()  { return m_Renderer; }
    
    
    
    // Attempts to solve constraints. 
    // movedPoint can be set for moving a point
    void SolveConstraints(Vec2 p = Vec2()); 
    
    
    bool DrawImGui();
    bool DrawImGui_Elements();
    bool DrawImGui_Constraints();

private:
    bool m_IsActive = false;
    
    ElementFactory m_Factory;
    SketchEvents m_Events;  
    SketchRenderer m_Renderer;
};
     

} // end namespace Sketch

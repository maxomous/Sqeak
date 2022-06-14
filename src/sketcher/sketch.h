#pragma once


#include "elementfactory.h"


namespace Sketch {

struct RenderData
{
    PointsCollection points;  // vector<vector<Vec2>>>
    LineStrings linestrings;  // vector<vector<Vec2>>>
};


/*
    use std::variant for class with 2 return types
    // TODO: Check if conincident point on arc works?
    TODO: Make PointRefs work for elements...
    TODO: Check if point on arc works on solver?
    TODO: May have an error where it gets index - 1 but if 1 gets deleted, thats not true, so search with id
    TODO: Dont pass ptrs around top level
    TODO: remove unnessesary dynamic_cast, change to static_cast
    TODO: Are we actually using ID?
    TODO: Dynamically allocate memory (or at leasst add up what we need), currently we have hardcoded a number#
    TODO: Change name of Point2D, to Point somehow
    * Rename pointType to more general type
    * 
    * Perhaps Constraints_ could be replaced by Constraints<> and move specific to solver
    * Would PointRef<...>() work?
    * 
    TODO: make sure circle elememts are also updating radius and distances 
    * remove 
        using namespace MaxLib::;
    * from header files
*/
    
class SketchRenderer
{
public:
    const RenderData& Render(ElementFactory& factory, int arcSegments = 8);
private:
    // draw list for viewer
    RenderData m_RenderData;
};



class Sketcher
{
public:
    Sketcher();
    
    const RenderData& Render();
    void Update(Sketch::Point* draggedPoint, Vec2 pos);
    
private:
    ElementFactory m_Factory;
    SketchRenderer sketchRenderer;
};
     

} // end namespace Sketch

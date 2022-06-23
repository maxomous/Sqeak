#pragma once

#include <string>
#include <vector>
#include <MaxLib.h>

using namespace MaxLib::Geom;

namespace Sketch {

typedef int ElementID;
typedef int ConstraintID;

// A point reference allows us to reference a specific point in an element (e.g. p1 of line)
// This stores a pointer to the element and the type refers to which point
struct SketchItem
{
    enum class Type { Unset, Point, Line, Arc, Circle, Line_P0, Line_P1, Arc_P0, Arc_P1, Arc_PC, Circle_PC };
    
    std::string Name() 
    {
        std::string id = std::to_string(element);
        if(type == Type::Unset)     { return "Unset"; }
        if(type == Type::Point)     { return "Point " + id; }
        if(type == Type::Line)      { return "Line " + id; }
        if(type == Type::Arc)       { return "Arc " + id; }
        if(type == Type::Circle)    { return "Circle " + id; }
        if(type == Type::Line_P0)   { return "Line " + id + " (P0)"; }
        if(type == Type::Line_P1)   { return "Line " + id + " (P1)"; }
        if(type == Type::Arc_P0)    { return "Arc " + id + " (P0)"; }
        if(type == Type::Arc_P1)    { return "Arc " + id + " (P1)"; }
        if(type == Type::Arc_PC)    { return "Arc " + id + " (PC)"; }
        if(type == Type::Circle_PC) { return "Circle " + id + " (PC)"; }
        assert(0 && "Type unknown"); return "";
    };
   // SketchItem(Type type, ElementID id) : item(type), element(id) {}
    Type type = Type::Unset;  
    ElementID element = -1;
};

typedef std::vector<Vec2>       Points;
typedef std::vector<Points>     PointsCollection;
typedef std::vector<Vec2>       LineString;
typedef std::vector<LineString> LineStrings;

struct RenderData
{
    PointsCollection points;  // vector<vector<Vec2>>>
    LineStrings linestrings;  // vector<vector<Vec2>>>
};

} // end namespace Sketch

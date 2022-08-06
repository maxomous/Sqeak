#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <MaxLib.h>
#include "../libs/geos.h" // TODO: Sketch shouldn't own geos


namespace Sketch {

typedef int ElementID;
typedef int ConstraintID;

// A point reference allows us to reference a specific point in an element (e.g. p1 of line)
// This stores a pointer to the element and the type refers to which point
struct SketchItem
{
    enum class Type { Unset, Point, Line, Arc, Circle, Line_P0, Line_P1, Arc_P0, Arc_P1, Arc_PC, Circle_PC };
    
    
    Type type = Type::Unset;  
    ElementID element = -1;
    
    std::string Name() 
    {
        std::string id = std::to_string(element);
        if(type == Type::Unset)     { return "Unset"; }
        if(type == Type::Point)     { return "Point " + id; }
        if(type == Type::Line)      { return "Line " + id; }
        if(type == Type::Arc)       { return "Arc " + id; }
        if(type == Type::Circle)    { return "Circle " + id; }
        if(type == Type::Line_P0)   { return "P0 of Line " + id; }
        if(type == Type::Line_P1)   { return "P1 of Line " + id; }
        if(type == Type::Arc_P0)    { return "P0 of Arc " + id; }
        if(type == Type::Arc_P1)    { return "P1 of Arc " + id; }
        if(type == Type::Arc_PC)    { return "PC of Arc " + id; }
        if(type == Type::Circle_PC) { return "PC of Circle " + id; }
        assert(0 && "Type unknown"); return "";
    };
    
    SketchItem P0() {
        if(type == Type::Line)          { return { Type::Line_P0, element }; }
        else if(type == Type::Arc)      { return { Type::Arc_P0, element }; }
        else { assert(0 && "Type does not have a P0"); }
    }
    SketchItem P1() {
        if(type == Type::Line)          { return { Type::Line_P1, element }; }
        else if(type == Type::Arc)      { return { Type::Arc_P1, element }; }
        else { assert(0 && "Type does not have a P1"); }
    }
    SketchItem PC() {
        if(type == Type::Arc)           { return { Type::Arc_PC, element }; }
        else if(type == Type::Circle)   { return { Type::Circle_PC, element }; }
        else { assert(0 && "Type does not have a PC"); }
    }
};
// Allows us to see if 2 SketchItems are identical
static inline bool  operator==(const SketchItem& a, const SketchItem& b) { return (a.type == b.type && a.element == b.element); }


} // end namespace Sketch

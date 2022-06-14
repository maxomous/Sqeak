#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include "libslvs/include/slvs.h"

using namespace std;


namespace Solver {

typedef uint32_t Constraint; // equiv. to Slvs_hConstraint
enum Group { Fixed = 1, Free = 2 };

// Forward declare  
class Constraints;
  
  
class Point
{
public:
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Point() = default;
    
    Slvs_hEntity entity;
private:
    // Only allow Point2D and Point3D to construct
    Point() = default;
    friend class Point2D;
    friend class Point3D;
};
  
class Point3D : public Point
{
public:
    Point3D() {}           // Blank Constructor
    Slvs_hParam paramX ;
    Slvs_hParam paramY;
    Slvs_hParam parameterZ;
    
private:
    // Only Constraints can construct this
    Point3D(Constraints* parent, Group group, double x, double y, double z);
    friend class Constraints;
    friend class Axis;
};
   
class Normal
{
public:
    Normal() {}           // Blank Constructor
    Slvs_hParam parameters[4];
    Slvs_hEntity entity;
    
private:
    // Only Constraints can construct this
    // default has basis vectors (1 0 0) and (0 1 0)
    Normal(Constraints* parent, Group group, double ux = 1, double uy = 0, double uz = 0, double vx = 0, double vy = 1, double vz = 0);
    friend class Constraints;
    friend class Axis;
};

class Workplane 
{
public:
    Workplane() {}           // Blank Constructor
    Slvs_hEntity entity;
    
private:
    // Only Constraints can construct this
    Workplane(Constraints* parent, Group group, Point3D& origin, Normal& normal);
    friend class Constraints;
    friend class Axis;
};
    
class Axis
{
public:
    Axis() {}           // Blank Constructor
    Point3D origin;
    Normal normal;
    Workplane plane;
private:
    // Only Constraints can construct this
    Axis(Constraints* parent, Group group, double x = 0, double y = 0, double z = 0);
    friend class Constraints;
};

class Distance
{
public:
    Slvs_hParam parameter;
    Slvs_hEntity entity;
    
private:
    // Only Constraints can construct this
    Distance(Constraints* parent, Group group, Axis& axis, double distance);
    friend class Constraints;
    friend class Circle;
};

         
// ********************* ELEMENTS ************************ //
class Element
{
public:
    Element() {}
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Element() = default;
};

class Point2D : public Point, public Element
{
public:
    Slvs_hParam paramX;
    Slvs_hParam paramY;
    
private:
    // Only Constraints can construct this
    Point2D(Constraints* parent, Group group, Axis& axis, double x, double y);
    friend class Constraints;
    friend class Line;
    friend class Arc;
    friend class Circle;
};

class Line : public Element
{
public:
    Slvs_hEntity entity;
    Point2D p0;
    Point2D p1;

private:
    // Only Constraints can construct this
    Line(Constraints* parent, Group group, Axis& axis, double x0, double y0, double x1, double y1);
    friend class Constraints;
};

class Arc : public Element
{
public:
    Slvs_hEntity entity;
    Point2D pC;
    Point2D p0;
    Point2D p1;

private:
    // Only Constraints can construct this
    Arc(Constraints* parent, Group group, Axis& axis, double xC, double yC, double x0, double y0, double x1, double y1);
    friend class Constraints;
};
    
    
class Circle : public Element
{
public:
    Slvs_hEntity entity;
    Point2D pC;
    Distance radius;
    
private:
    // Only Constraints can construct this
    Circle(Constraints* parent, Group group, Axis& axis, double xC, double yC, double r);
    friend class Constraints;
};


// Solver success values
enum Success { Okay, Inconsistent, DidntConverge, TooManyUnknowns };

// Solver result structure
struct SolverResult
{
    // Indicates whether the solver succeeded
    Success success;
    // Indicates the number of unconstrained degrees of freedom.
    int dof = -1;
    // Indicated the failed constraints
    std::vector<Slvs_hConstraint> failed;
};

    
class Constraints
{
public:
 
    Constraints(size_t allocatedSize = 50) : allocSize(allocatedSize) // Create our workplane with origin (x y z) = (0 0 0) and normal to XY plane
    {
        // to access, use [ID-1]
        sys.param = new Slvs_Param[allocSize];
        sys.entity = new Slvs_Entity[allocSize];
        sys.constraint = new Slvs_Constraint[allocSize];
        sys.failed = new Slvs_hConstraint[allocSize];
        sys.faileds = allocSize;
        sys.params = sys.constraints = sys.entities = 0;
        defaultAxis = CreateAxis();
    }
    
    ~Constraints() {
        delete[] sys.param;
        delete[] sys.entity;
        delete[] sys.constraint;
        delete[] sys.failed;
    }


    // The base group for an axis is kept constant and we'll solve for group_free
    Axis    CreateAxis(double x = 0, double y = 0, double z = 0)                                    { return { this, Group::Fixed, x, y, z }; }
    Point2D CreatePoint2D(Axis& axis, double x = 0, double y = 0)                                   { return { this, Group::Free, axis, x, y }; }
    Point3D CreatePoint3D(double x = 0, double y = 0, double z = 0)                                 { return { this, Group::Free, x, y, z }; }
    Line    CreateLine(Axis& axis, double x0, double y0, double x1, double y1)                      { return { this, Group::Free, axis, x0, y0, x1, y1 }; }
    Arc     CreateArc(Axis& axis, double xC, double yC, double x0, double y0, double x1, double y1) { return { this, Group::Free, axis, xC, yC, x0, y0, x1, y1 }; }
    Circle  CreateCircle(Axis& axis, double xC, double yC, double r)                                { return { this, Group::Free, axis, xC, yC, r }; }

    // Default axis
    Point2D CreatePoint2D(double x = 0, double y = 0)                                               { return CreatePoint2D(defaultAxis, x, y); }
    Line    CreateLine(double x0, double y0, double x1, double y1)                                  { return CreateLine(defaultAxis, x0, y0, x1, y1); }
    Arc     CreateArc(double xC, double yC, double x0, double y0, double x1, double y1)             { return CreateArc(defaultAxis, xC, yC, x0, y0, x1, y1); }
    Circle  CreateCircle(double xC, double yC, double r)                                            { return CreateCircle(defaultAxis, xC, yC, r); }


    // ********************* CONSTRAINTS ************************ //

//#define SLVS_C_POINTS_COINCIDENT        100000
//#define SLVS_C_PT_PT_DISTANCE           100001
    //#define SLVS_C_PT_PLANE_DISTANCE        100002
//#define SLVS_C_PT_LINE_DISTANCE         100003
    //#define SLVS_C_PT_FACE_DISTANCE         100004
    //#define SLVS_C_PT_IN_PLANE              100005
//#define SLVS_C_PT_ON_LINE               100006
    //#define SLVS_C_PT_ON_FACE               100007
//#define SLVS_C_EQUAL_LENGTH_LINES       100008
    //#define SLVS_C_LENGTH_RATIO             100009
    //#define SLVS_C_EQ_LEN_PT_LINE_D         100010
    //#define SLVS_C_EQ_PT_LN_DISTANCES       100011
    //#define SLVS_C_EQUAL_ANGLE              100012
    //#define SLVS_C_EQUAL_LINE_ARC_LEN       100013
    //#define SLVS_C_SYMMETRIC                100014
    //#define SLVS_C_SYMMETRIC_HORIZ          100015
    //#define SLVS_C_SYMMETRIC_VERT           100016
    //#define SLVS_C_SYMMETRIC_LINE           100017
    //#define SLVS_C_AT_MIDPOINT              100018
//#define SLVS_C_HORIZONTAL               100019
//#define SLVS_C_VERTICAL                 100020
//#define SLVS_C_DIAMETER                 100021
//#define SLVS_C_PT_ON_CIRCLE             100022
    //#define SLVS_C_SAME_ORIENTATION         100023
//#define SLVS_C_ANGLE                    100024
//#define SLVS_C_PARALLEL                 100025
//#define SLVS_C_PERPENDICULAR            100026
//#define SLVS_C_ARC_LINE_TANGENT         100027
    //#define SLVS_C_CUBIC_LINE_TANGENT       100028
//#define SLVS_C_EQUAL_RADIUS             100029
    //#define SLVS_C_PROJ_PT_DISTANCE         100030
    // ? #define SLVS_C_WHERE_DRAGGED            100031
//#define SLVS_C_CURVE_CURVE_TANGENT      100032
    //#define SLVS_C_LENGTH_DIFFERENCE        100033

    // Coincident Point to Point
    Constraint Add_Coincident_PointToPoint(Axis& axis, const Point& pointA, const Point& pointB) 
    {
        return MakeConstraint(axis, SLVS_C_POINTS_COINCIDENT, 0.0, pointA.entity, pointB.entity, 0, 0);
    }
    // Coincident Point to Line
    Constraint Add_Coincident_PointToLine(Axis& axis, const Point& point, const Line& line)
    {
        return MakeConstraint(axis, SLVS_C_PT_ON_LINE, 0.0, point.entity, 0, line.entity, 0);
    }
    // TODO: Check if point on arc works?
    // Coincident Point to Arc
    Constraint Add_Coincident_PointToArc(Axis& axis, const Point& point, const Arc& arc)
    {
        return MakeConstraint(axis, SLVS_C_PT_ON_CIRCLE/*TODO: NOT SURE ABOUT THIS*/, 0.0, point.entity, 0, arc.entity, 0);
    }
    // Coincident Point to Circle
    Constraint Add_Coincident_PointToCircle(Axis& axis, const Point& point, const Circle& circle)
    {
        return MakeConstraint(axis, SLVS_C_PT_ON_CIRCLE, 0.0, point.entity, 0, circle.entity, 0);
    }
    
    
    // Distance Point to Point
    Constraint Add_Distance_PointToPoint(Axis& axis, const Point& pointA, const Point& pointB, double value) 
    {
        return MakeConstraint(axis, SLVS_C_PT_PT_DISTANCE, value, pointA.entity, pointB.entity, 0, 0);
    }
    Constraint Add_Distance_PointToPoint(Axis& axis, const Line& line, double value) 
    {
        return MakeConstraint(axis, SLVS_C_PT_PT_DISTANCE, value, line.p0.entity, line.p1.entity, 0, 0);
    }
    // Distance Point to Line
    Constraint Add_Distance_PointToLine(Axis& axis, const Point& point, const Line& line, double value) 
    {
        return MakeConstraint(axis, SLVS_C_PT_LINE_DISTANCE, value, point.entity, 0, line.entity, 0);
    }
    
    // Radius
    Constraint Add_Radius(Axis& axis, const Circle& circle, double radius) 
    {
        return MakeConstraint(axis, SLVS_C_DIAMETER, radius*2, 0, 0, circle.entity, 0);
    }
    Constraint Add_Radius(Axis& axis, const Arc& arc, double radius) 
    {
        return MakeConstraint(axis, SLVS_C_DIAMETER, radius*2, 0, 0, arc.entity, 0);
    }
 
    // Angle
    Constraint Add_Angle(Axis& axis, const Line& line1, const Line& line2, double value) 
    {
        return MakeConstraint(axis, SLVS_C_ANGLE, value, 0, 0, line1.entity, line2.entity);
    }

    // Vertical
    Constraint Add_Vertical(Axis& axis, const Line& line) 
    {
        return MakeConstraint(axis, SLVS_C_VERTICAL, 0.0, 0, 0, line.entity, 0);
    }
    // Horizontal
    Constraint Add_Horizontal(Axis& axis, const Line& line) 
    {
        return MakeConstraint(axis, SLVS_C_HORIZONTAL, 0.0, 0, 0, line.entity, 0);
    }
    
    // Parallel
    Constraint Add_Parallel(Axis& axis, const Line& line1, const Line& line2) 
    {
        return MakeConstraint(axis, SLVS_C_PARALLEL, 0.0, 0, 0, line1.entity, line2.entity);
    }
    // Perpendicular
    Constraint Add_Perpendicular(Axis& axis, const Line& line1, const Line& line2) 
    {
        return MakeConstraint(axis, SLVS_C_PERPENDICULAR, 0.0, 0, 0, line1.entity, line2.entity);
    }
       
       
    // Tangent
    Constraint Add_Tangent(Axis& axis, const Arc& arc, const Line& line) 
    {
        return MakeConstraint(axis, SLVS_C_ARC_LINE_TANGENT, 0.0, 0, 0, arc.entity, line.entity);
    }
    Constraint Add_Tangent(Axis& axis, const Arc& arc1, const Arc& arc2) 
    {
        return MakeConstraint(axis, SLVS_C_CURVE_CURVE_TANGENT, 0.0, 0, 0, arc1.entity, arc2.entity);
    }
    // TODO: Check  circle + arc  &  circle + circle
    
           
    // Equal Length
    Constraint Add_EqualLength(Axis& axis, const Line& line1, const Line& line2) 
    {
        return MakeConstraint(axis, SLVS_C_EQUAL_LENGTH_LINES, 0.0, 0, 0, line1.entity, line2.entity);
    }
    
    // Equal Radius
    Constraint Add_EqualRadius(Axis& axis, const Arc& arc, const Circle& circle) 
    {
        return MakeConstraint(axis, SLVS_C_EQUAL_RADIUS, 0.0, 0, 0, arc.entity, circle.entity);
    }
    Constraint Add_EqualRadius(Axis& axis, const Arc& arc1, const Arc& arc2) 
    {
        return MakeConstraint(axis, SLVS_C_EQUAL_RADIUS, 0.0, 0, 0, arc1.entity, arc2.entity);
    }
    Constraint Add_EqualRadius(Axis& axis, const Circle& circle1, const Circle& circle2) {
        return MakeConstraint(axis, SLVS_C_EQUAL_RADIUS, 0.0, 0, 0, circle1.entity, circle2.entity);
    }
    
    // Constraints with Default Axis 
    Constraint Add_Coincident_PointToPoint(const Point& pointA, const Point& pointB)                { return Add_Coincident_PointToPoint(defaultAxis, pointA, pointB); }
    Constraint Add_Coincident_PointToLine(const Point& point, const Line& line)                     { return Add_Coincident_PointToLine(defaultAxis, point, line); }
    Constraint Add_Coincident_PointToArc(const Point& point, const Arc& arc)                        { return Add_Coincident_PointToArc(defaultAxis, point, arc); }
    Constraint Add_Coincident_PointToCircle(const Point& point, const Circle& circle)               { return Add_Coincident_PointToCircle(defaultAxis, point, circle); }
    Constraint Add_Distance_PointToPoint(const Point& pointA, const Point& pointB, double value)    { return Add_Distance_PointToPoint(defaultAxis, pointA, pointB, value); }
    Constraint Add_Distance_PointToPoint(const Line& line, double value)                            { return Add_Distance_PointToPoint(defaultAxis, line, value); }
    Constraint Add_Distance_PointToLine(const Point& point, const Line& line, double value)         { return Add_Distance_PointToLine(defaultAxis, point, line, value); }
    Constraint Add_Radius(const Circle& circle, double radius)                                      { return Add_Radius(defaultAxis, circle, radius); }
    Constraint Add_Radius(const Arc& arc, double radius)                                            { return Add_Radius(defaultAxis, arc, radius); }
    Constraint Add_Angle(const Line& line1, const Line& line2, double value)                        { return Add_Angle(defaultAxis, line1, line2, value); }
    Constraint Add_Vertical(const Line& line)                                                       { return Add_Vertical(defaultAxis, line); }
    Constraint Add_Horizontal(const Line& line)                                                     { return Add_Horizontal(defaultAxis, line); }
    Constraint Add_Parallel(const Line& line1, const Line& line2)                                   { return Add_Parallel(defaultAxis, line1, line2); }    
    Constraint Add_Perpendicular(const Line& line1, const Line& line2)                              { return Add_Perpendicular(defaultAxis, line1, line2); }
    Constraint Add_Tangent(const Arc& arc, const Line& line)                                        { return Add_Tangent(defaultAxis, arc, line); }
    Constraint Add_Tangent(const Arc& arc1, const Arc& arc2)                                        { return Add_Tangent(defaultAxis, arc1, arc2); }
    Constraint Add_EqualLength(const Line& line1, const Line& line2)                                { return Add_EqualLength(defaultAxis, line1, line2); }
    Constraint Add_EqualRadius(const Arc& arc, const Circle& circle)                                { return Add_EqualRadius(defaultAxis, arc, circle); }
    Constraint Add_EqualRadius(const Arc& arc1, const Arc& arc2)                                    { return Add_EqualRadius(defaultAxis, arc1, arc2); }
    Constraint Add_EqualRadius(const Circle& circle1, const Circle& circle2)                        { return Add_EqualRadius(defaultAxis, circle1, circle2); }
    
    // Attempts to not move given point when solving
    void SetDraggedPoint(Point2D& point)
    {
        sys.dragged[0] = point.paramX; // x
        sys.dragged[1] = point.paramY; // y
    }
    
    
    // returns true on success
    SolverResult Solve(bool calculateFailedConstraints = true)
    {
        // If the solver fails, report which constraints caused the problem
        sys.calculateFaileds = calculateFailedConstraints;

        // Solve for free group (other group is fixed)
        Slvs_Solve(&sys, (Slvs_hGroup)Group::Free);
        
        // Get result
        SolverResult result;
        result.success = (Success)sys.result;
        
        // if success, get DOFs
        if(result.success == Success::Okay) {
            result.dof = sys.dof;
        }
        else { // else get failed constraints
            for(int i = 0; i < sys.faileds; i++) {
                result.failed.push_back(sys.failed[i]);
            }
        }
        
        return result;
    }
    
    
    double GetResult(Slvs_hParam parameter) {
        assert(parameter >= 1);
        size_t index = parameter - 1;
        assert(index < allocSize);
        return sys.param[index].val;
    }
    
    // Helper function to get x & y result from solver
    std::array<double, 2> GetResult(const Solver::Point2D& point) {
        return { GetResult(point.paramX), GetResult(point.paramY) };
    }
    
    void ModifyParamValue(Slvs_hParam parameter, float newValue) {
        assert(parameter > 0 && parameter < allocSize);
        sys.param[parameter - 1].val = newValue;
    }
    
    void ModifyParamGroup(Slvs_hParam parameter, Group group) {
        sys.param[parameter - 1].group = (Slvs_hGroup)group;
    }
    
private:
    Slvs_System sys;
    Axis defaultAxis;
    
    size_t allocSize;
    
    
    Slvs_hParam MakeParam(Group group, double value) 
    {
        assert(sys.params < (int)allocSize);
        Slvs_hParam id = sys.params + 1;
        sys.param[sys.params++] = Slvs_MakeParam(id, (Slvs_hGroup)group, value);
        return id;
    }
       
    Slvs_hEntity MakeEntity(std::function<Slvs_Entity(Slvs_hEntity id)> cb) 
    {
        assert(sys.entities < (int)allocSize);
        Slvs_hEntity id = sys.entities + 1;
        sys.entity[sys.entities++] = cb(id);
        return id;
    }
                                                      
    Slvs_hConstraint MakeConstraint(Axis& axis, int type, double value, Slvs_hEntity pointA, Slvs_hEntity pointB, Slvs_hEntity entityA, Slvs_hEntity entityB) 
    {
        assert(sys.constraints < (int)allocSize);
        Slvs_hConstraint id = sys.constraints + 1;
        sys.constraint[sys.constraints++] = Slvs_MakeConstraint(id, (Slvs_hGroup)Group::Free, type, axis.plane.entity, value, pointA, pointB, entityA, entityB);
        return id;
    }


        
    friend class Point3D;
    friend class Normal;
    friend class Workplane;
    friend class Axis;
    friend class Point2D;
    friend class Distance;
    friend class Line;
    friend class Arc;
    friend class Circle;

}; // End class Constraints

} // End namespace Solver


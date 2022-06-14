#pragma once
#include <memory>
#include <cmath>
#include <MaxLib.h>

#include "deps/constraintsolver/solver.h"

namespace Sketch {

using namespace MaxLib::Geom;

typedef std::vector<Vec2>       Points;
typedef std::vector<Points>     PointsCollection;
typedef std::vector<Vec2>       LineString;
typedef std::vector<LineString> LineStrings;


typedef int ElementID;

// Forward declare
class Element;
class Point;
class Line;
class Arc;
class Circle;


enum class PointType { Point, Line, Arc, Circle, Line_P0, Line_P1, Arc_P0, Arc_P1, Arc_PC, Circle_PC };
    
// A point reference allows us to reference a specific point in an element (e.g. p1 of line)
// This stores a pointer to the element and the type refers to which point
struct PointRef
{

    PointRef(const Element* element, PointType type);
    PointRef(const Point* element);     // PointType::Point
    PointRef(const Line* element);      // PointType::Line
    PointRef(const Arc* element);       // PointType::Arc
    PointRef(const Circle* element);    // PointType::Circle
    
    //const &  ID();
    
    const Solver::Point2D&  GetPoint();
    const Solver::Line&     GetLine();
    const Solver::Arc&      GetArc(); 
    const Solver::Circle&   GetCircle();

    const Element* element;
    const PointType type;
};

class Element
{
public:
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Element() = default;
    
    ElementID ID() const;
    
    virtual void Print() = 0;
    
    virtual void AddToSolver(Solver::Constraints& constraints) = 0;
    virtual void UseSolverValues(Solver::Constraints& constraints) = 0;
    void         ClearSolverData();
    
    template<typename T>
    T* SolverElement() const 
    { 
        auto element = dynamic_cast<T*>(m_SolverElement.get());
        if(element) {
            return element;
        } else {
            assert(0 && "Casting to element failed!");
        }
    }
    
protected:
    Element();
    
    ElementID m_ID = 0;
    
    std::unique_ptr<Solver::Element> m_SolverElement;
    
    LineString RenderArc(int arcSegments, Direction direction, const Vec2 pC, float radius, double th_Start, double th_End) const
    {
        LineString linestring;
        // Clean up angles
        CleanAngles(th_Start, th_End, direction);
        // Calculate increment from n segments in 90 degrees
        float th_Incr = direction * (M_PI / 2.0) / arcSegments;
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
    
    
    Vec2 GetResult(Solver::Constraints& constraints, Solver::Point2D point) 
    {    
        std::array<double, 2> p = constraints.GetResult(point);
        return { p[0], p[1] };
    }
};



    
    
class Point : public Element
{
public:
    Point(const Vec2& p);
    
    const Vec2& P() const;
    
    PointRef Ref_P();
    
    void Print() override;
    
    void AddToSolver(Solver::Constraints& constraints) override;
    void UseSolverValues(Solver::Constraints& constraints) override;
    
private:

    Vec2 m_P;
};



class Line : public Element
{
public:
    Line(const Vec2& p0, const Vec2& p1);
    const Vec2& P0() const;
    const Vec2& P1() const;
    
    PointRef Ref_P0();
    PointRef Ref_P1();
    
    void Print() override;
    
    void AddToSolver(Solver::Constraints& constraints) override;
    void UseSolverValues(Solver::Constraints& constraints) override;
    
private:

    Vec2 m_P0;
    Vec2 m_P1;
};


class Arc : public Element
{
public:
    Arc(const Vec2& p0, const Vec2& p1, const Vec2& pC);
    const Vec2& P0() const;
    const Vec2& P1() const;
    const Vec2& PC() const;

    PointRef Ref_P0();
    PointRef Ref_P1();
    PointRef Ref_PC();
    
    void Print() override;
    
    void AddToSolver(Solver::Constraints& constraints) override;
    void UseSolverValues(Solver::Constraints& constraints) override;
    
    LineString RenderArc(int arcSegments, Direction direction) const
    {
        // get start and end points relative to the centre point
        Vec2 v_Start    = m_P0 - m_PC;
        Vec2 v_End      = m_P1 - m_PC;
        // get start and end angles
        double th_Start = atan2(v_Start.x, v_Start.y);
        double th_End   = atan2(v_End.x, v_End.y);
        // draw arc between angles
        return Element::RenderArc(arcSegments, direction, m_PC, hypot(v_End.x, v_End.y), th_Start, th_End);
    }
    
    
private:
    Vec2 m_P0;
    Vec2 m_P1;
    Vec2 m_PC;
};


class Circle : public Element
{
public:
    Circle(const Vec2& pC, float radius);
    const Vec2& PC() const;
    float Radius() const;

    PointRef Ref_PC();
    
    void Print() override;
    
    void AddToSolver(Solver::Constraints& constraints) override;
    void UseSolverValues(Solver::Constraints& constraints) override;
    
    
    LineString RenderCircle(int arcSegments) const {
        // draw arc between angles
        return RenderArc(arcSegments, Direction::CW, m_PC, m_Radius, 0.0, 2.0 * M_PI);
    }
    
private:
    Vec2 m_PC;
    float m_Radius;
};

} // end namespace Sketch

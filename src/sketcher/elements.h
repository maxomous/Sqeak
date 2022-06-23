#pragma once
#include <memory>
#include <cmath>
#include <MaxLib.h>

#include "sketch_common.h"

#include "deps/constraintsolver/solver.h"

namespace Sketch {

using namespace MaxLib::Geom;

// Forward declare
//typedef int ElementID;
//class SketchItem;
/*
    class Element;
    class Point;
    class Line;
    class Arc;
    class Circle;
*/


/*

class _
{
public:
    
private:
    VertexData m_VertexData;
};
*/



class Element
{
public:
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Element() = default;
    
    ElementID ID() const;
    
    virtual void Print() = 0;
    
    virtual void AddToSolver(Solver::ConstraintSolver& solver) = 0;
    virtual void UseSolverValues(Solver::ConstraintSolver& solver) = 0;
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
        
    Vec2 GetResult(Solver::ConstraintSolver& solver, Solver::Point2D point) 
    {    
        std::array<double, 2> p = solver.GetResult(point);
        return { p[0], p[1] };
    }
};



    
    
class Point : public Element
{
public:
    Point(const Vec2& p);
    
    const Vec2& P() const;
    
    const SketchItem Ref_P() const;
    
    void Print() override;
    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    void UseSolverValues(Solver::ConstraintSolver& solver) override;
    
private:

    Vec2 m_P;
};



class Line : public Element
{
public:
    Line(const Vec2& p0, const Vec2& p1);
    const Vec2& P0() const;
    const Vec2& P1() const;
    
    const SketchItem Ref_P0() const;
    const SketchItem Ref_P1() const;
    
    void Print() override;
    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    void UseSolverValues(Solver::ConstraintSolver& solver) override;
    
private:

    Vec2 m_P0;
    Vec2 m_P1;
};


class Arc : public Element
{
public:
    Arc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction);
    const Vec2& P0() const;
    const Vec2& P1() const;
    const Vec2& PC() const;
    const MaxLib::Geom::Direction& Direction() const;

    const SketchItem Ref_P0() const;
    const SketchItem Ref_P1() const;
    const SketchItem Ref_PC() const;
    
    void Print() override;
    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    void UseSolverValues(Solver::ConstraintSolver& solver) override;
    
private:
    Vec2 m_P0;
    Vec2 m_P1;
    Vec2 m_PC;
    MaxLib::Geom::Direction m_Direction;
};


class Circle : public Element
{
public:
    Circle(const Vec2& pC, double radius);
    const Vec2& PC() const;
    double Radius() const;

    const SketchItem Ref_PC() const;
    
    void Print() override;
    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    void UseSolverValues(Solver::ConstraintSolver& solver) override;
    
private:
    Vec2 m_PC;
    double m_Radius;
};

} // end namespace Sketch


#include <iostream>
#include "elements.h"

 

namespace Sketch
{
// Global counter for elements
static ElementID m_ElementCounter = 0;


Element::Element() : m_ID(++m_ElementCounter) {
    std::cout << "Adding Element: " << m_ID << std::endl;
}

ElementID Element::ID() const { 
    return m_ID; 
}

void Element::ClearSolverData() 
{ 
    m_SolverElement.reset(); 
}


// Point

Point::Point(const Vec2& p) 
    : Element(), m_P(p) {}

const Vec2& Point::P() const {
    return m_P; 
}

const SketchItem Point::Ref_P() const {
    return { SketchItem::Type::Point, m_ID }; 
}

void Point::Print() 
{
    std::cout << "Point Element (" << (int)m_ID << ")\n";
    std::cout << "\tP: (" << m_P.x << ", " << m_P.y << ")\n" << std::endl;
}

void Point::AddToSolver(Solver::ConstraintSolver& solver) 
{
    m_SolverElement = move(std::make_unique<Solver::Point2D>(solver.CreatePoint2D(m_P.x, m_P.y)));
}

void Point::UseSolverValues(Solver::ConstraintSolver& solver) 
{
    m_P = GetResult(solver, *SolverElement<Solver::Point2D>());
}


// Line

    
Line::Line(const Vec2& p0, const Vec2& p1) 
    : Element(), m_P0(p0), m_P1(p1) {}

const Vec2& Line::P0() const { 
    return m_P0; 
}
const Vec2& Line::P1() const { 
    return m_P1; 
}

const SketchItem Line::Ref_P0() const { 
    return { SketchItem::Type::Line_P0, m_ID }; 
}
const SketchItem Line::Ref_P1() const { 
    return { SketchItem::Type::Line_P1, m_ID }; 
}

void Line::Print() {
    std::cout << "Line Element (" << (int)m_ID << ")\n";
    std::cout << "\tP0: (" << m_P0.x << ", " << m_P0.y << ")\n";
    std::cout << "\tP1: (" << m_P1.x << ", " << m_P1.y << ")\n" << std::endl;
}

void Line::AddToSolver(Solver::ConstraintSolver& solver) {
    m_SolverElement = std::make_unique<Solver::Line>(solver.CreateLine(m_P0.x, m_P0.y, m_P1.x, m_P1.y));
}
void Line::UseSolverValues(Solver::ConstraintSolver& solver) {
    Solver::Line* element = SolverElement<Solver::Line>();
    m_P0 = GetResult(solver, element->p0);
    m_P1 = GetResult(solver, element->p1);
}

// Arc

Arc::Arc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction) 
    : Element(), m_P0(p0), m_P1(p1), m_PC(pC), m_Direction(direction) {}
    
const Vec2& Arc::P0() const { 
    return m_P0; 
}
const Vec2& Arc::P1() const { 
    return m_P1; 
}
const Vec2& Arc::PC() const { 
    return m_PC; 
}
const MaxLib::Geom::Direction& Arc::Direction() const { 
    return m_Direction; 
}

const SketchItem Arc::Ref_P0() const { 
    return { SketchItem::Type::Arc_P0, m_ID }; 
}
const SketchItem Arc::Ref_P1() const { 
    return { SketchItem::Type::Arc_P1, m_ID }; 
}
const SketchItem Arc::Ref_PC() const { 
    return { SketchItem::Type::Arc_PC, m_ID }; 
}

void Arc::Print() {
    std::cout << "Arc Element (" << (int)m_ID << ")\n";
    std::cout << "\tP0: (" << m_P0.x << ", " << m_P0.y << ")\n";
    std::cout << "\tP1: (" << m_P1.x << ", " << m_P1.y << ")\n";
    std::cout << "\tPC: (" << m_PC.x << ", " << m_PC.y << ")\n" << std::endl;
}

void Arc::AddToSolver(Solver::ConstraintSolver& solver) {
    m_SolverElement = std::make_unique<Solver::Arc>(solver.CreateArc(m_PC.x, m_PC.y, m_P0.x, m_P0.y, m_P1.x, m_P1.y));
}

void Arc::UseSolverValues(Solver::ConstraintSolver& solver) {
    Solver::Arc* element = SolverElement<Solver::Arc>(); 
    m_P0 = GetResult(solver, element->p0);
    m_P1 = GetResult(solver, element->p1);
    m_PC = GetResult(solver, element->pC);
}
    
// Circle

Circle::Circle(const Vec2& pC, double radius) 
    : Element(), m_PC(pC), m_Radius(radius) {}
    
const Vec2& Circle::PC() const { 
    return m_PC; 
}
double Circle::Radius() const { 
    return m_Radius; 
}

const SketchItem Circle::Ref_PC() const { 
    return { SketchItem::Type::Circle_PC, m_ID }; 
}

void Circle::Print() {
    std::cout << "Circle Element (" << (int)m_ID << ")\n";
    std::cout << "\tPC: (" << m_PC.x << ", " << m_PC.y << ")\n";
    std::cout << "\tRadius: " << m_Radius << std::endl;
}

void Circle::AddToSolver(Solver::ConstraintSolver& solver) {
    m_SolverElement = std::make_unique<Solver::Circle>(solver.CreateCircle(m_PC.x, m_PC.y, m_Radius));
}

void Circle::UseSolverValues(Solver::ConstraintSolver& solver) {
    Solver::Circle* element = SolverElement<Solver::Circle>();
    m_PC = GetResult(solver, element->pC);          
    m_Radius = solver.GetResult(element->radius.parameter);
}

} // end namespace Sketch

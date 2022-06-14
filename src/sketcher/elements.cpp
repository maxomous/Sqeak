
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

PointRef Point::Ref_P() {
    return { this, PointType::Point }; 
}

void Point::Print() 
{
    std::cout << "Point Element (" << (int)this << ")\n";
    std::cout << "\tP: (" << m_P.x << ", " << m_P.y << ")\n" << std::endl;
}

void Point::AddToSolver(Solver::Constraints& constraints) 
{
    m_SolverElement = move(std::make_unique<Solver::Point2D>(constraints.CreatePoint2D(m_P.x, m_P.y)));
}

void Point::UseSolverValues(Solver::Constraints& constraints) 
{
    m_P = GetResult(constraints, *SolverElement<Solver::Point2D>());
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

PointRef Line::Ref_P0() { 
    return { this, PointType::Line_P0 }; 
}
PointRef Line::Ref_P1() { 
    return { this, PointType::Line_P1 }; 
}

void Line::Print() {
    std::cout << "Line Element (" << (int)this << ")\n";
    std::cout << "\tP0: (" << m_P0.x << ", " << m_P0.y << ")\n";
    std::cout << "\tP1: (" << m_P1.x << ", " << m_P1.y << ")\n" << std::endl;
}

void Line::AddToSolver(Solver::Constraints& constraints) {
    m_SolverElement = std::make_unique<Solver::Line>(constraints.CreateLine(m_P0.x, m_P0.y, m_P1.x, m_P1.y));
}
void Line::UseSolverValues(Solver::Constraints& constraints) {
    Solver::Line* element = SolverElement<Solver::Line>();
    m_P0 = GetResult(constraints, element->p0);
    m_P1 = GetResult(constraints, element->p1);
}

// Arc

Arc::Arc(const Vec2& p0, const Vec2& p1, const Vec2& pC) 
    : Element(), m_P0(p0), m_P1(p1), m_PC(pC) {}
    
const Vec2& Arc::P0() const { 
    return m_P0; 
}
const Vec2& Arc::P1() const { 
    return m_P1; 
}
const Vec2& Arc::PC() const { 
    return m_PC; 
}

PointRef Arc::Ref_P0() { 
    return { this, PointType::Arc_P0 }; 
}
PointRef Arc::Ref_P1() { 
    return { this, PointType::Arc_P1 }; 
}
PointRef Arc::Ref_PC() { 
    return { this, PointType::Arc_PC }; 
}

void Arc::Print() {
    std::cout << "Arc Element (" << (int)this << ")\n";
    std::cout << "\tP0: (" << m_P0.x << ", " << m_P0.y << ")\n";
    std::cout << "\tP1: (" << m_P1.x << ", " << m_P1.y << ")\n";
    std::cout << "\tPC: (" << m_PC.x << ", " << m_PC.y << ")\n" << std::endl;
}

void Arc::AddToSolver(Solver::Constraints& constraints) {
    m_SolverElement = std::make_unique<Solver::Arc>(constraints.CreateArc(m_PC.x, m_PC.y, m_P0.x, m_P0.y, m_P1.x, m_P1.y));
}

void Arc::UseSolverValues(Solver::Constraints& constraints) {
    Solver::Arc* element = SolverElement<Solver::Arc>(); 
    m_P0 = GetResult(constraints, element->p0);
    m_P1 = GetResult(constraints, element->p1);
    m_PC = GetResult(constraints, element->pC);
}
    
// Circle

Circle::Circle(const Vec2& pC, float radius) 
    : Element(), m_PC(pC), m_Radius(radius) {}
    
const Vec2& Circle::PC() const { 
    return m_PC; 
}
float Circle::Radius() const { 
    return m_Radius; 
}

PointRef Circle::Ref_PC() { 
    return { this, PointType::Circle_PC }; 
}

void Circle::Print() {
    std::cout << "Circle Element (" << (int)this << ")\n";
    std::cout << "\tPC: (" << m_PC.x << ", " << m_PC.y << ")\n";
    std::cout << "\tRadius: " << m_Radius << std::endl;
}

void Circle::AddToSolver(Solver::Constraints& constraints) {
    m_SolverElement = std::make_unique<Solver::Circle>(constraints.CreateCircle(m_PC.x, m_PC.y, m_Radius));
}

void Circle::UseSolverValues(Solver::Constraints& constraints) {
    Solver::Circle* element = SolverElement<Solver::Circle>();
    m_PC = GetResult(constraints, element->pC);          
    m_Radius = constraints.GetResult(element->radius.parameter);
}

// Point Referenvce Constructors
PointRef::PointRef(const Element* e, PointType t) 
    : element(e), type(t) {}
PointRef::PointRef(const Point* e) 
    : element(dynamic_cast<const Element*>(e)), type(PointType::Point) {}
PointRef::PointRef(const Line* e) 
    : element(dynamic_cast<const Element*>(e)), type(PointType::Line) {}
PointRef::PointRef(const Arc* e) 
    : element(dynamic_cast<const Element*>(e)), type(PointType::Arc) {}
PointRef::PointRef(const Circle* e) 
    : element(dynamic_cast<const Element*>(e)), type(PointType::Circle) {}



const Solver::Point2D& PointRef::GetPoint() 
{
    if(type == PointType::Point ) {
        return *dynamic_cast<const Point*>(element)->SolverElement<Solver::Point2D>();
    }
    else if(type == PointType::Line_P0) {
        return dynamic_cast<const Line*>(element)->SolverElement<Solver::Line>()->p0;
    } 
    else if(type == PointType::Line_P1) {
        return dynamic_cast<const Line*>(element)->SolverElement<Solver::Line>()->p1;
    } 
    else if(type == PointType::Arc_P0) {
        return dynamic_cast<const Arc*>(element)->SolverElement<Solver::Arc>()->p0;
    } 
    else if(type == PointType::Arc_P1) {
        return dynamic_cast<const Arc*>(element)->SolverElement<Solver::Arc>()->p1;
    } 
    else if(type == PointType::Arc_PC) {
        return dynamic_cast<const Arc*>(element)->SolverElement<Solver::Arc>()->pC;
    } 
    else if(type == PointType::Circle_PC) {
        return dynamic_cast<const Circle*>(element)->SolverElement<Solver::Circle>()->pC;
    } 
    // Should never reach
    assert(0 && "Type unknown");
}

const Solver::Line& PointRef::GetLine() 
{
    if(type == PointType::Line)      { return *dynamic_cast<const Line*>(element)->SolverElement<Solver::Line>(); }  
    // Should never reach
    assert(0 && "Type is not a Line");
}

const Solver::Arc& PointRef::GetArc() 
{
    if(type == PointType::Arc)       { return *dynamic_cast<const Arc*>(element)->SolverElement<Solver::Arc>(); } 
    // Should never reach
    assert(0 && "Type is not a Arc");
}

const Solver::Circle& PointRef::GetCircle() 
{
    if(type == PointType::Circle)    { return *dynamic_cast<const Circle*>(element)->SolverElement<Solver::Circle>(); } 
    // Should never reach
    assert(0 && "Type is not a Circle");
}
} // end namespace Sketch

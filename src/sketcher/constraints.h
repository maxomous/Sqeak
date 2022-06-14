#pragma once

#include <vector>
#include "elements.h"

namespace Sketch {


typedef int ConstraintID;

class Constraint
{
public:
    
    // Constructor
    Constraint();
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Constraint() = default;
    
    ConstraintID ID() const { return m_ID; }
    
    // Adds constraint to solver
    virtual void AddToSolver(Solver::Constraints& constraints) = 0;
    
    // Passes each element to Callback Function
    virtual void ForEachElement(std::function<void(PointRef&)> cb) = 0;
    
    void ClearSolverData();
                            
    bool Failed() const;
    void SetFailed();
    Solver::Constraint SolverConstraint() const;
        
protected:
    ConstraintID m_ID;
    bool m_IsFailed = false;
    Solver::Constraint m_SolverConstraint = 0;
    
    // Passes each of these elements to Callback Function
   // void ForTheseElements(std::function<void(PointRef&)> cb, std::vector<PointRef&> elements);
private:
    void ResetFailed();
    friend class ElementFactory;
};


// Templates

class Constraint_Template_OneItem : public Constraint
{
public:
    Constraint_Template_OneItem(const PointRef& item)                               : m_Ref(item) {}
    // Passes each element to Callback Function 
    void ForEachElement(std::function<void(PointRef&)> cb)                          { cb(m_Ref); }
protected:
    PointRef m_Ref;
};


class Constraint_Template_TwoItems : public Constraint
{
public:
    Constraint_Template_TwoItems(const PointRef& item1, const PointRef& item2)      : m_Ref_1(item1), m_Ref_2(item2) {}
    // Passes each element to Callback Function 
    void ForEachElement(std::function<void(PointRef&)> cb)                          { cb(m_Ref_1); cb(m_Ref_2); }
protected:
    PointRef m_Ref_1;
    PointRef m_Ref_2;
};

// Coincident: Point To Point

class Coincident_PointToPoint : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToPoint(const PointRef& p0, const PointRef& p1)         : Constraint_Template_TwoItems(p0, p1) {}
    // Adds constraint to solver    
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Coincident_PointToPoint(m_Ref_1.GetPoint(), m_Ref_2.GetPoint()); }
};
  
// Coincident: Point To Line

class Coincident_PointToLine : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToLine(const PointRef& point, const Line* line)         : Constraint_Template_TwoItems(point, line) {}
    // Adds constraint to solver    
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Coincident_PointToLine(m_Ref_1.GetPoint(), m_Ref_2.GetLine()); }
};
 
// Coincident: Point To Arc

class Coincident_PointToArc : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToArc(const PointRef& point, const Arc* arc)            : Constraint_Template_TwoItems(point, arc) {}
    // Adds constraint to solver        
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Coincident_PointToArc(m_Ref_1.GetPoint(), m_Ref_2.GetArc()); }
};  
    
// Coincident: Point To Circle  
    
class Coincident_PointToCircle : public Constraint_Template_TwoItems    
{   
public: 
    Coincident_PointToCircle(const PointRef& point, const Circle* circle)   : Constraint_Template_TwoItems(point, circle) {}
    // Adds constraint to solver        
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Coincident_PointToCircle(m_Ref_1.GetPoint(), m_Ref_2.GetCircle()); }
};



// Distance Point to Point

class Distance_PointToPoint : public Constraint_Template_TwoItems
{
public:
    Distance_PointToPoint(const PointRef& p0, const PointRef& p1, float distance)       : Constraint_Template_TwoItems(p0, p1), m_Distance(distance) {}
    Distance_PointToPoint(const Line* line, float distance)                             : Constraint_Template_TwoItems({ line, PointType::Line_P0 }, { line, PointType::Line_P1 }), m_Distance(distance) {}
    // Adds constraint to solver        
    void AddToSolver(Solver::Constraints& constraints) override                         { m_SolverConstraint = constraints.Add_Distance_PointToPoint(m_Ref_1.GetPoint(), m_Ref_2.GetPoint(), m_Distance); }
private:
    float m_Distance;
};



// Distance Point to Line

class Distance_PointToLine : public Constraint_Template_TwoItems
{
public:
    Distance_PointToLine(const PointRef& point, const Line* line, float distance)       : Constraint_Template_TwoItems(point, line), m_Distance(distance) {}
    // Adds constraint to solver        
    void AddToSolver(Solver::Constraints& constraints) override                         { m_SolverConstraint = constraints.Add_Distance_PointToLine(m_Ref_1.GetPoint(), m_Ref_2.GetLine(), m_Distance); }
private:
    float m_Distance;
};


// Radius
    
class AddRadius_Arc : public Constraint_Template_OneItem
{
public:
    AddRadius_Arc(const Arc* arc, float radius)                             : Constraint_Template_OneItem(arc), m_Radius(radius) {}
    // Adds constraint to solver                
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Radius(m_Ref.GetArc(), m_Radius); }
private:            
    float m_Radius;         
};          
            
class AddRadius_Circle : public Constraint_Template_OneItem         
{           
public:         
    AddRadius_Circle(const Circle* circle, float radius)                    : Constraint_Template_OneItem(circle), m_Radius(radius) {}
    // Adds constraint to solver                
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Radius(m_Ref.GetCircle(), m_Radius); }
private:
    float m_Radius;
};


// Angle

class Angle_LineToLine : public Constraint_Template_TwoItems
{
public:
    Angle_LineToLine(const Line* line1, const Line* line2, double angle)    : Constraint_Template_TwoItems(line1, line2), m_Angle(angle) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Angle(m_Ref_1.GetLine(), m_Ref_2.GetLine(), m_Angle); }
private:
    double m_Angle;
};

// Vertical
       
class Vertical : public Constraint_Template_OneItem     
{       
public:     
    Vertical(const Line* line)                                              : Constraint_Template_OneItem(line) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Vertical(m_Ref.GetLine()); }
};

// Horizontal

class Horizontal : public Constraint_Template_OneItem     
{       
public:     
    Horizontal(const Line* line)                                            : Constraint_Template_OneItem(line) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Horizontal(m_Ref.GetLine()); }
};


// Parallel

class Parallel : public Constraint_Template_TwoItems
{
public:
    Parallel(const Line* line1, const Line* line2)                          : Constraint_Template_TwoItems(line1, line2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Parallel(m_Ref_1.GetLine(), m_Ref_2.GetLine()); }
};

// Perpendicular

class Perpendicular : public Constraint_Template_TwoItems
{
public:
    Perpendicular(const Line* line1, const Line* line2)                     : Constraint_Template_TwoItems(line1, line2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Perpendicular(m_Ref_1.GetLine(), m_Ref_2.GetLine()); }
};


// Tangent

class Tangent_Arc_Line : public Constraint_Template_TwoItems
{
public:
    Tangent_Arc_Line(const Arc* arc, const Line* line)                      : Constraint_Template_TwoItems(arc, line) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Tangent(m_Ref_1.GetArc(), m_Ref_2.GetLine()); }
};

class Tangent_Arc_Arc : public Constraint_Template_TwoItems
{
public:
    Tangent_Arc_Arc(const Arc* arc1, const Arc* arc2)                       : Constraint_Template_TwoItems(arc1, arc2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_Tangent(m_Ref_1.GetArc(), m_Ref_2.GetArc()); }
};
// TODO: Check  circle + arc  &  circle + circle


// Equal Length

class EqualLength : public Constraint_Template_TwoItems
{
public:
    EqualLength(const Line* line1, const Line* line2)                       : Constraint_Template_TwoItems(line1, line2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_EqualLength(m_Ref_1.GetLine(), m_Ref_2.GetLine()); }
};


// Equal Radius

class EqualRadius_Arc_Arc : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Arc_Arc(const Arc* arc1, const Arc* arc2)                   : Constraint_Template_TwoItems(arc1, arc2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_EqualRadius(m_Ref_1.GetArc(), m_Ref_2.GetArc()); }
};

class EqualRadius_Arc_Circle : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Arc_Circle(const Arc* arc,const Circle* circle)             : Constraint_Template_TwoItems(arc, circle) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_EqualRadius(m_Ref_1.GetArc(), m_Ref_2.GetCircle()); }
};

class EqualRadius_Circle_Circle : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Circle_Circle(const Circle* circle1, const Circle* circle2) : Constraint_Template_TwoItems(circle1, circle2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::Constraints& constraints) override             { m_SolverConstraint = constraints.Add_EqualRadius(m_Ref_1.GetCircle(), m_Ref_2.GetCircle()); }
};



 
} // end namespace Sketch

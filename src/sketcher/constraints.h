#pragma once

#include <vector>

#include "sketch_common.h"

#include "deps/constraintsolver/solver.h"


namespace Sketch {

// TODO: make sure a sketch item is the correct element on construction

// forward declare
class ElementFactory;
//typedef int ElementID;
//class SketchItem;



class Constraint
{
public:
    
    // Constructor
    Constraint();
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Constraint() = default;
    
    ConstraintID ID() const { return m_ID; }
    
    // Adds constraint to solver
    virtual void AddToSolver(Solver::ConstraintSolver& solver) = 0;
    
    // Passes each element to Callback Function
    virtual void ForEachElement(std::function<void(SketchItem&)> cb) = 0;
    
    void ClearSolverData();
                            
    bool Failed() const;
    void SetFailed();
    Solver::Constraint SolverConstraint() const;
        
protected:
    ConstraintID m_ID;
    bool m_IsFailed = false;
    Solver::Constraint m_SolverConstraint = 0;
    
    // Passes each of these elements to Callback Function
   // void ForTheseElements(std::function<void(SketchItem&)> cb, std::vector<SketchItem&> elements);
private:
    void ResetFailed();
    friend class ElementFactory;
};


// Templates

class Constraint_Template_OneItem : public Constraint
{
public:
    Constraint_Template_OneItem(ElementFactory* parent, SketchItem item) : m_Parent(parent), m_Ref(item) {}
    // Passes each element to Callback Function
    void ForEachElement(std::function<void(SketchItem&)> cb)                          { cb(m_Ref); }
protected:
    ElementFactory* m_Parent;
    SketchItem m_Ref;
};


class Constraint_Template_TwoItems : public Constraint
{
public:
    Constraint_Template_TwoItems(ElementFactory* parent, SketchItem item1, SketchItem item2) : m_Parent(parent), m_Ref_1(item1), m_Ref_2(item2) {}
    // Passes each element to Callback Function
    void ForEachElement(std::function<void(SketchItem&)> cb)                          { cb(m_Ref_1); cb(m_Ref_2); }
protected:
    ElementFactory* m_Parent;
    SketchItem m_Ref_1;
    SketchItem m_Ref_2;
};

// Coincident: Point To Point

class Coincident_PointToPoint : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToPoint(ElementFactory* parent, SketchItem p0, SketchItem p1)         : Constraint_Template_TwoItems(parent, p0, p1) {}
    // Adds constraint to solver    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};
  
    
  
  
// Coincident: Point To Line

class Coincident_PointToLine : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToLine(ElementFactory* parent, SketchItem point, SketchItem line)         : Constraint_Template_TwoItems(parent, point, line) {}
    // Adds constraint to solver    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};
 
// Coincident: Point To Arc

class Coincident_PointToArc : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToArc(ElementFactory* parent, SketchItem point, SketchItem arc)            : Constraint_Template_TwoItems(parent, point, arc) {}
    // Adds constraint to solver        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};  
    
// Coincident: Point To Circle  
    
class Coincident_PointToCircle : public Constraint_Template_TwoItems    
{   
public: 
    Coincident_PointToCircle(ElementFactory* parent, SketchItem point, SketchItem circle)   : Constraint_Template_TwoItems(parent, point, circle) {}
    // Adds constraint to solver        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};



// Distance Point to Point

class Distance_PointToPoint : public Constraint_Template_TwoItems
{
public:                                                                                                                             
    Distance_PointToPoint(ElementFactory* parent, SketchItem p0, SketchItem p1, double distance)       : Constraint_Template_TwoItems(parent, p0, p1), m_Distance(distance) {}
    Distance_PointToPoint(ElementFactory* parent, SketchItem line, double distance)                           : Constraint_Template_TwoItems(parent, { SketchItem::Type::Line_P0, line.element }, { SketchItem::Type::Line_P1, line.element }), m_Distance(distance) {}
    // Adds constraint to solver        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
private:
    double m_Distance;
};



// Distance Point to Line

class Distance_PointToLine : public Constraint_Template_TwoItems
{
public:
    Distance_PointToLine(ElementFactory* parent, SketchItem point, SketchItem line, double distance)       : Constraint_Template_TwoItems(parent, point, line), m_Distance(distance) {}
    // Adds constraint to solver        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
private:
    double m_Distance;
};


// Radius
    
class AddRadius_Arc : public Constraint_Template_OneItem
{
public:
    AddRadius_Arc(ElementFactory* parent, SketchItem arc, double radius)                             : Constraint_Template_OneItem(parent, arc), m_Radius(radius) {}
    // Adds constraint to solver                
    void AddToSolver(Solver::ConstraintSolver& solver) override;
private:            
    double m_Radius;         
};          
            
class AddRadius_Circle : public Constraint_Template_OneItem         
{           
public:         
    AddRadius_Circle(ElementFactory* parent, SketchItem circle, double radius)                    : Constraint_Template_OneItem(parent, circle), m_Radius(radius) {}
    // Adds constraint to solver                
    void AddToSolver(Solver::ConstraintSolver& solver) override;
private:
    double m_Radius;
};


// Angle

class Angle_LineToLine : public Constraint_Template_TwoItems
{
public:
    Angle_LineToLine(ElementFactory* parent, SketchItem line1, SketchItem line2, double angle)    : Constraint_Template_TwoItems(parent, line1, line2), m_Angle(angle) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
private:
    double m_Angle;
};

// Vertical
       
class Vertical : public Constraint_Template_OneItem     
{       
public:     
    Vertical(ElementFactory* parent, SketchItem line)                                              : Constraint_Template_OneItem(parent, line) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

// Horizontal

class Horizontal : public Constraint_Template_OneItem     
{       
public:     
    Horizontal(ElementFactory* parent, SketchItem line)                                            : Constraint_Template_OneItem(parent, line) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};


// Parallel

class Parallel : public Constraint_Template_TwoItems
{
public:
    Parallel(ElementFactory* parent, SketchItem line1, SketchItem line2)                          : Constraint_Template_TwoItems(parent, line1, line2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

// Perpendicular

class Perpendicular : public Constraint_Template_TwoItems
{
public:
    Perpendicular(ElementFactory* parent, SketchItem line1, SketchItem line2)                     : Constraint_Template_TwoItems(parent, line1, line2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};


// Tangent

class Tangent_Arc_Line : public Constraint_Template_TwoItems
{
public:
    Tangent_Arc_Line(ElementFactory* parent, SketchItem arc, SketchItem line)                      : Constraint_Template_TwoItems(parent, arc, line) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

class Tangent_Arc_Arc : public Constraint_Template_TwoItems
{
public:
    Tangent_Arc_Arc(ElementFactory* parent, SketchItem arc1, SketchItem arc2)                       : Constraint_Template_TwoItems(parent, arc1, arc2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};
// TODO: Check  circle + arc  &  circle + circle


// Equal Length

class EqualLength : public Constraint_Template_TwoItems
{
public:
    EqualLength(ElementFactory* parent, SketchItem line1, SketchItem line2)                       : Constraint_Template_TwoItems(parent, line1, line2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};


// Equal Radius

class EqualRadius_Arc_Arc : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Arc_Arc(ElementFactory* parent, SketchItem arc1, SketchItem arc2)                   : Constraint_Template_TwoItems(parent, arc1, arc2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

class EqualRadius_Arc_Circle : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Arc_Circle(ElementFactory* parent, SketchItem arc,SketchItem circle)             : Constraint_Template_TwoItems(parent, arc, circle) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

class EqualRadius_Circle_Circle : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Circle_Circle(ElementFactory* parent, SketchItem circle1, SketchItem circle2) : Constraint_Template_TwoItems(parent, circle1, circle2) {}
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};



 
} // end namespace Sketch

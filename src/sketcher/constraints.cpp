
#include <initializer_list>
#include "constraints.h"

#include "elementfactory.h"

namespace Sketch {

// Global counter for elements
static ConstraintID m_ConstraintCounter = 0; 


// Constructor
Constraint::Constraint() : m_ID(++m_ConstraintCounter) { 
    std::cout << "Adding Constraint: " << m_ID << std::endl;
}
    
    
// Constraint

void Constraint::ClearSolverData() { 
    m_SolverConstraint = 0; 
}
                        
bool Constraint::Failed() const { 
    return m_IsFailed; 
}
void Constraint::SetFailed() { 
    m_IsFailed = true; 
}
Solver::Constraint Constraint::SolverConstraint() const { 
    return m_SolverConstraint; 
}
/*
// Passes each of these elements to Callback Function
void Constraint::ForTheseElements(std::function<void(SketchItem&)> cb, std::vector<SketchItem&> elements) {
    for(auto element : elements) { 
        cb(element); 
    }
}
*/
void Constraint::ResetFailed() { 
    m_IsFailed = false; 
}


void Coincident_PointToPoint::AddToSolver(Solver::ConstraintSolver& solver)     { m_SolverConstraint = solver.Add_Coincident_PointToPoint(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverPoint(m_Ref_2)); }
void Coincident_PointToLine::AddToSolver(Solver::ConstraintSolver& solver)      { m_SolverConstraint = solver.Add_Coincident_PointToLine(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }
void Coincident_PointToArc::AddToSolver(Solver::ConstraintSolver& solver)       { m_SolverConstraint = solver.Add_Coincident_PointToArc(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverArc(m_Ref_2)); }
void Coincident_PointToCircle::AddToSolver(Solver::ConstraintSolver& solver)    { m_SolverConstraint = solver.Add_Coincident_PointToCircle(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverCircle(m_Ref_2)); }
void Distance_PointToPoint::AddToSolver(Solver::ConstraintSolver& solver)       { m_SolverConstraint = solver.Add_Distance_PointToPoint(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverPoint(m_Ref_2), distance); }
void Distance_PointToLine::AddToSolver(Solver::ConstraintSolver& solver)        { m_SolverConstraint = solver.Add_Distance_PointToLine(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2), distance); }
void AddMidPoint_PointToLine::AddToSolver(Solver::ConstraintSolver& solver)     { m_SolverConstraint = solver.Add_MidPoint(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }
void AddRadius_Circle::AddToSolver(Solver::ConstraintSolver& solver)            { m_SolverConstraint = solver.Add_Radius(m_Parent->GetSolverCircle(m_Ref), radius); }
void AddRadius_Arc::AddToSolver(Solver::ConstraintSolver& solver)               { m_SolverConstraint = solver.Add_Radius(m_Parent->GetSolverArc(m_Ref), radius); }
void Angle_LineToLine::AddToSolver(Solver::ConstraintSolver& solver)            { m_SolverConstraint = solver.Add_Angle(m_Parent->GetSolverLine(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2), angle); }
void Vertical::AddToSolver(Solver::ConstraintSolver& solver)                    { m_SolverConstraint = solver.Add_Vertical(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverPoint(m_Ref_2)); }
void Horizontal::AddToSolver(Solver::ConstraintSolver& solver)                  { m_SolverConstraint = solver.Add_Horizontal(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverPoint(m_Ref_2)); }
void Parallel::AddToSolver(Solver::ConstraintSolver& solver)                    { m_SolverConstraint = solver.Add_Parallel(m_Parent->GetSolverLine(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }        
void Perpendicular::AddToSolver(Solver::ConstraintSolver& solver)               { m_SolverConstraint = solver.Add_Perpendicular(m_Parent->GetSolverLine(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }          
void Tangent_Arc_Line::AddToSolver(Solver::ConstraintSolver& solver)            { 
    const Solver::Arc& arc = m_Parent->GetSolverArc(m_Ref_1);
    // flip tangent point if one or the other
    bool flipTangentPoint = (arc.isCW ^ tangentPoint);
    m_SolverConstraint = solver.Add_Tangent(arc, m_Parent->GetSolverLine(m_Ref_2), flipTangentPoint); 
}
void Tangent_Arc_Arc::AddToSolver(Solver::ConstraintSolver& solver)             { m_SolverConstraint = solver.Add_Tangent(m_Parent->GetSolverArc(m_Ref_1), m_Parent->GetSolverArc(m_Ref_2)); }
void EqualLength::AddToSolver(Solver::ConstraintSolver& solver)                 { m_SolverConstraint = solver.Add_EqualLength(m_Parent->GetSolverLine(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }         
void EqualRadius_Arc_Arc::AddToSolver(Solver::ConstraintSolver& solver)         { m_SolverConstraint = solver.Add_EqualRadius(m_Parent->GetSolverArc(m_Ref_1), m_Parent->GetSolverArc(m_Ref_2)); }          
void EqualRadius_Arc_Circle::AddToSolver(Solver::ConstraintSolver& solver)      { m_SolverConstraint = solver.Add_EqualRadius(m_Parent->GetSolverArc(m_Ref_1), m_Parent->GetSolverCircle(m_Ref_2)); }
void EqualRadius_Circle_Circle::AddToSolver(Solver::ConstraintSolver& solver)   { m_SolverConstraint = solver.Add_EqualRadius(m_Parent->GetSolverCircle(m_Ref_1), m_Parent->GetSolverCircle(m_Ref_2)); }

 
} // end namespace Sketch

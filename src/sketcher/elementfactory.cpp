
#include "elementfactory.h"

namespace Sketch {

void ElementFactory::PrintElements() {
    for(auto& element : m_Elements) {
        element->Print();
    }
}
    

    
Sketch::Point* ElementFactory::AddPoint(const Vec2& p) {
    return AddElement<Sketch::Point>(p);
}

Sketch::Line* ElementFactory::AddLine(const Vec2& p0, const Vec2& p1) {
    return AddElement<Sketch::Line>(p0, p1);
}

Sketch::Arc* ElementFactory::AddArc(const Vec2& p0, const Vec2& p1, const Vec2& pC) {
    return AddElement<Sketch::Arc>(p0, p1, pC);
}

Sketch::Circle* ElementFactory::AddCircle(const Vec2& pC, float radius) {
    return AddElement<Sketch::Circle>(pC, radius);
}
    
    
    

// Runs solver with current constraints, 
// On success, Element positions are updated
// On failure, failed Elements are flagged
// Returns: success
bool ElementFactory::UpdateSolver(Sketch::Point* draggedPoint, const Vec2& draggedPosition, uint maxItems) 
{
    Solver::Constraints constraints(maxItems);

    // Clear the Solver Data from Elements / Constraints
    ResetSolverElements();
    ResetSolverConstraints();
    // Set failed constraint flags to false 
    ResetSolverFailedConstraints();
        
    // Add Elements
    for(auto& element : m_Elements) {
        element->AddToSolver(constraints);
    }
    // Add Constraints
    for(auto& constraint : m_Constraints) {
        constraint->AddToSolver(constraints);
    }
    
    // Fix dragged point to the dragged position
    SetDraggedPoint(constraints, draggedPoint, draggedPosition);
    
    // Solve
    Solver::SolverResult result = constraints.Solve();
    
    // Success
    if(result.success == Solver::Success::Okay)
    {
        for(auto& element : m_Elements) {
            element->UseSolverValues(constraints);
        }
        
    }
    // Failure
    else 
    {
        // Display failure type
        if(result.success == Solver::Success::DidntConverge)        { std::cout << "Solver Failed: Didn't Converge" << std::endl; }
        else if(result.success == Solver::Success::Inconsistent)    { std::cout << "Solver Failed: Inconsistent" << std::endl; }
        else if(result.success == Solver::Success::TooManyUnknowns) { std::cout << "Solver Failed: Too Many Unknowns" << std::endl; }
        else { assert(0 && "Unkown result success type"); }
        
        // Mark constraints which failed
        for(Solver::Constraint& failed : result.failed) {
            std::cout << "Constraint " << failed << " Failed" << std::endl;
            bool isFound = false;
            for(size_t i = 0; i < m_Constraints.Size(); i++)
            {
                if(m_Constraints[i].SolverConstraint() == failed) {
                    m_Constraints[i].SetFailed();
                    isFound = true;
                }
            }
            assert(isFound && "Couldn't find constraint in list");
            
            
            /*
            
            // Find failed in constraints list
            auto& failedConstraint = std::find_if(begin(m_Constraints), end(m_Constraints), [&failed](Sketch::Constraint& constraint){ 
                return (constraint.SolverConstraint() == failed);
            });
            assert(failedConstraint != std::end(m_Constraints) && "Couldn't find constraint in list");
            // Mark as failed
            failedConstraint->SetFailed(true);*/
        }
    }
    
    // Clear the Solver Data from Elements / Constraints
    ResetSolverElements();
    ResetSolverConstraints();
    
    // Return success
    return (result.success == Solver::Success::Okay);
}

// Resets the failed flags on constraints
void ElementFactory::ResetSolverFailedConstraints() {
    for(auto& constraint : m_Constraints) {
        constraint->ResetFailed();
    }
}


// Temporarily modifies the dragged point's solver parameters to 
// the new dragged position, and fixes it there by changing its group
void ElementFactory::SetDraggedPoint(Solver::Constraints& constraints, Sketch::Point* draggedPoint, const Vec2& draggedPosition) 
{
    Solver::Point2D* p = draggedPoint->SolverElement<Solver::Point2D>();
    // Update the parameters for the new dragged position
    constraints.ModifyParamValue(p->paramX, draggedPosition.x);
    constraints.ModifyParamValue(p->paramY, draggedPosition.y);
    // Move the point into the fixed group (this was done as a fix because SetDraggedPoint would only make it close to the correct position)
    constraints.ModifyParamGroup(p->paramX, Solver::Group::Fixed);
    constraints.ModifyParamGroup(p->paramY, Solver::Group::Fixed);
    // Set dragged point in solver (replaced with ModifyGroup as this only got close)
    //constraints.SetDraggedPoint(*p); // removed as it 
}

// Clear the Solver Data from Elements / Constraints
void ElementFactory::ResetSolverElements() {
    for(auto& element : m_Elements) {
        element->ClearSolverData();
    }
}
void ElementFactory::ResetSolverConstraints() {
    for(auto& constraint : m_Constraints) {
        constraint->ClearSolverData();
    }
}
    
    
    
} // end namespace Sketch

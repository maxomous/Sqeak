
#include <initializer_list>
#include "constraints.h"

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
void Constraint::ForTheseElements(std::function<void(PointRef&)> cb, std::vector<PointRef&> elements) {
    for(auto element : elements) { 
        cb(element); 
    }
}
*/
void Constraint::ResetFailed() { 
    m_IsFailed = false; 
}
 
 
} // end namespace Sketch

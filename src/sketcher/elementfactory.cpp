
#include "elementfactory.h"

namespace Sketch {


// Get SketchItem functions

Solver::Point2D& ElementFactory::GetPoint(SketchItem item) 
{
   //Element* element = GetElementByID<Element>(item.element);
   /* 
    if(item.type == SketchItem::Type::Point)           { return *dynamic_cast<Point*>(element)->SolverElement<Solver::Point2D>(); }
    else if(item.type == SketchItem::Type::Line_P0)    { return dynamic_cast<Line*>(element)->SolverElement<Solver::Line>()->p0; } 
    else if(item.type == SketchItem::Type::Line_P1)    { return dynamic_cast<Line*>(element)->SolverElement<Solver::Line>()->p1; } 
    else if(item.type == SketchItem::Type::Arc_P0)     { return dynamic_cast<Arc*>(element)->SolverElement<Solver::Arc>()->p0; } 
    else if(item.type == SketchItem::Type::Arc_P1)     { return dynamic_cast<Arc*>(element)->SolverElement<Solver::Arc>()->p1; } 
    else if(item.type == SketchItem::Type::Arc_PC)     { return dynamic_cast<Arc*>(element)->SolverElement<Solver::Arc>()->pC; } 
    else if(item.type == SketchItem::Type::Circle_PC)  { return dynamic_cast<Circle*>(element)->SolverElement<Solver::Circle>()->pC; } 
*/

    if(item.type == SketchItem::Type::Point)           { return *GetElementByID<Sketch::Point>(item.element)->SolverElement<Solver::Point2D>(); }
    else if(item.type == SketchItem::Type::Line_P0)    { return GetElementByID<Sketch::Line>(item.element)->SolverElement<Solver::Line>()->p0; } 
    else if(item.type == SketchItem::Type::Line_P1)    { return GetElementByID<Sketch::Line>(item.element)->SolverElement<Solver::Line>()->p1; } 
    else if(item.type == SketchItem::Type::Arc_P0)     { return GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>()->p0; } 
    else if(item.type == SketchItem::Type::Arc_P1)     { return GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>()->p1; } 
    else if(item.type == SketchItem::Type::Arc_PC)     { return GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>()->pC; } 
    else if(item.type == SketchItem::Type::Circle_PC)  { return GetElementByID<Sketch::Circle>(item.element)->SolverElement<Solver::Circle>()->pC; } 
    
    else if(item.type == SketchItem::Type::Line)    { return GetElementByID<Sketch::Line>(item.element)->SolverElement<Solver::Line>()->p0; } 
    else if(item.type == SketchItem::Type::Arc)     { return GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>()->pC; } 
    else if(item.type == SketchItem::Type::Circle)  { return GetElementByID<Sketch::Circle>(item.element)->SolverElement<Solver::Circle>()->pC; } 


    // Should never reach
    assert(0 && "Type unknown");
}

Solver::Line& ElementFactory::GetLine(SketchItem item)
{    
    if(item.type == SketchItem::Type::Line)      { return *GetElementByID<Sketch::Line>(item.element)->SolverElement<Solver::Line>(); }  
    // Should never reach
    assert(0 && "Type is not a Line");
}

Solver::Arc& ElementFactory::GetArc(SketchItem item)
{
    if(item.type == SketchItem::Type::Arc)       { return *GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>(); } 
    // Should never reach
    assert(0 && "Type is not a Arc");
}

Solver::Circle& ElementFactory::GetCircle(SketchItem item) 
{
    if(item.type == SketchItem::Type::Circle)    { return *GetElementByID<Sketch::Circle>(item.element)->SolverElement<Solver::Circle>(); } 
    // Should never reach
    assert(0 && "Type is not a Circle");
}

  
  
    
Sketch::ElementID ElementFactory::AddPoint(const Vec2& p) {
    return AddElement<Sketch::Point>(p)->ID();
}

Sketch::ElementID ElementFactory::AddLine(const Vec2& p0, const Vec2& p1) {
    return AddElement<Sketch::Line>(p0, p1)->ID();
}

Sketch::ElementID ElementFactory::AddArc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction) {
    return AddElement<Sketch::Arc>(p0, p1, pC, direction)->ID();
}

Sketch::ElementID ElementFactory::AddCircle(const Vec2& pC, double radius) {
    return AddElement<Sketch::Circle>(pC, radius)->ID();
}




// Runs solver with current constraints, 
// On success, Element positions are updated
// On failure, failed Elements are flagged
// Returns: success
bool ElementFactory::UpdateSolver(std::optional<Vec2> pDif)
{
        
    Solver::ConstraintSolver solver;

    // Clear the Solver Data from Elements / Constraints
    ResetSolverElements();
    ResetSolverConstraints();
    // Set failed constraint flags to false 
    ResetSolverFailedConstraints();
        
    // Add Elements
    for(auto& element : m_Elements) {
        element->AddToSolver(solver);
    }
    
    if(pDif) 
    {        
        ForEachItemPoint([&](Item_Point& item) 
        {
            if(!item.IsSelected()) { return; }
            // Fix dragged point(s) to their new dragged position            
            SetDraggedPoint(solver, item, *pDif);   
            
        });
        /* 
        ForEachItemElement([&](Item_Element& item) {
            if(item.IsSelected()) {
                draggedItems.push_back(item.Reference());
            }
        });
        */
    }
    // Add Constraints
    for(auto& constraint : m_Constraints) { 
        constraint->AddToSolver(solver);            
//      // if all of a constraint's items are selected, dont add the constraint to the solver
//      bool allItemsSelected = true;
//      constraint->ForEachElement([&](SketchItem& item) {
//          // item found which isnt in selection
//          if(!GetItemBySketchItem(item).IsSelected()) { 
//              allItemsSelected = false; 
//          }
//      });
//      if(!allItemsSelected) {
//          constraint->AddToSolver(solver);            
//      }
    }
    
    // Solve
    Solver::SolverResult result = solver.Solve();
    
    // Success
    if(result.success == Solver::Success::Okay)
    {
        for(auto& element : m_Elements) {
            element->UseSolverValues(solver);
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
            
            //assert(isFound && "Couldn't find constraint in list");
            if(!isFound) { std::cout << "Couldn't find constraint in list" << std::endl; }
            
            
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
void ElementFactory::SetDraggedPoint(Solver::ConstraintSolver& solver, Item_Point& draggedPoint, const Vec2& pDif) 
{   
    
    Solver::Point2D& p = GetPoint(draggedPoint.Reference());  
    
    Vec2 draggedPosition = draggedPoint.p + pDif;
    // Update the parameters for the new dragged position
    solver.ModifyParamValue(p.paramX, draggedPosition.x);
    solver.ModifyParamValue(p.paramY, draggedPosition.y);
    // Move the point into the fixed group (this was done as a fix because SetDraggedPoint would only make it close to the correct position)
    solver.ModifyParamGroup(p.paramX, Solver::Group::Fixed);
    solver.ModifyParamGroup(p.paramY, Solver::Group::Fixed);


    // Set point to be dragged in solver
//    solver.SetDraggedPoint(p);
//    // Add Dragged Constraint
//    solver.Add_Dragged(p); 
    
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

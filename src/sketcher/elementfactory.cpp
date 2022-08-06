
#include "elementfactory.h"

namespace Sketch {


// Get SketchItem functions

Solver::Point2D& ElementFactory::GetSolverPoint(SketchItem item) 
{

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

Solver::Line& ElementFactory::GetSolverLine(SketchItem item)
{    
    if(item.type == SketchItem::Type::Line)      { return *GetElementByID<Sketch::Line>(item.element)->SolverElement<Solver::Line>(); }  
    // Should never reach
    assert(0 && "Type is not a Line");
}

Solver::Arc& ElementFactory::GetSolverArc(SketchItem item)
{
    if(item.type == SketchItem::Type::Arc)       { return *GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>(); } 
    // Should never reach
    assert(0 && "Type is not a Arc");
}

Solver::Circle& ElementFactory::GetSolverCircle(SketchItem item) 
{
    if(item.type == SketchItem::Type::Circle)    { return *GetElementByID<Sketch::Circle>(item.element)->SolverElement<Solver::Circle>(); } 
    // Should never reach
    assert(0 && "Type is not a Circle");
}


Slvs_hEntity ElementFactory::GetSolverEntity(SketchItem item) 
{

    if(item.type == SketchItem::Type::Point)            { return GetElementByID<Sketch::Point>(item.element)->SolverElement<Solver::Point2D>()->entity; }
    else if(item.type == SketchItem::Type::Line_P0)     { return GetElementByID<Sketch::Line>(item.element)->SolverElement<Solver::Line>()->p0.entity; } 
    else if(item.type == SketchItem::Type::Line_P1)     { return GetElementByID<Sketch::Line>(item.element)->SolverElement<Solver::Line>()->p1.entity; } 
    else if(item.type == SketchItem::Type::Arc_P0)      { return GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>()->p0.entity; } 
    else if(item.type == SketchItem::Type::Arc_P1)      { return GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>()->p1.entity; } 
    else if(item.type == SketchItem::Type::Arc_PC)      { return GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>()->pC.entity; } 
    else if(item.type == SketchItem::Type::Circle_PC)   { return GetElementByID<Sketch::Circle>(item.element)->SolverElement<Solver::Circle>()->pC.entity; } 
    
    else if(item.type == SketchItem::Type::Line)        { return GetElementByID<Sketch::Line>(item.element)->SolverElement<Solver::Line>()->entity; }  
    else if(item.type == SketchItem::Type::Arc)         { return GetElementByID<Sketch::Arc>(item.element)->SolverElement<Solver::Arc>()->entity; } 
    else if(item.type == SketchItem::Type::Circle)      { return GetElementByID<Sketch::Circle>(item.element)->SolverElement<Solver::Circle>()->entity; } 
    // Should never reach
    assert(0 && "SketchType is not recognised");
}

  
  
    
SketchItem ElementFactory::AddPoint(const Vec2& p) {
    return AddElement<Sketch::Point>(p)->Item_Elem().Reference();
}

SketchItem ElementFactory::AddLine(const Vec2& p0, const Vec2& p1) {
    return AddElement<Sketch::Line>(p0, p1)->Item_Elem().Reference();
}

SketchItem ElementFactory::AddArc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction) {
    return AddElement<Sketch::Arc>(p0, p1, pC, direction)->Item_Elem().Reference();
}

SketchItem ElementFactory::AddCircle(const Vec2& pC, double radius) {
    return AddElement<Sketch::Circle>(pC, radius)->Item_Elem().Reference();
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
    
    // Mark those which should be fixed
    
    if(pDif) 
    {        
        // for any elements selected, drag all of it's points
        ForEachItemPoint([&](Item_Point& item) 
        {  // overwrite position of selected item and set it to be fixed
            SetDraggedPoint(solver, item, *pDif);   
        }, [&](Sketch::Element* element) { return element->Item_Elem().IsSelected(); }); // if condition callback (element is selected) is met
        
        // for any points selected, drag all of them
        ForEachItemPoint([&](Item_Point& item) 
        {   // overwrite position of selected item and set it to be fixed
            if(item.IsSelected()) { SetDraggedPoint(solver, item, *pDif); }
        });
    }
    
    // is group free???
    ForEachItemPoint([&](Item_Point& item) 
    {   
        std::cout << "item of element: " << item.Reference().element << " is of type: " << item.Reference().Name() 
        << " of solver entity: " << GetSolverEntity(item.Reference())
        << " is " << solver.IsEntityGroupFree(GetSolverEntity(item.Reference())) << std::endl;
    });
    
    
    // Add Constraints
    for(auto& constraint : m_Constraints) { 
        
    // NOT WORKING
      // Dont add constraint if all items are fixed
        bool hasFreeItem = false;
        // Go through each item in constraint
        constraint->ForEachItem([&](SketchItem& item) {
            // check if item is free
            hasFreeItem |= solver.IsEntityGroupFree(GetSolverEntity(item));
           // hasFreeItem |= (all of the points in element);
           // for each item point in item...

        });
        // Add to solver
        if(hasFreeItem) {
          constraint->AddToSolver(solver);            
        } else {
            std::cout << "Constraint is fixed, ignoring: " << constraint->ID() << std::endl;
        }
    }
       
       /*
       
    if 2 items share a point,   ... not quite
       
       
    Constraint
        -> element(s);
            ->point(s);
        -> point(s);
       
       */
       
       
       
       
       
  /*  
    
    OLD: 
    
        // Returns true if entity's group is free
        auto IsGroupFree = [&](Slvs_hEntity entity) {
            // ensure valid id
            if(entity < 1 || (size_t)entity > m_Capacity_Constraint) { return false; }
            // return if group is free
            return sys.entity[entity - 1].group == (Slvs_hGroup)Group::Free;
        };
        // If all of the constriant's items are fixed, fix the constraint also
        for(size_t i = 0; i < (size_t)sys.constraints; i++)
        {
            bool hasItemsFree = false;
            
            hasItemsFree |= IsGroupFree(sys.constraint[i].ptA);
            hasItemsFree |= IsGroupFree(sys.constraint[i].ptB);
            hasItemsFree |= IsGroupFree(sys.constraint[i].entityA);
            hasItemsFree |= IsGroupFree(sys.constraint[i].entityB);
            hasItemsFree |= IsGroupFree(sys.constraint[i].entityC);
            hasItemsFree |= IsGroupFree(sys.constraint[i].entityD);
            
            // fix constraint too
            if(!hasItemsFree) {
                ModifyConstraintGroup((Slvs_hConstraint)(i + 1), Group::Fixed);
            }
        }
        */
        
    
    
    
    
    
    
    
    
    
    
    // Solve
    Solver::SolverResult result = solver.Solve();
    
    // Success
    if(result.success == Solver::Success::Okay)
    {
        std::cout << "Solver Succeeded" << std::endl;
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
        // update failed elements
        UpdateFailedElements();
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
    // clear the failed elements
    ClearFailedElements();
}


// Temporarily modifies the dragged point's solver parameters to 
// the new dragged position, and fixes it there by changing its group
void ElementFactory::SetDraggedPoint(Solver::ConstraintSolver& solver, Item_Point& draggedPoint, const Vec2& pDif) 
{   
    
    Solver::Point2D& p = GetSolverPoint(draggedPoint.Reference());  
    
    Vec2 draggedPosition = draggedPoint.p + pDif;
    
    std::cout << "Dragging point: " << p.entity << " from " << draggedPoint.p << "   pdif = " << pDif << std::endl;
    // Update the parameters for the new dragged position
    solver.ModifyParamValue(p.paramX, draggedPosition.x);
    solver.ModifyParamValue(p.paramY, draggedPosition.y);
    // Move the point into the fixed group (this was done as a fix because SetDraggedPoint would only make it close to the correct position)
    solver.ModifyEntityGroup(p.entity, Solver::Group::Fixed);
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

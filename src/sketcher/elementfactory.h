#pragma once

#include <MaxLib.h>

#include "elements.h"
#include "constraints.h"


namespace Sketch {


class ElementFactory
{
public:
    // TODO: Test these ptrs * with references & instead
    void ForEachElement(std::function<void(const Sketch::Element*)> cb_Element) 
    {
        // Call callback for each element
        for(size_t i = 0; i < m_Elements.Size(); i++) {
            cb_Element(m_Elements.CastItem<Sketch::Element>(i));         
        }
    }
    void ForEachConstraint(std::function<void(const Sketch::Constraint*)> cb_Constraint) 
    {
        // Call callback for each constraint
        for(size_t i = 0; i < m_Constraints.Size(); i++) {
            cb_Constraint(m_Constraints.CastItem<Sketch::Constraint>(i));       
        }
    }
    
    void PrintElements();
    
    Point*  AddPoint(const Vec2& p) ;
    Line*   AddLine(const Vec2& p0, const Vec2& p1) ;
    Arc*    AddArc(const Vec2& p0, const Vec2& p1, const Vec2& pC) ;
    Circle* AddCircle(const Vec2& pC, float radius);
  
         // Usage:
    //    AddConstraint<Coincident_PointToPoint>(p0, p1)
    // See "constraints.h"
    template<typename T, typename... Args>
    T* AddConstraint(Args&&... args)  {
        m_Constraints.Add<T>(std::forward<Args>(args)...);
        return m_Constraints.CastItem<T>(m_Constraints.Size()-1);
    }
       
    void RemoveElement(ElementID id) 
    {
        // Stores any constraints which need to be deleted
        std::vector<ConstraintID> constraintsToDelete;
        
        // For each Constraint
        for(size_t i = 0; i < m_Constraints.Size(); i++)
        {
            // For each Element inside Constraint
            m_Constraints[i].ForEachElement([&](PointRef& ref) {
                
                // Check if the element ids match
                if(ref.element->ID() == id) {
                    // Mark this Constraint to be deleted
                    constraintsToDelete.push_back(m_Constraints[i].ID());
                }
            });
        }
        // Go through all the Constraints to be deleted, and delete them
        for(ConstraintID constraintID : constraintsToDelete) {
            RemoveConstraint(constraintID);
        }
        
        // Remove the Element
        m_Elements.Remove(GetElementIndex(id));
    }
    
    void RemoveConstraint(ConstraintID id) {
        m_Constraints.Remove(GetConstraintIndex(id));
    }
        
    // Runs solver with current constraints, 
    // On success, Element positions are updated
    // On failure, failed Elements are flagged
    // Returns: success
    bool UpdateSolver(Point* draggedPoint, const Vec2& draggedPosition, uint maxItems = 1000);
    // Resets the failed flags on constraints
    void ResetSolverFailedConstraints();
    
  
private:   
 
    size_t GetElementIndex(ElementID id)  
    {
        for(size_t i = 0; i < m_Elements.Size(); i++) {
            if(m_Elements[i].ID() == id) { return i; }            
        }
        assert("Could not find element");
        return 0; // never reaches
    }
    
    size_t GetConstraintIndex(ConstraintID id)  
    {
        for(size_t i = 0; i < m_Constraints.Size(); i++) {
            if(m_Constraints[i].ID() == id) { return i; }            
        }
        assert("Could not find constraint");
        return 0; // never reaches
    }
    
    
    MaxLib::Vector::Vector_SelectablePtrs<Element> m_Elements;
    MaxLib::Vector::Vector_SelectablePtrs<Constraint> m_Constraints; // Constraints holds links to elements
    
    template <typename T, typename... Args>
    T* AddElement(Args&&... args) {
        // Add Point to Elements List
        m_Elements.Add<T>(std::forward<Args>(args)...);
        // Cast last Element item (we just added) back to point
        if(T* element = m_Elements.CastItem<T>(m_Elements.Size()-1)) {
            return element;
        }
        // Failsafe for debugging
        assert(0 && "Element could not be casted");
        return nullptr; // will never reach
    }
    // Temporarily modifies the dragged point's solver parameters to 
    // the new dragged position, and fixes it there by changing its group
    void SetDraggedPoint(Solver::Constraints& constraints, Point* draggedPoint, const Vec2& draggedPosition);
    
    // Clear the Solver Data from Elements / Constraints
    void ResetSolverElements();
    void ResetSolverConstraints();
    
    
    
    
};

} // End namespace Sketch

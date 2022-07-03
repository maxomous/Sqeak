#pragma once

#include "sketch_common.h"

#include "elements.h"
#include "constraints.h"


namespace Sketch {

using namespace MaxLib::Vector;
using namespace MaxLib::Geom;

class ElementFactory
{
public:
    // TODO: Test these ptrs * with references & instead
    void ForEachElement(std::function<void(Sketch::Element*)> cb_Element) 
    {
        // Call callback for each element
        for(size_t i = 0; i < m_Elements.Size(); i++) {
            // Cast so we can get the ptr
            cb_Element(m_Elements.CastItem<Sketch::Element>(i)); 
        }
    }
    void ForEachConstraint(std::function<void(Sketch::Constraint*)> cb_Constraint) 
    {
        // Call callback for each constraint
        for(size_t i = 0; i < m_Constraints.Size(); i++) {
            // Cast so we can get the ptr
            cb_Constraint(m_Constraints.CastItem<Sketch::Constraint>(i));       
        }
    }
    
    

    void ForEachItemElement(std::function<void(Item_Element&)> cb) {
        
        ForEachElement([&](Sketch::Element* element) {
            cb(element->Item_Elem());
        });
    }

    void ForEachItemPoint(std::function<void(Item_Point&)> cb) {
        
        ForEachElement([&](Sketch::Element* element) {
            if(auto* point = dynamic_cast<Sketch::Point*>(element)) { 
                point->ForEachItemPoint(cb); 
            } else if(auto* line = dynamic_cast<Sketch::Line*>(element)) { 
                line->ForEachItemPoint(cb);
            } else if(auto* arc = dynamic_cast<Sketch::Arc*>(element)) { 
                arc->ForEachItemPoint(cb);
            } else if(auto* circle = dynamic_cast<Sketch::Circle*>(element)) { 
                circle->ForEachItemPoint(cb); 
            } else { // Should never reach
                assert(0 && "Could not cast element, type unknown");                
            }
        });
    }

    void ForEachItem(std::function<void(Item&)> cb) 
    {
        ForEachItemElement(cb);
        ForEachItemPoint(cb);
    }

    


    Item& GetItemBySketchItem(SketchItem item) {
        
        if(item.type == SketchItem::Type::Point)            { return GetElementByID<Sketch::Point>(item.element)->Item_Elem(); }
        else if(item.type == SketchItem::Type::Line)        { return GetElementByID<Sketch::Line>(item.element)->Item_Elem(); }
        else if(item.type == SketchItem::Type::Arc)         { return GetElementByID<Sketch::Arc>(item.element)->Item_Elem(); }
        else if(item.type == SketchItem::Type::Circle)      { return GetElementByID<Sketch::Circle>(item.element)->Item_Elem(); }
        else if(item.type == SketchItem::Type::Line_P0)     { return GetElementByID<Sketch::Line>(item.element)->Item_P0(); }
        else if(item.type == SketchItem::Type::Line_P1)     { return GetElementByID<Sketch::Line>(item.element)->Item_P1(); }
        else if(item.type == SketchItem::Type::Arc_P0)      { return GetElementByID<Sketch::Arc>(item.element)->Item_P0(); }
        else if(item.type == SketchItem::Type::Arc_P1)      { return GetElementByID<Sketch::Arc>(item.element)->Item_P1(); }
        else if(item.type == SketchItem::Type::Arc_PC)      { return GetElementByID<Sketch::Arc>(item.element)->Item_PC(); }
        else if(item.type == SketchItem::Type::Circle_PC)   { return GetElementByID<Sketch::Circle>(item.element)->Item_PC(); }
        else { assert(0 && "Could not cast element, type unknown"); }// Should never reach 
    }

       
    

    void ClearHovered() 
    {
        ForEachItem([](Item& item) {
            item.SetHovered(false);
        });
    }

    void ClearSelection() 
    {
        ForEachItem([](Item& item) {
            item.SetSelected(false);
        });
    }
  
    
    // Finds points within a tolerance to position p
    // Result a list of points and their distance to p 
    void AddSelectionByPosition(Vec2 p, double tolerance)
    {
        // Check each point on each element to see whether it falls within tolerance
        ForEachItemPoint([&](Sketch::Item_Point& item) {
            // Adds point to pointsFound if point falls within tolerance
            double distance = Hypot(item.p - p);
            if(distance <= tolerance) { 
                item.SetSelected(true);
            }
        });
       //  // Check each point on each element to see whether it falls within tolerance
       // ForEachItemElement([&](const Sketch::Item_Element& item) {
       //     // Adds point to pointsFound if point falls within tolerance
       //     LiesOnElement = Geos::Intersect_Point_Element
       //     double distance = Hypot(centroid - p);
       //     if(distance <= tolerance) { 
       //         item.SetSelected(true);
       //     }
       // });
    }
    
    
/*  
    // Finds points within a tolerance to position p
    // Result a list of points and their distance to p 
    std::vector<std::pair<SketchItem, double>> GetItemsByPosition(Vec2 p, double tolerance)
    {
        std::vector<std::pair<SketchItem, double>> pointsFound; // and distance from p
        
        // Check each point on each element to see whether it falls within tolerance
        ForEachItemPoint([&](const Sketch::Item_Point& item) {
            // Adds point to pointsFound if point falls within tolerance
            double distance = Hypot(item.P() - p);
            if(distance <= tolerance) { 
                pointsFound.push_back(std::make_pair(item.Reference(), distance)); 
            }
        }
        
        // Sort by distance
        std::sort(pointsFound.begin(), pointsFound.end(), [](auto &a, auto &b) {
            return a.second < b.second;
        });
        
        
        return std::move(pointsFound);
    }
*/
     //   // Adds point to pointsFound if point falls within tolerance
     //   auto CheckPosition = [&](const Vec2& pos, SketchItem ref) {
     //       double distance = Hypot(pos - p);
     //       if(distance <= tolerance) { 
     //           pointsFound.push_back(std::make_pair(ref, distance)); 
     //       }
     //   };
     //   
     //   // Check each point on each element to see whether it falls within tolerance
     //   ForEachElement([&](const Sketch::Element* element) {
     //                   
     //       if(auto* point = dynamic_cast<const Sketch::Point*>(element)) {
     //           CheckPosition(point->P(), point->Ref_P());
     //       }   
     //       else if(auto* line = dynamic_cast<const Sketch::Line*>(element)) {
     //           CheckPosition(line->P0(), line->Ref_P0());
     //           CheckPosition(line->P1(), line->Ref_P1());
     //       }
     //       else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element)) {
     //           CheckPosition(arc->P0(), arc->Ref_P0());
     //           CheckPosition(arc->P1(), arc->Ref_P1());
     //           CheckPosition(arc->PC(), arc->Ref_PC());
     //       }
     //       else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element)) {
     //           CheckPosition(circle->PC(), circle->Ref_PC());
     //       }
     //       else { // Should never reach
     //           assert(0 && "Cannot render element, type unknown");                
     //       }
     //   });
        
    
    
    
    
    /*
    
    std::vector<SketchItem> GetSelection() {
        
        std::vector<SketchItem> itemsSelected;
        
        // Check each point on each element to see whether it falls within tolerance
        ForEachItemPoint([&](const Sketch::Item_Point& item) {
            if(item.IsSelected()) {
                itemsSelected.push_back(item.Reference()));                
            }
        }        
        
        return std::move(itemsSelected);
    }*/
    
    
    
    
   
    void PrintElements();
    
  
  
    
    const Solver::Point2D&  GetPoint(SketchItem item);
    const Solver::Line&     GetLine(SketchItem item);
    const Solver::Arc&      GetArc(SketchItem item); 
    const Solver::Circle&   GetCircle(SketchItem item);

    
    
    ElementID AddPoint(const Vec2& p) ;
    ElementID AddLine(const Vec2& p0, const Vec2& p1) ;
    ElementID AddArc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction);
    ElementID AddCircle(const Vec2& pC, double radius);
  
         // Usage:
    //    AddConstraint<Coincident_PointToPoint>(p0, p1)
    // See "constraints.h"
    template<typename T, typename... Args>
    T* AddConstraint(Args&&... args)  {
        m_Constraints.Add<T>(this, std::forward<Args>(args)...);
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
            m_Constraints[i].ForEachElement([&](SketchItem& ref) {
                // Check if the element ids match
                if(ref.element == id) {                    
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
        m_Elements.Remove(GetElementIndexByID(id));
    }
    
    void RemoveConstraint(ConstraintID id) {
        m_Constraints.Remove(GetConstraintIndexByID(id));
    }
        
    // Runs solver with current constraints, 
    // On success, Element positions are updated
    // On failure, failed Elements are flagged
    // if dragPoint is set, solver will fix this point to dragPosition
    // Returns: success
    bool UpdateSolver(std::optional<Vec2> dragPosition = {});
  
private:    

    
    Vector_SelectablePtrs<Element> m_Elements;
    Vector_SelectablePtrs<Constraint> m_Constraints; // Constraints holds links to elements
    
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
    void SetDraggedPoint(Solver::ConstraintSolver& solver, SketchItem draggedPoint, const Vec2& draggedPosition);
    
    // Clear the Solver Data from Elements / Constraints
    void ResetSolverElements();
    void ResetSolverConstraints();
    // Resets the failed flags on constraints
    void ResetSolverFailedConstraints();
        
        
    // Access functinos
    template<typename T>
    T* GetElementByID(ElementID id) 
    {
        size_t index = GetElementIndexByID(id);
        return m_Elements.CastItem<T>(index);
    }
    size_t GetElementIndexByID(ElementID id)  
    {
        for(size_t i = 0; i < m_Elements.Size(); i++) {
            if(m_Elements[i].ID() == id) { return i; }            
        }
        assert("Could not find element");
        return 0; // never reaches
    }
    size_t GetConstraintIndexByID(ConstraintID id)  
    {
        for(size_t i = 0; i < m_Constraints.Size(); i++) {
            if(m_Constraints[i].ID() == id) { return i; }            
        }
        assert("Could not find constraint");
        return 0; // never reaches
    }
    
};

} // End namespace Sketch

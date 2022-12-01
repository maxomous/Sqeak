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

    ElementFactory() {
        // Create the Origin point
        // This point is like a normal point but cannot be modified
        m_Origin = AddPoint({ 0.0, 0.0 });
    }

    const int ARC_SEGMENTS = 30;

    // tolerance to make selection
    double selectionTolerance = 10.0;
    
    
    void SetSelected(SketchItem item, bool value) {
        // clear selection
        ClearSelected();
        // Find item
        GetItemPointBySketchItem(item).SetSelected(value);
    }
    
    // TODO: Test these ptrs * with references & instead
    void ForEachElement(std::function<void(Sketch::Element*)> cb_Element, bool isOriginIncluded = false) 
    {
        // Call callback for each element
        size_t startIndex = (isOriginIncluded) ? 0 : 1;
        for(size_t i = startIndex; i < m_Elements.Size(); i++) {
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
    
           
    void ForEachItemElement(std::function<void(Item_Element&)> cb, bool isOriginIncluded = false) {
        
        ForEachElement([&](Sketch::Element* element) {
            cb(element->Item_Elem());
        }, isOriginIncluded);
    }

    void ForEachItemPoint(std::function<void(Item_Point&)> cb, bool isOriginIncluded = false, std::function<bool(Sketch::Element*)> cb_ElementCondition = [](Sketch::Element* element) { (void)element; return true; }) {
        
        ForEachElement([&](Sketch::Element* element) {
            // check condition callback
            if(!cb_ElementCondition(element)) { return; }
            // find type of element and call cb for each point
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
                    
        }, isOriginIncluded);
    }

    void ForEachItem(std::function<void(Item&)> cb, bool isOriginIncluded = false) 
    {
        ForEachItemElement(cb, isOriginIncluded);
        ForEachItemPoint(cb, isOriginIncluded);
    }

    
    
    Vec2 GetPositionBySketchItem(SketchItem item) {
        // elements
        if(item.type == SketchItem::Type::Point)            { return GetElementByID<Sketch::Point>(item.element)->P(); }
        else if(item.type == SketchItem::Type::Line)        { 
            Sketch::Line* line = GetElementByID<Sketch::Line>(item.element);
            return (line->P0() + line->P1()) / 2.0;
        }
        else if(item.type == SketchItem::Type::Arc)         { return GetElementByID<Sketch::Arc>(item.element)->PC(); }
        else if(item.type == SketchItem::Type::Circle)      { return GetElementByID<Sketch::Circle>(item.element)->PC(); }
        // points
        else if(item.type == SketchItem::Type::Line_P0)     { return GetElementByID<Sketch::Line>(item.element)->P0(); }
        else if(item.type == SketchItem::Type::Line_P1)     { return GetElementByID<Sketch::Line>(item.element)->P1(); }
        else if(item.type == SketchItem::Type::Arc_P0)      { return GetElementByID<Sketch::Arc>(item.element)->P0(); }
        else if(item.type == SketchItem::Type::Arc_P1)      { return GetElementByID<Sketch::Arc>(item.element)->P1(); }
        else if(item.type == SketchItem::Type::Arc_PC)      { return GetElementByID<Sketch::Arc>(item.element)->PC(); }
        else if(item.type == SketchItem::Type::Circle_PC)   { return GetElementByID<Sketch::Circle>(item.element)->PC(); }
        else { assert(0 && "SketchItem Type is not an element point"); }// Should never reach 
    }
    // returns radius of a circle, will only accept a circle
    double GetRadiusOfCircleBySketchItem(SketchItem item) 
    {
        if(item.type == SketchItem::Type::Circle)           { return GetElementByID<Sketch::Circle>(item.element)->Radius(); }
        else { assert(0 && "SketchItem Type is not a Circle"); }// Should never reach 
    }
    // returns direction of an arc, will only accept an arc
    Direction GetDirectionOfArcBySketchItem(SketchItem item) 
    {
        if(item.type == SketchItem::Type::Arc)              { return GetElementByID<Sketch::Arc>(item.element)->Direction(); }
        else { assert(0 && "SketchItem Type is not an Arc"); }// Should never reach 
    }
        
    Item& GetItemBySketchItem(SketchItem item) 
    {
        if(item.type == SketchItem::Type::Point)            { return GetElementByID<Sketch::Point>(item.element)->Item_Elem(); }
        else if(item.type == SketchItem::Type::Line)        { return GetElementByID<Sketch::Line>(item.element)->Item_Elem(); }
        else if(item.type == SketchItem::Type::Arc)         { return GetElementByID<Sketch::Arc>(item.element)->Item_Elem(); }
        else if(item.type == SketchItem::Type::Circle)      { return GetElementByID<Sketch::Circle>(item.element)->Item_Elem(); }
        else                                                { return GetItemPointBySketchItem(item); }
    }
    
    Item_Point& GetItemPointBySketchItem(SketchItem item) 
    {
        if(item.type == SketchItem::Type::Point)            { return GetElementByID<Sketch::Point>(item.element)->Item_P(); }
        else if(item.type == SketchItem::Type::Line_P0)     { return GetElementByID<Sketch::Line>(item.element)->Item_P0(); }
        else if(item.type == SketchItem::Type::Line_P1)     { return GetElementByID<Sketch::Line>(item.element)->Item_P1(); }
        else if(item.type == SketchItem::Type::Arc_P0)      { return GetElementByID<Sketch::Arc>(item.element)->Item_P0(); }
        else if(item.type == SketchItem::Type::Arc_P1)      { return GetElementByID<Sketch::Arc>(item.element)->Item_P1(); }
        else if(item.type == SketchItem::Type::Arc_PC)      { return GetElementByID<Sketch::Arc>(item.element)->Item_PC(); }
        else if(item.type == SketchItem::Type::Circle_PC)   { return GetElementByID<Sketch::Circle>(item.element)->Item_PC(); }
        else { assert(0 && "SketchItem Type is not an element point"); }// Should never reach 
    }

       
        
 //   
 //   selecting 1st point     closest (prioritises closest point, if none, then it will select the first element within tolerence)
 //       
 //   
 //   shift click 2nd point   closest (dont clear selection)
 //       
 //   
 //   drag right              any fully inside 
 //       create a bounding box of mouse drag
 //       create a bounding box around element's linestring
 //       check for inside
 //       
 //   drag left               any partially inside 
 //       "
 //       check for intersect
    
    
    
    
    void ClearFailedElements() 
    {
        ForEachItem([](Item& item) {
            item.SetFailed(false);
        }, true); // include origin point
    }
    // Finds points within a tolerance to position p and sets their selected flag to true
    void UpdateFailedElements() 
    {
        ForEachConstraint([&](Sketch::Constraint* constraint) {
            // if the constraint has failed
            if(constraint->Failed()) {
                // set all of its elements failed flags true also
                constraint->ForEachItem([&](Sketch::SketchItem& item) {
                    GetItemBySketchItem(item).SetFailed(true);
                });                
            }
        });    
    }  
    
    // A Point's item_element and item_point have their selected and hovered flags tied together 
    
    // Clears all hovered flags
    void ClearHovered() 
    {
        ForEachItem([](Item& item) {
            item.SetHovered(false);
        }, true); // include origin point
    }

    void ClearSelected() 
    {
        ForEachItem([](Item& item) {
            item.SetSelected(false);
        }, true); // include origin point
    }

    // Finds points within a tolerance to position p and sets their hovered flag to true
    bool SetHoveredByPosition(const Vec2& p, SelectionFilter filter) 
    { 
        return EditItemByPosition(p, selectionTolerance, [](Sketch::Item& item) {
            item.SetHovered(true);
        }, filter, true); // include origin point    
    }
    
    // Finds points within a tolerance to position p and sets their selected flag to true
    bool SetSelectedByPosition(const Vec2& p, SelectionFilter filter) 
    {
        bool hasSelection = EditItemByPosition(p, selectionTolerance, [](Sketch::Item& item) {
            item.SetSelected(!item.IsSelected());
        }, filter, true); // include origin point
        return hasSelection;
    }  
    
    
    
    bool AddHoveredBetween(const Vec2& p0, const Vec2& p1, SelectionFilter filter, bool includePartiallyInside) {
        return EditItemBetween(p0, p1, includePartiallyInside, [](Sketch::Item& item) {
            item.SetHovered(true);
        }, filter, true); // include origin point
    }
    bool AddSelectionBetween(const Vec2& p0, const Vec2& p1, SelectionFilter filter, bool includePartiallyInside) {
        return EditItemBetween(p0, p1, includePartiallyInside, [](Sketch::Item& item) {
            item.SetSelected(true);
        }, filter, true); // include origin point
    }
    
        
    // Deletes all items in selection
    void DeleteSelection() 
    {
        // make a list of all the elements to delete
        std::vector<ElementID> elementsToDelete;
        
        ForEachItemElement([&](Item_Element& item) {
            // if the item is selected, delete it
            if(item.IsSelected()) {
                elementsToDelete.push_back(item.Reference().element);
            }
        });
        // Remove each element
        for(ElementID element : elementsToDelete) {
            RemoveElement(element);            
        }
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
    
  
  
    
    Solver::Point2D&    GetSolverPoint(SketchItem item);
    Solver::Line&       GetSolverLine(SketchItem item);
    Solver::Arc&        GetSolverArc(SketchItem item); 
    Solver::Circle&     GetSolverCircle(SketchItem item);
    Slvs_hEntity        GetSolverEntity(SketchItem item);
 
    
    
    SketchItem AddPoint(const Vec2& p);
    SketchItem AddLine(const Vec2& p0, const Vec2& p1);
    SketchItem AddArc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction);
    SketchItem AddCircle(const Vec2& pC, double radius);
  
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
        assert(id != m_Origin.element && "Cannot remove origin");
        // Stores any constraints which need to be deleted
        std::vector<ConstraintID> constraintsToDelete;
        
        // For each Constraint 
        for(size_t i = 0; i < m_Constraints.Size(); i++)
        {
            // For each Element inside Constraint
            m_Constraints[i].ForEachItem([&](SketchItem& ref) {
                // Check if the element ids match
                if(ref.element == id) {                    
                    // Mark this Constraint to be deleted
                    constraintsToDelete.push_back(m_Constraints[i].ID());
                }
            });
        }
        // sort and remove duplicates
        std::sort(constraintsToDelete.begin(), constraintsToDelete.end());
        constraintsToDelete.erase(std::unique(constraintsToDelete.begin(), constraintsToDelete.end()), constraintsToDelete.end());
        
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
    bool UpdateSolver(std::optional<Vec2> pDif = {});
    
    SketchItem Origin()       { return m_Origin; }
    Point& OriginElement()    { return *GetElementByID<Sketch::Point>(m_Origin.element); }
        
private:    
    
    Vector_SelectablePtrs<Element> m_Elements;
    Vector_SelectablePtrs<Constraint> m_Constraints; // Constraints holds links to elements
    
    SketchItem m_Origin;
    
    template <typename T, typename... Args>
    T* AddElement(Args&&... args) {
        // Add Point to Elements List
        m_Elements.Add<T>(std::forward<Args>(args)...);
        // Cast last Element item (we just added) back to Element
        if(T* element = m_Elements.CastItem<T>(m_Elements.Size()-1)) {
            return element;
        }
        // Failsafe for debugging
        assert(0 && "Element could not be casted");
        return nullptr; // will never reach
    }
    
    
    void ModifySolverPoint(Solver::ConstraintSolver& solver, Solver::Point2D& p, const Vec2& position);
    void ModifySolverPoint(Solver::ConstraintSolver& solver, Solver::Point2D& p, Solver::Group group);

    // Temporarily modifies the dragged point's solver parameters to 
    // the new dragged position, and fixes it there by changing its group
    void SetDraggedPoint(Solver::ConstraintSolver& solver, Item_Point& draggedPoint, const Vec2& pDif);
     
     
    // Finds points within a tolerance to position p
    bool EditItemBetween(const Vec2& p0, const Vec2& p1, bool includePartiallyInside, std::function<void(Sketch::Item&)> cb, SelectionFilter filter, bool isOriginIncluded = false)
    {        
        // Check each element to see whether it falls within tolerance
        bool isElementFound = false;
        
        Geos geos;
        // Make bounding box (minimum in bottom left, max in top right)
        Vec2 boundary_p0 = { std::min(p0.x, p1.x), std::min(p0.y, p1.y) };
        Vec2 boundary_p1 = { std::max(p0.x, p1.x), std::max(p0.y, p1.y) };
        LineString boundingBox = { boundary_p0, { boundary_p0.x, boundary_p1.y }, boundary_p1, { boundary_p1.x, boundary_p0.y }, boundary_p0 };
        
        // Only check if points filter is enabled
        if(filter & SelectionFilter::Points) {
            // Check each point on each element to see whether it falls within tolerance
            ForEachItemPoint([&](Sketch::Item_Point& item) {
                // Adds point to selected if point falls within bounding box
                if(boundary_p0 <= item.p && item.p <= boundary_p1) {
                    cb(item);
                    // Points are a specical case where we also set its element flag
                    if(item.Type() == SketchItem::Type::Point) { 
                        cb(GetElementByID<Point>(item.Reference().element)->Item_Elem());
                    }
                    isElementFound |= true;    
                }
            }, isOriginIncluded);
        }
        // Check each element to see whether it falls within tolerance
        ForEachElement([&](Sketch::Element* element) {
                                     
            LineString l;
            if(auto* point = dynamic_cast<const Sketch::Point*>(element))           { (void)point; return; } // do nothing, handled above
            else if(auto* line = dynamic_cast<const Sketch::Line*>(element))        { if(!(filter & SelectionFilter::Lines)) { return; }   l = RenderLine(line->P0(), line->P1()); }
            else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element))          { if(!(filter & SelectionFilter::Arcs)) { return; }    l = RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction(), ARC_SEGMENTS); }
            else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element))    { if(!(filter & SelectionFilter::Circles)) { return; } l = RenderCircle(circle->PC(), circle->Radius(), ARC_SEGMENTS); }
            else { assert(0 && "Cannot render element, type unknown"); }            // Should never reach
            
            assert(!l.empty() && "Linestring is empty");
             
            // return value
            bool success = false;
            // if dragging box to left, include anything intersecting with it
            if(includePartiallyInside) {
                // circles require overlap | contains,  arc / line require intersects, points are not included
                bool isCircle = l.front() == l.back();
                if(isCircle) { 
                    // done like this instead of intersects() becuase when dragging box inside circle, you dont want to select the circle
                    if(std::optional<bool> s = geos.Overlaps(boundingBox, l)) { success |= *s; }
                    if(std::optional<bool> s = geos.Contains(boundingBox, l)) { success |= *s; }
                } 
                else {
                    if(std::optional<bool> s = geos.Intersects(boundingBox, l)) { success |= *s; }
                }
            } 
            // if dragging box to right, include only anything contained within
            else {
                if(std::optional<bool> s = geos.Contains(boundingBox, l)) { success |= *s; }
            }
            
            // call callback function if item is inside bounding box
            if(success) {
                cb(element->Item_Elem());
                isElementFound |= true;
            }
        }, isOriginIncluded); 
        
        // return whether element was found or not
        return isElementFound;
    }

    // Finds points within a tolerance to position p and calls callback function, passing the item as a parameter
    // Points are prioritised over elements
    // Callback on a Point element will be ignored and handled within the ItemPoint instead to prevent testing position twice
    // Returns success
    bool EditItemByPosition(const Vec2& p, double tolerance, std::function<void(Sketch::Item&)> cb, SelectionFilter filter, bool isOriginIncluded = false)
    {     
        Sketch::SketchItem closestItem;
        double closestDistance = tolerance;
        
        // find points
        if(filter & SelectionFilter::Points) {
            // Check each point on each element to see whether it falls within tolerance
            ForEachItemPoint([&](Sketch::Item_Point& item) {
                // Adds point to pointsFound if point falls within tolerance
                double distance = Hypot(item.p - p);
                if(distance < closestDistance) {
                    closestItem = item.Reference();
                    closestDistance = distance;
                }
            }, isOriginIncluded); // include origin
            
            // prioritise point if one is within tolerance and set closest to selected
            if(closestItem.type != Sketch::SketchItem::Type::Unset) {
                // add point to selection
                cb(GetItemBySketchItem(closestItem));
                // A point is a special case where we also set its element
                if(closestItem.type == Sketch::SketchItem::Type::Point) {
                    cb(GetElementByID<Point>(closestItem.element)->Item_P());
                }
                return true;
            } 
        }
        
        // Check each element to see whether it falls within tolerance
        bool isElementFound = false;
        // draw a tolerence ring around point, to check if intersect
        LineString p_with_tol = RenderCircle(p, tolerance, ARC_SEGMENTS);
         
        ForEachElement([&](Sketch::Element* element) {
            // The first element found is the one which is set to selected
            // So skip until end if element was already found
            if(isElementFound) { return; } 
            
            LineString l;
            if(auto* point = dynamic_cast<const Sketch::Point*>(element))           { (void)point; return; } // do nothing, this is handled above
            else if(auto* line = dynamic_cast<const Sketch::Line*>(element))        { if(!(filter & SelectionFilter::Lines)) { return; }   l = RenderLine(line->P0(), line->P1()); }
            else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element))          { if(!(filter & SelectionFilter::Arcs)) { return; }    l = RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction(), ARC_SEGMENTS); }
            else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element))    { if(!(filter & SelectionFilter::Circles)) { return; } l = RenderCircle(circle->PC(), circle->Radius(), ARC_SEGMENTS); }
            else { assert(0 && "Cannot render element, type unknown"); }            // Should never reach
            
            assert(!l.empty() && "Linestring is empty");
            // Check if point and element intersect 
            Geos geos;
            // circles require overlap, arc + line require intersects 
            auto success = (l.front() == l.back()) ? geos.Overlaps(p_with_tol, l) : geos.Intersects(p_with_tol, l);
            if(success) {
                if(*success) {
                    cb(element->Item_Elem());
                    isElementFound = true;
                }
            }    
        }, isOriginIncluded); 
        return isElementFound;
    }
    
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
        assert(0 && "Could not find element");
        return 0; // never reaches
    }
    size_t GetConstraintIndexByID(ConstraintID id)  
    {
        for(size_t i = 0; i < m_Constraints.Size(); i++) {
            if(m_Constraints[i].ID() == id) { return i; }            
        }
        assert(0 && "Could not find constraint");
        return 0; // never reaches
    }
    
    friend class SketchRenderer;
};

} // End namespace Sketch

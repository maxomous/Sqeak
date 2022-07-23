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

    // segments per 90 degrees of arc
    int arcSegments = 32;
    
    // Render element to linestring
    LineString RenderLine(const Vec2& p0, const Vec2& p1)
    {
        return std::move(LineString({ p0, p1 }));
    }

    LineString RenderArc(const Vec2& pC, double radius, Direction direction, double th_Start, double th_End)
    {
        LineString linestring;
        // Clean up angles
        CleanAngles(th_Start, th_End, direction);
        // Calculate increment from n segments in 90 degrees
        double th_Incr = direction * (M_PI / 2.0) / arcSegments;
        // Calculate number of incrments for loop
        int nIncrements = floorf(fabsf((th_End - th_Start) / th_Incr));
        
        // from 'n == 1' because we have already calculated the first angle
        // to 'n == nIncremenets' to ensure last point is added
        for (int n = 0; n <= nIncrements; n++) {
            
            double th = (n == nIncrements) ? th_End : th_Start + n * th_Incr;
            // Calculate position from radius and angle
            Vec2 p = pC + Vec2(fabsf(radius) * sin(th), fabsf(radius) * cos(th));       
            
            // This prevents double inclution of point 
            if(!linestring.empty()) { 
                if(p == linestring.back()) { continue; }
            }
            
            //Add Line to output
            linestring.emplace_back(move(p));
        }
        return std::move(linestring);
    }
    LineString RenderArc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction)
    {
        // get start and end points relative to the centre point
        Vec2 v_Start    = p0 - pC;
        Vec2 v_End      = p1 - pC;
        // get start and end angles
        double th_Start = atan2(v_Start.x, v_Start.y);
        double th_End   = atan2(v_End.x, v_End.y);
        double radius   = hypot(v_End.x, v_End.y);
        // draw arc between angles
        return std::move(RenderArc(pC, radius, direction, th_Start, th_End));
    }

    LineString RenderCircle(const Vec2& pC, double radius) {
        // draw arc between angles
        LineString circle = RenderArc(pC, radius, Direction::CW, 0.0, 2.0 * M_PI);
        // make sure the first and last points match as 2PI produces a rounding error
        if(!circle.empty()) { circle.back() = circle.front(); }
        return std::move(circle);
    }

    Vec2 AdjustCentrePoint(const Vec2& p0, const Vec2& p1, const Vec2& pC) {
        // Get distance between line and point
        double d = DistanceBetween(p0, p1, pC);
        
        double th = Polar(p1 - p0).th;
        double thPerpendicular = CleanAngle(th + M_PI_2);
        Polar newCentre = Polar(d, thPerpendicular);
        
        Vec2 pMid = (p0 + p1) / 2.0;
        int flipSide = LeftOfLine(p0, p1, pC) ? -1 : 1;
        return pMid + newCentre.Cartesian() * flipSide;
    }
    

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
    
    
    
    
    
    // This should be updated whenever selected is modified
    void UpdateSelectionList() 
    {
        // Clear old data
        m_SelectedPoints.clear();
        m_SelectedElements.clear();
        // find all the selected points
        ForEachItemPoint([&](Item_Point& item) {
            if(item.IsSelected()) { m_SelectedPoints.push_back(item.Reference()); }
        });
        // find all the selected elements
        ForEachItemElement([&](Item_Element& item) {
            // ignore points as elements (they are handled as points)
            if(item.Type() == SketchItem::Type::Point) { return; }
            if(item.IsSelected()) { m_SelectedElements.push_back(item.Reference()); }
        });
    }
    
    const std::vector<SketchItem>& GetSelectedPoints()  { return m_SelectedPoints; }
    const std::vector<SketchItem>& GetSelectedElements() { return m_SelectedElements; }
    
    // Make an array of the selected points / elements
    std::vector<SketchItem> m_SelectedPoints;
    std::vector<SketchItem> m_SelectedElements;
    
    
    
    void ClearFailedElements() 
    {
        ForEachItem([](Item& item) {
            item.SetFailed(false);
        });
    }
    // Finds points within a tolerance to position p and sets their selected flag to true
    void UpdateFailedElements() 
    {
        ForEachConstraint([&](Sketch::Constraint* constraint) {
            // if the constraint has failed
            if(constraint->Failed()) {
                // set all of its elements failed flags true also
                constraint->ForEachElement([&](Sketch::SketchItem& item) {
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
        });
    }

    // Finds points within a tolerance to position p and sets their hovered flag to true
    bool SetHoveredByPosition(const Vec2& p, double tolerance) 
    {
        return EditItemByPosition(p, tolerance, [](Sketch::Item& item) {
            item.SetHovered(true);
        });        
    }
    
    void ClearSelected() 
    {
        ForEachItem([](Item& item) {
            item.SetSelected(false);
        });
        UpdateSelectionList();
    }

    // Finds points within a tolerance to position p and sets their selected flag to true
    bool SetSelectedByPosition(const Vec2& p, double tolerance) 
    {
        bool hasSelection = EditItemByPosition(p, tolerance, [](Sketch::Item& item) {
            item.SetSelected(true);
        });
        UpdateSelectionList();
        return hasSelection;
    }  
    
    
    // Finds points within a tolerance to position p
    void AddSelectionBetween(const Vec2& p0, const Vec2& p1, bool includePartiallyInside)
    {        
        Geos geos;
        // Make bounding box (minimum in bottom left, max in top right)
        Vec2 boundary_p0 = { std::min(p0.x, p1.x), std::min(p0.y, p1.y) };
        Vec2 boundary_p1 = { std::max(p0.x, p1.x), std::max(p0.y, p1.y) };
        LineString boundingBox = { boundary_p0, { boundary_p0.x, boundary_p1.y }, boundary_p1, { boundary_p1.x, boundary_p0.y }, boundary_p0 };
                
        // Check each point on each element to see whether it falls within tolerance
        ForEachItemPoint([&](Sketch::Item_Point& item) {
            // Adds point to selected if point falls within bounding box
            if(boundary_p0 <= item.p && item.p <= boundary_p1) {
                item.SetSelected(true);
                // Points are a specical case where we also set its element flag
                if(item.Type() == SketchItem::Type::Point) { 
                    GetElementByID<Point>(item.Reference().element)->Item_Elem().SetSelected(true);
                }
            }
        });
        // Check each element to see whether it falls within tolerance
        ForEachElement([&](Sketch::Element* element) {
                                    
            LineString l;
            if(auto* point = dynamic_cast<const Sketch::Point*>(element))           { (void)point; return; } // do nothing, handled above
            else if(auto* line = dynamic_cast<const Sketch::Line*>(element))        { l = RenderLine(line->P0(), line->P1()); }
            else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element))          { l = RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction()); }
            else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element))    { l = RenderCircle(circle->PC(), circle->Radius()); }
            else { assert(0 && "Cannot render element, type unknown"); }            // Should never reach
            
            assert(!l.empty() && "Linestring is empty");
            
            // Return values      
            if(auto success = (includePartiallyInside) ? geos.Intersect(boundingBox, l) : geos.Contains(boundingBox, l)) {
                if(*success) {
                    element->Item_Elem().SetSelected(true);
                }
            } 
            
        }); 
        UpdateSelectionList();
    }
    // Deletes all items in selcetion
    void DeleteSelection() 
    {
        // make a list of all the elements to delete
        std::vector<ElementID> elementsToDelete;
        
        ForEachItemElement([&](Item_Element& item) {
            if(item.IsSelected()) {
                elementsToDelete.push_back(item.Reference().element);
            }
        });
        // Remove each element
        for(ElementID element : elementsToDelete) {
            RemoveElement(element);            
        }
        UpdateSelectionList();
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
    
  
  
    
    Solver::Point2D&  GetPoint(SketchItem item);
    Solver::Line&     GetLine(SketchItem item);
    Solver::Arc&      GetArc(SketchItem item); 
    Solver::Circle&   GetCircle(SketchItem item);
 
    
    
    ElementID AddPoint(const Vec2& p);
    ElementID AddLine(const Vec2& p0, const Vec2& p1);
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
    void SetDraggedPoint(Solver::ConstraintSolver& solver, Item_Point& draggedPoint, const Vec2& pDif);
     
    
    // Finds points within a tolerance to position p and calls callback function, passing the item as a parameter
    // Points are prioritised over elements
    // Callback on a Point element will be ignored and handled within the ItemPoint instead to prevent testing position twice
    // Returns success
    bool EditItemByPosition(const Vec2& p, double tolerance, std::function<void(Sketch::Item&)> cb)
    {     
        Sketch::SketchItem closestItem;
        double closestDistance = tolerance;
        
        // Check each point on each element to see whether it falls within tolerance
        ForEachItemPoint([&](Sketch::Item_Point& item) {
            // Adds point to pointsFound if point falls within tolerance
            double distance = Hypot(item.p - p);
            if(distance < closestDistance) {
                closestItem = item.Reference();
                closestDistance = distance;
            }
        });
        
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
        
        // Check each element to see whether it falls within tolerance
        bool isElementFound = false;
        // draw a tolerence ring around point, to check if intersect
        LineString p_with_tol = RenderCircle(p, tolerance);
         
        ForEachElement([&](Sketch::Element* element) {
            // The first element found is the one which is set to selected
            // So skip until end if element was already found
            if(isElementFound) { return; } 
            
            LineString l;
            if(auto* point = dynamic_cast<const Sketch::Point*>(element))           { (void)point; return; } // do nothing, this is handled above
            else if(auto* line = dynamic_cast<const Sketch::Line*>(element))        { l = RenderLine(line->P0(), line->P1()); }
            else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element))          { l = RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction()); }
            else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element))    { l = RenderCircle(circle->PC(), circle->Radius()); }
            else { assert(0 && "Cannot render element, type unknown"); }            // Should never reach
            
            assert(!l.empty() && "Linestring is empty");
            // Check if point and element intersect 
            Geos geos;
            if(auto success = geos.Intersect(p_with_tol, l)) {
                if(*success) {
                    cb(element->Item_Elem());
                    isElementFound = true;
                }
            }    
        }); 
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
    
};

} // End namespace Sketch

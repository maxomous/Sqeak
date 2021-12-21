#pragma once
#include "../common.h" 



/* UNSURE ABOUT
    - Destructors should delete everything inside (linestring deletes elements, which deletes points) 
*/

// forward declaration
class RawPoint;
class A_DrawingElement;


enum CompensateCutter {
    None, Left, Right, Pocket
};

typedef size_t RawPointID;
typedef size_t ElementID;
typedef size_t LineLoopID;

// link between 
struct Ref_PointToElement {
public:
    Ref_PointToElement() {}
    void SetReferences(RawPoint* p, A_DrawingElement* e)    { rawPoint = p; element = e; }
    void SetReferences(RawPoint* p)                         { rawPoint = p; }
    void SetReferences(A_DrawingElement* e)                 { element = e; }
    RawPoint*           rawPoint;
    A_DrawingElement*   element;
   
};

// references in points are used to determine if the point is no longer needed
class RawPoint
{
public: 
    RawPoint(RawPointID id, const glm::vec2& position)
        : m_ID(id), m_Position(position) {}
    
    RawPointID ID()             { return m_ID; }
    float X()                   { return m_Position.x; }
    float Y()                   { return m_Position.y; }
    glm::vec2& Vec2()           { return m_Position; }
    
    // Adds a drawing element reference
    void AddReference(Ref_PointToElement* ref);
    // Removes a drawing element reference (returns true if no references left)
    bool RemoveReference(Ref_PointToElement* ref);
    
    bool HasReferences() { return !m_ElementRefs.empty(); }
    
    // for debugging
    bool DrawImGui();
private:
    RawPointID m_ID;
    glm::vec2 m_Position;
    //std::vector<A_DrawingElement*> m_ElementRefs;
    std::vector<Ref_PointToElement*> m_ElementRefs;
    
    
};
static inline std::ostream& operator<<(std::ostream& os, RawPoint& p) { os << "(" << p.X() << ", " << p.Y() << ")"; return os; }

// a drawing element holds references to points
class A_DrawingElement
{
public:
    ElementID           ID() { return m_ID; } 
    Ref_PointToElement* P0() { return m_Ref_P0; }
    //RawPoint*           P0() { return m_Ref_P0.rawPoint; }
    //void                SetP0(RawPoint* p) { m_P0 = p; }
    
    virtual Ref_PointToElement*   Last() = 0;
    virtual void Path(std::vector<glm::vec2>& returnPath, int arcSegments) {
        (void) arcSegments;
        returnPath.push_back(Last()->rawPoint->Vec2());
    }    
    // Adds a drawing element reference
    void AddReference_P0(Ref_PointToElement* ref) { 
        m_Ref_P0 = ref;
    }
    // Removes a rawpoint reference (returns true if no references left)
    virtual bool RemoveReference(Ref_PointToElement* ref) = 0;
    
    virtual void GetReferences(std::function<void(Ref_PointToElement*)> callback_BreakReference) = 0;
    virtual bool HasDanglingReference() = 0;
    
    virtual bool DrawImGui() = 0;
protected:
    A_DrawingElement(ElementID id) 
        : m_ID(id) {}
    
    ElementID m_ID;
    //RawPoint* m_P0;
    Ref_PointToElement* m_Ref_P0;
    
};
    
    
// holds references to element points
class Element_Point : public A_DrawingElement
{
public: 
    Element_Point(ElementID id, Ref_PointToElement* ref_p0)
        :   A_DrawingElement(id)
    { m_Ref_P0 = ref_p0; }
    
    Ref_PointToElement* Last() override  { return m_Ref_P0; };
    // Removes a rawpoint reference (returns true if no references left)
    bool RemoveReference(Ref_PointToElement* ref) override;
    
    void GetReferences(std::function<void(Ref_PointToElement*)> callback_BreakReference) { 
        callback_BreakReference(m_Ref_P0);
    }
    bool HasDanglingReference() override  { return m_Ref_P0 == nullptr; }
    
    bool DrawImGui() override;
    
private:
};
    
class Element_Line : public A_DrawingElement
{
public: 
    Element_Line(ElementID id, Ref_PointToElement* ref_p0, Ref_PointToElement* ref_p1) 
        :   A_DrawingElement(id) {
        m_Ref_P0 = ref_p0; 
        m_Ref_P1 = ref_p1;
    }
    Ref_PointToElement* P1()             { return m_Ref_P1; }
    Ref_PointToElement* Last() override  { return m_Ref_P1; };
    // Removes a rawpoint reference (returns true if no references left)
    bool RemoveReference(Ref_PointToElement* ref) override;
    
    void GetReferences(std::function<void(Ref_PointToElement*)> callback_BreakReference) { 
        callback_BreakReference(m_Ref_P0);
        callback_BreakReference(m_Ref_P1);
    }
    bool HasDanglingReference() override  { return (m_Ref_P0 == nullptr) || (m_Ref_P1 == nullptr); }
    
    bool DrawImGui() override;
private:
    Ref_PointToElement* m_Ref_P1;
};

class Element_Arc : public A_DrawingElement
{
public: 
  // define by centre point
    Element_Arc(ElementID id, Ref_PointToElement* ref_p0, Ref_PointToElement* ref_p1, int direction, Ref_PointToElement* ref_centre) 
        : A_DrawingElement(id) {
        m_Ref_P0 = ref_p0; 
        m_Ref_P1 = ref_p1;
        m_Ref_Centre = ref_centre;
        m_Direction = direction;
        RecalculateRadiusFromCentre();
    }
    
    Ref_PointToElement* P1()              { return m_Ref_P1; }
    Ref_PointToElement* Last() override   { return m_Ref_P1; };
    Ref_PointToElement* Centre()          { return m_Ref_Centre; }
    int Direction()             { return m_Direction; }
    float Radius()              { return m_Radius; }
    
    void Path(std::vector<glm::vec2>& returnPath, int arcSegments) override {
        glm::vec2& p0       = m_Ref_P0->rawPoint->Vec2();
        glm::vec2& p1       = m_Ref_P1->rawPoint->Vec2();
        glm::vec2& pCentre  = m_Ref_Centre->rawPoint->Vec2();
        // get start and end points relative to the centre point
        glm::vec2 v_Start = p0 - pCentre;
        glm::vec2 v_End = p1 - pCentre;
        
        double th_Start  = atan2(v_Start.x, v_Start.y);
        double th_End    = atan2(v_End.x, v_End.y);
        
        Geom::CleanAngles(th_Start, th_End, m_Direction);
        
        float th_Incr   = m_Direction * deg2rad(90.0f / arcSegments);
        
        int nIncrements = floorf(fabsf((th_End - th_Start) / th_Incr));
        
        glm::vec2 p;
        for (int n = 0; n < nIncrements; n++) {
            float th = th_Start + n * th_Incr;
            p = { m_Radius * sin(th), m_Radius * cos(th) };            
            returnPath.push_back(pCentre + p);
        }
        // ensure last point is added
        if(p != p1) {         
            returnPath.push_back(p1);
        }
    }
    // Removes a rawpoint reference (returns true if no references left)
    bool RemoveReference(Ref_PointToElement* ref) override;
    
    void GetReferences(std::function<void(Ref_PointToElement*)> callback_BreakReference) { 
        callback_BreakReference(m_Ref_P0);
        callback_BreakReference(m_Ref_P1);
        callback_BreakReference(m_Ref_Centre);
    }
    bool HasDanglingReference() override  { return (m_Ref_P0 == nullptr) || (m_Ref_P1 == nullptr) || (m_Ref_Centre == nullptr); }
    
    bool DrawImGui() override;
    
private:
    Ref_PointToElement* m_Ref_P1;
    Ref_PointToElement* m_Ref_Centre;
    int m_Direction;
    float m_Radius;

    float MinimumRadius()
    {
        glm::vec2& p0       = m_Ref_P0->rawPoint->Vec2();
        glm::vec2& p1       = m_Ref_P1->rawPoint->Vec2();
        glm::vec2 dif = p1 - p0;
        return hypotf(dif.x, dif.y) / 2.0f;
    }
    // recalculates radius from centre point
    void RecalculateRadiusFromCentre() 
    {
        glm::vec2& p1       = m_Ref_P1->rawPoint->Vec2();
        glm::vec2& pCentre  = m_Ref_Centre->rawPoint->Vec2();
        glm::vec2 dif = p1 - pCentre;
        m_Radius = hypotf(dif.x, dif.y);
    }   
    // recalculates centre point from radius
    void RecalculateCentreFromRadius() 
    {
        glm::vec2& p0       = m_Ref_P0->rawPoint->Vec2();
        glm::vec2& p1       = m_Ref_P1->rawPoint->Vec2();
        glm::vec2& pCentre  = m_Ref_Centre->rawPoint->Vec2();
        point2D centre = Geom::ArcCentreFromRadius(point2D(p0.x, p0.y), point2D(p1.x, p1.y), m_Radius, m_Direction);
        pCentre = { centre.x, centre.y };
    }
};




// Container of elements, controlled by the element factory
class LineLoop 
{
public: 
    
    size_t  Size()      { return m_Elements.size(); }
    bool    IsEmpty()   { return Size() == 0; }
    
    LineLoopID ID()     { return m_ID; }

    // only allow ElementFactory to produce and modify these
    LineLoop(LineLoopID id) : m_ID(id) {}
    LineLoop(LineLoopID id, ElementID pointElement) : m_ID(id) {
        SetStartPoint(pointElement);
    }
    
private:

    void SetStartPoint(ElementID elementID) 
    {
        if(!m_Elements.size()) {
            m_Elements.push_back(elementID);
        } else {
            m_Elements[0] = elementID;
        }
    }    
    void DeleteAllElements() 
    {
        m_Elements.clear();
    }
    void AddLine(ElementID elementID) {
        assert(!IsEmpty() && "Line loop requires a start point");
        m_Elements.push_back(elementID);
    }
    void AddArc(ElementID elementID) {
        assert(!IsEmpty() && "Line loop requires a start point");
        m_Elements.push_back(elementID);
    }
    
    ElementID GetElement(size_t i) {
        assert((i < Size()) && "Invalid index provided");
        return m_Elements[i];
    }

    
    LineLoopID m_ID = -1;
    std::vector<ElementID> m_Elements;
    bool m_IsClosed = false;
    
    friend class ElementFactory;
};

class ElementFactory {
public:


    bool RawPoint_DrawImGui();
    void RefPointToElement_DrawImGui();
    
    std::vector<glm::vec2> RawPoint_PointsList() {
        std::vector<glm::vec2> points;
        for (size_t i = 0; i < m_Points.Size(); i++) {
            points.push_back(m_Points[i]->Vec2());
        }
        return move(points);
    }
    std::vector<glm::vec2> LineLoop_PointsList(LineLoopID id, int arcSegments) {
        
        LineLoop& lineLoop = LineLoop_GetByID(id);
        //std::cout << std::endl;
        std::vector<glm::vec2> points;
        
        for (size_t i = 0; i < lineLoop.m_Elements.size(); i++) {
            //std::cout << i << "  :  p1 = " << m_Elements[i]->Last()->Vec2() << std::endl;
            A_DrawingElement* element = Element_GetByID(lineLoop.m_Elements[i]);
            element->Path(points, arcSegments);
        }
        
        for(size_t i = 0; i < points.size(); i++) {
            std::cout << "Line Loop Point List: " << points[i] << std::endl;
        }
        
        return points;
    }
    bool LineLoop_IsLoop (LineLoopID id) { 
        
        LineLoop& lineLoop = LineLoop_GetByID(id);
        if(lineLoop.Size() <= 2) { 
            return false; 
        }
        Ref_PointToElement* elementFirst = Element_GetByID(lineLoop.m_Elements[0])->Last();
        Ref_PointToElement* elementLast = Element_GetByID(lineLoop.m_Elements.back())->Last();
        // compare front and back points
        glm::vec2& p0       = elementFirst->rawPoint->Vec2();
        glm::vec2& p1       = elementLast->rawPoint->Vec2();
        return p0 == p1;
    }
    
private:
    RawPoint* LineLoop_LastPoint(LineLoopID& id) 
    {
        LineLoop& lineLoop = LineLoop_GetByID(id);
        assert(!lineLoop.IsEmpty() && "Line loop is empty");
        ElementID lastElementID = lineLoop.m_Elements.back();
        return Element_GetByID(lastElementID)->Last()->rawPoint;
    }
public:
    // returns true if update viewer required
    bool LineLoop_DrawImGui(LineLoopID id);

private:

    LineLoop& LineLoop_GetByID(LineLoopID id) 
    {
        for (size_t i = 0; i < m_LineLoops.size(); i++) {
            if(m_LineLoops[i]->ID() == id) {
                return *m_LineLoops[i];
            }
        }
        assert(0 && "Couldn't find lineloop ID");
        return *m_LineLoops[0]; // never reaches
    }
    
public:
    

    size_t LineLoop_Size(LineLoopID id) { return LineLoop_GetByID(id).Size(); }
    // Creates a basic Line Loop
    LineLoopID LineLoop_Create();
    // Creates a basic Line Loop
    LineLoopID LineLoop_Create(const glm::vec2& startPoint);
    // Create a lineloop of just lines from vector
    LineLoopID LineLoop_Create(const std::vector<glm::vec2>& points);
    // Set start point
    void LineLoop_AddStartPoint(LineLoopID id, const glm::vec2& startPoint);
    // Adds a line to the Line Loop
    void LineLoop_AddLine(LineLoopID id, const glm::vec2& p1);
    // Adds an arc to the Line Loop from centre point
    void LineLoop_AddArc(LineLoopID id, const glm::vec2& p1, int direction, const glm::vec2& centre);
    // Adds an arc to the Line Loop from radius
    void LineLoop_AddArc(LineLoopID id, const glm::vec2& p1, int direction, float radius);
    // Deletes Last element in Line Loop
    void LineLoop_DeleteLast(LineLoopID id);
    
/*    A Lineloop is container of elements in which the elements can share points
//      A reference of both Element and RawPoint is held inside a class inside m_References which is held within both items
// 
//    Point0     Line0     Line1  
//        \      /  \      /  \
//        ref  ref  ref  ref  ref  
//          \  /      \  /      \
//           p0        p1       p2
*/ 
    // Deletes element in Line Loop
    void LineLoop_DeleteElement(LineLoopID id, size_t i) 
    {
        LineLoop& lineLoop = LineLoop_GetByID(id);

        assert((i < lineLoop.Size()) && "Invalid index provided");
        
        // element is a point
        if(i == 0) { // replace start point ref TODO: if(i == 0) make 1 the start point and delete 0
            
                for (size_t i = 0; i < lineLoop.Size(); i++) {
                    std::cout << "0LineLoop element  = " << lineLoop.GetElement(i) << std::endl;
                    ElementID id = Element_GetByID(lineLoop.GetElement(i))->ID();
                    std::cout << "LineLoop ID = " << (int)id << std::endl;
                }
                
                
            Ref_PointToElement* ref_point0_p0   = Element_GetByID(lineLoop.m_Elements[0])->Last();
            Ref_PointToElement* ref_line0_p0  = Element_GetByID(lineLoop.m_Elements[1])->P0();
            Ref_PointToElement* ref_line0_p1  = Element_GetByID(lineLoop.m_Elements[1])->Last();
            
            // break the reference and delete the point element (+ rawPoint (p0) if size == 1
            Element_BreakReference(ref_point0_p0);
            
            // delete the old point from the line loop
            lineLoop.m_Elements.erase(lineLoop.m_Elements.begin() + 0);
            
                for (size_t i = 0; i < lineLoop.Size(); i++) {
                    std::cout << "1LineLoop element  = " << lineLoop.GetElement(i) << std::endl;
                    ElementID id = Element_GetByID(lineLoop.GetElement(i))->ID();
                    std::cout << "LineLoop ID = " << (int)id << std::endl;
                }
                
                
            // there is only a point element and a rawpoint
            if(lineLoop.Size() > 1) {
                // make a new point element which references p1
                ElementID pointElementID = Element_CreatePoint(ref_line0_p1->rawPoint);
                // break the reference and delete the line element
                Element_BreakReference(ref_line0_p0);
                Element_BreakReference(ref_line0_p1);
                // set lineloop start point
                lineLoop.SetStartPoint(pointElementID);
                
                
                for (size_t i = 0; i < lineLoop.Size(); i++) {
                    std::cout << "2LineLoop element  = " << lineLoop.GetElement(i) << std::endl;
                    ElementID id = Element_GetByID(lineLoop.GetElement(i))->ID();
                    std::cout << "LineLoop ID = " << (int)id << std::endl;
                }
                
                
            }
            
            for (size_t i = 0; i < lineLoop.Size(); i++) {
                    std::cout << "3LineLoop element  = " << lineLoop.GetElement(i) << std::endl;
               // ElementID id = Element_GetByID(lineLoop.GetElement(i))->ID();
            }
                
                
        } else {        
            // element is not last point (Modify references between i-1 & i+1 if)
            if(i < lineLoop.Size()-1) {
                Ref_PointToElement* ref_NextP0      = Element_GetByID(lineLoop.m_Elements[i+1])->P0();
                Ref_PointToElement* ref_PrevPLast   = Element_GetByID(lineLoop.m_Elements[i-1])->Last();
                Element_ReplaceRawPointReference(ref_NextP0, ref_PrevPLast->rawPoint);
            }
            // break element references
            Element_GetByID(lineLoop.m_Elements[i])->GetReferences([&](Ref_PointToElement* ref) { 
                Element_BreakReference(ref);
                Reference_Delete(ref);
            });
            // delete the old element from the line loop
            lineLoop.m_Elements.erase(lineLoop.m_Elements.begin() + i);
        }
    }
            
    // removes references from element and rawpoint within ref, deletes the element/rawpoint if they have no references left
    void Element_BreakReference(Ref_PointToElement* ref);
    // removes references from element and rawpoint within ref, deletes the element/rawpoint if they have no references left
    void Element_ReplaceRawPointReference(Ref_PointToElement* ref, RawPoint* pNew);

private:
    void Element_DeleteByID(ElementID elementID) {
        for (size_t i = 0; i < m_Elements.size(); i++) {
            if(m_Elements[i]->ID() == elementID) {
                m_Elements.erase(m_Elements.begin() + i);
                return;
            }
        }
        assert(0 && "Couldn't find element ID");
    }
    
public:
    A_DrawingElement* Element_GetByID(ElementID id) 
    {
        for (size_t i = 0; i < m_Elements.size(); i++) {
            if(m_Elements[i]->ID() == id) {
                return m_Elements[i].get();
            }
        }
        assert(0 && "Couldn't find element ID");
        return m_Elements[0].get(); // never reaches
    }
    
    // Element creation
    ElementID Element_CreatePoint(const glm::vec2& p) {
        // make a point/element reference and add to list
        auto ref = std::make_unique<Ref_PointToElement>();
        // make raw point
        RawPoint* point = RawPoint_Create(p, ref.get());
        // make point element
        std::unique_ptr<A_DrawingElement> element = std::make_unique<Element_Point>(m_ElementIDCounter++, ref.get());
        int elementID = element->ID();
        // link references
        ref->SetReferences(point, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref));
        
        return elementID;
    }
    ElementID Element_CreateLine(const glm::vec2& p0, const glm::vec2& p1) {
        // make a point/element reference and add to list
        auto ref0 = std::make_unique<Ref_PointToElement>();
        auto ref1 = std::make_unique<Ref_PointToElement>();
        // make raw points
        RawPoint* point0 = RawPoint_Create(p0, ref0.get());
        RawPoint* point1 = RawPoint_Create(p1, ref1.get());
        // make line element
        std::unique_ptr<A_DrawingElement> element = std::make_unique<Element_Line>(m_ElementIDCounter++, ref0.get(), ref1.get());
        int elementID = element->ID();
        // link references
        ref0->SetReferences(point0, element.get());
        ref1->SetReferences(point1, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref0));
        m_References.push_back(move(ref1));

        return elementID;
    }
    ElementID Element_CreateArc(const glm::vec2& p0, const glm::vec2& p1, int direction, const glm::vec2& centre) { 
        // make a point/element reference and add to list
        auto ref0 = std::make_unique<Ref_PointToElement>();
        auto ref1 = std::make_unique<Ref_PointToElement>();
        auto refCentre = std::make_unique<Ref_PointToElement>();
        // make raw points
        RawPoint* point0 = RawPoint_Create(p0, ref0.get());
        RawPoint* point1 = RawPoint_Create(p1, ref1.get());
        RawPoint* pointCentre = RawPoint_Create(centre, refCentre.get());
        // make line element
        std::unique_ptr<A_DrawingElement> element = std::make_unique<Element_Arc>(m_ElementIDCounter++, ref0.get(), ref1.get(), direction, refCentre.get());
        int elementID = element->ID();
        // link references
        ref0->SetReferences(point0, element.get());
        ref1->SetReferences(point1, element.get());
        refCentre->SetReferences(pointCentre, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref0));
        m_References.push_back(move(ref1));
        m_References.push_back(move(refCentre));
        
        return elementID;
    }
    
private:
 
               
    // Element creation
    ElementID Element_CreatePoint(RawPoint* point) {
        // make a point/element reference and add to list
        auto ref = std::make_unique<Ref_PointToElement>();
        // add reference onto point
        point->AddReference(ref.get());
        // make point element
        std::unique_ptr<A_DrawingElement> element = std::make_unique<Element_Point>(m_ElementIDCounter++, ref.get());
        int elementID = element->ID();
        // link references
        ref->SetReferences(point, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref));
        
        return elementID;
    }
    // for elements which share points (reference to previous element's end point) - used for line loop
    ElementID Element_CreateLine(RawPoint* point0, const glm::vec2& p1) {
        // make a point/element reference and add to list
        auto ref0 = std::make_unique<Ref_PointToElement>();
        auto ref1 = std::make_unique<Ref_PointToElement>();
        // add reference to p0
        point0->AddReference(ref0.get());
        // make raw point
        RawPoint* point1 = RawPoint_Create(p1, ref1.get());
        // make line element
        std::unique_ptr<A_DrawingElement> element = std::make_unique<Element_Line>(m_ElementIDCounter++, ref0.get(), ref1.get());
        int elementID = element->ID();
        // link references
        ref0->SetReferences(point0, element.get());
        ref1->SetReferences(point1, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref0));
        m_References.push_back(move(ref1));

        return elementID;
    }
        
    
    ElementID Element_CreateArc(RawPoint* point0, const glm::vec2& p1, int direction, const glm::vec2& centre) {
        // make a point/element reference and add to list
        auto ref0 = std::make_unique<Ref_PointToElement>();
        auto ref1 = std::make_unique<Ref_PointToElement>();
        auto refCentre = std::make_unique<Ref_PointToElement>();
        // add reference to p0
        point0->AddReference(ref0.get());
        // make raw point
        RawPoint* point1 = RawPoint_Create(p1, ref1.get());
        RawPoint* pointCentre = RawPoint_Create(centre, refCentre.get());
        // make line element
        std::unique_ptr<A_DrawingElement> element = std::make_unique<Element_Arc>(m_ElementIDCounter++, ref0.get(), ref1.get(), direction, refCentre.get());
        int elementID = element->ID();
        // link references
        ref0->SetReferences(point0, element.get());
        ref1->SetReferences(point1, element.get());
        refCentre->SetReferences(pointCentre, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref0));
        m_References.push_back(move(ref1));
        m_References.push_back(move(refCentre));

        return elementID;
    }
    
    RawPoint* RawPoint_Create(glm::vec2 p, Ref_PointToElement* ref) {
        std::unique_ptr<RawPoint> rawPoint = std::make_unique<RawPoint>(m_PointIDCounter++, p);
        rawPoint->AddReference(ref);
        m_Points.Add(move(rawPoint));
        return m_Points[m_Points.Size()-1].get();
    }   
    
    RawPoint* RawPoint_GetByID(RawPointID pointID) {
        for (size_t i = 0; i < m_Points.Size(); i++) {
            if(m_Points[i]->ID() == pointID) {
                return m_Points[i].get();
            }
        }
        assert(0 && "Couldn't find point ID");
        return m_Points[0].get(); // never reaches
    }
    void RawPoint_Delete(RawPoint* point) {
        for (size_t i = 0; i < m_Points.Size(); i++) {
            if(m_Points[i].get() == point) {
                m_Points.Remove(i);
                return;
            }
        }
        assert(0 && "Couldn't find point to delete");
    }
    
    void DrawingElement_Delete(A_DrawingElement* element) {
        for (size_t i = 0; i < m_Elements.size(); i++) {
            if(m_Elements[i].get() == element) {
                m_Elements.erase(m_Elements.begin() + i);
                return;
            }
        }
        assert(0 && "Couldn't find element to delete");
    }

    void Reference_Delete(Ref_PointToElement* ref) {
        for (size_t i = 0; i < m_References.size(); i++) {
            if(m_References[i].get() == ref) {
                m_References.erase(m_References.begin() + i);
                return;
            }
        }
        assert(0 && "Couldn't find reference to delete");
    }
    /*
    void RawPoint_DeleteByID(PointID pointID) {
        for (size_t i = 0; i < m_Points.Size(); i++) {
            if(m_Points[i]->ID() == pointID) {
                m_Points.Remove(i);
                return;
            }
        }
        assert(0 && "Couldn't find point ID");
    }
*/
    
    // lists of all elements, lineloops & points in drawing
    std::vector<std::unique_ptr<LineLoop>> m_LineLoops;
    std::vector<std::unique_ptr<A_DrawingElement>> m_Elements;
    VectorSelectable<std::unique_ptr<RawPoint>> m_Points;
    // references between m_Points and m_Elements
    std::vector<std::unique_ptr<Ref_PointToElement>> m_References;
        
    // id counters
    ElementID m_ElementIDCounter = 0;
    LineLoopID m_LineLoopIDCounter = 0;
    RawPointID m_PointIDCounter = 0;
};
 
 
 
 
 /* MAYBE SCRAP THIS..... INSTEAD OF HAVING 
        VectorSelectable<Point> m_Points;
    WE HAVE
        VectorSelectable<std::unique_ptr<Point>> m_Points;
        references are then to the point itself and not anything to do with the vector... *m_Points[i]
    But i thin the factory should own everything still...
    
ElementFactory new

    contains everything and passes back IDs when we create items:
    
        ElementCollection (ID / ElementRef ID)
            Lineloop    
        Elements (ID / RawPointRef ID / CollectionRef ID)
            pointElement
            lineElement
            arcElement
        RawPoints (ID / ElementRef ID)
 
 */
 
 
 
 
 
 

class Function
{
public: 
    Function(std::string name) : m_Name(name) {}
    const std::string& Name() { return m_Name; }
    
    virtual bool HandleEvents(Settings& settings, InputEvent& inputEvent, ElementFactory& elementFactory) = 0;
    // returns true if update is required
    virtual bool DrawImGui(ElementFactory& elementFactory, Settings& settings) = 0;
    
    int InterpretGCode(Settings& settings, ElementFactory& elementFactory, std::function<int(std::vector<std::string> gcode)> callback);
    
    // adds linelists to viewerLineList
    virtual void UpdateViewer(Settings& settings, ElementFactory& elementFactory, std::vector<DynamicBuffer::DynamicVertexList>* viewerLineLists, bool isDisabled) = 0;
    
protected:
    std::string m_Name;
    // error checks
    bool IsValidInputs(Settings& settings, ElementFactory& elementFactory);
    // returns the header text for the gcode output
    std::string HeaderText(Settings& settings, ElementFactory& elementFactory);
    // exports gcode from current paramaters
    virtual std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings, ElementFactory& elementFactory) = 0;
};

 

class Function_Draw : public Function
{
public: 

    Function_Draw(ElementFactory& elementFactory, std::string name = "Draw");
    // handles mouse move / keypresses
    bool HandleEvents(Settings& settings, InputEvent& inputEvent, ElementFactory& elementFactory) override;
    // draws ImGuiWidgets. returns true if update is required
    bool DrawImGui(ElementFactory& elementFactory, Settings& settings) override;
    
    
private:    
    // error checks
    bool IsValidInputs(Settings& settings, ElementFactory& elementFactory);
    // returns the header text for the gcode output
    std::string HeaderText(Settings& settings, ElementFactory& elementFactory);
    // exports gcode from current paramaters
    std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings, ElementFactory& elementFactory) override;
    
    void UpdateViewer(Settings& settings, ElementFactory& elementFactory, std::vector<DynamicBuffer::DynamicVertexList>* viewerLineList, bool isDisabled) override;
     //Event<Event_DisplayShapeOffset>::Dispatch( { elementFactory.LineLoop_PointsList(m_LineLoop, settings.p.pathCutter.QuadrantSegments), elementFactory.RawPoint_PointsList(), false } );
   
    struct Function_Draw_Parameters {
        glm::vec2 z;
        int cutSide = CompensateCutter::None;
        float finishingPass = 1.0f;
    } m_Params;
    
    LineLoopID m_LineLoop = -1;
};


class A_Drawing
{
public: 
    A_Drawing(const std::string& name = "Drawing") : m_Name(name) { }
    
    const std::string& Name() { return m_Name; }
    
    bool HandleEvents(Settings& settings, InputEvent& inputEvent);
    
    bool DrawImGui(Settings& settings);
    
    // export gcode and run
    void ActiveFunction_Run(GRBL& grbl, Settings& settings);
    // export gcode and save
    int ActiveFunction_ExportGCode(Settings& settings, std::string saveFileDirectory);
    // delete current active function
    void ActiveFunction_Delete();
    // update viewer
    void ActiveFunction_UpdateViewer(Settings& settings, std::vector<DynamicBuffer::DynamicVertexList>* viewerLineLists);

    void RawPoints_UpdateViewer(Settings& settings, std::vector<DynamicBuffer::DynamicVertexList>* viewerLineLists);
    
private:
    
    std::string m_Name;
    // contains a list of all the points in a drawing
    ElementFactory m_ElementFactory;
    // contains a list of all the active functions in a drawing
    VectorSelectable<std::unique_ptr<Function>> m_ActiveFunctions;
    int m_FunctionIDCounter = 0;
};


class Sketch
{
public:
    Sketch();
    // pushes updates to any active drawings and their functions
    void HandleEvents(Settings& settings, InputEvent& inputEvent);
    // draws the ImGui Widgets
    void DrawImGui(GRBL& grbl, Settings& settings);
    // returns true if in sketch mode
    bool IsActive() { return m_IsActive; }
    
private:
    VectorSelectable<A_Drawing> m_Drawings;
    int m_DrawingIDCounter = 0;
    bool m_IsActive = false;

    // draw list for viewer
    std::vector<DynamicBuffer::DynamicVertexList> m_ViewerLineLists;
    std::vector<DynamicBuffer::DynamicVertexList> m_ViewerPointLists;
    
    // updates viewer for active drawing
    void ActiveDrawing_UpdateViewer(Settings& settings);
    // starts / stops sketch mode
    void Activate();
    void Deactivate();
};













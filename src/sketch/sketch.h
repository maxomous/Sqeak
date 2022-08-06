#pragma once
#include <MaxLib.h> 
#include "../common.h" 



namespace Sqeak { 
    
namespace sketch
{

using namespace MaxLib::Geom;


enum CompensateCutter {
    None, Left, Right, Pocket
};
static inline int GetCutSide(CompensateCutter cutSide) {
    if(cutSide == CompensateCutter::None)  { return 0; }
    if(cutSide == CompensateCutter::Right) { return -1; } 
    else { return 1; } //(cutSide == CompensateCutter::Left || cutSide == CompensateCutter::Pocket) 
}
       
typedef size_t RawPointID;
typedef size_t ReferenceID;
typedef size_t ElementID;
typedef size_t LineLoopID;

// forward declaration
class RawPoint;
class Element;

// link between 
struct Ref_PointToElement {
public:
    Ref_PointToElement() {}
    void SetReference(RawPoint* p, Element* e)    { rawPoint = p; element = e; }
    void SetReference(RawPoint* p)                { rawPoint = p; }
    void SetReference(Element* e)                 { element = e; }
    RawPoint*  rawPoint = nullptr;
    Element*   element = nullptr;
   
};

// references in points are used to determine if the point is no longer needed
class RawPoint
{
public: 
    RawPoint(RawPointID id, const MaxLib::Geom::Vec2& position)
        : m_ID(id), m_Position(position) {}
    
    RawPointID ID()             { return m_ID; }
    float X()                   { return m_Position.x; }
    float Y()                   { return m_Position.y; }
    MaxLib::Geom::Vec2& Vec2()                { return m_Position; }
    
    // Adds a drawing element reference
    void AddReference(Ref_PointToElement* ref);
    // Removes a drawing element reference (returns true if no references left)
    bool RemoveReference(Ref_PointToElement* ref);
    
    bool HasReferences() { return !m_ElementRefs.empty(); }
    
    void GetReferences(std::function<void(Ref_PointToElement*)> callback);
    
    void SetThisRawPointFromRefs(const MaxLib::Geom::Vec2& p);

    // for debugging
    void DrawImGui(Settings& settings);
private:
    RawPointID m_ID;
    MaxLib::Geom::Vec2 m_Position;
    //std::vector<Element*> m_ElementRefs;
    std::vector<Ref_PointToElement*> m_ElementRefs;
    
    
};
static inline std::ostream& operator<<(std::ostream& os, RawPoint& p) { os << "(" << p.X() << ", " << p.Y() << ")"; return os; }

// a drawing element holds references to points
class Element
{
public:
    virtual ~Element() = default;
    ElementID           ID() { return m_ID; } 
    Ref_PointToElement* P0() { return m_Ref_P0; }
    virtual Ref_PointToElement* P1()     { return nullptr; }
    virtual Ref_PointToElement* Centre() { return nullptr; }
    virtual Ref_PointToElement* Last() = 0;
    // Adds points to return path
    virtual void Path(std::vector<Vec2>& returnPath, int arcSegments) {
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
    
    virtual void Update() {};
    
    virtual void DrawImGui(Settings& settings) = 0;
    
    virtual void SetP0(const Vec2& p0)         { m_Ref_P0->rawPoint->Vec2() = p0;  Update(); }
    virtual void SetP1(const Vec2& p1)         { (void)p1; }
    virtual void SetCentre(const Vec2& centre) { (void)centre; }
    virtual void SetRadius(float r)                 { (void)r; }
protected:
    Element(ElementID id) 
        : m_ID(id) {}
    
    ElementID m_ID;
    Ref_PointToElement* m_Ref_P0 = nullptr;
};
    
    
// holds references to element points
class Element_Point : public Element
{
public: 
    Element_Point(ElementID id, Ref_PointToElement* ref_p0)
        :   Element(id)
    { m_Ref_P0 = ref_p0; }
    
    Ref_PointToElement* Last() override  { return m_Ref_P0; };
    // Removes a rawpoint reference (returns true if no references left)
    bool RemoveReference(Ref_PointToElement* ref) override;
    
    void GetReferences(std::function<void(Ref_PointToElement*)> callback_BreakReference) { 
        callback_BreakReference(m_Ref_P0);
    }
    bool HasDanglingReference() override  { return m_Ref_P0 == nullptr; }
    
    void DrawImGui(Settings& settings) override;
    
private:
};
    
class Element_Line : public Element
{
public: 
    Element_Line(ElementID id, Ref_PointToElement* ref_p0, Ref_PointToElement* ref_p1) 
        :   Element(id) {
        m_Ref_P0 = ref_p0; 
        m_Ref_P1 = ref_p1;
    }
    Ref_PointToElement* P1() override    { return m_Ref_P1; }
    Ref_PointToElement* Last() override  { return m_Ref_P1; };
    // Removes a rawpoint reference (returns true if no references left)
    bool RemoveReference(Ref_PointToElement* ref) override;
    
    void GetReferences(std::function<void(Ref_PointToElement*)> callback_BreakReference) { 
        callback_BreakReference(m_Ref_P0);
        callback_BreakReference(m_Ref_P1);
    }
    bool HasDanglingReference() override  { return (m_Ref_P0 == nullptr) || (m_Ref_P1 == nullptr); }
    
    void DrawImGui(Settings& settings) override;
    
    void SetP1(const Vec2& p1) override { m_Ref_P1->rawPoint->Vec2() = p1;  Update(); }
private:
    Ref_PointToElement* m_Ref_P1 = nullptr;
};

    
class Element_Arc : public Element
{
public: 

    enum class DefineArcBy { Centre, Radius, P0, P1 };
  // define by centre point
    Element_Arc(ElementID id, Ref_PointToElement* ref_p0, Ref_PointToElement* ref_p1, int direction, Ref_PointToElement* ref_centre) 
        : Element(id) {
        m_Ref_P0 = ref_p0; 
        m_Ref_P1 = ref_p1;
        m_Ref_Centre = ref_centre;
        m_Direction = direction;
         
    }
    
    Ref_PointToElement* P1()     override { return m_Ref_P1; }
    Ref_PointToElement* Last()   override { return m_Ref_P1; };
    Ref_PointToElement* Centre() override { return m_Ref_Centre; }
    int Direction()             { return m_Direction; }
    float Radius()              { return m_Radius; }
 
    //  
    // for angles: 0 is vertically upward, clockwise is positive 
    // returns success
    bool SetTangentRadiusAndDirection() {
        
        // if the rawpoint has another element attached, use this as it's tangent basis
        RawPoint* RawPointPrev = nullptr;
        m_Ref_P0->rawPoint->GetReferences([&](Ref_PointToElement* ref) { 
            // get the other element attached to this point
            if(ref->element->ID() != ID()) { RawPointPrev = ref->element->P0()->rawPoint; }
        });
        if(!RawPointPrev) { return false; }
        
        Vec2 pLast = { RawPointPrev->Vec2().x, RawPointPrev->Vec2().y };
        
        Vec2 p0  = { m_Ref_P0->rawPoint->Vec2().x, m_Ref_P0->rawPoint->Vec2().y };
        Vec2 p1  = { m_Ref_P1->rawPoint->Vec2().x, m_Ref_P1->rawPoint->Vec2().y };
        // start to end line
        Vec2 dif = p1 - p0;
        Polar pol = Polar(Vec2(fabsf(dif.x), fabsf(dif.y)));
        
        // angle between pLast to p0 to p1  
        auto totalAngle = AngleBetween(pLast, p0, p1);
        if(!totalAngle) { return false; } 
                
        // change direction if left of line
        SetDirection((*totalAngle < M_PI) ? Direction::CCW : Direction::CW);
        
        // if lines are colinear
        if(*totalAngle == 0.0 || *totalAngle == M_PI) {
            m_Radius = pol.r / 2.0;
        } else {
            m_Radius = (pol.r / 2.0) / fabs(cos(CleanAngle(*totalAngle - M_PI_2)));
        }
        return true; 
    }
    
    void SetP0(const Vec2& p0)                         override {
        m_Ref_P0->rawPoint->Vec2() = p0;
        m_Priority = DefineArcBy::P0;
        if(SetTangentRadiusAndDirection()) {
            m_Priority = DefineArcBy::Radius;
        } 
        Update(); 
    }
   
    void SetP1(const Vec2& p1)                         override {
        m_Ref_P1->rawPoint->Vec2() = p1;
        m_Priority = DefineArcBy::P1;
        if(SetTangentRadiusAndDirection()) {
            m_Priority = DefineArcBy::Radius;
        }
        Update(); 
    }
   
    
    void SetCentre(const Vec2& centre)                 override {
        m_Ref_Centre->rawPoint->Vec2() = centre; 
          
        // Geos::PerdendicularDistance(p0, p1, p)
        Vec2 p0 = { m_Ref_P0->rawPoint->Vec2().x, m_Ref_P0->rawPoint->Vec2().y };
        Vec2 p1 = { m_Ref_P1->rawPoint->Vec2().x, m_Ref_P1->rawPoint->Vec2().y };
        Vec2 pC = { m_Ref_Centre->rawPoint->Vec2().x, m_Ref_Centre->rawPoint->Vec2().y };
        
        
        auto th = AngleBetween(p0, p1, pC);
        if(!th) return;
        float H = Hypot(p1-pC);
        float a = H * -sin(*th);
        float b = Hypot(p1-p0) / 2.0f;  
        
        /*
        std::cout << "angle (p0, p1, pC): " << rad2deg(*th) << std::endl;
        std::cout << "H (p1 to pC): " << H << std::endl;
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        std::cout << "sign(a): " << sign(a, 1) << std::endl;
        */
        
        m_Radius = sqrtf(a*a + b*b);
        
        m_Radius += 0.00001;
        
        // swap direction if centre is left of line (p0 -> p1)
        m_Direction = LeftOfLine(p0, p1, pC) ? Direction::CCW : Direction::CW;
                
        // direction fixes
        m_Radius *= m_Direction * Sign(a, 1); // zero has value of 1
        
        m_Priority = DefineArcBy::Radius;
        //m_Priority = DefineArcBy::Centre;
        Update(); 
    }
    void SetRadius(float r)                                 override {
        m_Radius = r;
        m_Priority = DefineArcBy::Radius;
        Update();
    }
    void SetDirection(int direction) {
        if(direction == Direction::CCW) {
            m_Direction = Direction::CCW;
            m_DirectionImGui = 1;
        } else {
            m_Direction = Direction::CW;
            m_DirectionImGui = 0;
        }
    }
    
    
    void Path(std::vector<Vec2>& returnPath, int arcSegments) override {
        Vec2 p0       = { m_Ref_P0->rawPoint->Vec2().x, m_Ref_P0->rawPoint->Vec2().y };
        Vec2 p1       = { m_Ref_P1->rawPoint->Vec2().x, m_Ref_P1->rawPoint->Vec2().y };
        Vec2 pCentre  = { m_Ref_Centre->rawPoint->Vec2().x, m_Ref_Centre->rawPoint->Vec2().y };
        
        std::cout << "p0: " << p0.x << ", " << p0.y << std::endl;
        std::cout << "p1: " << p1.x << ", " << p1.y << std::endl;
        std::cout << "p2: " << pCentre.x << ", " << pCentre.y << std::endl;
        
        // get start and end points relative to the centre point
        Vec2 v_Start = p0 - pCentre;
        Vec2 v_End = p1 - pCentre;
        
        std::cout << "v_Start: " << v_Start.x << ", " << v_Start.y << std::endl;
        std::cout << "v_End: " << v_End.x << ", " << v_End.y << std::endl;
        
        double th_Start  = atan2(v_Start.x, v_Start.y);
        double th_End    = atan2(v_End.x, v_End.y);
        
        CleanAngles(th_Start, th_End, (MaxLib::Geom::Direction)m_Direction);
        
        std::cout << "th_Start: " << th_Start << std::endl;
        std::cout << "th_End: " << th_End << std::endl;
        
        float th_Incr   = m_Direction * Radians(90.0 / arcSegments);
        std::cout << "th_Incr: " << th_Incr << std::endl;
        
        int nIncrements = floorf(fabsf((th_End - th_Start) / th_Incr));
        std::cout << "nIncrements: " << nIncrements << std::endl;
        
        Vec2 p;
        for (int n = 0; n < nIncrements; n++) {
            double th = th_Start + n * th_Incr;
            p = { fabsf(m_Radius) * sin(th), fabsf(m_Radius) * cos(th) };       
                 
            returnPath.push_back({ pCentre.x + p.x, pCentre.y + p.y });
        }
        // ensure last point is added
        if(p != p1) {         
            returnPath.push_back({ p1.x, p1.y }); 
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
    



    
    
    void Update() override
    {
        switch (m_Priority)
        {
            case DefineArcBy::Centre:
                RecalculateRadiusFromCentre(); 
                // m_Priority = DefineArcBy::Radius; // force radius priority 
                break;
            case DefineArcBy::Radius:
                RecalculateCentreFromRadius(); 
                break;
            case DefineArcBy::P0:
                RecalculateCentreFromRadius(); 
                break;
            case DefineArcBy::P1:
                RecalculateCentreFromRadius(); 
                break;
        }
    }
    
    void DrawImGui(Settings& settings) override;
    
    float MinimumRadius()
    {
        Vec2& p0       = m_Ref_P0->rawPoint->Vec2();
        Vec2& p1       = m_Ref_P1->rawPoint->Vec2();
        Vec2 dif = p1 - p0;
        return hypot(dif.x, dif.y) / 2.0;
    }
    
    // recalculates radius from centre point
    // base can be either 0 or 1 (p0 or p1)
    void RecalculateRadiusFromCentre(int basePoint = 1) 
    {
        Vec2 p0       = { m_Ref_P0->rawPoint->Vec2().x, m_Ref_P0->rawPoint->Vec2().y };
        Vec2 p1       = { m_Ref_P1->rawPoint->Vec2().x, m_Ref_P1->rawPoint->Vec2().y };
        Vec2 pCentre  = { m_Ref_Centre->rawPoint->Vec2().x,  m_Ref_Centre->rawPoint->Vec2().y };
        std::cout << "p1: " << p1.x << ", " << p1.y << std::endl;
        std::cout << "pCentre: " << pCentre.x << ", " << pCentre.y << std::endl;
        Vec2 dif = (basePoint) ? (p1 - pCentre) : (p0 - pCentre);
        m_Radius = hypot((double)dif.x, (double)dif.y);
        if(LeftOfLine(p0, p1, pCentre)) { m_Radius = -m_Radius; }
    }   
    // recalculates centre point from radius
    void RecalculateCentreFromRadius() 
    {
        Vec2 p0       = { m_Ref_P0->rawPoint->Vec2().x, m_Ref_P0->rawPoint->Vec2().y };
        Vec2 p1       = { m_Ref_P1->rawPoint->Vec2().x, m_Ref_P1->rawPoint->Vec2().y };
        Vec2 pCentre = ArcCentre(p0, p1, m_Radius, (MaxLib::Geom::Direction)m_Direction);
        m_Ref_Centre->rawPoint->Vec2() = { pCentre.x, pCentre.y };
    }
    
private:
    Ref_PointToElement* m_Ref_P1 = nullptr;
    Ref_PointToElement* m_Ref_Centre = nullptr;
    int m_Direction;
    int m_DirectionImGui = 0;
    float m_Radius;
    DefineArcBy m_Priority = DefineArcBy::Centre; // this should start at centre for initialisation
    float m_TangentRadius;
};

class ElementFactory {
public:
    std::optional<Vec2> RawPoint_GetClosest(const Vec2& p, float tolerance)
    {
        RawPoint* rawPoint = RawPoint_GetByPosition(p, tolerance);
        if(rawPoint) { 
            return rawPoint->Vec2();
        }
        return {};
    }

    void ActivePoint_Unset() { m_ActiveSelection = nullptr; }

    
    std::optional<Vec2> ActivePoint_GetPosition()
    {
        if(!m_ActiveSelection) return {};
        return m_ActiveSelection->Vec2();
    }
    bool ActivePoint_SetByPosition(const Vec2& p, float tolerance)
    {
        if(auto rawPoint = RawPoint_GetByPosition(p, tolerance)) {
            m_ActiveSelection = rawPoint;
            std::cout << "Setting active point to: " << m_ActiveSelection->ID() << std::endl; 
            return true;
        }
        return false;
    }
    
    // returns true when update required
    bool ActivePoint_Move(const Vec2& p)
    {
        if(!m_ActiveSelection)              
            return false;
        if(m_ActiveSelection->Vec2() == p)  
            return false;
        std::cout << "Move active point: " << m_ActiveSelection->ID() << "   to   " << p.x << ", " << p.y << std::endl;

        
        m_ActiveSelection->SetThisRawPointFromRefs(p);
        
        return true;
    }
    // returns true when update required
    std::optional<RawPointID> ActivePoint_GetID()
    {
        if(!m_ActiveSelection)              
            return {};
        return m_ActiveSelection->ID();
    }
    // returns true when update required
    bool ActivePoint_Delete()
    {
        if(!m_ActiveSelection)              
            return false;
        
        std::cout << "Delete Active Point (UNCODED YET)" << std::endl;
        //m_ActiveSelection->Delete();
        return true;
    }
    struct SketchOld_Element_Identifier 
    {
        SketchOld_Element_Identifier(ElementID ID, ElementFactory* parent) : id(ID), m_Parent(parent) {}
        ~SketchOld_Element_Identifier() { std::cout << "DELETING ELEMENT" << std::endl; m_Parent->Element_Delete(id); }

        ElementID id = -1;
    private:
        ElementFactory* m_Parent = nullptr;
    };
    
    struct SketchOld_LineLoop_Identifier
    { 
        SketchOld_LineLoop_Identifier() {}
        SketchOld_LineLoop_Identifier(LineLoopID ID, ElementFactory* parent) : id(ID), m_Parent(parent) {}
        ~SketchOld_LineLoop_Identifier() { std::cout << "DELETING LINELOOP" << std::endl; m_Parent->LineLoop_Delete(id); }

        LineLoopID id = -1;
    private:
        ElementFactory* m_Parent = nullptr;
    };
        
    typedef std::unique_ptr<SketchOld_Element_Identifier>  SketchOld_Element;
    typedef std::unique_ptr<SketchOld_LineLoop_Identifier> SketchOld_LineLoop;

    // Container of elements
    class LineLoop // TODO rename to Container_LineLoop
    {
    public: 
        // only allow ElementFactory to produce and modify these
        LineLoop(LineLoopID id) : m_ID(id) {}
        
        size_t  Size()      { return m_Elements.size(); }
        bool    IsEmpty()   { return Size() == 0; }
        
        LineLoopID ID()     { return m_ID; }

    private:
        LineLoopID m_ID = -1;
        std::vector<SketchOld_Element> m_Elements;
        
        friend class ElementFactory;
    };


    void RawPoint_DrawImGui(Settings& settings);
    void RefPointToElement_DrawImGui();
    
    std::vector<Vec2> RawPoint_PointsList() {
        std::vector<Vec2> points;
        for (size_t i = 0; i < m_Points.Size(); i++) {
            points.push_back(m_Points[i]->Vec2());
        }
        return move(points);
    }
    std::vector<Vec2> LineLoop_PointsList(SketchOld_LineLoop& sketchLineLoop, int arcSegments) {
        
        LineLoop& lineLoop = LineLoop_GetByID(sketchLineLoop->id);
        //std::cout << std::endl;
        std::vector<Vec2> points;
        
        for (size_t i = 0; i < lineLoop.m_Elements.size(); i++) {
            //std::cout << i << "  :  p1 = " << m_Elements[i]->Last()->Vec2() << std::endl;
            Element* element = Element_GetByID(lineLoop.m_Elements[i]->id);
            element->Path(points, arcSegments);
        }
        
        return points;
    }
    bool LineLoop_IsLoop (SketchOld_LineLoop& sketchLineLoop) { 
        
        LineLoop& lineLoop = LineLoop_GetByID(sketchLineLoop->id);
        if(lineLoop.Size() <= 2) { 
            return false; 
        }
        Ref_PointToElement* elementFirst = Element_GetByID(lineLoop.m_Elements[0]->id)->Last();
        Ref_PointToElement* elementLast = Element_GetByID(lineLoop.m_Elements.back()->id)->Last();
        // compare front and back points
        return elementFirst->rawPoint->Vec2() == elementLast->rawPoint->Vec2();
    }
    
    
    // returns true if update viewer required
    void LineLoop_DrawImGui(Settings& settings, SketchOld_LineLoop& sketchLineLoop);

    size_t LineLoop_Size(SketchOld_LineLoop& sketchLineLoop) { return LineLoop_GetByID(sketchLineLoop->id).Size(); }
    // Creates a basic Line Loop
    SketchOld_LineLoop LineLoop_Create();
    // Set start point
    void LineLoop_SetStartPoint(LineLoop& lineLoop, const Vec2& startPoint);
    void LineLoop_SetStartPoint(LineLoop& lineLoop, SketchOld_Element pointElement);
    // Adds a line to the Line Loop
    void LineLoop_AddLine(SketchOld_LineLoop& lineLoop, const Vec2& p1);
    // Adds an arc to the Line Loop from centre point
    void LineLoop_AddArc(SketchOld_LineLoop& lineLoop, const Vec2& p1, int direction);
    // Adds an arc to the Line Loop from centre point
    void LineLoop_AddArc(SketchOld_LineLoop& lineLoop, const Vec2& p1, int direction, const Vec2& centre);
    // Adds an arc to the Line Loop from radius
    void LineLoop_AddArc(SketchOld_LineLoop& lineLoop, const Vec2& p1, int direction, float radius);
    /*
    void LineLoop_SetLastArcCentre(SketchOld_LineLoop& sketchLineLoop, const Vec2& p1) 
    {
        LineLoop& lineLoop = LineLoop_GetByID(sketchLineLoop->id);
        assert(!lineLoop.IsEmpty() && "Line loop is empty");
        ElementID lastElementID = lineLoop.m_Elements.back()->id;
        Element* element = Element_GetByID(lastElementID);
        element->SetCentre(p1);
    }
    
    void LineLoop_SetLastArcRadius(SketchOld_LineLoop& sketchLineLoop, float r) 
    {
        LineLoop& lineLoop = LineLoop_GetByID(sketchLineLoop->id);
        assert(!lineLoop.IsEmpty() && "Line loop is empty");
        ElementID lastElementID = lineLoop.m_Elements.back()->id;
        Element* element = Element_GetByID(lastElementID);
        element->SetRadius(r);
    }
    */
/* 
    // set lineloop element position
    void Element_SetP0(ElementID id, Vec2 p0)         { Element_GetByID(id)->SetP0(p0); }
    void Element_SetP1(ElementID id, Vec2 p1)         { Element_GetByID(id)->SetP1(p1); }    
    void Element_SetCentre(ElementID id, Vec2 centre) { Element_GetByID(id)->SetCentre(centre); }    
    void Element_SetRadius(ElementID id, float r)          { Element_GetByID(id)->SetRadius(r); }    
*/
    
    // Element creation
    SketchOld_Element Element_CreatePoint(const Vec2& p) {
        RawPoint* point = RawPoint_Create(p);
        return move(Element_CreatePoint(point));
    }
    SketchOld_Element Element_CreateLine(const Vec2& p0, const Vec2& p1) {
        RawPoint* point0 = RawPoint_Create(p0);
        return move(Element_CreateLine(point0, p1));
    }
    SketchOld_Element Element_CreateArc(const Vec2& p0, const Vec2& p1, int direction, const Vec2& centre) { 
        RawPoint* point0 = RawPoint_Create(p0);
        return move(Element_CreateArc(point0, p1, direction, centre));
    }
    
    std::vector<Vec2> Element_GetArcPath(const Vec2& p0, const Vec2& p1, int direction, const Vec2& centre, int arcSegments) {
        
        // make a point/element reference and add to list
        Ref_PointToElement ref0, ref1, refCentre;
        // make raw points (unused ids, added for debugging)
        RawPoint point0(99999991, p0);
        RawPoint point1(99999992, p1);
        RawPoint pointCentre(99999993, centre);
        //rawPoint.AddReference(ref);
        
        // make line element
        Element_Arc arcElement(99999990, &ref0, &ref1, direction, &refCentre);
        // link references
        ref0.SetReference(&point0, &arcElement); // (Element*)&arcElement
        ref1.SetReference(&point1, &arcElement);
        refCentre.SetReference(&pointCentre, &arcElement);
        // to ensure update (radius etc)
        arcElement.Update();
        
        std::vector<Vec2> points;
        arcElement.Path(points, arcSegments);
        return points;
    }
private:

/*
// not this:
// in an attempt to ensure elements and points always have referenece
// but the other model makes more sense

    create Reference ref()
    
    create Element(ref)
        Element->AddReference(ref);
    
    create point(ref, Element)
    {
        create Point(ref)
            Point->AddReference(ref);
        
        ref.Add(Point, Element)
        {
            point = Point;
            element = Element;
        }

        m_Points.push_back(Point)
        m_Refs_PointElement.push_back(ref)
    }
    
    m_Elements.push_back(Element)



// we want to add multiple points onto element so this is better

    create Element()

    p0 = CreatePoint(Element)
    {
        create Point()
        
        create Reference(Point, Element)
        {
            point = Point;
            element = Element;
            Point->AddReference(this);
            Element->AddReference(this);
        }
        
        m_Points.push_back(Point)
        m_Refs_PointElement.push_back(Reference)
    }
    
    m_Elements.push_back(Element)



// ----- A BETTER IMPLIMENTATION -----

// Creates a 2-way reference between 2 items
// both items must have a SetReferences(Reference<T1, T2>* ref) function
template<typename T1, typename T2>
class Reference
{
public
	Reference(T1* item1, T2* item2) : m_Item1(item1), m_Item2(item2) {
		item1->SetReference(this);
		item2->SetReference(this);
	}
	T1* Item1() { return m_Item1; };
	T2* Item2() { return m_Item2; };
private:
	T1* m_Item1;
	T2* m_Item2;
};



	// Create Point
	m_Points.push_back(std::unique_ptr<Point>(new Point(cp_model, x, y));
    Point* point = m_Points.back().get();
    
   // Create Constraint
	m_Constraints.push_back(std::unique_ptr<Constraint>(new Constraint()));
	Constraint* constraint = m_Constraints.back().get();
	    
	// create Reference
	m_References.push_back(std::unique_ptr<Reference<Point, Constraint>>>(new Reference<Point, Constraint>>(point, constraint)));
	Reference<Point, Constraint>* reference = m_References.back().get();



*/ 



    // Element creation
    SketchOld_Element Element_CreatePoint(RawPoint* point) {
        // make a point/element reference and add to list
        auto ref = std::make_unique<Ref_PointToElement>();
        // add reference onto point
        point->AddReference(ref.get());
        // make point element
        std::unique_ptr<Element> element = std::make_unique<Element_Point>(m_ElementIDCounter++, ref.get());
        ElementID elementID = element->ID();
        // link references
        ref->SetReference(point, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref));
        return std::make_unique<SketchOld_Element_Identifier>(elementID, this);
    }
    // for elements which share points (reference to previous element's end point) - used for line loop
    SketchOld_Element Element_CreateLine(RawPoint* point0, const Vec2& p1) {
        // make a point/element reference and add to list
        auto ref0 = std::make_unique<Ref_PointToElement>();
        auto ref1 = std::make_unique<Ref_PointToElement>();
        // make raw point
        RawPoint* point1 = RawPoint_Create(p1);
        // add reference to p0
        point0->AddReference(ref0.get());
        point1->AddReference(ref1.get());
        // make line element
        std::unique_ptr<Element> element = std::make_unique<Element_Line>(m_ElementIDCounter++, ref0.get(), ref1.get());
        ElementID elementID = element->ID();
        // link references
        ref0->SetReference(point0, element.get());
        ref1->SetReference(point1, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref0));
        m_References.push_back(move(ref1));
        return std::make_unique<SketchOld_Element_Identifier>(elementID, this);
    }
        
    SketchOld_Element Element_CreateArc(RawPoint* point0, const Vec2& p1, int direction, const Vec2& centre) {
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
        std::unique_ptr<Element> element = std::make_unique<Element_Arc>(m_ElementIDCounter++, ref0.get(), ref1.get(), direction, refCentre.get());
        ElementID elementID = element->ID();        //m_Ref_P0->rawPoint->Vec2();
        // link references
        ref0->SetReference(point0, element.get());
        ref1->SetReference(point1, element.get());
        refCentre->SetReference(pointCentre, element.get());
        // add element and reference to list
        m_Elements.push_back(move(element));
        m_References.push_back(move(ref0));
        m_References.push_back(move(ref1));
        m_References.push_back(move(refCentre));
        // update radius from centre point
        m_Elements.back()->Update();
        return std::make_unique<SketchOld_Element_Identifier>(elementID, this);
    }

    // Deletes an Element (Called by the destructor)
    void Element_Delete(ElementID id)
    {      
        Element* element = Element_GetByID(id);
        // break element references (i.e. refs for a line are p0 & p1)
        // if a rawpoint or element has no references left, it is deleted
        element->GetReferences([&](Ref_PointToElement* ref) { 
            Reference_Break(ref);
            //bool isRawPointAndElementDeleted = Reference_Break(ref);
            //assert(isRawPointAndElementDeleted, "Raw Point & Element weren't deleted when reference was broken")
        });
        
        
        
    }
    
    RawPoint* LineLoop_LastPoint(LineLoopID id) 
    {
        LineLoop& lineLoop = LineLoop_GetByID(id);
        assert(!lineLoop.IsEmpty() && "Line loop is empty");
        ElementID lastElementID = lineLoop.m_Elements.back()->id;
        return Element_GetByID(lastElementID)->Last()->rawPoint;
    }
    
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
    // Deletes a LineLoop
    void LineLoop_Delete(LineLoopID id)
    {      
        // delete all elements  
        for (size_t i = 0; i < LineLoop_GetByID(id).Size(); i++) {
            LineLoop_DeleteElement(id, i);
        }
        LineLoop_DeleteFromFactory(id);
    }

    /*    A Lineloop is container of elements in which the elements can share points
//      A reference of both Element and RawPoint is held inside a class inside m_References which is held within both items
// 
//    .--------LineLoop---------.
//   /                           \
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
        size_t size = lineLoop.Size();
        assert((i < size) && "Invalid index provided");
        
        // element is a point
        if(i == 0) { // replace start point ref TODO: if(i == 0) make 1 the start point and delete 0
                            

            // break the reference and delete the point element (+ rawPoint (p0) if size == 1
            // HANDLED BY DESTRUCTOR       Reference_Break(ref_point0_p0);
            // delete the old point from the line loop
            lineLoop.m_Elements.erase(lineLoop.m_Elements.begin() + 0);
            // return if that was the only element in the lineloop
            if(!lineLoop.m_Elements.size()) { return; }
            
            // HANDLED BY DESTRUCTOR       Ref_PointToElement* ref_point0_p0   = Element_GetByID(lineLoop.m_Elements[0]->id)->Last();
            //ElementID                line0ID = lineLoop.m_Elements[0]->id;
            //Ref_PointToElement* ref_line0_p0 = (size == 1) ? nullptr : Element_GetByID(line0ID)->P0();
            Ref_PointToElement* ref_line0_p1 = Element_GetByID(lineLoop.m_Elements[0]->id)->Last();
            
            // there is only a point element and a rawpoint
            // make a new point element which references p1
            SketchOld_Element pointElement = Element_CreatePoint(ref_line0_p1->rawPoint);
            // break all references in line and delete the line element
            // HANDLED BY DESTRUCTOR       Element_Delete(line0ID);
            // replace lineloop start point (Line0 with pointElememt)
            LineLoop_SetStartPoint(lineLoop, move(pointElement));
               
        } else {        
            // element is not last point (Modify references between i-1 & i+1 if)
            if(i < size-1) {
                Ref_PointToElement* ref_NextP0      = Element_GetByID(lineLoop.m_Elements[i+1]->id)->P0();
                Ref_PointToElement* ref_PrevPLast   = Element_GetByID(lineLoop.m_Elements[i-1]->id)->Last();
                Reference_ReplaceRawPoint(ref_NextP0, ref_PrevPLast->rawPoint);
            }
            // break element references and delete it
            // HANDLED BY DESTRUCTOR       Element_Delete(lineLoop.m_Elements[i]->id);
            // delete the old element from the line loop
            lineLoop.m_Elements.erase(lineLoop.m_Elements.begin() + i);
        }
    }
            
    // removes references from element and rawpoint within ref, deletes the element/rawpoint if they have no references left
    // returns true if element and rawpoint are deleted
    bool Reference_Break(Ref_PointToElement* ref);
    // removes references from element and rawpoint within ref, deletes the element/rawpoint if they have no references left
    // returns true if rawpoint is deleted
    bool Reference_ReplaceRawPoint(Ref_PointToElement* ref, RawPoint* pNew);

    void Element_DeleteByID(ElementID elementID);
    Element* Element_GetByID(ElementID id) ;
    // should not be called with nullptr!
    RawPoint* RawPoint_Create(Vec2 p, Ref_PointToElement* ref = nullptr);
    
    RawPoint* RawPoint_GetByID(RawPointID pointID);
    RawPoint* RawPoint_GetByPosition(Vec2 p, float tolerance);
    void RawPoint_DeleteFromFactory(RawPoint* point);
    void Element_DeleteFromFactory(Element* element);
    void LineLoop_DeleteFromFactory(LineLoopID id);
    void Reference_DeleteFromFactory(Ref_PointToElement* ref);
    
    // lists of all elements, lineloops & points in drawing
    std::vector<std::unique_ptr<LineLoop>> m_LineLoops;
    std::vector<std::unique_ptr<Element>> m_Elements;
    Vector_SelectablePtrs<std::unique_ptr<RawPoint>> m_Points;
    // references between m_Points and m_Elements
    std::vector<std::unique_ptr<Ref_PointToElement>> m_References;
        
    // id counters
    ElementID m_ElementIDCounter = 0;
    LineLoopID m_LineLoopIDCounter = 0;
    RawPointID m_PointIDCounter = 0;
    
    RawPoint* m_ActiveSelection = nullptr;
    

    
};
 
class Function
{
public: 
    Function(std::string name) : m_Name(name) {}
    // to ensure inherited destructor is called
    virtual ~Function() = default;
    std::string& Name() { return m_Name; }
    
    virtual void HandleEvents(Settings& settings, InputEvent& inputEvent, ElementFactory& elementFactory) = 0;
    // draws ImGui widgets
    virtual void DrawImGui_Tools(Settings& settings) = 0;
    // draws ImGui widgets
    virtual void DrawImGui(ElementFactory& elementFactory, Settings& settings) = 0;
    // returns value if error
    int InterpretGCode(Settings& settings, ElementFactory& elementFactory, std::function<int(std::vector<std::string> gcode)> callback);
    // bool is success
    std::optional<std::vector<std::string>> InterpretGCode(Settings& settings, ElementFactory& elementFactory);
    // adds linelists to viewerLineList
    virtual void UpdateViewer(Settings& settings, ElementFactory& elementFactory, std::vector<DynamicBuffer::ColouredVertexList>* viewerLineLists, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists, bool isDisabled) = 0;
    
protected:
    std::string m_Name;
    // error checks
    bool IsValidInputs(Settings& settings, ElementFactory& elementFactory);
    // returns the header text for the gcode output
    std::string HeaderText(Settings& settings, ElementFactory& elementFactory);
    // exports gcode from current paramaters
    virtual std::optional<std::vector<std::string>> ExportGCode(Settings& settings, ElementFactory& elementFactory) = 0;
};

 
class Function_Draw : public Function
{
public: 
    enum class Command { Select, Line, Arc };

    Function_Draw(ElementFactory& elementFactory, std::string name = "Draw");
    // handles mouse move / keypresses
    void HandleEvents(Settings& settings, InputEvent& inputEvent, ElementFactory& elementFactory) override;
    // draws ImGui widgets
    void DrawImGui_Tools(Settings& settings);
    // draws ImGui widgets
    void DrawImGui(ElementFactory& elementFactory, Settings& settings) override;
    
    
private:    
    // error checks
    bool IsValidInputs(Settings& settings, ElementFactory& elementFactory);
    // returns the header text for the gcode output
    std::string HeaderText(Settings& settings, ElementFactory& elementFactory);
    // exports gcode from current paramaters
    std::optional<std::vector<std::string>> ExportGCode(Settings& settings, ElementFactory& elementFactory) override;
    
    void UpdateViewer(Settings& settings, ElementFactory& elementFactory, std::vector<DynamicBuffer::ColouredVertexList>* viewerLineList, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists, bool isActive) override;
     //Event<Event_DisplayShapeOffset>::Dispatch( { elementFactory.LineLoop_PointsList(m_LineLoop, settings.p.pathCutter.geosParameters.QuadrantSegments), elementFactory.RawPoint_PointsList(), false } );
   
    struct Function_Draw_Parameters {
        int polygoniseOutput = 0;
        Vec2 z = { 20.0f, 0.0f };
        int cutSide = (int)CompensateCutter::None;
        float finishingPass = 1.0f;
    } m_Params;
    
    ElementFactory::SketchOld_LineLoop m_LineLoop;
    Command m_ActiveCommand = Command::Line;
};


class A_Drawing
{
public: 
    A_Drawing(const std::string& name = "Drawing") : m_Name(name) { }
    
    const std::string& Name() { return m_Name; }
    
    void HandleEvents(Settings& settings, InputEvent& inputEvent);
    // get name of active function
    void ActiveFunction_DrawImGui_Tools(Settings& settings);
    void DrawImGui_Functions(Settings& settings);
    void DrawImGui(Settings& settings);
    // get active function name
    std::string ActiveFunction_Name();
    // returns true if active function selected
    bool ActiveFunction_HasItemSelected() { return m_ActiveFunctions.HasItemSelected(); }      
    // export gcode and run
    void ActiveFunction_Run(GRBL& grbl, Settings& settings);
    // export gcode and save
    int ActiveFunction_ExportGCode(Settings& settings, std::string saveFileDirectory);
    // delete current active function
    void ActiveFunction_Delete();
    // returns the gcode strings
    std::optional<std::vector<std::string>> ActiveFunction_UpdateViewer(Settings& settings);
    // update viewer
    void UpdateViewer(Settings& settings, std::vector<DynamicBuffer::ColouredVertexList>* viewerLineLists, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists);

    void ActivePoint_UpdateViewer(Settings& settings, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists);

    void RawPoints_UpdateViewer(Settings& settings, std::vector<DynamicBuffer::ColouredVertexList>* viewerPointLists);
    
private:
    
    std::string m_Name;
    // contains a list of all the points in a drawing
    ElementFactory m_ElementFactory;
    // contains a list of all the active functions in a drawing
    Vector_SelectablePtrs<std::unique_ptr<Function>> m_ActiveFunctions;
    int m_FunctionIDCounter = 0;
    // used to open tree node of new function
    bool m_IsActiveFunctionChanged = true;
    // allow sketch to access private members
    friend class SketchOld;
};


class SketchOld
{
public:
    SketchOld();
    
    std::string  ActiveFunction_Name();
    
    std::optional<Vec2> RawPoint_GetClosest(const Vec2& p, float tolerance);
    
    void ActiveFunction_Run(GRBL& grbl, Settings& settings);
    void ActiveFunction_Export(Settings& settings);
    void ActiveFunction_Delete(Settings& settings);
        
    // pushes updates to any active drawings and their functions
    void HandleEvents(Settings& settings, InputEvent& inputEvent);
    
    
    // draws the ImGui Widgets for the active function's tools
    void ActiveFunction_DrawImGui_Tools(Settings& settings);
    // draws the ImGui Widgets for the active drawing's functions
    void ActiveDrawing_DrawImGui_Functions(Settings& settings);
    // draws the ImGui Widgets for connecting (returns true if activated)
    bool DrawImGui_StartSketchOld(Settings& settings);
    // draws the popup at the cursor
    void DrawPopup_Cursor(Settings& settings);
    // draws the ImGui Widgets for the sketch
    void DrawImGui(Settings& settings);
    // update viewer
    void UpdateViewer(Settings& settings);
    
    void ClearViewer();
    
    // returns true if in sketch mode
    bool IsActive() { return m_IsActive; }
    
    // starts / stops sketch mode
    void Activate();
    void Deactivate();
    
private:
    Vector_SelectablePtrs<A_Drawing> m_Drawings;
    int m_DrawingIDCounter = 0;
    bool m_IsActive = false;

    // draw list for viewer
    std::vector<DynamicBuffer::ColouredVertexList> m_ViewerLineLists;
    std::vector<DynamicBuffer::ColouredVertexList> m_ViewerPointLists;
    
    // updates viewer for active drawing
    void ActiveFunction_UpdateViewer(Settings& settings);
    void ActiveDrawing_UpdateViewer(Settings& settings);
};


} // end of namespace

} // end namespace Sqeak

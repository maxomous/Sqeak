#pragma once
#include "../../common.h"

       
     
class DrawingElement 
{
public:
    // P0 is actually a reference to previous p1    
    DrawingElement(glm::vec2& p0, const glm::vec2& p1) 
        : m_P0(p0), m_P1(p1) {}
    virtual ~DrawingElement() {}
        
    const glm::vec2& P0()  { return m_P0; }
    const glm::vec2& P1()  { return m_P1; }
    glm::vec2& P1Ref()     { return m_P1; }
    
    // adds the points of this element to path
    virtual void AddPath(std::vector<glm::vec2>& path, int arcSegments) = 0;
    // draws the imgui widgets for this element
    virtual bool DrawImGui() = 0;
    // adds the header text of this element to stream
    virtual void AddElementHeaderText(std::ostringstream& stream) = 0;
protected:
    // P0 is a reference to the previous line
    glm::vec2& m_P0;
    glm::vec2 m_P1;
};


class DrawingElement_Line : public DrawingElement
{
public:
    DrawingElement_Line(glm::vec2& p0, const glm::vec2& p1) 
        : DrawingElement(p0, p1) {}
        
    // adds points to path for this element
    void AddPath(std::vector<glm::vec2>& path, int arcSegments) override
    {   // unused variable
        (void) arcSegments;
        path.push_back(m_P1);
    }
    // draws editable imgui widgets, returns true if variable is changed
    bool DrawImGui() override 
    {
        bool isChanged = false;
        isChanged |= ImGui::InputFloat2(va_str("End Point## %d", (int)&m_P1[0]).c_str(), &m_P1[0]); // using pointer as hidden id for imgui
        return isChanged;
    }
    // adds header text to stream
    void AddElementHeaderText(std::ostringstream& stream) override
    {
        stream << "; \t\t Line: " << m_P0 << " to " << m_P1 << '\n';
    }
};

class DrawingElement_Arc : public DrawingElement
{
public:
    // define by centre point
    DrawingElement_Arc(glm::vec2& p0, const glm::vec2& p1, int direction, const glm::vec2& centre) 
        : DrawingElement(p0, p1), m_Direction(direction), m_Centre(centre) 
    {
        RecalculateRadiusFromCentre();
    }
    // define by radius
    DrawingElement_Arc(glm::vec2& p0, const glm::vec2& p1, int direction, float r) 
        : DrawingElement(p0, p1), m_Direction(direction), m_Radius(r) 
    {
        RecalculateCentreFromRadius();
    }
    
    int Direction()     { return m_Direction; }
    glm::vec2 Centre()  { return m_Centre; }
    float Radius()      { return m_Radius; }
    
    float MinimumRadius()
    {
        glm::vec2 dif = m_P1 - m_P0;
        return hypotf(dif.x, dif.y);
    }
    // recalculates radius from centre point
    void RecalculateRadiusFromCentre() 
    {
        glm::vec2 dif = m_P1 - m_Centre;
        m_Radius = hypotf(dif.x, dif.y);
    }   
    // recalculates centre point from radius
    void RecalculateCentreFromRadius() 
    {
        point2D centre = Geom::ArcCentreFromRadius(point2D(m_P0.x, m_P0.y), point2D(m_P1.x, m_P1.y), m_Radius, m_Direction);
        m_Centre = { centre.x, centre.y };
    }
    
    // adds points to path for this element
    void AddPath(std::vector<glm::vec2>& path, int arcSegments) override
    { 
        // get start and end points relative to the centre point
        glm::vec2 v_Start = m_P0 - m_Centre;
        glm::vec2 v_End = m_P1 - m_Centre;
        
        double th_Start  = atan2(v_Start.x, v_Start.y);
        double th_End    = atan2(v_End.x, v_End.y);
        
        Geom::CleanAngles(th_Start, th_End, m_Direction);
        
        float th_Incr   = m_Direction * deg2rad(90.0f / arcSegments);
        
        int nIncrements = floorf(fabsf((th_End - th_Start) / th_Incr));
        
        glm::vec2 p;
        for (int n = 0; n < nIncrements; n++) {
            float th = th_Start + n * th_Incr;
            p = { m_Radius * sin(th), m_Radius * cos(th) };            
            path.push_back(m_Centre + p);
        }
        // ensure last point is added
        if(p != m_P1) {         
            path.push_back(m_P1);
        }
    }
    // draws editable imgui widgets, returns true if variable is changed
    bool DrawImGui() override 
    {
        bool isChanged = false;
        ImGui::BeginGroup();
            // end point
            if(ImGui::InputFloat2(va_str("End Point## %d", (int)&m_P1[0]).c_str(), &m_P1[0])) { // using pointer as hidden id for imgui
                // increase size of radius if too small
                float minRadius = MinimumRadius();
                if(m_Radius < minRadius) {
                    m_Radius = minRadius;
                }
                RecalculateCentreFromRadius();
                isChanged = true;
            }
            // direction
            static int directionCombo = 0;
            if(ImGui::Combo(va_str("Direction## %d", (int)&m_Direction).c_str(), &directionCombo, "CW\0CCW\0\0")) {
                m_Direction = (directionCombo == 0) ? CLOCKWISE : ANTICLOCKWISE;
                isChanged = true;
            }
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
            if(ImGui::InputFloat2(va_str("Mid Point## %d", (int)&m_Centre[0]).c_str(), &m_Centre[0])) { // using pointer as hidden id for imgui
                RecalculateRadiusFromCentre();
                isChanged = true;
            }
        
            if(ImGui::InputFloat(va_str("Radius## %d", (int)&m_Radius).c_str(), &m_Radius)) { // using pointer as hidden id for imgui
                RecalculateCentreFromRadius();
                isChanged = true;
            }
        ImGui::EndGroup();
        
        return isChanged;
    }
    // adds header text to stream
    void AddElementHeaderText(std::ostringstream& stream) override
    {
        stream << "; \t\t " << ((m_Direction == CLOCKWISE) ? "CW Arc: " : "CCW Arc: ") << m_P0 << " to " << m_P1;
        stream << " Centre: " << m_Centre << " Radius: " << m_Radius << '\n';
    }
private:
    // cw (1) or ccw (-1)
    int m_Direction;
    glm::vec2 m_Centre;
    float m_Radius;
};
    

class Drawing 
{
public:
    // returns number of elements in drawing
    size_t Size() { return m_Elements.size(); }
    // returns true if there are no elements
    bool IsEmpty() { return m_Elements.size() == 0; }
    // returns true if drawing is a loop
    bool IsLoop() { return LastPoint() == m_StartPoint; }
    
    // adds a line or arc to the drawing
    int AddElement(std::unique_ptr<DrawingElement>& element) 
    {
        if(m_Elements.size() > 0) {
            // check start point is same as end point of last line
            if(element->P0() != m_Elements[m_Elements.size()-1]->P1()) {
                Log::Error("This element's start point does not match the previous elements end point");
                return -1;
            }
        }
        // add element to the list
        m_Elements.push_back(std::move(element));
        return 0;
    }  
    
    void SetStartPoint(const glm::vec2& p0) {
        m_StartPoint = p0;
    }

    void AddLine(const glm::vec2& p1) {
        std::unique_ptr<DrawingElement> element = std::make_unique<DrawingElement_Line>(PreviousPoint(), p1);
        AddElement(element);
    }
    void AddArc(const glm::vec2& p1, int direction, const glm::vec2& centre) {
        std::unique_ptr<DrawingElement> element = std::make_unique<DrawingElement_Arc>(PreviousPoint(), p1, direction, centre);
        AddElement(element);
    }
    void AddArc(const glm::vec2& p1, int direction, float r) {
        std::unique_ptr<DrawingElement> element = std::make_unique<DrawingElement_Arc>(PreviousPoint(), p1, direction, r);
        AddElement(element);
    }
    // deletes the last element in the drawing
    void DeleteLastElement() 
    {
        assert((m_Elements.size() > 0) && "There are no elements in this drawing");
        m_Elements.pop_back();
    }
    // returns end point of last element in drawing
    glm::vec2 LastPoint() 
    {
        assert((m_Elements.size() > 0) && "There are no elements in this drawing");
        return m_Elements[m_Elements.size()-1]->P1();
    }
    
    // returns a vector of points which represent the drawing as line segments (arcs are converted to many lines)
    std::vector<glm::vec2> Path(int arcSegments)
    {
        assert((m_Elements.size() > 0) && "There are no elements in this drawing");
        assert(arcSegments > 0 && "Arc segments must be > 0");
        // add the initial point
        std::vector<glm::vec2> path { m_Elements[0]->P0() };
        // add the points of each element in this drawing
        for (size_t i = 0; i < m_Elements.size(); i++) {
            m_Elements[i]->AddPath(path, arcSegments);
        }
        return path;
    }
    // draws editable imgui widgets for each element in drawing, returns true if variable is changed
    bool DrawImGui()
    {    
        bool isChanged = false;
        ImGui::Text("Start Point");
        isChanged |= ImGui::InputFloat2(va_str("Start Point## %d", (int)&m_StartPoint[0]).c_str(), &m_StartPoint[0]); // using pointer as hidden id for imgui
       
        for (size_t i = 0; i < m_Elements.size(); i++) {
            ImGui::Text("Element %u", i+1);
            isChanged |= m_Elements[i]->DrawImGui();
        }
        return isChanged;
    }
    void AddElementsHeaderText(std::ostringstream& stream)
    {    
        for (size_t i = 0; i < m_Elements.size(); i++) {
            m_Elements[i]->AddElementHeaderText(stream);
        }
    }
   
    
private:
    std::vector<std::unique_ptr<DrawingElement>> m_Elements;
    glm::vec2 m_StartPoint; 
    
        // get reference of last end point
    glm::vec2& PreviousPoint() {
        return (m_Elements.size() == 0) ? m_StartPoint : m_Elements[m_Elements.size()-1]->P1Ref();
    }
};




class FunctionType_Draw : public FunctionType
{    
    struct Draw_Parameters {
        glm::vec2 z;
        Drawing drawing;
        int cutSide = 0;//CompensateCutter::None;
        float finishingPass = 1.0f;
    };

public:
    FunctionType_Draw() : FunctionType("Draw") { } 
    FunctionType_Draw(uint count) : FunctionType("Draw " + std::to_string(count)) 
    {
        
        m_Params.drawing.SetStartPoint(glm::vec2(0.0f, 0.0f));
        m_Params.drawing.AddLine(glm::vec2(150.0f, 50.0f));
        m_Params.drawing.AddLine(glm::vec2(150.0f, 150.0f));
        m_Params.drawing.AddLine(glm::vec2(0.0f, 0.0f));
        //m_Params.drawing.AddArc(glm::vec2(250.0f, 250.0f), CLOCKWISE, 100.0f);
        
        
        
    } 
    ~FunctionType_Draw() override {}
    
    void DrawPopup(Settings& settings) override;
    std::pair<bool, std::vector<std::string>> ExportGCode(Settings& settings) override;
    
    std::unique_ptr<FunctionType> CreateNew() override
    {
        static uint counter = 0;
        std::unique_ptr<FunctionType_Draw> newFunction = std::make_unique<FunctionType_Draw>(++counter);
        return move(newFunction);
    }
    
    void Update(glm::vec2 mouseClickPos) override 
    { 
        m_Params.drawing.AddLine(mouseClickPos);
    }
    
private:
    Draw_Parameters m_Params;
    
    // error checks
    bool IsValidInputs(Settings& settings);
    // returns the header text for the gcode output
    std::string HeaderText(Settings& settings);
    

};

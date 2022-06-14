
#include <iostream>
#include "sketch.h"

namespace Sketch {

const RenderData& SketchRenderer::Render(ElementFactory& factory, int arcSegments)
{     
    PointsCollection& pointsCollection = m_RenderData.points;   // vector<vector<Vec2>>>
    LineStrings& linestrings = m_RenderData.linestrings;        // vector<vector<Vec2>>>
    
    // Clear any existing data
    pointsCollection.clear();
    linestrings.clear();
     
    // TODO: These would be nicer if virtual functions on the elemenets?
    auto RenderElement = [&](const Sketch::Element* element) 
    {
        Points points;
        LineString linestring;
        if(auto* point = dynamic_cast<const Sketch::Point*>(element)) {
            points.push_back(point->P());
        }
        else if(auto* line = dynamic_cast<const Sketch::Line*>(element)) {
            points.push_back(line->P0());
            points.push_back(line->P1());
            linestring = { line->P0(), line->P1() };
        }
        else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element)) {
            points.push_back(arc->P0());
            points.push_back(arc->P1());
            points.push_back(arc->PC());
            linestring = arc->RenderArc(arcSegments, Direction::CW);
        }
        else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element)) {
            points.push_back(circle->PC());
            linestring = circle->RenderCircle(arcSegments);
        }
        else { // Should never reach
            assert(0 && "Cannot render element, type unknown");                
        }
        
        // Reeturn values
        if(!points.empty())             { pointsCollection.emplace_back(std::move(points)); } 
        else if(!linestring.empty())    { linestrings.emplace_back(std::move(linestring)); } 
        else { assert(0 && "Both pointslist and lineslist are empty"); }// Should never reach        
    };
    
    factory.ForEachElement(RenderElement);
    
    
    auto RenderConstraint = [&](const Sketch::Constraint* constraint) 
    {
        
        if(auto* c = dynamic_cast<const Sketch::Constraint_Template_OneItem*>(constraint)) {
            (void)c;
            //points.push_back(c->P());
            return;
        } else if(auto* c = dynamic_cast<const Sketch::Constraint_Template_OneItem*>(constraint)) {
            (void)c;
            //points.push_back(c->P());
            return;
        }
    };
    
    factory.ForEachConstraint(RenderConstraint);
    

        
       /*
    std::cout << "\n\nPoints: " << std::endl;
    
    for(const Vec2& point : points) {
        std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
    }
    
    std::cout << "\n\nLines: " << std::endl;
    
    for(const Vec2& point : lines) {
        std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
    }
    */
    return m_RenderData;
}
   
   
   
Sketcher::Sketcher() 
{
    Sketch::Point* p0 = m_Factory.AddPoint({ 10.0f, 10.0f });
    // TODO: Check if point on arc works?
    
    Sketch::Point* p1 = m_Factory.AddPoint({ 40.0f, 10.0f });
    Sketch::Point* p2 = m_Factory.AddPoint({ 40.0f, 50.0f });
    
    Sketch::Line* l1 = m_Factory.AddLine({ 590.0f, 810.0f }, { 60.0f, 13.0f });
    
    Sketch::Circle* circle = m_Factory.AddCircle({ -6.0f, -7.0f }, 200.0f);
        
    //m_Factory.AddConstraint<Sketch::Coincident_PointToPoint>(l1.p1, p1);
    
    m_Factory.AddConstraint<Sketch::Coincident_PointToCircle>(l1->Ref_P0(), circle);
    m_Factory.AddConstraint<Sketch::AddRadius_Circle>(circle, 200.0f);
    m_Factory.AddConstraint<Sketch::Coincident_PointToPoint>(l1->Ref_P1(), p1);
    m_Factory.AddConstraint<Sketch::Coincident_PointToPoint>(circle->Ref_PC(), p2);
    m_Factory.AddConstraint<Sketch::Distance_PointToPoint>(p0, p1, 30.0f);
    m_Factory.AddConstraint<Sketch::Distance_PointToPoint>(p1, p2, 40.0f);
    m_Factory.AddConstraint<Sketch::Distance_PointToPoint>(p2, p0, 55.0f);
    
    
    
    
    std::cout << "\n\n**** Elements Before Solving ****\n" << std::endl;
    m_Factory.PrintElements();
    
}

const RenderData& Sketcher::Render() 
{
    return sketchRenderer.Render(m_Factory);
}

void Sketcher::Update(Sketch::Point* draggedPoint, Vec2 pos) 
{
    // Update Solver set dragged point and its position
    if(bool success = m_Factory.UpdateSolver(draggedPoint, pos)) {
        (void)success;
        std::cout << "\n\n\n\n**** Elements After Solving ****\n" << std::endl;
        m_Factory.PrintElements();
    }
}

} // end namespace Sketch

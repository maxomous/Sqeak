
#include <cmath>
#include <iostream>

#include <sstream>
#include <iomanip> // for setprecision

#include "geos.h"

using namespace std;


 
std::vector<Geos::LineString> Geos::Offset(const LineString& points, float offset, GeosBufferParams& params)
{
    // err if no points
    if(points.size() < 2) return {};
    // check if loop and call according offset function
    if(points.front() == points.back()) {
        return move(OffsetPolygon(points, offset, params));
    } else {
        return move(OffsetLine(points, offset, params));
    }
}

std::vector<Geos::LineString> Geos::OffsetLine(const LineString& points, float offset, GeosBufferParams& params) 
{     
    auto err = std::vector<LineString>();
    // Define line string  
    GEOSGeometry* lineString = GeosLineString(points);   
    if(!lineString) return err;   
    
    // offset line  
    GEOSGeometry* bufferOp = GEOSOffsetCurve(lineString, offset, params.QuadrantSegments, params.JoinStyle, params.MitreLimit);
    if(!bufferOp) return err; 
    
    // put coords into vector
    std::optional<LineString> ls = GeosGetCoords(bufferOp, (offset < 0.0f));  
    if(!ls) return err;
    // make the return vector
    std::vector<Geos::LineString> offsetGeom;
    offsetGeom.push_back(*ls);
    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(bufferOp);
    
    return move(offsetGeom);
}
    
std::vector<Geos::LineString> Geos::OffsetPolygon(const LineString& points, float offset, GeosBufferParams& params) 
{    
    auto err = std::vector<LineString>();
    // Define as polygon
    GEOSGeometry* poly = GeosPolygon(points); 
    if(!poly) return err;
    // offset polygon
    GEOSGeometry* bufferOp = GEOSBufferWithStyle(poly, -offset, params.QuadrantSegments, params.CapStyle, params.JoinStyle, params.MitreLimit); 
    if(!bufferOp) return err;
    
    std::vector<Geos::LineString> offsetGeom; 

    // there may be multiple polygons produced so loop through and add the return vector
    // type 3 is a polygon      int GEOSGeomTypeId(bufferOp);
    // type 6 multipolygon      char* GEOSGeomType(bufferOp);
    for (int i = 0; i < GEOSGetNumGeometries(bufferOp); i++)
    {
        const GEOSGeometry* polygon = GEOSGetGeometryN(bufferOp, i);
        if(!polygon) return err;         
        // Get exterior ring 
        const GEOSGeometry* exteriorRing = GEOSGetExteriorRing(polygon); 
        if(!exteriorRing) return err;  
        // put coords into vector   
        std::optional<LineString> lineString = GeosGetCoords(exteriorRing, true); 
        if(!lineString) return err;  
        offsetGeom.push_back({ *lineString }); 
    }
    // Frees memory of all as memory ownership is passed along 
    GEOSGeom_destroy(bufferOp);
    return move(offsetGeom);
} 




Geos::Geos() {
    initGEOS(MsgHandler, MsgHandler);
}
Geos::~Geos() {
    finishGEOS();
}


std::optional<glm::vec2> Geos::Centroid(const LineString& points) 
{
    // turn into geos geometry
    GEOSGeometry* geom = GeosLinearRing(points);
    if(!geom) return {};
    // get thet centroid
    GEOSGeometry* centroid = GEOSGetCentroid(geom);
    if(!centroid) return {};
    // get point
    double x = 0, y = 0;
    if(!GEOSGeomGetX(centroid, &x)) return {};
    if(!GEOSGeomGetY(centroid, &y)) return {};
    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(centroid);
    return {{ x, y }};
}

// Returns true if Line is inside polygon
// Will also return true if line is touching boundary but is inside polygon
std::optional<bool> Geos::LineIsInsidePolygon(glm::vec2 p0, glm::vec2 p1, const LineString& polygon) 
{   
    // turn into geometry
    LineString lineString;
    lineString.push_back(p0);
    lineString.push_back(p1);
    GEOSGeometry* line = GeosLineString(lineString);
    if(!line) return {};
    
    GEOSGeometry* poly = GeosPolygon(polygon);
    if(!poly) return {};
    
    int result = GEOSWithin(line, poly);
    if(result < 0 || result > 1) return {};
    // successful result
    return result;
}


GEOSGeometry* Geos::GeosLinearRing(const LineString& points)   
{  // error checks
    // make sure there are at least 3 points in vector
    if(points.size() < 3) return nullptr; 
    // make sure first point == last point  
    if(points.front() != points.back()) return nullptr;
    // make coord sequence from points
    auto seq = GeosCoordSequence(points);
    if(!seq) return nullptr; 
    // Define linear ring 
    return GEOSGeom_createLinearRing(seq); 
}    
  
GEOSGeometry* Geos::GeosPolygon(const LineString& points)
{
    GEOSGeometry* geom = GeosLinearRing(points);
    if(!geom) return nullptr;
    // Define as polygon
    return GEOSGeom_createPolygon(geom, NULL, 0);   
}   

GEOSGeometry* Geos::GeosLineString(const LineString& points)
{
    // error check  
    if(points.size() < 2) return nullptr;   
    // make coord sequence from points 
    auto seq = GeosCoordSequence(points);    
    if(!seq) return nullptr;  
    // Define line string  
    return GEOSGeom_createLineString(seq);
}


GEOSCoordSequence* Geos::GeosCoordSequence(const Geos::LineString& points) 
{       
    GEOSCoordSequence* seq = GEOSCoordSeq_create(points.size(), 2); // 2 dimensions
    if(!seq) return nullptr;
    
    for (size_t i = 0; i < points.size(); i++) { 
        GEOSCoordSeq_setX(seq, i, points[i].x);
        GEOSCoordSeq_setY(seq, i, points[i].y);
    }
    return seq;
}

std::optional<Geos::LineString> Geos::GeosGetCoords(const GEOSGeometry* points, bool reversePoints) 
{
     // Convert to coord sequence and draw points
    const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq(points);
    if(!coordSeq) return {};
    
    // get number of points
    int nPoints = GEOSGeomGetNumPoints(points);
    if(nPoints <= 0) return {};
    // output onto vector to return
    LineString output;
    
    // build vector
    for (size_t i = 0; i < (size_t)nPoints; i++) {
        // points are in reverse order if negative offset
        size_t index = reversePoints ? nPoints-i-1 : i;
        double xCoord, yCoord;
        GEOSCoordSeq_getX(coordSeq, index, &xCoord);
        GEOSCoordSeq_getY(coordSeq, index, &yCoord);
        output.push_back({ xCoord, yCoord });
    }
     
    return move(output);
} 

void Geos::MsgHandler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}


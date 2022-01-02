
#include <cmath>
#include <iostream>

#include <sstream>
#include <iomanip> // for setprecision

#include "geos.h"

using namespace std;

// add a small amount of randomness to prevent the wierd quirks of perfectly round numbers...
Geos::LineString introduceRandomness(const Geos::LineString& points)
{
    if(!points.size()) { return points; }
    
    Geos::LineString output = points;
    output[0].x += 0.00001f;
    output[0].y += 0.00002f;
    return output;  
}


GEOSCoordSequence* Geos::MakeCoordSequence(const Geos::LineString& points) 
{       
    GEOSCoordSequence* seq = GEOSCoordSeq_create(points.size(), 2); // 2 dimensions
    if(!seq) return nullptr;
    
    for (size_t i = 0; i < points.size(); i++) { 
        GEOSCoordSeq_setX(seq, i, points[i].x);
        GEOSCoordSeq_setY(seq, i, points[i].y);
    }
    return seq;
}

std::optional<Geos::LineString> Geos::OutputCoords(const GEOSGeometry* points, bool reversePoints) 
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

std::vector<Geos::LineString> Geos::Offset(const LineString& points, float offset, int quadrantSegments, int endCapStyle, int joinStyle, double mitreLimit)
{
    GeosBufferParams params = { quadrantSegments, endCapStyle, joinStyle, mitreLimit };
    return move(Offset(points, offset, params));
}
 
std::vector<Geos::LineString> Geos::Offset(const LineString& points, float offset, GeosBufferParams& params)
{
    // err if no points
    if(!points.size()) return {};
    // check if loop and call according offset function
    if(points.front() == points.back()) {
        return move(OffsetPolygon(points, offset, params));
    } else {
        return move(OffsetLine(points, offset, params));
    }
}


std::vector<Geos::LineString> Geos::OffsetLine(const LineString& pointsRaw, float offset, GeosBufferParams& params) 
{     
    auto err = std::vector<LineString>();
    // add a small amount of randomness to prevent the wierd quirks of perfectly round numbers...  
    const Geos::LineString& points = introduceRandomness(pointsRaw);
    // Define line string  
    GEOSGeometry* lineString = GeosLineString(points);   
    if(!lineString) return err;   
    
    // offset line  
    GEOSGeometry* bufferOp = GEOSOffsetCurve(lineString, offset, params.QuadrantSegments, params.JoinStyle, params.MitreLimit);
    if(!bufferOp) return err; 
    
    // put coords into vector
    std::optional<LineString> ls = OutputCoords(bufferOp, (offset < 0.0f));  
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
        std::optional<LineString> lineString = OutputCoords(exteriorRing, true); 
        if(!lineString) return err;  
        offsetGeom.push_back({ *lineString }); 
    }
    // Frees memory of all as memory ownership is passed along 
    GEOSGeom_destroy(bufferOp);
    return move(offsetGeom);
} 

GEOSGeometry* Geos::GeosLinearRing(const LineString& points) 
{  // error checks
    // make sure there are at least 3 points in vector
    if(points.size() < 3) return nullptr;
    // make sure first point == last point  
    if(points.front() != points.back()) return nullptr;
    // make coord sequence from points
    auto seq = MakeCoordSequence(points);
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
    auto seq = MakeCoordSequence(points);    
    if(!seq) return nullptr;  
    // Define line string  
    return GEOSGeom_createLineString(seq);
}

void Geos::MsgHandler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}


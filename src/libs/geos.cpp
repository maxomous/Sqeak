
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
    // Define line string   
    GEOSGeometry* lineString = GeosLineString(points);   
    if(!lineString) return {};   
    
    // offset line  
    GEOSGeometry* bufferOp = GEOSOffsetCurve(lineString, offset, params.QuadrantSegments, params.JoinStyle, params.MitreLimit);
    if(!bufferOp) return {};   
    
    // put coords into vector 
    std::optional<LineString> ls = GeosGetCoords(bufferOp, (offset < 0.0f));  
    if(!ls) return {};
    // make the return vector 
    std::vector<Geos::LineString> offsetGeom;
    offsetGeom.push_back(*ls);
    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(bufferOp);
     
    return move(offsetGeom); 
}

std::vector<Geos::LineString> Geos::OffsetPolygon(const LineString& points, float offset, GeosBufferParams& params) 
{    
    // Define as polygon
    GEOSGeometry* poly = GeosPolygon(points); 
    if(!poly) return {};
    // offset polygon
    GEOSGeometry* bufferOp = GEOSBufferWithStyle(poly, -offset, params.QuadrantSegments, params.CapStyle, params.JoinStyle, params.MitreLimit); 
    if(!bufferOp) return {};
    
    // there may be multiple polygons produced so loop through and add the return vector
    // type 3 is a polygon      int GEOSGeomTypeId(bufferOp);
    // type 6 multipolygon      char* GEOSGeomType(bufferOp);
    std::vector<Geos::LineString> offsetGeom = GeosGetPolygons(bufferOp); 

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
std::optional<bool> Geos::LineIsInsidePolygon(const glm::vec2& p0, const glm::vec2& p1, const LineString& polygon) 
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

std::optional<Geos::LineString> Geos::GeosGetCoords(const GEOSGeometry* geometry, bool reversePoints) 
{
     // Convert to coord sequence and draw points
    const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq(geometry); 
    if(!coordSeq) return {}; 
    
    // get number of points
    int nPoints = GEOSGeomGetNumPoints(geometry);
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


// Converts geometry collection to std::vector<Geos::LineString> 
std::vector<Geos::LineString> Geos::GeosGetLineStrings(const GEOSGeometry* geometry)
{  
    std::vector<Geos::LineString> returnPolygons;
    for (int i = 0; i < GEOSGetNumGeometries(geometry); i++)
    { 
        const GEOSGeometry* lineString = GEOSGetGeometryN(geometry, i);
        if(!lineString) return {};
        
        assert(GEOSGeomTypeId(lineString) == GEOS_LINESTRING && "Type is not a LineString");
        
        std::optional<Geos::LineString> coords = GeosGetCoords(lineString, false);
        if(!coords) return {};  
        returnPolygons.push_back(*coords); 
    }
    return std::move(returnPolygons); 
}


// Converts geometry collection to std::vector<Geos::LineString>
std::vector<Geos::LineString> Geos::GeosGetPolygons(const GEOSGeometry* geometry)
{  
    std::vector<Geos::LineString> returnPolygons;
    for (int i = 0; i < GEOSGetNumGeometries(geometry); i++)
    {
        const GEOSGeometry* polygon = GEOSGetGeometryN(geometry, i);
        if(!polygon) return {};
        
        assert(GEOSGeomTypeId(polygon) == GEOS_POLYGON && "Type is not a polygon");
        
        // Get interior rings
        for(int i = 0; i < GEOSGetNumInteriorRings(polygon); i++)
        {
            const GEOSGeometry* interiorRing = GEOSGetInteriorRingN(polygon, i); 
            if(!interiorRing) return {};
            
            // Put coords into vector   
            std::optional<LineString> ls = GeosGetCoords(interiorRing, true); 
            if(!ls) return {};  
            returnPolygons.push_back(*ls); 
        }
        
        // Get exterior ring
        const GEOSGeometry* exteriorRing = GEOSGetExteriorRing(polygon); 
        if(!exteriorRing) return {};  
        
        // Put coords into vector   
        std::optional<LineString> ls = GeosGetCoords(exteriorRing, true); 
        if(!ls) return {};  
        returnPolygons.push_back(*ls); 
    }
    return std::move(returnPolygons);
}

void Geos::MsgHandler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}


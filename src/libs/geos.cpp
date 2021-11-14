#include "geos.h"


// negative for right side offset.
// positive for left side offset.
std::vector<glm::vec2> Geos::offsetLine(std::vector<glm::vec2> points, float offset, int quadrantSegments, int joinStyle, double mitreLimit) 
{
    GEOSCoordSequence* seq = GEOSCoordSeq_create(points.size(), 2); // +1 to include start point as end point  /  2 dimensions
    if(!seq) return {};
    
    for (size_t i = 0; i < points.size(); i++) { 
        GEOSCoordSeq_setX(seq, i, points[i].x);
        GEOSCoordSeq_setY(seq, i, points[i].y);
    }
    GEOSGeometry* lineString = GEOSGeom_createLineString(seq); 
    if(!lineString) return {};
    GEOSGeometry* bufferOp = GEOSOffsetCurve(lineString, offset, quadrantSegments, joinStyle, mitreLimit);
    if(!bufferOp) return {};
    
    // Convert to coord sequence and draw points
    const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq(bufferOp);
    if(!coordSeq) return {};
    
    // output onto vector to return
    std::vector<glm::vec2> output;
    for (uint i = 0; i < (uint)GEOSGeomGetNumPoints(bufferOp); i++) {
        double xCoord, yCoord;
        GEOSCoordSeq_getX(coordSeq, i, &xCoord);
        GEOSCoordSeq_getY(coordSeq, i, &yCoord);
        output.push_back({ xCoord, yCoord });
    }
    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(bufferOp);
    return output;
}


// negative for right side offset.
// positive for left side offset.
std::vector<glm::vec2> Geos::offsetPolygon(std::vector<glm::vec2> points, float offset, int quadrantSegments, int endCapStyle, int joinStyle, double mitreLimit) 
{
    GEOSCoordSequence* seq = GEOSCoordSeq_create(points.size() + 1, 2); // +1 to include start point as end point  /  2 dimensions
    if(!seq) return {};
    
    for (size_t i = 0; i < points.size(); i++) { 
        GEOSCoordSeq_setX(seq, i, points[i].x);
        GEOSCoordSeq_setY(seq, i, points[i].y);
    }
    // Define linear ring
    GEOSGeometry* geom = GEOSGeom_createLinearRing(seq); 
    if(!geom) return {};
    // Define a polygon
    GEOSGeometry* poly = GEOSGeom_createPolygon(geom, NULL, 0);
    if(!poly) return {};
    // offset curve
    GEOSGeometry* bufferOp = GEOSBufferWithStyle(poly, offset, quadrantSegments, endCapStyle, joinStyle, mitreLimit);
    if(!bufferOp) return {};
    // Get exterior ring
    const GEOSGeometry* exteriorRing = GEOSGetExteriorRing(bufferOp);
    if(!exteriorRing) return {};
    // Convert to coord sequence and draw points
    const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq(exteriorRing);
    if(!coordSeq) return {};
    // output onto vector to return
    std::vector<glm::vec2> output;
    for (uint i = 0; i < (uint)GEOSGeomGetNumPoints(exteriorRing); i++) {
        double xCoord, yCoord;
        GEOSCoordSeq_getX(coordSeq, i, &xCoord);
        GEOSCoordSeq_getY(coordSeq, i, &yCoord);
        output.push_back({ xCoord, yCoord });
    }
    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(bufferOp);
    return output;
}

void Geos::msgHandler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}


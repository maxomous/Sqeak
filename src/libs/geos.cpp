
#include <cmath>
#include <iostream>

#include <sstream>
#include <iomanip> // for setprecision

using namespace std;

#include "geos.h"

// add a small amount of randomness to prevent the wierd quirks of perfectly round numbers...
std::vector<glm::vec2> introduceRandomness(const std::vector<glm::vec2>& points)
{
    std::vector<glm::vec2> output = points;
    output[0].x += 0.00001f;
    output[0].y += 0.00002f;
    return output;
}


GEOSCoordSequence* Geos::makeCoordSequence(const std::vector<glm::vec2>& points) 
{       
    GEOSCoordSequence* seq = GEOSCoordSeq_create(points.size(), 2); // 2 dimensions
    if(!seq) return {};
    
    for (size_t i = 0; i < points.size(); i++) { 
        GEOSCoordSeq_setX(seq, i, points[i].x);
        GEOSCoordSeq_setY(seq, i, points[i].y);
    }
    return seq;
}

std::vector<glm::vec2> Geos::outputCoords(const GEOSGeometry* points, bool reversePoints) 
{
     // Convert to coord sequence and draw points
    const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq(points);
    if(!coordSeq) return {};
    
    // get number of points
    int nPoints = GEOSGeomGetNumPoints(points);
    if(nPoints == -1) return {};
    
    // output onto vector to return
    std::vector<glm::vec2> output;
    
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


std::vector<glm::vec2> Geos::offsetLine(const std::vector<glm::vec2>& pointsRaw, float offset, int quadrantSegments, int joinStyle, double mitreLimit) 
{    
    // error check
    if(pointsRaw.size() < 2) return {};
    
    // add a small amount of randomness to prevent the wierd quirks of perfectly round numbers...
    const std::vector<glm::vec2>& points = introduceRandomness(pointsRaw);
        
    // make coord sequence from points
    GEOSCoordSequence* seq = makeCoordSequence(points);

    // Define line string
    GEOSGeometry* lineString = GEOSGeom_createLineString(seq); 
    if(!lineString) return {};
    
    // offset line
    GEOSGeometry* bufferOp = GEOSOffsetCurve(lineString, offset, quadrantSegments, joinStyle, mitreLimit);
    if(!bufferOp) return {};
    
    // put coords into vector
    std::vector<glm::vec2> output = outputCoords(bufferOp, (offset < 0.0f));

    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(bufferOp);
    
    return output;
}

std::vector<glm::vec2> Geos::offsetPolygon(const std::vector<glm::vec2>& points, float offset, int quadrantSegments, int endCapStyle, int joinStyle, double mitreLimit) 
{
    // error check
    if(points.size() < 3) return {};
    
    // make coord sequence from points
    GEOSCoordSequence* seq = makeCoordSequence(points);
    
    // Define linear ring
    GEOSGeometry* geom = GEOSGeom_createLinearRing(seq); 
    if(!geom) return {};
    
    // Define as polygon
    GEOSGeometry* poly = GEOSGeom_createPolygon(geom, NULL, 0);
    if(!poly) return {};
    
    // offset polygon
    GEOSGeometry* bufferOp = GEOSBufferWithStyle(poly, -offset, quadrantSegments, endCapStyle, joinStyle, mitreLimit);
    if(!bufferOp) return {};
    
    // Get exterior ring
    const GEOSGeometry* exteriorRing = GEOSGetExteriorRing(bufferOp);
    if(!exteriorRing) return {};
    
    // put coords into vector
    std::vector<glm::vec2> output = outputCoords(exteriorRing, true);
    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(bufferOp);
    return move(output);
}

void Geos::msgHandler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}


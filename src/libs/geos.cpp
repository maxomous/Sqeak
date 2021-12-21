
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


std::pair<bool, GEOSCoordSequence*> Geos::makeCoordSequence(const std::vector<glm::vec2>& points) 
{       
    auto err = make_pair<bool, GEOSCoordSequence*>(false, {});
    GEOSCoordSequence* seq = GEOSCoordSeq_create(points.size(), 2); // 2 dimensions
    if(!seq) return err;
    
    for (size_t i = 0; i < points.size(); i++) { 
        GEOSCoordSeq_setX(seq, i, points[i].x);
        GEOSCoordSeq_setY(seq, i, points[i].y);
    }
    return { true, seq };
}

std::pair<bool, std::vector<glm::vec2>> Geos::outputCoords(const GEOSGeometry* points, bool reversePoints) 
{
    auto err = make_pair<bool, std::vector<glm::vec2>>(false, {});
     // Convert to coord sequence and draw points
    const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq(points);
    if(!coordSeq) return err;
    
    // get number of points
    int nPoints = GEOSGeomGetNumPoints(points);
    if(nPoints <= 0) return err;
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
    
    return { true, move(output) };
}

std::pair<bool, std::vector<glm::vec2>> Geos::offset(GEOSType type, const std::vector<glm::vec2>& points, float offset, int quadrantSegments, int endCapStyle, int joinStyle, double mitreLimit)

{
    if(type == GEOSType::Line)
        return offsetLine(points, offset, quadrantSegments, joinStyle, mitreLimit);
    else //if(type == GEOSType::Polygon) {
        return offsetPolygon(points, offset, quadrantSegments, endCapStyle, joinStyle, mitreLimit);
}

bool Geos::offset_AddToVector(GEOSType type, std::vector<glm::vec2>& returnPoints, const std::vector<glm::vec2>& points, float offset, int quadrantSegments, int endCapStyle, int joinStyle, double mitreLimit) 
{
    if(type == GEOSType::Line)
        return offsetLine_AddToVector(returnPoints, points, offset, quadrantSegments, joinStyle, mitreLimit);
    else //if(type == GEOSType::Polygon) {
        return offsetPolygon_AddToVector(returnPoints, points, offset, quadrantSegments, endCapStyle, joinStyle, mitreLimit);
}

bool Geos::offsetLine_AddToVector(std::vector<glm::vec2>& returnPoints, const std::vector<glm::vec2>& pointsRaw, float offset, int quadrantSegments, int joinStyle, double mitreLimit)
{    
    auto offsetPoints = offsetLine(pointsRaw, offset, quadrantSegments, joinStyle, mitreLimit);
    if(!offsetPoints.first) { return false; } 
    
    // append points
    for(glm::vec2& p : offsetPoints.second) {
        returnPoints.push_back(p);
    }
    return true;
}

std::pair<bool, std::vector<glm::vec2>> Geos::offsetLine(const std::vector<glm::vec2>& pointsRaw, float offset, int quadrantSegments, int joinStyle, double mitreLimit) 
{    
    auto err = make_pair<bool, std::vector<glm::vec2>>(false, {});
    // error check
    if(pointsRaw.size() < 2) return err;
    
    // add a small amount of randomness to prevent the wierd quirks of perfectly round numbers...
    const std::vector<glm::vec2>& points = introduceRandomness(pointsRaw);
        
    // make coord sequence from points
    auto s = makeCoordSequence(points);
    if(!s.first) return err;
    GEOSCoordSequence* seq = s.second;

    // Define line string
    GEOSGeometry* lineString = GEOSGeom_createLineString(seq); 
    if(!lineString) return err;
    
    // offset line
    GEOSGeometry* bufferOp = GEOSOffsetCurve(lineString, offset, quadrantSegments, joinStyle, mitreLimit);
    if(!bufferOp) return err;
    
    // put coords into vector
    auto p = outputCoords(bufferOp, (offset < 0.0f));
    if(!p.first) return err;
    std::vector<glm::vec2> output = p.second;

    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(bufferOp);
    
    return { true, move(output) };
}

bool Geos::offsetPolygon_AddToVector(std::vector<glm::vec2>& pointsList, const std::vector<glm::vec2>& points, float offset, int quadrantSegments, int endCapStyle, int joinStyle, double mitreLimit) 
{
    auto offsetPoints = offsetPolygon(points, offset, quadrantSegments, endCapStyle, joinStyle, mitreLimit);
    if(!offsetPoints.first) { return false; } 
    cout << "Adding points from offset" << endl;
    // append points
    for(glm::vec2& p : offsetPoints.second) {
        pointsList.push_back(p);
    }
    return true;
}
std::pair<bool, std::vector<glm::vec2>> Geos::offsetPolygon(const std::vector<glm::vec2>& points, float offset, int quadrantSegments, int endCapStyle, int joinStyle, double mitreLimit) 
{
    auto err = make_pair<bool, std::vector<glm::vec2>>(false, {});
    // error check
    if(points.size() < 3) return err;
    
    // make coord sequence from points
    auto s = makeCoordSequence(points);
    if(!s.first) return err;
    GEOSCoordSequence* seq = s.second;
    // Define linear ring
    GEOSGeometry* geom = GEOSGeom_createLinearRing(seq); 
    if(!geom) return err;
    
    // Define as polygon
    GEOSGeometry* poly = GEOSGeom_createPolygon(geom, NULL, 0);
    if(!poly) return err;
    
    // offset polygon
    GEOSGeometry* bufferOp = GEOSBufferWithStyle(poly, -offset, quadrantSegments, endCapStyle, joinStyle, mitreLimit);
    if(!bufferOp) return err;
    
    // Get exterior ring
    const GEOSGeometry* exteriorRing = GEOSGetExteriorRing(bufferOp);
    if(!exteriorRing) return err;
    
    // put coords into vector
    auto p = outputCoords(exteriorRing, true);
    if(!p.first) return err;
    std::vector<glm::vec2> output = p.second;
    // Frees memory of all as memory ownership is passed along
    GEOSGeom_destroy(bufferOp);
    return { true, move(output) };
}

void Geos::msgHandler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}



#pragma once

#include <geos_c.h>  
#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <optional>
#include <algorithm>
#include <glm/glm.hpp> 	

struct GeosBufferParams {
    int QuadrantSegments    = 30; // # lines segments in 90 degree arc
    int CapStyle            = GEOSBufCapStyles::GEOSBUF_CAP_ROUND;
    int JoinStyle           = GEOSBufJoinStyles::GEOSBUF_JOIN_ROUND;
    double MitreLimit       = 1.0;
};

class Geos {
public:    
    
    typedef std::vector<glm::vec2> LineString;

    Geos() {
        initGEOS(MsgHandler, MsgHandler);
    }
    ~Geos() {
        finishGEOS();
    }
    
    
    std::optional<glm::vec2> Centroid(const LineString& points) 
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
    /* 
    GEOSWithin
        Line is Inside: is within
        Line Crosses: is outside
        Line is Outside: is outside

        Line is touching boundary Inside: is within
        Line is touching boundary Outside: is outside
    */
    std::optional<bool> LineIsInsidePolygon(glm::vec2 p0, glm::vec2 p1, const LineString& polygon) 
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
        if(result == 0 || result == 1) return result;
        return {};
    }
    
    std::vector<LineString> OffsetPolygon_Recursive(const std::vector<LineString>& lineStrings, float pathOffset, bool isReversed, int arcSegments = 30, int endCapStyle = GEOSBufCapStyles::GEOSBUF_CAP_ROUND, int joinStyle = GEOSBufJoinStyles::GEOSBUF_JOIN_ROUND, double mitreLimit = 1.0)
    {
        GeosBufferParams params = { arcSegments, endCapStyle, joinStyle, mitreLimit };
        return OffsetPolygon_Recursive(lineStrings, pathOffset, isReversed, params);
    }
    
    std::vector<LineString> OffsetPolygon_Recursive(const std::vector<LineString>& lineStrings, float pathOffset, bool isReversed, GeosBufferParams& params)
    {
        std::vector<LineString> return_LineStrings;
        LineString buffer;
        // dynamic start point (finds closest point in offset)
        std::vector<size_t> startIndex(lineStrings.size(), 0);
        OffsetPolygon_Recursive(return_LineStrings, lineStrings, pathOffset, buffer, startIndex, params);
        // reverse
        if(isReversed) {
            for(size_t i = 0; i < return_LineStrings.size(); i++) {
                std::reverse(return_LineStrings[i].begin(), return_LineStrings[i].end());
            }
            std::reverse(return_LineStrings.begin(), return_LineStrings.end());
        }
        return return_LineStrings;
    }
    
private:    

    // adds linestring and subsequent offsets into returnPoints
    // if offset > 0 we recursively make offsets until offset fails
    void OffsetPolygon_Recursive(std::vector<LineString>& return_LineStrings, const std::vector<LineString>& lineStrings, float pathOffset, LineString& buffer, std::vector<size_t> startIndex, GeosBufferParams& params)
    {    
        auto endOffsetGroup = [&]() {
            return_LineStrings.push_back(buffer);
            buffer.clear();
        };
        
        assert(lineStrings.size() == startIndex.size() && "Start index size isnt equal to linestring size");

        for(size_t n = 0; n < lineStrings.size(); n++) {
        //const LineString& l: lineStrings) {
            const LineString& l = lineStrings[n];
            
            // check there is 2 or more points
            if(l.size() < 2) { continue; }
            
            std::cout << "path offset: " << pathOffset << std::endl;
            
            
            assert(startIndex[n] < l.size() && "Start index isnt within linestring");
            
            // add input linestring to buffer
            for(size_t i = 0; i < l.size()-1; i++) { // -1 because first and last point are the same
                size_t index = (int)(i + startIndex[n]) % (l.size()-1);
                assert(index < l.size() && "Index out of range");
                buffer.push_back(l[index]);
            }
            // make first and last point the same
            size_t firstPointIdx = startIndex[n];
            assert(firstPointIdx < l.size());
            buffer.push_back(l[firstPointIdx]);
            // break if this is just a single lineloop
            if(pathOffset == 0.0f) { 
                endOffsetGroup();
                continue; 
            } 
            // recursively offset the lineString to bore out
            std::vector<LineString> OffsetLines = OffsetPolygon(l, pathOffset, params);
            // if no more offsets, add centroid point as last position
            if(!OffsetLines.size()) {
                if(auto centroid = Centroid(l)) { 
                    buffer.push_back(*centroid); 
                }
                endOffsetGroup();
                continue;
            }
            // we have finished a set of offsets, add buffer to return vector
            //if(!OffsetLines.size()  ||  ( (isReversed) ? (OffsetLines.size() < lineStrings.size()) : (OffsetLines.size() > lineStrings.size()) )) {
            if(OffsetLines.size() > lineStrings.size()) {
                endOffsetGroup();
            }
            // check for the closest point and make this the new start point
            std::vector<size_t> nextStartIndex = DetermineStartPoints(OffsetLines, l, startIndex[n]);
            // repeat function
            OffsetPolygon_Recursive(return_LineStrings, OffsetLines, pathOffset, buffer, nextStartIndex, params);
        }
    } 
    
    std::vector<size_t> DetermineStartPoints(std::vector<LineString>& OffsetLines, const LineString& l, size_t startIndex)
    {
        assert(OffsetLines.size());
        if(startIndex >= l.size()) {
            std::cout << "index("<< startIndex << ") < size(" <<  l.size() << ")" << std::endl;
            
            exit(1);
        }
        assert(startIndex < l.size());
        const glm::vec2& p0 = l[startIndex];
        
        std::vector<size_t> startIndexNew;
        
        for(size_t n = 0; n < OffsetLines.size(); n++)
        {   // reset the start position & minimum length
            float LMin = -1.0f;
            size_t newIndex = 0;
            
            for(size_t i = 0; i < OffsetLines[n].size()-1; i++) // -1 because first and last point are the same
            {   // get next point in offset and compare its length to prev
                glm::vec2& p1 = OffsetLines[n][i];
                glm::vec2 dif = p1 - p0;
                // compare length
                float L = hypotf(dif.x, dif.y);
                if(L < LMin || LMin == -1.0f) {
                    if(LineIsInsidePolygon(p0, p1, l)) {
                        newIndex = i;
                        LMin = L;
                    }
                }
            }
            startIndexNew.push_back(newIndex);
            std::cout << "pushing back new index " << newIndex << "  size: " << startIndexNew.size() << std::endl;
        }
        return move(startIndexNew);
    }
public:
    // determines whether points are a line or polygon by if the first and last point is the same 
    // offset is negative for right side offset / positive for left side
    std::vector<LineString> Offset(const LineString& points, float offset, int quadrantSegments = 30, int endCapStyle = GEOSBufCapStyles::GEOSBUF_CAP_ROUND, int joinStyle = GEOSBufJoinStyles::GEOSBUF_JOIN_ROUND, double mitreLimit = 1.0);
    std::vector<LineString> Offset(const LineString& points, float offset, GeosBufferParams& params);
    std::vector<LineString> OffsetLine(const LineString& pointsRaw, float offset, GeosBufferParams& params);
    std::vector<LineString> OffsetPolygon(const LineString& points, float offset, GeosBufferParams& params);
    
private:

    GEOSGeometry* GeosLinearRing(const LineString& points);
    GEOSGeometry* GeosPolygon(const LineString& points);
    GEOSGeometry* GeosLineString(const LineString& points);
    
    GEOSCoordSequence*         MakeCoordSequence(const LineString& points);
    std::optional<LineString>  OutputCoords(const GEOSGeometry* coords, bool reversePoints);

    static void   MsgHandler(const char* fmt, ...);
};

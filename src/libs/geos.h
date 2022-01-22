
#pragma once

#include <geos_c.h>  
#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <optional>
#include <algorithm>
#include <glm/glm.hpp> 	

// delete this, for debugging only
//static void printVec2(const std::string& text, const glm::vec2& v) {
//    std::cout << text << ": (" << v.x << ", " << v.y << ")" << std::endl;
//}


// original is here: geos::operation::buffer::BufferParameters

struct GeosBufferParams {
    int QuadrantSegments    = 30; // # lines segments in 90 degree arc
    int CapStyle            = GEOSBufCapStyles::GEOSBUF_CAP_ROUND;
    int JoinStyle           = GEOSBufJoinStyles::GEOSBUF_JOIN_ROUND;
    double MitreLimit       = 1.0;
};

class Geos {
public:    
    // a linestring is a polygon when the first and last point are identical
    typedef std::vector<glm::vec2> LineString;

    Geos();
    ~Geos();
    // returns centroid of a polygon
    std::optional<glm::vec2> Centroid(const LineString& points);
    // Returns true if Line is inside polygon or line is touching boundary but is inside polygon
    std::optional<bool> LineIsInsidePolygon(glm::vec2 p0, glm::vec2 p1, const LineString& polygon);
    // returns offset of a line or polygon. result may be multiple linestrings
    // offset is negative for right side offset / positive for left side
    // determines whether points are a line or polygon by if the first and last point is the same 
    std::vector<LineString> Offset(const LineString& points, float offset, GeosBufferParams& params);
    std::vector<LineString> OffsetLine(const LineString& points, float offset, GeosBufferParams& params);
    std::vector<LineString> OffsetPolygon(const LineString& points, float offset, GeosBufferParams& params);
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
        // sanity check
        assert(lineStrings.size() == startIndex.size() && "Start index size isnt equal to linestring size");
        
        auto endOffsetGroup = [&]() {
            return_LineStrings.push_back(buffer);
            buffer.clear();
        };
        

        for(size_t n = 0; n < lineStrings.size(); n++) {
            const LineString& l = lineStrings[n];
            
            // check there is 2 or more points
            if(l.size() < 2) { continue; }
            
            // sanity checks
            assert(l.front() == l.back() && "Linestring start and end points do not match");
            assert(startIndex[n] < l.size() && "Start index isnt within linestring");
            
            // add input linestring to buffer
            for(size_t i = 0; i < l.size()-1; i++) { // -1 because first and last point are the same
                size_t index = (int)(i + startIndex[n]) % (l.size()-1);
                assert(index < l.size() && "Index out of range");
                
                //printVec2("Adding to buffer", l[index]);
                buffer.push_back(l[index]);
            }
            // make first and last point the same
            //printVec2("Adding to buffer", l[startIndex[n]]);
            buffer.push_back(l[startIndex[n]]);
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
                    //printVec2("Adding to buffer", *centroid);
                    buffer.push_back(*centroid); 
                }
                endOffsetGroup();
                continue;
            }
            // we have finished a set of offsets, add buffer to return vector
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
        assert(startIndex < l.size());
        
        const glm::vec2& p0 = l[startIndex];
        
        std::vector<size_t> startIndexNew;
        
        for(size_t n = 0; n < OffsetLines.size(); n++)
        {   
            size_t newIndex = 0;
            // ensure the line falls within the current offset polygon (to prevent cutting material we wouldn't want to)
            for(size_t i = 0; i < OffsetLines[n].size()-1; i++) // -1 because first and last point are the same
            {
                glm::vec2& p1 = OffsetLines[n][i];
                if(LineIsInsidePolygon(p0, p1, l)) {
                    newIndex = i;
                    break;
                }
            }
            
            /* FIND THE CLOSEST POINT (VERY SLOW...)
            // reset the start position & minimum length
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
            * */
            startIndexNew.push_back(newIndex);
        }
        return move(startIndexNew);
    }

    
private:

    GEOSGeometry* GeosLinearRing(const LineString& points);
    GEOSGeometry* GeosPolygon(const LineString& points);
    GEOSGeometry* GeosLineString(const LineString& points);
    
    GEOSCoordSequence*         GeosCoordSequence(const LineString& points);
    std::optional<LineString>  GeosGetCoords(const GEOSGeometry* coords, bool reversePoints);

    static void   MsgHandler(const char* fmt, ...);
};

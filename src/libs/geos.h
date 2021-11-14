
#pragma once

#include <geos_c.h>  
#include <vector>
#include <stdio.h>
#include <stdarg.h>
#include <glm/glm.hpp> 	

class Geos {
public:
    Geos() {
        initGEOS(msgHandler, msgHandler);
    }
    ~Geos() {
        finishGEOS();
    }
    std::vector<glm::vec2> offsetLine(std::vector<glm::vec2> points, float offset, int quadrantSegments = 30, int joinStyle = GEOSBufJoinStyles::GEOSBUF_JOIN_ROUND, double mitreLimit = 1.0);
    std::vector<glm::vec2> offsetPolygon(std::vector<glm::vec2> points, float offset, int quadrantSegments = 30, int endCapStyle = GEOSBufCapStyles::GEOSBUF_CAP_ROUND, int joinStyle = GEOSBufJoinStyles::GEOSBUF_JOIN_ROUND, double mitreLimit = 1.0);

private:
    static void msgHandler(const char* fmt, ...);
};

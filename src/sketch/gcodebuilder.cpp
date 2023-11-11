#include "gcodebuilder.h"

using namespace std;
using namespace MaxLib;
using namespace MaxLib::String;
using namespace MaxLib::Geom;


namespace Sqeak { 

void GCodeBuilder::Add(std::string gcode) {
    m_gcodes.push_back(gcode);
}
void GCodeBuilder::Clear() {
    m_gcodes.clear();
}
void GCodeBuilder::Retract(float distance) {
    Add("G91\t(Incremental Mode)");
    Add("G0 Z" + std::to_string(distance) + "\t(Move upward)");
    Add("G90\t(Absolute Mode)");
}
void GCodeBuilder::RetractToZPlane() {
    Add("G91\t(Incremental Mode)");
    Add("G28 Z0\t(Move To ZPlane)");
    Add("G90\t(Absolute Mode)");
}
void GCodeBuilder::MoveToHomePosition() {
    Add("G91\t(Incremental Mode)");
    Add("G28 Z0\t(Move To ZPlane)");
    Add("G28 X0 Y0\t(Move To Home Position)");
    Add("G90\t(Absolute Mode)");
}

void GCodeBuilder::Initialisation(float spindleSpeed) {
    // Add("G54 (Coord System 0)");
    Add("G17\t(Select XY Plane)");
    Add("G90\t(Absolute Mode)");
    Add("G94\t(Feedrate/min Mode)"); 
    Add("G21\t(Set units to mm)");
    
    RetractToZPlane();
    
    if(spindleSpeed) {
        Add("M3 S" + std::to_string((int)spindleSpeed) + "\t(Start Spindle)");
        Add("G4 P" + std::to_string((int)roundf(spindleSpeed / 2000.0f)) + "\t(Wait For Spindle)");
    }
    Add(""); // blank line
}

void GCodeBuilder::EndCommands() {
    Add(""); // blank line
    Initialisation(0);
    Add("M5\t(Stop Spindle)");
    MoveToHomePosition();
    Add("M30\t(End Program)");
}

//std::vector<std::pair<size_t, Vec2>> DepthCutter::GetTabPositions(const std::vector<Vec2>& path, const CutPathParams& params)
//{ ErrorCheckTabs
//    // error checks
//    if(!params.tabs.isActive)   { return {}; }
//    if(path.empty())            { return {}; }
//    float tabSpacing = params.tabs.spacing;
//    float tabWidth   = params.tabs.width;
//    
//    // Tab variables
//    Vec2 p0 = path[0];
//    float distanceAtLastPoint = 0.0f;
//    float nextTabPos = tabSpacing;
//    int tabCount = 0;
//    // vector to return
//    std::vector<std::pair<size_t, Vec2>> tabPositions;
//    
//    for (size_t i = 1; i < path.size(); i++) 
//    {
//        Vec2 p1 = path[i];
//        // calculate distance
//        Vec2 dif = p1-p0;
//        float distance = hypotf(dif.x, dif.y);
//        
//        // make tab
//        while(distanceAtLastPoint + distance > nextTabPos) {
//            
//            float tabPosAlongLine = nextTabPos - distanceAtLastPoint;
//            
//            // if the next tab falls too close to a corner, keep incrementing it until it's posible to produce
//            float toolRadius = params.tool.diameter / 2.0f;
//            float minDistance = (tabWidth/2.0f) + toolRadius + 1.0f; // +1mm just to be sure
//            
//            if(tabPosAlongLine < minDistance || distance-tabPosAlongLine < minDistance) {
//                nextTabPos += 1.0f;
//                continue;
//            }
//            
//            Vec2 normalised = dif / distance;
//            Vec2 tabPos = p0 + (normalised * tabPosAlongLine);
//            
//            // add tab position to vector //
//            tabPositions.push_back(std::make_pair(i, tabPos));
//            
//            tabCount++;
//            nextTabPos = tabSpacing * (float)(tabCount+1);
//            // check tab is far enough away from p0 and p1
//        }
//        
//        distanceAtLastPoint += distance;
//        p0 = p1;
//    }
//    return move(tabPositions);
//}
//        
//void DepthCutter::AddTabs(const std::vector<Vec2>& path, const CutPathParams& params, const std::vector<std::pair<size_t, Vec2>>& tabPositions, float zCurrent, bool isMovingForward, int& tabIndex, size_t i) 
//{ErrorCheckTabs
//    if(!params.tabs.isActive) {
//        return; 
//    }
//    float tabHeight  = params.tabs.height;
//    float tabWidth   = params.tabs.width;
//    float tabZPos = params.depth.zBottom + tabHeight;
//    
//    // get start and end points of current line
//    const Vec2& pLast = (isMovingForward) ? path[i-1] : path[path.size()-i];
//    const Vec2& pNext = (isMovingForward) ? path[i]   : path[path.size()-i-1];
//    Vec2 pDif = pNext - pLast;
//    
//    auto NewTab = [&]() {
//        // get tab position
//        Vec2& tabPosition = tabPositions[tabIndex].second;
//        // calculate tab start / end
//        float distance = hypotf(pDif.x, pDif.y);
//        
//        Vec2 normalised = pDif / distance;
//        float toolRadius = params.tool.diameter / 2.0f;
//        Vec2 tabOffset = normalised * ((tabWidth / 2.0f) +  toolRadius);
//        Vec2 tabStart = tabPosition - tabOffset;
//        Vec2 tabEnd = tabPosition + tabOffset;
//        
//        // start of tab
//        Add(va_str("G1 X%.3f Y%.3f Z%.3f F%.0f", tabStart.x, tabStart.y, zCurrent, params.tool.feedCutting));
//        Add(va_str("G1 Z%.3f F%.0f\t(Start Tab)", tabZPos, params.tool.feedCutting));
//        // end of tab
//        Add(va_str("G1 X%.3f Y%.3f F%.0f", tabEnd.x, tabEnd.y, params.tool.feedCutting));
//        Add(va_str("G1 Z%.3f F%.0f\t(End Tab)", zCurrent, params.tool.feedCutting));
//    };
//    // continue if below top of tab
//    if(zCurrent >= tabZPos)
//        return;
//        
//    // if moving forward along path
//    if(isMovingForward && (tabIndex < (int)tabPositions.size())) { 
//        // add a tab if index matches with position
//        while(tabPositions[tabIndex].first == i) {
//            NewTab();
//            if(++tabIndex >= (int)tabPositions.size()) {
//                break;
//            }
//        }
//    } // if moving backward along path
//    else if(!isMovingForward && (tabIndex >= 0)) {   
//        // add a tab if index matches with position
//        while(tabPositions[tabIndex].first == path.size()-i) {
//            NewTab();
//            if(--tabIndex < 0) {
//                break;
//            }
//        }
//    }
//}

    

int DepthCutter::CutPathDepths(GCodeBuilder& gcodes, const GeometryCollection& path, RetractType retractType) 
{
    for(auto& lineString : path.lineStrings) { 
        if(CutPathDepths(gcodes, lineString, retractType)) { return -1; }
    }
    for(auto& polygon : path.polygons) { 
        // Shell
        if(CutPathDepths(gcodes, polygon.shell, retractType)) { return -1; }
        // Holes
        for (auto& hole : polygon.holes) {
            if(CutPathDepths(gcodes, hole, retractType)) { return -1; }
        }
        
    }
    return 0;
}

int DepthCutter::CutPathDepths(GCodeBuilder& gcodes, const std::vector<Vec2>& path, RetractType retractType) 
{
    // check the path for errors
    if(path.size() < 2) {
        Log::Error("There should be 2 or more points");
        return -1;
    }
    // check the parameters for errors
    if(ErrorCheckParameters()) { return -1; }

    float zCurrent = depth.zTop;
    int zDirection = ((depth.zBottom - depth.zTop) > 0) ? 1 : -1; // negative multiplier
    bool isMovingForward = true;
    bool isLoop = path.front() == path.back();
        
    do {
        // retract if first run (before we've moved anywhere) or requires retract for pocketing, move to safe z
        if((zCurrent == depth.zTop) || retractType == RetractType::Full) {
            gcodes.RetractToZPlane();
        } else { // or retract a small distance
            gcodes.Retract(depth.partialRetractDistance);
        }
        // then move to initial x, y position
        gcodes.Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", path[0].x, path[0].y));
        // plunge to next z
        gcodes.Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", zCurrent, tool.feedPlunge));
        // tabStartIndex gets modified by CheckForTab whilst looping through path
//        int tabStartIndex = (isMovingForward) ? 0 : tabPositions.size()-1;
        // Feed along path
        for (size_t i = 1; i < path.size(); i++) {
            // check for and draw tabs
//            AddTabs(path, params, tabPositions, zCurrent, isMovingForward, tabStartIndex, i);
            // get start and end points of current line
            const Vec2& pNext = (isMovingForward) ? path[i]   : path[path.size()-i-1];
            // move to next point in linestring
            gcodes.Add(va_str("G1 X%.3f Y%.3f F%.0f", pNext.x, pNext.y, tool.feedCutting));
        }
        // reverse direction at end of linestring
        if(!isLoop) { isMovingForward = !isMovingForward; }
        // if we have reached the final z depth, break out of loop
        if(zCurrent == depth.zBottom) { break; }
        // update z
        zCurrent += zDirection * fabsf(tool.cutDepth);
        
        // if z zepth is further than final depth, adjust to final depth
        if((zDirection == 1 && zCurrent > depth.zBottom) || (zDirection == -1 && zCurrent < depth.zBottom)) {
            zCurrent = depth.zBottom;
        }
    } while(1);

    return 0;
}


     
int DepthCutter::ErrorCheckParameters() 
{
    if(depth.zTop < depth.zBottom) { 
        Log::Error("Z Bottom must be below Z Top");
        return -1;
    }
    if(tool.cutDepth <= 0.0f) {                                                                                                                                           
        Log::Error("Cut Depth must be greater than 0");
        return -1;
    }
    if(tool.feedPlunge <= 0.0f) {                                                                                                                                           
        Log::Error("Plunge Feedrate must be greater than 0");
        return -1;
    }
    if(tool.feedCutting <= 0.0f) {
        Log::Error("Cutting Feedrate must be greater than 0");
        return -1;
    }
    return 0;
}

//int DepthCutter::ErrorCheckTabs() 
//{
//    if(tool.diameter == 0.0f) {
//        Log::Error("Tool Diameter must be greater than 0");
//        return -1;
//    }
//    if(tabs.spacing <= tabs.width) {                                                                                                                                           
//        Log::Error("Tab Spacing must be greater than tab width");
//        return -1;
//    }
//    if(tabs.height <= 0.0f) {                                                                                                                                           
//        Log::Error("Tab Height must be greater than 0");
//        return -1;
//    }
//    if(tabs.width <= 0.0f) {                                                                                                                                           
//        Log::Error("Tab width must be greater than 0");
//        return -1;
//    }
//}


//int GCodeBuilder::CutPathDepths(const std::vector<Vec2>& path, const CutPathParams& params, const Tool& tool, const std::vector<std::pair<size_t, Vec2>>& tabPositions) {
//    
//    // error check
//    if(path.size() < 2) {
//        Log::Error("There should be 2 or more points");
//        return -1;
//    }
//    if(params.tool.feedPlunge == 0.0f) {                                                                                                                                           
//        Log::Error("Plunge feedrate requires a value");
//        return -1;
//    }
//    if(params.tool.feedCutting == 0.0f) {
//        Log::Error("Cutting feedrate requires a value");
//        return -1;
//    }
//    float zCurrent = params.depth.zTop;
//    int zDirection = ((params.depth.zBottom - params.depth.zTop) > 0) ? 1 : -1; // negative multiplier
//    bool isMovingForward = true;
//    bool isLoop = path.front() == path.back();
//        
//    do {
//        // retract then move to initial x, y position
//        // if first run (before we've moved anywhere) or requires retract for pocketing, move to safe z
//        if(params.depth.retract == RetractType::Full || (zCurrent == params.depth.zTop)) {    //params.isLoop && (path.front() != path.back());
//            RetractToZPlane();
//            Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", path[0].x, path[0].y));
//        }
//        // or retract a small distance
//        else if(params.depth.retract == RetractType::Partial) {
//            Retract(params.depth.partialRetractDistance);
//            Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", path[0].x, path[0].y));
//        }
//        // plunge to next z
//        Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", zCurrent, params.tool.feedPlunge));
//        // tabStartIndex gets modified by CheckForTab whilst looping through path
//        int tabStartIndex = (isMovingForward) ? 0 : tabPositions.size()-1;
//        // Feed along path
//        for (size_t i = 1; i < path.size(); i++) {
//            // check for and draw tabs
//            CheckForTab(path, params, tabPositions, zCurrent, isMovingForward, tabStartIndex, i);
//            // get start and end points of current line
//            const Vec2& pNext = (isMovingForward) ? path[i]   : path[path.size()-i-1];
//            // move to next point in linestring
//            Add(va_str("G1 X%.3f Y%.3f F%.0f", pNext.x, pNext.y, params.tool.feedCutting));
//        }
//        // reverse direction at end of linestring
//        if(!isLoop) { isMovingForward = !isMovingForward; }
//        // if we have reached the final z depth, break out of loop
//        if(zCurrent == params.depth.zBottom) { break; }
//        // update z
//        zCurrent += zDirection * fabsf(params.depth.cutDepth);
//        
//        // if z zepth is further than final depth, adjust to final depth
//        if((zDirection == 1 && zCurrent > params.depth.zBottom) || (zDirection == -1 && zCurrent < params.depth.zBottom)) {
//            zCurrent = params.depth.zBottom;
//        }
//    } while(1);
//
//    return 0;
//}


} // end namespace Sqeak

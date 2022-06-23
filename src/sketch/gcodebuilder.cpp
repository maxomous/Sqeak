#include "gcodebuilder.h"

    
using namespace std;
using namespace MaxLib::String;


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

void GCodeBuilder::InitCommands(float spindleSpeed) {
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
    InitCommands(0);
    Add("M5\t(Stop Spindle)");
    MoveToHomePosition();
    Add("M30\t(End Program)");
}

// executes length in one axis and then moves width of cutter in other axis
void GCodeBuilder::FacingCutXY(Settings& settings, glm::vec2 p0, glm::vec2 p1, bool isYFirst) 
{
    ParametersList::Tools::Tool& tool = settings.p.tools.toolList.CurrentItem();
    ParametersList::Tools::Tool::ToolData& toolData = tool.Data.CurrentItem();
    
    float cutWidth  = tool.Diameter - settings.p.pathCutter.CutOverlap;
    glm::vec2 pNext = p0; 
    bool forward    = true;
        
    if(isYFirst) {
        int xDirection = ((p1.x - p0.x) > 0) ? FORWARD : BACKWARD;
        
        do {
            // y direction
            pNext.y = (forward) ? p1.y : p0.y;
            Add(va_str("G1 Y%.3f F%.0f", pNext.y, toolData.feedCutting));
            
            // x direction
            if(pNext.x == p1.x) {
                break;
            }
            
            pNext.x += xDirection * cutWidth;
            
            if((xDirection == FORWARD && pNext.x > p1.x) || (xDirection == BACKWARD && pNext.x < p1.x)) {
                pNext.x = p1.x;
            }
            Add(va_str("G1 X%.3f F%.0f", pNext.x, toolData.feedCutting));
        
            // change direction
            forward = !forward;
        } while(1);
    } 
    else {
        int yDirection = ((p1.y - p0.y) > 0) ? FORWARD : BACKWARD;
        
        do {
            // x direction
            pNext.x = (forward) ? p1.x : p0.x;
            Add(va_str("G1 X%.3f F%.0f", pNext.x, toolData.feedCutting));
            
            // y direction
            if(pNext.y == p1.y) {
                break;
            }
            
            pNext.y += yDirection * cutWidth;
            
            if((yDirection == FORWARD && pNext.y > p1.y) || (yDirection == BACKWARD && pNext.y < p1.y)) {
                pNext.y = p1.y;
            }
            Add(va_str("G1 Y%.3f F%.0f", pNext.y, toolData.feedCutting));
        
            // change direction
            forward = !forward;
        } while(1);
    }
}
 

std::vector<std::pair<size_t, glm::vec2>> GCodeBuilder::GetTabPositions(Settings& settings, const CutPathParams& params)
{ 
    if(!settings.p.pathCutter.CutTabs) {
        return {}; 
    }
    float& tabSpacing = settings.p.pathCutter.TabSpacing;
    float& tabWidth   = settings.p.pathCutter.TabWidth;
    
    const std::vector<glm::vec2>* points = params.points;
    
    // Tab variables
    glm::vec2 p0 = (*points)[0];
    float distanceAtLastPoint = 0.0f;
    float nextTabPos = tabSpacing;
    int tabCount = 0;
    // vector to return
    std::vector<std::pair<size_t, glm::vec2>> tabPositions;
    
    for (size_t i = 1; i < points->size(); i++) 
    {
        glm::vec2 p1 = (*points)[i];
        // calculate distance
        glm::vec dif = p1-p0;
        float distance = hypotf(dif.x, dif.y);
        
        // make tab
        while(distanceAtLastPoint + distance > nextTabPos) {
            
            float tabPosAlongLine = nextTabPos - distanceAtLastPoint;
            
            // if the next tab falls too close to a corner, keep incrementing it until it's posible to produce
            float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
            float minDistance = (tabWidth/2.0f) + toolRadius + 1.0f; // +1mm just to be sure
            
            if(tabPosAlongLine < minDistance || distance-tabPosAlongLine < minDistance) {
                nextTabPos += 1.0f;
                continue;
            }
            
            glm::vec2 normalised = dif / distance;
            glm::vec2 tabPos = p0 + (tabPosAlongLine * normalised);
            
            // add tab position to vector //
            tabPositions.push_back(std::make_pair(i, tabPos));
            
            tabCount++;
            nextTabPos = tabSpacing * (float)(tabCount+1);
            // check tab is far enough away from p0 and p1
        }
        
        distanceAtLastPoint += distance;
        p0 = p1;
    }
    return move(tabPositions);
}

void GCodeBuilder::CheckForTab(Settings& settings, const CutPathParams& params, std::vector<std::pair<size_t, glm::vec2>> tabPositions, glm::vec2 pDif, float zCurrent, bool isMovingForward, int& tabIndex, size_t i) 
{
    if(!settings.p.pathCutter.CutTabs) {
        return; 
    }
    float& tabHeight  = settings.p.pathCutter.TabHeight;
    float& tabWidth   = settings.p.pathCutter.TabWidth;
    float tabZPos = params.z1 + tabHeight;
    
    auto addTab = [&]() {
        // get tab position
        glm::vec2& tabPosition = tabPositions[tabIndex].second;
        // calculate tab start / end
        float distance = hypotf(pDif.x, pDif.y);
        
        glm::vec2 normalised = pDif / distance;
        float toolRadius = settings.p.tools.toolList.CurrentItem().Diameter / 2.0f;
        glm::vec2 tabOffset = ((tabWidth / 2.0f) +  toolRadius) * normalised;
        glm::vec2 tabStart = tabPosition - tabOffset;
        glm::vec2 tabEnd = tabPosition + tabOffset;
        
        // start of tab
        Add(va_str("G1 X%.3f Y%.3f Z%.3f F%.0f", tabStart.x, tabStart.y, zCurrent, params.feedCutting));
        Add(va_str("G1 Z%.3f F%.0f\t(Start Tab)", tabZPos, params.feedCutting));
        // end of tab
        Add(va_str("G1 X%.3f Y%.3f F%.0f", tabEnd.x, tabEnd.y, params.feedCutting));
        Add(va_str("G1 Z%.3f F%.0f\t(End Tab)", zCurrent, params.feedCutting));
    };
    // continue if below top of tab
    if(zCurrent >= tabZPos)
        return;
        
    // if moving forward along path
    if(isMovingForward && (tabIndex < (int)tabPositions.size())) { 
        // add a tab if index matches with position
        while(tabPositions[tabIndex].first == i) {
            addTab();
            if(++tabIndex >= (int)tabPositions.size()) {
                break;
            }
        }
    } // if moving backward along path
    else if(!isMovingForward && (tabIndex >= 0)) {   
        // add a tab if index matches with position
        while(tabPositions[tabIndex].first == params.points->size()-i) {
            addTab();
            if(--tabIndex < 0) {
                break;
            }
        }
    }
}
    
int GCodeBuilder::CutPathDepths(Settings& settings, const CutPathParams& params) {
    
    // error check
    if(params.points->size() < 2) {
        Log::Error("There should be 2 or more points");
        return -1;
    }
    if(params.feedPlunge == 0.0f) {
        Log::Error("Plunge feedrate requires a value");
        return -1;
    }
    if(params.feedCutting == 0.0f) {
        Log::Error("Cutting feedrate requires a value");
        return -1;
    }
    // get the positions of where tabs should lie and their indexes within points[]
    std::vector<std::pair<size_t, glm::vec2>> tabPositions = GetTabPositions(settings, params);
    
    float zCurrent = params.z0;
    int zDirection = ((params.z1 - params.z0) > 0) ? FORWARD : BACKWARD; // 1 or -1
    bool isMovingForward = true;
    
    const std::vector<glm::vec2>* points = params.points;
        
    do {
        // retract then move to initial x, y position
        // if first run or requires retract for pocketing, move to safe z
        if((zCurrent == params.z0) || params.retract == RetractType::Full) {    //params.isLoop && (points->front() != points->back());
            RetractToZPlane();
            Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", (*points)[0].x, (*points)[0].y));
        }
        // or retract a small distance
        if(params.retract == RetractType::Partial) {
            Retract(settings.p.pathCutter.PartialRetractDistance);
            Add(va_str("G0 X%.3f Y%.3f\t(Move To Initial X & Y)", (*points)[0].x, (*points)[0].y));
        }
        // plunge to next z
        Add(va_str("G1 Z%.3f F%.0f\t(Move To Z)", zCurrent, params.feedPlunge));
        
        int tabIndex = (isMovingForward) ? 0 : tabPositions.size()-1;
        // Feed along path
        for (size_t i = 1; i < (*points).size(); i++) {
            // get start and end points of current line
            const glm::vec2& pLast = (isMovingForward) ? (*points)[i-1] : (*points)[points->size()-i];
            const glm::vec2& pNext = (isMovingForward) ? (*points)[i]   : (*points)[points->size()-i-1];
            // check for and draw tabs
            CheckForTab(settings, params, tabPositions, pNext-pLast, zCurrent, isMovingForward, tabIndex, i);
            // move to next point in linestring
            Add(va_str("G1 X%.3f Y%.3f F%.0f", pNext.x, pNext.y, params.feedCutting));
        }
        // reverse direction at end of linestring
        if(!params.isLoop) {
            isMovingForward = !isMovingForward;
        }
        // if we have reached the final z depth, break out of loop
        if(zCurrent == params.z1) {
            break;
        }
        // update z
        zCurrent += zDirection * fabsf(params.cutDepth);
        
        // if z zepth is further than final depth, adjust to final depth
        if((zDirection == FORWARD && zCurrent > params.z1) || (zDirection == BACKWARD && zCurrent < params.z1)) {
            zCurrent = params.z1;
        }
    } while(1);

    return 0;
}


} // end namespace Sqeak

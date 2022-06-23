#pragma once
#include "../common.h" 

namespace Sqeak { 
    
class GCodeBuilder
{
public:
    
    // TODO this should inlclude a tool & tab settings, this would prevent needing to take Settings&
    enum RetractType { Full, Partial, None };
    
    typedef struct {
        std::vector<glm::vec2>* points;
        float z0;
        float z1;
        float cutDepth;
        float feedPlunge;
        float feedCutting;
        bool isLoop = false;
        RetractType retract = RetractType::None;
    } CutPathParams;
    
    void Add(std::string gcode);
    void Clear();
    // move 
    void Retract(float distance = 1.0f);
    void RetractToZPlane();
    void MoveToHomePosition();
    void InitCommands(float spindleSpeed);
    void EndCommands();
        
    std::vector<std::string> Get() {
        return m_gcodes;
    };
    
    // executes length in one axis and then moves width of cutter in other axis
    void FacingCutXY(Settings& settings, glm::vec2 p0, glm::vec2 p1, bool isYFirst = false);
    // adds gcodes to cut a generic path or loop at depths between z0 and z1
    // moves to safe z position, then moves to p0
    // returns value on error
    int CutPathDepths(Settings& settings, const CutPathParams& params);

private:
    std::vector<std::string> m_gcodes;
        // determines the positions of tabs along path
    std::vector<std::pair<size_t, glm::vec2>> GetTabPositions(Settings& settings, const CutPathParams& params);
    void CheckForTab(Settings& settings, const CutPathParams& params, std::vector<std::pair<size_t, glm::vec2>> tabPositions, glm::vec2 pDif, float zCurrent, bool isMovingForward, int& tabIndex, size_t i);
};

 
} // end namespace Sqeak

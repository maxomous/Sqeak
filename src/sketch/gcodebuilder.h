#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <MaxLib.h>

namespace Sqeak { 

using namespace MaxLib::Geom;

class GCodeBuilder 
{
public:
    enum RetractType { Full, Partial };
    
    struct CutPathParams 
    {
        struct TabParameters {
            bool isActive                   = false;
            float spacing                   = 80; 
            float height                    = 5;
            float width                     = 5;
        } tabs;  
           
        struct DepthParameters {
            float zTop                      = 30;
            float zBottom                   = 0;
            float cutDepth                  = 2;
            float partialRetractDistance    = 5; // mm              // **Only used for pocketing
            RetractType retract             = RetractType::Full;    // **Only used for pocketing (should be on full on any other type)
        } depth;
        
        struct Tool {
            float diameter                  = 10;
            float feedPlunge                = 500;
            float feedCutting               = 1000;
        } tool;
    };
    
    // adds gcodes to cut a generic path or loop at depths between z0 and z1
    // moves to safe z position, then moves to p0
    // returns value on error
    int CutPathDepths(const std::vector<Vec2>& path, const CutPathParams& params, std::vector<std::pair<size_t, Vec2>> tabPositions);
    // determines the positions of tabs along path (see CutPathDepths())
    std::vector<std::pair<size_t, Vec2>> GetTabPositions(const std::vector<Vec2>& path, const CutPathParams& params);
    
    void Add(std::string gcode);
    void Clear();
    // move 
    void Retract(float distance);
    void RetractToZPlane();
    void MoveToHomePosition();
    void InitCommands(float spindleSpeed);
    void EndCommands();
        
    std::vector<std::string>& Output() {
        return m_gcodes;
    }
    
private:
    std::vector<std::string> m_gcodes;
    void CheckForTab(const std::vector<Vec2>& path, const CutPathParams& params, std::vector<std::pair<size_t, Vec2>> tabPositions, float zCurrent, bool isMovingForward, int& tabIndex, size_t i);

};

 
} // end namespace Sqeak

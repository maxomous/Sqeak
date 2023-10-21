#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <MaxLib.h>
#include "../libs/Geos.h"

namespace Sqeak { 
using namespace MaxLib::Geom;

class GCodeBuilder 
{
public:
    void Add(std::string gcode);
    void Clear();
    // move 
    void Retract(float distance);
    void RetractToZPlane();
    void MoveToHomePosition();
    void Initialisation(float spindleSpeed);
    void EndCommands();
        
    std::vector<std::string>& Output() {
        return m_gcodes;
    }
private:
    std::vector<std::string> m_gcodes;
};

class DepthCutter
{ 
public:
    
    struct Depth
    {
        float zTop                      = 30;  
        float zBottom                   = 0;
        float partialRetractDistance    = 1;
    } depth;
    
    struct TabParameters {
        bool isActive                   = false;
        float spacing                   = 80; 
        float height                    = 5;
        float width                     = 5;
    } tabs;  

    struct Tool {
        float diameter                  = 10;
        float cutDepth                  = 2;
        float feedCutting               = 1000;
        float feedPlunge                = 500;
    } tool;
    
    enum class RetractType { Full, Partial };

    
    // adds gcodes to cut a generic path or loop at depths between z0 and z1
    // moves up in z by retract distance (or to zMax if retract = 0), then moves to p0
    // returns value on error
    int CutPathDepths(GCodeBuilder& gcodes, const GeometryCollection& path, RetractType retractType = RetractType::Full);
    int CutPathDepths(GCodeBuilder& gcodes, const std::vector<Vec2>& path, RetractType retractType = RetractType::Full);
    
private:
    int ErrorCheckParameters();
    int ErrorCheckTabs();
//    // determines the positions of tabs along path (see CutPathDepths())
//    std::vector<std::pair<size_t, Vec2>> GetTabPositions(const std::vector<Vec2>& path, const CutPathParams& params);
//    // check if there is a tab at current index
//    void CheckForTab(const std::vector<Vec2>& path, const std::vector<std::pair<size_t, Vec2>>& tabPositions, float zCurrent, bool isMovingForward, int& tabIndex, size_t i);

};
     

// T can be Geom::LineString or Geom::Polygon

class PathCutter
{    
public:

    enum CompensateCutter { None, Left, Right, Pocket };
    
    struct PathData {
        GeometryCollection cutPath;     // offset / bore
        GeometryCollection finishPath;
        //struct TabPositions {
        //    std::vector<std::pair<size_t, Vec2>> pathTabs;
        //    std::vector<std::pair<size_t, Vec2>> finishPathTabs;
        //} tabPositions;
    };
    
    
    // CompensateCutter
    CompensateCutter cutSide = CompensateCutter::None;
    // width of the finishing pass
    float finishPass = 1.0f;
    // width of overlap between adjacent passes **only used for pocket
    float cutOverlap = 1.0f;
    
    
    // Calculate the Offset / Boring path
    template<typename T>
    PathData CalculatePaths(const T& inputPath, float toolRadius, GeosCPP::Operation::OffsetParameters geosParameters)
    {
        // Initialise geos (for geometry offseting)
        GeosCPP geos;
        // Container to hold the offset paths, later to be returned
        PathData outputPaths;
        // Cut only the input path, no finish pass
        if(cutSide == CompensateCutter::None) {
            outputPaths.cutPath.lineStrings = { inputPath };
            return outputPaths;
        } 
        // Compensate path by radius Left or Right
        else if((cutSide == CompensateCutter::Left) || (cutSide == CompensateCutter::Right))  {
            // perform an offset on the inputPath
            float offset = GetCutSide() * (fabsf(toolRadius) + fabsf(finishPass));
            outputPaths.cutPath = geos.operation.Offset(inputPath, offset, geosParameters);
        }
        // Boring Operation
        else if(cutSide == CompensateCutter::Pocket)  {
            // start a recursive loop of offsetting path until it fails
            float boringOffset = 2.0f * fabsf(toolRadius) - std::max(0.0f, cutOverlap); // offset distance per pass
           // outputPaths.cutPath = geos.operation.OffsetPolygon_Recursive(inputPath, boringOffset, geosParameters, true); // true is to reverse
        } 
        // Sanity check
        else { assert(0 && "Unknown cut side"); }
        
        // Finishing path (Calculate the path offset radius away from inputPath)
        if(finishPass != 0.0f) {                  
            outputPaths.finishPath = geos.operation.Offset(inputPath, GetCutSide() * fabsf(toolRadius), geosParameters);
        }
    }
    
    // 0 = no compensation, 1 = compensate left / pocket, -1 = compensate right
    int GetCutSide() 
    {
        if      (cutSide == (int)CompensateCutter::None)  { return 0; }
        else if (cutSide == (int)CompensateCutter::Right) { return -1; } 
        else    /* (Left || Pocket) */                    { return 1; }
    }
    
};

//              TABS
         
  //     auto CalculatePathTabs = [&]()
  //     {
  //         
  //         Vec2 p0 = { 100.0, 100.0 };
  //         Vec2 p1 = { 150.0, 200.0 };
  //         Vec2 pStart = (p0+p1) / 2.0;
  //         
  //         double distance = 50.0;
  //         int direction = 1;
  //         
  //         Vec2 pMid = PointPerpendicularToLine(p0, p1, direction * distance, pStart);
  //         
  //         
  //         
  //         
  //         tabPositions = GetTabPositions(inputPath, m_Params.cutPathParameters);
  //         
  //         
  //         cutSide * pathOffset
  //         cutSide * toolRadius
  //         
  //         
  //         // Calculate Tab positions (where tabs should lie and their indexes within path[])
  //         pathTabs = gcodes.GetTabPositions(path, m_Params.cutPathParameters);
  //         finishPathTabs = gcodes.GetTabPositions(finishPath, m_Params.cutPathParameters);
  //
  //     };
        

   // struct CutPathParams 
   // {
   //     struct TabParameters {
   //         bool isActive                   = false;
   //         float spacing                   = 80; 
   //         float height                    = 5;
   //         float width                     = 5;
   //     } tabs;  
   //        
   //     struct DepthParameters {
   //         float zTop                      = 30;
   //         float zBottom                   = 0;
   //         float cutDepth                  = 2;
   //         float partialRetractDistance    = 5; // mm              // **Only used for pocketing
   //         RetractType retract             = RetractType::Full;    // **Only used for pocketing (should be on full on any other type)
   //     } depth;
   //     
   //     struct Tool {
   //         float diameter                  = 10;
   //         float feedPlunge                = 500;
   //         float feedCutting               = 1000;
   //     } tool;
   // }; 
   
} // end namespace Sqeak


#pragma once
#include "common.h"


namespace Sqeak { 
    
class GCodeReader
{
public:
    enum XYZSetFlag {
        None = 0x00, 
        X    = 0x01 << 0,
        Y    = 0x01 << 1,
        Z    = 0x01 << 2
    };
    enum class CoordSystem { Machine, Local };
    enum class MotionType { Absolute /*G90*/, Incremental /*G91*/ };
    enum class Plane { XY /*G17*/, XZ /*G18*/, YZ /*G19*/ };
    
    GCodeReader(Settings& settings);
    
    int OpenVector(std::vector<std::string>& gcodes);
    int OpenFile(const std::string& filePath);
    std::vector<Vec3>& GetVertices()   { return m_Vertices; }
    std::vector<Vec3>& GetColours()    { return m_Colours; }
private:
    Settings& m_Settings;

    std::vector<Vec3> m_Vertices;
    std::vector<Vec3> m_Colours;
    
    Vec3 m_Colour; 
    // modal values
    Vec3 m_MPos;
    Vec3 m_WPos;
    float m_G_Val = 0.0f;               // value to execute at end of block (Motion Commands Only, 1 only
    Plane m_Plane = Plane::XY; 
    MotionType m_MotionType = MotionType::Absolute;
    CoordSystem m_MotionCoordSys = CoordSystem::Local; // for non-modal G53 command
    
    Vec3 m_WCO; // sum of these 3:
    Vec3 m_CoordSystem;
    Vec3 m_G92Offset;
    Vec3 m_ToolLengthOffset;
    
    // letter values
    Vec3 m_XYZ;
    XYZSetFlag m_XYZ_Set;
    Vec3 m_IJK;
    float m_R;
    
    bool m_Execute;     // true when XYZ, F or IJK changed - used when g code is ommited, canned cycles 
    
    // Add a vertex to the vertex array
    void AddVertex(Vec3 p, CoordSystem coordSys = CoordSystem::Local);

    void Reset();
    
    void UpdateModalValues();
    
    int ReadFile(std::string filepath);
    
    void cleanString(std::string& str);
    // Computes a single line of GCode
    int InputGCodeBlock(std::string& inputString);
    
    int ExecuteGCode(float gValue);

    void UpdateWCO();
    // Get the Absolute position of p if incremental
    Vec3 GetAbsoluteWPos(Vec3 p);
    // G43.1
    void SetToolLength(bool update = true);
    // G92
    void SetG92Offset(bool update = true);
    // G54-G59
    int SetCoordSystem(uint coordSys);
    // G28/G30
    void ReturnToHome(int homePos);
    // G0/G1
    void MotionLinear();
    // G2/G3
    void MotionArc(MaxLib::Geom::Direction dir); 
    // returns point relative to selected plane (set convertDirection to -1 to reverse)
    Vec3 PointRelativeToPlane(Vec3 p, Plane plane, int convertDirection = 1);
    // reverse of above
    Vec3 ReversePointRelativeToPlane(Vec3 p, Plane plane);
    // set current colour for path
    void SetPathColour(float gValue);
};

} // end namespace Sqeak

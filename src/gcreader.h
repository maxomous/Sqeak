
#pragma once



#define XYZ_SET_FLAG_NONE     0x0
#define XYZ_SET_FLAG_X        0x1 << 0
#define XYZ_SET_FLAG_Y        0x1 << 1
#define XYZ_SET_FLAG_Z        0x1 << 2

class GCodeReader
{
public:
    enum CoordSystem { Machine, Local };
    enum MotionType { Absolute /*G90*/, Incremental /*G91*/ };
    enum Plane { XY /*G17*/, XZ /*G18*/, YZ /*G19*/ };
    
    GCodeReader(GRBLVals& grblVals);
    
    int OpenVector(std::vector<std::string>& gcodes);
    int OpenFile(const std::string& filePath);
    std::vector<glm::vec3>& GetVertices() { return m_Vertices; }
    std::vector<uint>& GetIndices() { return m_Indices; }
private:
    GRBLVals& m_GrblVals;

    std::vector<glm::vec3> m_Vertices;
    std::vector<uint> m_Indices;
    
    // modal values
    glm::vec3 m_MPos;
    glm::vec3 m_WPos;
    float m_G_Val = 0.0f;               // value to execute at end of block (Motion Commands Only, 1 only
    Plane m_Plane = Plane::XY; 
    MotionType m_MotionType = MotionType::Absolute;
    CoordSystem m_MotionCoordSys = CoordSystem::Local; // for non-modal G53 command
    
    glm::vec3 m_WCO; // sum of these 3:
    glm::vec3 m_CoordSystem;
    glm::vec3 m_G92Offset;
    glm::vec3 m_ToolLengthOffset;
    
    // letter values
    glm::vec3 m_XYZ;
    int m_XYZ_Set;
    glm::vec3 m_IJK;
    float m_R;
    
    bool m_Execute;     // true when XYZ, F or IJK changed - used when g code is ommited, canned cycles 
    
    
    // Add a vertex to the vertex array
    void AddVertex(glm::vec3 p, CoordSystem coordSys = CoordSystem::Local);

    void Reset();
    
    void UpdateModalValues();
    
    int ReadFile(std::string filepath);
    
    void cleanString(std::string& str);
    // Computes a single line of GCode
    int InputGCodeBlock(std::string& inputString);
    
    int ExecuteGCode(float gValue);

    void UpdateWCO();
    // Get the Absolute position of p if incremental
    glm::vec3 GetAbsoluteWPos(glm::vec3 p);
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
    void MotionArc(int direction); 
    // returns point relative to selected plane (set convertDirection to -1 to reverse)
    glm::vec3 PointRelativeToPlane(glm::vec3 p, Plane plane, int convertDirection = 1);
    // reverse of above
    glm::vec3 ReversePointRelativeToPlane(glm::vec3 p, Plane plane);
    // calculates centre from radius, start & end points (-r will return the second possible arc)
    glm::vec2 ArcCentreFromRadius(glm::vec2 p0, glm::vec2 p1, float r, int direction);
};


#include "common.h" 
using namespace std;


GCodeReader::GCodeReader(Settings& settings)
    : m_Settings(settings) {
    // set initial colour to rapid (G0)
    SetPathColour(0.0f);
}

// adds vertices in machine coords
void GCodeReader::AddVertex(glm::vec3 p, CoordSystem coordSys)
{
    glm::vec3 vertex = (coordSys == CoordSystem::Machine) ? p : p + m_WCO;
    // prevent duplicate vertices
    if(!m_Vertices.empty()) {
        if(vertex == m_Vertices.back()) 
            return;
    }
    m_Vertices.emplace_back(vertex);
    m_Colours.emplace_back(m_Colour);
    m_MPos = vertex;
    m_WPos = m_MPos - m_WCO;
    
    Log::Debug(DEBUG_GCREADER, "Adding Vertex: (%g, %g, %g)\tColour: (%g, %g, %g)", vertex.x, vertex.y, vertex.z, m_Colour.x, m_Colour.y, m_Colour.z); 
}


void GCodeReader::Reset()
{   // reset
    UpdateModalValues();
    // clear vertices array
    m_Vertices.clear();
    m_Colours.clear();
}


int GCodeReader::OpenVector(vector<string>& gcodes)
{
    Reset();
    
    AddVertex(m_MPos, CoordSystem::Machine);
    
    for (uint i = 0; i < gcodes.size(); i++) {
        int err = InputGCodeBlock(gcodes[i]);
        if(err) return err;
    }
    return 0;
}

int GCodeReader::OpenFile(const string& filePath)
{
    Reset();
    
    AddVertex(m_MPos, CoordSystem::Machine);
    
    if(ReadFile(filePath)) {
        Log::Error("Couldn't open GCode file");
        return -1;
    }
    return 0;
}

void GCodeReader::UpdateModalValues()
{
    GRBLStatus_vals& s = m_Settings.grblVals.status;
    
    m_MPos = s.MPos;
    
    GRBLModal_vals& m = m_Settings.grblVals.modal;
    // motion mode
    if(m.MotionMode == 0.0f || m.MotionMode == 1.0f || m.MotionMode == 2.0f || m.MotionMode == 3.0f)
        m_G_Val = m.MotionMode;
    else
        Log::Error("Unknown motion mode");
        
    // plane
    if(m.Plane >= 17 && m.Plane <= 19)
        m_Plane = (Plane)(m.Plane - 17);
    else
        Log::Error("Unknown plane");
        
    // motion type
    if(m.DistanceMode == 90 || m.DistanceMode == 91)
        m_MotionType = (MotionType)(m.DistanceMode - 90);
    else
        Log::Error("Unknown distance type");
    
    // coord system
    if(m.CoordinateSystem >= 54 && m.CoordinateSystem <= 59)
        SetCoordSystem(m.CoordinateSystem - 54);
    else
        Log::Error("Unknown coordinate system");
        
    GRBLCoords_vals& coords = m_Settings.grblVals.coords;
    
    m_G92Offset = coords.offsetCoords;
    m_ToolLengthOffset = { 0.0f, 0.0f, coords.toolLengthOffset };
    
    UpdateWCO();
    
    
    m_Execute = false;
}

int GCodeReader::ReadFile(string filepath)
{
    auto executeLine = [this](std::string& str) {
        //Log::Info("GCode Viewer Reading = %s", str.c_str());
        return (str == "") ? 0 : InputGCodeBlock(str);
    }; 
        
    if(File::Read(filepath, executeLine)) {
        Log::Error(std::string("Could not open file ") + filepath);
        return -1;
    }
   
    return 0;
}

// THIS SHOUDL BE COMBINED WITH gclist
// takes a GCode linbe and cleans up
// (removes spaces, comments, make uppercase, ensures '\n' is present)
void GCodeReader::cleanString(std::string& str) 
{
    // remove comments within () 
    size_t a = str.find("(");
    if(a != std::string::npos) {
    size_t b = str.find(")", a);
    if(b != std::string::npos)
        str.erase(a, b-a+1);
    }
    // remove comments after ;
    size_t c = str.find(";");
    if(c != std::string::npos)
    str.erase(c, str.length()-c );
    // make all uppercase
    upperCase(str);
    // strip out whitespace & out non printable characters- this means we can fit more in the buffer
    str.erase(remove_if(str.begin(), str.end(), [](char c){ return !isgraph(c); }), str.end());
    // add a newline character to end
    str.append("\n");
}

// Input G Code Block function
// Computes a single line of GCode
int GCodeReader::InputGCodeBlock(string& inputString)
{        
    cleanString(inputString);
    if(inputString.empty() || inputString == "\n")
        return 0;
        
    Log::Debug(DEBUG_GCREADER, "Input: %s", inputString.c_str());   
        
    // reset values
    float m_G_Val_NonModal    = 0.0f;
    m_IJK               = glm::vec3();
    m_R                 = 0;
    m_XYZ_Set           = XYZ_SET_FLAG_NONE;
    m_XYZ = (m_MotionType == MotionType::Incremental) ? glm::vec3(0.0f, 0.0f, 0.0f) : m_WPos;
    m_MotionCoordSys = CoordSystem::Local;
    
    istringstream stream(inputString);
    char letter;
    float value;
    
    while(stream >> letter && stream >> value) 
    {
        switch (letter)
        {
            case 'G' : 
                // non modal motion commands
                if(value == 28.0f || value == 30.0f || value == 43.1f || value == 92.0f) {
                    m_G_Val_NonModal = value;
                    m_Execute = true; 
                }
                // modal motion commands
                else if(value == 0.0f || value == 1.0f || value == 2.0f || value == 3.0f) {
                    m_G_Val = value;
                    m_Execute = true; 
                } // otherwise execute now (other non-modal commands)
                else {
                    int err = ExecuteGCode(value);
                    if(err) return err;
                }
                break;
                
            case 'I' :
                m_IJK.x = value;
                m_Execute = true; 
                break;
            
            case 'J' :
                m_IJK.y = value;
                m_Execute = true; 
                break;
        
            case 'K' :
                m_IJK.z = value;
                m_Execute = true; 
                break;
        
            case 'R' :
                m_R = value;
                m_Execute = true; 
                break;

            case 'X' :
                m_XYZ.x = value;
                m_Execute = true; 
                m_XYZ_Set |= XYZ_SET_FLAG_X;
                break;
        
            case 'Y' :
                m_XYZ.y = value;
                m_Execute = true; 
                m_XYZ_Set |= XYZ_SET_FLAG_Y;
                break;
        
            case 'Z' :
                m_XYZ.z = value;
                m_Execute = true;
                m_XYZ_Set |= XYZ_SET_FLAG_Z;
                break;
            
            case 'M' :
            case 'L' :
            case 'F' :         
            case 'H' :           
            case 'N' :
            case 'D' :
            case 'P' :
            case 'Q' :
            case 'S' :
            case 'T' :
                break;
                
            default:
                Log::Error("Illegal letter: %c", letter);
                return -1;
        }
    }

    // if G has a value   and if X or Y or Z or i/j/k or p has changed 
    if(m_Execute) {
        m_Execute = false;    // has XYZ, F or IJK changed
        int err = 0;
        if(m_G_Val_NonModal) {
            err = ExecuteGCode(m_G_Val_NonModal);
            m_G_Val_NonModal = 0.0f;
        } else {
            err = ExecuteGCode(m_G_Val);
        }   
        return err;
    }

    return 0;
}
   
/* grbl supports:
    Modal Group	Member Words
    Motion Mode	G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
    Coordinate System Select	G54, G55, G56, G57, G58, G59
    Plane Select	G17, G18, G19
    Distance Mode	G90, G91
    Arc IJK Distance Mode	G91.1
    Feed Rate Mode	G93, G94
    Units Mode	G20, G21
    Cutter Radius Compensation	G40
    Tool Length Offset	G43.1, G49
    Program Mode	M0, M1, M2, M30
    Spindle State	M3, M4, M5
    Coolant State	M7, M8, M9
    Supported Non-Modal Commands
    G4, G10 L2, G10 L20, G28, G30, G28.1, G30.1, G53, G92, G92.1
*/
int GCodeReader::ExecuteGCode(float gValue)
{
    Log::Debug(DEBUG_GCREADER, "XYZ Input = (%g, %g, %g)", m_XYZ.x, m_XYZ.y, m_XYZ.z);
    
    SetPathColour(gValue);
    
    if(gValue == 0.0f || gValue == 1.0f)
        MotionLinear();
    else if(gValue == 2.0f)
        MotionArc(CLOCKWISE);
    else if(gValue == 3.0f)
        MotionArc(ANTICLOCKWISE);
        
    else if(gValue == 4.0f) 
        {}  // ignore dwell
    
    else if(gValue == 10.0f) 
        Log::Warning("G10 (Set coord system) is not supported in GCode Viewer");
        
    else if(gValue == 17.0f)
        m_Plane = Plane::XY;
    else if(gValue == 18.0f)
        m_Plane = Plane::XZ;
    else if(gValue == 19.0f)
        m_Plane = Plane::YZ;
    
    else if(gValue == 20.0f) {
        Log::Error("G20 (Set units to inches) is not supported in GCode Viewer");
        return -1;  
    }
    else if(gValue == 21.0f) 
        {}  // ignore units mm
    
    else if(gValue == 28.0f)
        ReturnToHome(28);
    else if(gValue == 30.0f)
        ReturnToHome(30);
    
    else if(gValue == 38.2f || gValue == 38.3f || gValue == 38.4f || gValue == 38.5f) 
        Log::Warning("G38.x (Probe) is not supported in GCode Viewer");
    
    else if(gValue == 40.0f || gValue == 41.0f || gValue == 42.0f) 
        Log::Warning("G40/G41/G42 (Cutter compensation) is not supported in GCode Viewer");
        
    else if(gValue == 43.1f)
        SetToolLength();
    else if(gValue == 49.0f)
        SetToolLength(false);
        
    else if(gValue == 53.0f)
        m_MotionCoordSys = CoordSystem::Machine;
        
    else if(gValue == 54.0f)
        return SetCoordSystem(0);
    else if(gValue == 55.0f)
        return SetCoordSystem(1);
    else if(gValue == 56.0f)
        return SetCoordSystem(2);
    else if(gValue == 57.0f)
        return SetCoordSystem(3);
    else if(gValue == 58.0f)
        return SetCoordSystem(4);
    else if(gValue == 59.0f)
        return SetCoordSystem(5);
        
    else if(gValue == 80.0f) 
        {}  // ignore canned cycle
    
    else if(gValue == 90.0f) 
        m_MotionType = MotionType::Absolute;
    else if(gValue == 91.0f) 
        m_MotionType = MotionType::Incremental;
    else if(gValue == 91.1f) 
        {}  // ignore Incremental IJK (this is always true)
    
    
    else if(gValue == 92.0f)
        SetG92Offset();
    else if(gValue == 92.1f)
        SetG92Offset(false);
    
    else if(gValue == 93.0f) 
        {}  // ignore Inverse feed mode
    else if(gValue == 94.0f) 
        {}  // ignore Feed Per Minute
    
    else {
        Log::Error("G%d (Unknown) is not supported in GCode Viewer", gValue);
        return -1;
    }
    
    return 0;
}

void GCodeReader::UpdateWCO()
{
    m_WCO = m_CoordSystem + m_G92Offset + m_ToolLengthOffset;
    Log::Debug(DEBUG_GCREADER, "WCO = (%g, %g, %g)", m_WCO.x, m_WCO.y, m_WCO.z);
    m_WPos = m_MPos - m_WCO;
}
    
glm::vec3 GCodeReader::GetAbsoluteWPos(glm::vec3 p)
{
    if(m_MotionType == MotionType::Incremental) 
        return p + m_WPos;
    
    // else if absolute
    
    if(m_MotionCoordSys == CoordSystem::Local)
        return p;
        
    // else if move in machine coords (G53)
    
    return p - m_WCO;
}


void GCodeReader::SetToolLength(bool update)
{
    if(!update) {
        m_ToolLengthOffset = { 0.0f, 0.0f, 0.0f };
    } 
    else {
        if(m_XYZ_Set & XYZ_SET_FLAG_Z) {
            m_ToolLengthOffset.z = m_XYZ.z;
        }
    }
    UpdateWCO();
}

void GCodeReader::SetG92Offset(bool update)
{
    if(!update) {
        m_G92Offset = { 0.0f, 0.0f, 0.0f };
    } 
    else {
        glm::vec3 offset = { 0.0f, 0.0f, 0.0f };
        glm::vec3 currentPosPreOffset = m_WPos - m_G92Offset;
        
        if(m_XYZ_Set & XYZ_SET_FLAG_X) {
            offset.x = currentPosPreOffset.x - m_XYZ.x;
        }
        if(m_XYZ_Set & XYZ_SET_FLAG_Y) {
            offset.y = currentPosPreOffset.y - m_XYZ.y;
        }
        if(m_XYZ_Set & XYZ_SET_FLAG_Z) {
            offset.z = currentPosPreOffset.z - m_XYZ.z;
        }
        m_G92Offset = offset;
    }
    UpdateWCO();
}

int GCodeReader::SetCoordSystem(uint coordSys)
{
    if(coordSys > 5) {
        Log::Error("Unknown Coord System: %u", coordSys);
        return -1;
    }
    
    GRBLCoords_vals& coords = m_Settings.grblVals.coords;
    m_CoordSystem = coords.workCoords[coordSys];
    
    UpdateWCO();
    return 0;
}

void GCodeReader::ReturnToHome(int homePos) {

    if(homePos != 28 && homePos != 30) {
        Log::Critical("Not a valid home position: %d", homePos);
    } 
    
    GRBLCoords_vals& coords = m_Settings.grblVals.coords;
    size_t index = (homePos == 28) ? 0 : 1;
    
    if(m_XYZ_Set == XYZ_SET_FLAG_NONE) {
        glm::vec3 p = coords.homeCoords[index];
        AddVertex(p, CoordSystem::Machine);
    }
    else {
        glm::vec3 pFirst = m_WPos;
        glm::vec3 pSecond = m_WPos;
        glm::vec3 p_XYZ = GetAbsoluteWPos(m_XYZ);
        
        if(m_XYZ_Set & XYZ_SET_FLAG_X) {
            pFirst.x = p_XYZ.x;
            pSecond.x = coords.homeCoords[index].x - m_WCO.x;
        }
        if(m_XYZ_Set & XYZ_SET_FLAG_Y) {
            pFirst.y = p_XYZ.y;
            pSecond.y = coords.homeCoords[index].y - m_WCO.y;
        }
        if(m_XYZ_Set & XYZ_SET_FLAG_Z) {
            pFirst.z = p_XYZ.z;
            pSecond.z = coords.homeCoords[index].z - m_WCO.z;
        }
        
        AddVertex(pFirst, CoordSystem::Local);
        AddVertex(pSecond, CoordSystem::Local);
    } 
}

/*----------------------------------------------------------------------------
G00 / G01
*----------------------------------------------------------------------------*/
void GCodeReader::MotionLinear()
{
    glm::vec3 p_XYZ = GetAbsoluteWPos(m_XYZ);
    AddVertex(p_XYZ);
}


/*----------------------------------------------------------------------------
*    G Code:         G02 / G03
*    Description:    Clockwise / Anticlockwise Arc to Endpoint with Radius
* 
*    Input:            x y z - End point        r - radius        f - feed rate
*             If r is negative, it will produce the 2nd version of the curve 
*                 (when the centrepoint is on the opposite side of the midway line between start and end) 
*
*                             *OR*
*
*     Input:            x y z - End point        i j k - Centre point        f - feed rate
*              A full circle can be produced using IJK only
*                 (leave XYZ values unchanged)
* 
*     Plane selection (G17/18/19) is possible for G02 & G03 
*----------------------------------------------------------------------------*/
void GCodeReader::MotionArc(int Direction)
{
    // get start & end coords relative to the selected plane 
    glm::vec3 xyz_Start   = PointRelativeToPlane(m_WPos, m_Plane);
    glm::vec3 xyz_End     = PointRelativeToPlane(GetAbsoluteWPos(m_XYZ), m_Plane);
    
    glm::vec2 xy_Start = { xyz_Start.x, xyz_Start.y }; 
    glm::vec2 xy_End   = { xyz_End.x, xyz_End.y };
    
    // flip direction if XZ Plane
    int direction = (m_Plane == Plane::XZ) ? -Direction : Direction;
     
    glm::vec2 xy_Centre;
    float r;
    
    if(m_R) { // r
        // calculate centre from the radius, start & end points
        point2D centre = Geom::ArcCentreFromRadius(point2D(xy_Start), point2D(xy_End), m_R, direction);
        xy_Centre = { centre.x, centre.y };
        // -r is the second (larger) version of the arc
        r = fabsf(m_R);
    } 
    else { // ijk
        // ijk is always incremental (G90/G91 doesnt matter) 
        xy_Centre = PointRelativeToPlane(m_WPos + m_IJK, m_Plane);
        // calculate r
        glm::vec2 dif = xy_End - xy_Centre;
        r = hypotf(dif.x, dif.y);
    }
    
    glm::vec2 v_Start = xy_Start - xy_Centre;
    glm::vec2 v_End = xy_End - xy_Centre;
    
    double th_Start  = atan2(v_Start.x, v_Start.y);
    double th_End    = atan2(v_End.x, v_End.y);
    
    Geom::CleanAngles(th_Start, th_End, direction);
    
    float th_Incr   = direction * deg2rad(5);
    
    int nIncrements = floorf(fabsf((th_End - th_Start) / th_Incr));
    float zIncrement = (xyz_End.z - xyz_Start.z) / nIncrements;
    
    glm::vec3 p;
    for (int n = 0; n < nIncrements; n++) {
        float th = th_Start + n * th_Incr;
        p.x = xy_Centre.x + r * sin(th);
        p.y = xy_Centre.y + r * cos(th);
        p.z = xyz_Start.z + n * zIncrement;
        glm::vec3 v = PointRelativeToPlane(p, m_Plane, -1);
        AddVertex(v);
    }
    
    AddVertex(GetAbsoluteWPos(m_XYZ));
} 

 
//convert point relative to plane selected
glm::vec3 GCodeReader::PointRelativeToPlane(glm::vec3 p, Plane plane, int convertDirection)
{    
	if (plane == Plane::XY) {
        return { p.x, p.y, p.z };
    }
	else if (plane == Plane::XZ) {
        return { p.x, p.z, p.y };
    }
	else {//if (plane == Plane::YZ) 
        if(convertDirection == -1) {
            return { p.z, p.x, p.y };
        } else {
            return { p.y, p.z, p.x };
        }
    }
}
void GCodeReader::SetPathColour(float gValue) 
{   
    glm::vec3 p0 = m_WPos;
    glm::vec3 p1 = GetAbsoluteWPos(m_XYZ);
    
    // rapid
    if(gValue == 0.0f) {
        m_Colour = m_Settings.p.viewer.toolpath.Colour_Rapid;
    } // z feed 
    else if (gValue == 1.0f && (p0.x == p1.x && p0.y == p1.y)) {
        m_Colour = m_Settings.p.viewer.toolpath.Colour_FeedZ;
    } // feed 
    else if (gValue == 1.0f || gValue == 2.0f || gValue == 3.0f) {
        m_Colour = m_Settings.p.viewer.toolpath.Colour_Feed;
    } // home 
    else if (gValue == 28.0f || gValue == 30.0f) {
        m_Colour = m_Settings.p.viewer.toolpath.Colour_Home;
    }
}

  /*
 * grbl.cpp
 *  Max Peglar-Willis 2021
 */    

#include "common.h" 
using namespace std;
using namespace MaxLib;
 

namespace Sqeak { 

// returns a copy of vals
const GRBLCoords_vals GRBLCoords::getVals() { 
    std::lock_guard<std::mutex> guard(m_mutex);
    return m_vals;
} 

// returns a copy of vals
const GRBLModal_vals GRBLModal::getVals() { 
    std::lock_guard<std::mutex> guard(m_mutex);
    return m_vals;
}

bool GRBLStatus::isCheckMode() {
    return (getVals().state == GRBLState::Status_Check);
}

// returns a copy of vals
const GRBLStatus_vals GRBLStatus::getVals() { 
    std::lock_guard<std::mutex> guard(m_mutex);
    return m_vals;
}

//- `Hold:0` Hold complete. Ready to resume.
//- `Hold:1` Hold in-progress. Reset will throw an alarm.
//- `Door:0` Door closed. Ready to resume.
//- `Door:1` Machine stopped. Door still ajar. Can't resume until closed.
//- `Door:2` Door opened. Hold (or parking retract) in-progress. Reset will throw an alarm.
//- `Door:3` Door closed and resuming. Restoring from park, if applicable. Reset will throw an alarm.

// returns value 
const std::string GRBLStatus::stateStr(GRBLState state)
{
    switch (state)
    {
    case GRBLState::Status_Idle:
        return "Idle";
    case GRBLState::Status_Hold0:
        return "Hold (Ready)";
    case GRBLState::Status_Hold1:
        return "Hold (Busy)";
    case GRBLState::Status_Sleep:
        return "Sleep";
    case GRBLState::Status_Run:
        return "Running";
    case GRBLState::Status_Jog:
        return "Jogging";
    case GRBLState::Status_Check:
        return "Check Mode";
    case GRBLState::Status_Home:
        return "Homimg";
    case GRBLState::Status_Alarm:
        return "Alarm";
    case GRBLState::Status_Door0:
        return "Door (Ready)";
    case GRBLState::Status_Door1:
        return "Door 1";
    case GRBLState::Status_Door2:
        return "Door 2";
    case GRBLState::Status_Door3:
        return "Door 3";
    case GRBLState::Status_Unknown:
        return "Unknown";
    default:
        Log::Error("Unknown Grbl state");
    }
    return "";
}
// for externally setting the state
void GRBLStatus::setState(GRBLState state) { 
    std::lock_guard<std::mutex> guard(m_mutex);
    m_vals.state = state; 
}
    
// returns a copy of vals
const GRBLSettings_vals GRBLSettings::getVals() { 
    GRBLSettings_vals vals;
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        vals = m_vals;
    }
    vals.min_SpindleSpeed = (int)vals.RawValues[31];
    vals.max_SpindleSpeed = (int)vals.RawValues[30];
    vals.max_FeedRateX = vals.RawValues[110];
    vals.max_FeedRateY = vals.RawValues[111];
    vals.max_FeedRateZ = vals.RawValues[112];
    vals.max_FeedRate = std::max(std::max(vals.max_FeedRateX, vals.max_FeedRateY), vals.max_FeedRateZ);
    
    return move(vals);
}
// sets units to mm or inches
void GRBLSettings::setUnitsInches(bool isInches) 
{
    std::lock_guard<std::mutex> guard(m_mutex);
    
    if(isInches) {
        m_vals.units_Distance    = "in";
        m_vals.units_Feed    = "in/min";
    }
    else /*mm*/ {
        m_vals.units_Distance    = "mm";
        m_vals.units_Feed    = "mm/min";
    }    
}


    
// checks Startup Line Execution for error    msg = ">G54G20:ok" or ">G54G20:error:X"
// it is very unlikely that there will be an error as this is checked before it is saves onto the eeprom
void GRBLSystem::checkStartupLine(const std::string& msg) 
{
    // retrieve position of :
    size_t a = msg.find(":");
    // checks to prevent errors
    if(a == std::string::npos) {
    Log::Critical("Something is not right here, didn't find ':'");
    return;
    }
    
    // (***add this if we change any data in future***) lock the mutex 
    //std::lock_guard<std::mutex> guard(m_mutex);
    {
        // retrieve value of response ('ok' or 'error:x')
        std::string response = msg.substr(a+1, msg.length()-a-1);
        if(response == "ok")
            return;
            
        size_t b = response.find(":");
        if(b != std::string::npos) {
            int errCode = stoi(response.substr(b+1));
            Log::Critical("Startup Line Execution has encountered an error: %d", errCode);
        }
        else
            Log::Critical("Something is not right here, didn't find 2nd ':'");
    } 
}    

// decodes GCode Parameters
// and stores inside grbl Parameters
void GRBLSystem::decodeCoords(const std::string& msg) 
{
    // get name i.e 'G54'
    std::string param = msg.substr(1, 3);
    // get number std::string i.e. '4.000,0.000,0.000'
    std::string num = msg.substr(5, msg.length()-6);
    // lock the mutex
    std::lock_guard<std::mutex> guard(coords.m_mutex);
    // [G54:4.000,0.000,0.000] - [G59:4.000,0.000,0.000]
    if(!param.compare("G54")) 
        coords.m_vals.workCoords[0] = stoVec3(num);
    else if(!param.compare("G55")) 
        coords.m_vals.workCoords[1] = stoVec3(num);
    else if(!param.compare("G56")) 
        coords.m_vals.workCoords[2] = stoVec3(num);
    else if(!param.compare("G57")) 
        coords.m_vals.workCoords[3] = stoVec3(num);
    else if(!param.compare("G58")) 
        coords.m_vals.workCoords[4] = stoVec3(num);
    else if(!param.compare("G59")) 
        coords.m_vals.workCoords[5] = stoVec3(num);
    // [G28:1.000,2.000,0.000]  / [G30:4.000,6.000,0.000]
    else if(!param.compare("G28")) 
        coords.m_vals.homeCoords[0] = stoVec3(num);
    else if(!param.compare("G30")) 
        coords.m_vals.homeCoords[1] = stoVec3(num);
    // [G92:0.000,0.000,0.000]
    else if(!param.compare("G92")) 
        coords.m_vals.offsetCoords = stoVec3(num);
    // [TLO:0.000]    
    else if(!param.compare("TLO")) 
        coords.m_vals.toolLengthOffset = stof(num);
    // [PRB:0.000,0.000,0.000:0]
    else if(!param.compare("PRB")) {
        coords.m_vals.probeOffset = stoVec3(msg.substr(5, msg.length()-8));
        coords.m_vals.probeSuccess = (bool)stoi(msg.substr(msg.length()-2, 1));
    }
    else {
        Log::Error("Something's not right... Parameter unrecognised: %s", msg.c_str());
    } 
    
}
// decodes the startup block
void GRBLSystem::decodeStartupBlock(const std::string& msg) 
{
    int blockNum = stoi(msg.substr(2, 1));
    {     // lock the mutex
        std::lock_guard<std::mutex> guard(modal.m_mutex);
        if(blockNum == 0 || blockNum == 1)
            modal.m_vals.StartupBlock[blockNum] = msg.substr(4);
        else {
            Log::Error("Something's not right... Startup block number unrecognised: %s", msg.c_str());
        }
    }
}
// decodes modal groups
// and stores inside grbl Parameters
void GRBLSystem::decodeMode(const std::string& msg) 
{
    std::string s = msg.substr(4, msg.length()-5);
    std::stringstream stream(s);
    
    // lock the mutex
    std::lock_guard<std::mutex> guard(modal.m_mutex);
    
    do {
        std::string code;
        stream >> code;
    
        if(code == "")
            continue; 
        float value = stof(code.substr(1));
        // [GC:G0 G54 G17 G21 G90 G94 M0 M5 M9 T0 S0.0 F500.0]
        if(code == "G0" || code == "G1" || code ==  "G2" || code ==  "G3" || code ==  "G38.2" || code ==  "G38.3" || code ==  "G38.4 "|| code ==  "G38.5" || code ==  "G80")
            modal.m_vals.MotionMode = value;
        else if(code == "G54" || code == "G55" || code == "G56" || code == "G57" || code ==  "G58" || code == "G59")
            modal.m_vals.CoordinateSystem = (uint)value;
        else if(code == "G17" || code == "G18" || code == "G19")
            modal.m_vals.Plane = (uint)value;
        else if(code == "G90" || code == "G91")
            modal.m_vals.DistanceMode = (uint)value;
        else if(code == "G91.1")
            modal.m_vals.ArcIJKDistanceMode = value;
        else if(code == "G93" || code == "G94")
            modal.m_vals.FeedRateMode = (uint)value;
        else if(code == "G20" || code == "G21")
            modal.m_vals.UnitsMode = (uint)value;
        else if(code == "G40")
            modal.m_vals.CutterRadCompensation = (uint)value;
        else if(code == "G43.1" || code == "G49")
            modal.m_vals.ToolLengthOffset = value;
        else if(code == "M0" || code == "M1" || code == "M2" || code == "M30")
            modal.m_vals.ProgramMode = (uint)value;
        else if(code == "M3" || code == "M4" || code == "M5")
            modal.m_vals.SpindleState = (uint)value;
        else if(code == "M7" || code == "M8" || code == "M9")
            modal.m_vals.CoolantState = (uint)value;
        else if(code.compare(0, 1, "T"))
            modal.m_vals.toolNumber = (uint)value;
        else if(code.compare(0, 1, "S"))
            modal.m_vals.spindleSpeed = value;
        else if(code.compare(0, 1, "F"))
            modal.m_vals.feedRate = value;
        else {
            Log::Error("Something's not right... Mode unrecognised: %s", code.c_str());
        }
    } while (stream);
}


// decodes status response 
// and stores inside grbl Parameters
// The $10 status report mask setting can alter what data is present and certain data fields can be reported intermittently (see descriptions for details.)
// The $13 report inches settings alters the units of some data values. $13=0 false indicates mm-mode, while $13=1 true indicates inch-mode reporting.
// "<Idle|WPos:828.000,319.000,49.100|FS:0,0|Pn:PXYZ>"
void GRBLSystem::decodeStatus(const std::string& msg) 
{

    std::stringstream stream(msg.substr(1, msg.length()-2));
    // reserve enough space for all possible segments
    static std::vector<std::string> segs;
    segs.clear();
    segs.reserve(16);

    static std::string segment(32, 0);
    // iterate backwards through segs[i]s
    // this is to ensure WCO can be calculated after MPos / WPos
    while(getline(stream, segment, '|')) {
        segs.emplace_back(segment);
    }
    
        // lock the mutex
    std::lock_guard<std::mutex> guard(status.m_mutex);

    for (int i = segs.size()-1; i >= 0; i--) 
    {
        // Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep
        //- `Hold:0` Hold complete. Ready to resume.
        //- `Hold:1` Hold in-progress. Reset will throw an alarm.
        //- `Door:0` Door closed. Ready to resume.
        //- `Door:1` Machine stopped. Door still ajar. Can't resume until closed.
        //- `Door:2` Door opened. Hold (or parking retract) in-progress. Reset will throw an alarm.
        //- `Door:3` Door closed and resuming. Restoring from park, if applicable. Reset will throw an alarm.
        if (segs[i] == "Idle")
            status.m_vals.state = GRBLState::Status_Idle;
        else if(segs[i] == "Sleep")
            status.m_vals.state = GRBLState::Status_Sleep;
        else if(segs[i] == "Run")
            status.m_vals.state = GRBLState::Status_Run;
        else if(segs[i] == "Jog")
            status.m_vals.state = GRBLState::Status_Jog;
        else if(segs[i] == "Check")
            status.m_vals.state = GRBLState::Status_Check;
        else if(segs[i] == "Home")
            status.m_vals.state = GRBLState::Status_Home;
        else if(segs[i] == "Alarm")
            status.m_vals.state = GRBLState::Status_Alarm;
        else if(segs[i].substr(0, 4)    == "Hold")
            status.m_vals.state = (segs[i].substr(5, 1) == "0") ? GRBLState::Status_Hold0 : GRBLState::Status_Hold1;
        else if(segs[i].substr(0, 4) == "Door") 
        {
            if(segs[i].substr(5, 1) == "0")
            status.m_vals.state = GRBLState::Status_Door0;
            else if(segs[i].substr(5, 1) == "1")
            status.m_vals.state = GRBLState::Status_Door1;
            else if(segs[i].substr(5, 1) == "2")
            status.m_vals.state = GRBLState::Status_Door2;
            else // == 3
            status.m_vals.state = GRBLState::Status_Door3;
        }

        // MPos:0.000,-10.000,5.000 machine position  or  WPos:-2.500,0.000,11.000 work position
        // WPos = MPos - WCO
        else if(segs[i].substr(0, 4) == "MPos") {
            status.m_vals.MPos = stoVec3(segs[i].substr(5));
            status.m_vals.WPos = status.m_vals.MPos - status.m_vals.WCO;
        }
        else if(segs[i].substr(0, 4) == "WPos") {
            status.m_vals.WPos = stoVec3(segs[i].substr(5));
            status.m_vals.MPos = status.m_vals.WPos + status.m_vals.WCO;
        }
        // work coord offset - shown every 10-30 times
        // the current work coordinate system, G92 offsets, and G43.1 tool length offset
        else if(segs[i].substr(0, 3) == "WCO") {
            status.m_vals.WCO = stoVec3(segs[i].substr(4));
        }

        // Buffer State - mainly used for debugging
        // Bf:15,128. number of available blocks in the planner buffer / number of available bytes in the serial RX buffer.
        else if(segs[i].substr(0, 2) == "Bf") 
        {
            size_t a = segs[i].find(",");
            if (a != std::string::npos) {
                status.m_vals.bufferPlannerAvail = stof(segs[i].substr(3, a-3)); 
                status.m_vals.bufferSerialAvail = stoi(segs[i].substr(a+1)); 
            } else
            Log::Error("Can't find ',' in Bf");
        }
        // line number Ln:99999
        else if(segs[i].substr(0, 2) == "Ln") {
            status.m_vals.lineNum = stoi(segs[i].substr(3)); 
        }
        // feed & speed
        // FS:500,8000 (feed rate / spindle speed)
        else if(segs[i].substr(0, 2) == "FS") 
        {
            size_t a = segs[i].find(",");
            if (a != std::string::npos) {
                status.m_vals.feedRate = stof(segs[i].substr(3, a-3)); 
                status.m_vals.spindleSpeed = stoi(segs[i].substr(a+1)); 
            } else
            Log::Error("Can't find ',' in FS");
        }
        // feed only
        // F:500 (feed rate only) - when VARIABLE_SPINDLE is disabled in config.h
        else if(segs[i].substr(0, 1) == "F") 
        {
            status.m_vals.feedRate = stof(segs[i].substr(2)); 
        }
        // Input Pin State
        // Pn:XYZPDHRS - can be any number of letters
        else if(segs[i].substr(0, 2) == "Pn") 
        {
            // set all pins to default
            status.m_vals.inputPin_LimX = false;
            status.m_vals.inputPin_LimY = false;
            status.m_vals.inputPin_LimZ = false;
            status.m_vals.inputPin_Probe = false;
            status.m_vals.inputPin_Door = false;
            status.m_vals.inputPin_Hold = false;
            status.m_vals.inputPin_SoftReset = false;
            status.m_vals.inputPin_CycleStart = false;
            
            std::string str = segs[i].substr(3);
            for (size_t j = 0; j < str.length(); j++) {
            if(str[j] == 'X')
                status.m_vals.inputPin_LimX = true;
            else if(str[j] == 'Y')
                status.m_vals.inputPin_LimY = true;
            else if(str[j] == 'Z')
                status.m_vals.inputPin_LimZ = true;
            else if(str[j] == 'P')
                status.m_vals.inputPin_Probe = true;
            else if(str[j] == 'D')
                status.m_vals.inputPin_Door = true;
            else if(str[j] == 'H')
                status.m_vals.inputPin_Hold = true;
            else if(str[j] == 'R')
                status.m_vals.inputPin_SoftReset = true;
            else if(str[j] == 'S')
                status.m_vals.inputPin_CycleStart = true;
            else
                Log::Error("Input pin unrecognised: %c", str[j]);
            }
            
        }
        //Override Values:
        // Ov:100,100,100 current override values in percent of programmed values for feed, rapids, and spindle speed, respectively.
        else if(segs[i].substr(0, 2) == "Ov") 
        {
            glm::vec3 ov = stoVec3(segs[i].substr(3));
            status.m_vals.override_Feedrate = (int)ov.x;
            status.m_vals.override_RapidFeed = (int)ov.y;
            status.m_vals.override_SpindleSpeed = (int)ov.z;
        }
        // Accessory State
        //     'A:SFM' - can be any number of letters
        // S indicates spindle is enabled in the CW direction. This does not appear with C.
        // C indicates spindle is enabled in the CCW direction. This does not appear with S.
        // F indicates flood coolant is enabled.
        // M indicates mist coolant is enabled.
        else if(segs[i].substr(0, 2) == "A:") {
            // set all pins to default
            status.m_vals.accessory_SpindleDir = false;
            status.m_vals.accessory_FloodCoolant = false;
            status.m_vals.accessory_MistCoolant = false;
            
            std::string str = segs[i].substr(2);
            for (size_t j = 0; j < str.length(); j++) 
            {
            if(str[j] == 'S')
                status.m_vals.accessory_SpindleDir = (int)Direction::CW;    // (1)
            else if(str[j] == 'C')
                status.m_vals.accessory_SpindleDir = (int)Direction::CCW;    // (-1)
            else if(str[j] == 'F')
                status.m_vals.accessory_FloodCoolant = true;
            else if(str[j] == 'M')
                status.m_vals.accessory_MistCoolant = true;
            else
                Log::Error("Accessory pin unrecognised: %c", str[j]);
            }
        }
        else {
            Log::Error(std::string("Unknown segment in status report: ") + segs[i]);
        }
    }
}
/*    
$0 = 10 (us) : Step pulse time
$1 = 25 (ms) : Step idle delay
$2 = 7 (00000111) : Step pulse invert
$3 = 6 (00000110) : Step direction invert
$4 = 0 (boolean) : Invert step enable pin
$5 = 0 (boolean) : Invert limit pins
$6 = 1 (boolean) : Invert probe pin
$10 = 2 (00000010) : Status report options
$11 = 0.01 (mm) : Junction deviation
$12 = 0.002 (mm) : Arc tolerance
$13 = 0 (boolean) : Report in inches
$20 = 0 (boolean) : Soft limits enable
$21 = 1 (boolean) : Hard limits enable
$22 = 1 (boolean) : Homing cycle enable
$23 = 3 (00000011) : Homing direction invert
$24 = 25 (mm/min) : Homing locate feed rate
$25 = 2500 (mm/min) : Homing search seek rate
$26 = 25 (ms) : Homing switch debounce delay
$27 = 1 (mm) : Homing switch pull-off distance
$30 = 24000 (RPM) : Maximum spindle speed
$31 = 0 (RPM) : Minimum spindle speed
$32 = 0 (boolean) : Laser-mode enable
$100 = 320 (step/mm) : X-axis travel resolution
$101 = 320 (step/mm) : Y-axis travel resolution
$102 = 640 (step/mm) : Z-axis travel resolution
$110 = 6000 (mm/min) : X-axis maximum rate
$111 = 6000 (mm/min) : Y-axis maximum rate
$112 = 3000 (mm/min) : Z-axis maximum rate
$120 = 200 (mm/sec^2) : X-axis acceleration
$121 = 200 (mm/sec^2) : Y-axis acceleration
$122 = 200 (mm/sec^2) : Z-axis acceleration
$130 = 950 (mm) : X-axis maximum travel
$131 = 530 (mm) : Y-axis maximum travel
$132 = 160 (mm) : Z-axis maximum travel
*/
// decodes the settings froms grbl
// just prints them for now
std::string GRBLSystem::decodeSettings(const std::string& msg) 
{
    // retrieve settings code & current value
    size_t a = msg.find("=");
    
    // checks to prevent errors
    if((a == std::string::npos) || (a <= 0) || msg.length() <= a)
        return "Error";
    
    int settingsCode = stoi(msg.substr(1, a-1));
    float value = stof(msg.substr(a+1));
    
    //determine which units we are using
    // this has its own mutex lock
    if(settingsCode == 13) {
        settings.setUnitsInches((int)value);
    }
    
    {   
        // lock the mutex
        std::unique_lock<std::mutex> locker(settings.m_mutex);
            
        settings.m_vals.RawValues[settingsCode] = value;
            
        if(settingsCode == 30)
            settings.m_vals.max_SpindleSpeed = value;
        if(settingsCode == 31)
            settings.m_vals.min_SpindleSpeed = value;

        if(settingsCode == 110 || settingsCode == 111 || settingsCode == 112) 
        {
            if(settingsCode == 110)
            settings.m_vals.max_FeedRateX = value;
            if(settingsCode == 111)
            settings.m_vals.max_FeedRateY = value;
            if(settingsCode == 112)
            settings.m_vals.max_FeedRateZ = value;
            // set largest value to max_feedrate
            settings.m_vals.max_FeedRate = std::max(settings.m_vals.max_FeedRateX, settings.m_vals.max_FeedRateY);
            settings.m_vals.max_FeedRate = std::max(settings.m_vals.max_FeedRate, settings.m_vals.max_FeedRateZ);
        }
    }

    // retrieve name, desc & unit of setting
    std::string name, unit, desc;
    if(getSettingsMsg(settingsCode, &name, &unit, &desc)) {    
        Log::Error("Error %d: Can't find settings code", settingsCode);
    }

    // display unit and name for setting
    std::stringstream s;
    s << " (";
    // if it's a mask, display as in binary instead of unit
    (unit == "mask") ? (s << std::bitset<8>(value)) : (s << unit);
    // append the name
    s << ") : " << name; //<< " (" << desc << ")";
    return s.str();
    
}

GRBL::GRBL() {
    // mutex is unlikely to be needed but just in case grbl was somehow called after threads
    std::lock_guard<std::mutex> guard(m_mutex);
    m_mainThreadID = this_thread::get_id();
    m_runCommand = GRBL_CMD_RUN;
    
    // create threads
    t_Read = thread(&GRBL::thread_read, this);
    t_Write = thread(&GRBL::thread_write, this);
    //t_StatusReport = thread(&GRBL::thread_statusReport, this);
}

GRBL::~GRBL() {
    // send signal to threads to shutdown
    shutdown();
    
    // join all threads to main thread
    t_Read.join();
    Log::Info("Read Thread Joined");
    t_Write.join();
    Log::Info("Write Thread Joined");
    //t_StatusReport.join();
    //Log::Info("Status Report Thread Joined");
}

void GRBL::connect(std::string device, int baudrate)
{        
    Log::Info("Connecting...");
    serial.connect(device, baudrate);
    softReset();
    Log::Info("Connected");
    sendUpdateSettings();
}

void GRBL::disconnect()
{        
    Log::Info("Disconnecting...");
    softReset();
    serial.disconnect();
    sys.status.setState(GRBLState::Status_Unknown);
    Log::Info("Disconnected");
}

bool GRBL::isConnected() {
    return serial.isConnected();
}

// this makes a copy of a const std::string (i.e Send("G90")) 
// so that we can pass it to and manipulate it in lower 
// down functions (i.e. removing whitespace etc)
int GRBL::send(const std::string& cmd, PreCheck prechecks) 
{
    std::string str = cmd;
    return send(str, prechecks);
}
// adds to the GCode list, ready to be written when buffer has space
// sending a pointer is slightly quicker as it wont have to be be copied, 
// it will however, modify the original std::string to remove whitespace and comments etc
// returns 0 on success, -1 on failure
int GRBL::send(std::string& cmd, PreCheck prechecks) 
{
    int err = send_preChecks(prechecks);
    if(err) return err;    
    
    Log::Debug(DEBUG_GCLIST_BUILD, "Adding streamed item = %s", cmd.c_str());
    
    // add command to GCList
    if(gcList.add(cmd)) 
        return -1;    // line was longer than GRBL_MAX_BUFFER

    return 0;
}

int GRBL::sendFile(const std::string& file) 
{    
    auto executeLine = [this](std::string& str) {
           
        Log::Debug(DEBUG_GCLIST_BUILD, "Adding streamed item from file = %s", str.c_str());
        
         // add command to GCList
        if(gcList.addMany(str)) 
            return -1;    // line was longer than GRBL_MAX_BUFFER

        return 0;
    }; 
    
    // do pre checks
    int err = send_preChecks(PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle);
    if(err) return err;    
    
    if(File::Read(file, executeLine)) {
        Log::Error(std::string("Could not open file ") + file);
        gcList.addManyEnd();
        return -1;
    }
    gcList.addManyEnd();
    sendUpdateSettings();
    return 0;
}

int GRBL::sendArray(const vector<string>& gcodes) 
{    
    // do pre checks
    int err = send_preChecks(PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle);
    if(err) return err;    

    for(string gcode : gcodes) {
         // add command to GCList
        if(gcList.addMany(gcode))
            return -1;    // line was longer than GRBL_MAX_BUFFER
    }
    
    gcList.addManyEnd();
    sendUpdateSettings();
    return 0;
}

bool GRBL::isFileRunning() {
    return gcList.isFileRunning();
}
void GRBL::getFilePos(uint& posIndex, uint& pos, uint& total) {
    gcList.getFilePos(posIndex, pos, total);
}
// checks to be done prior to sending gcodes
// this is seperated to allow checks to be done just once
// if lots of gcodes are to be sent
// these checks require mutexes to be locked and therefor may slow
// down transfer if done many times
int GRBL::send_preChecks(PreCheck prechecks)
{
    if(prechecks & PreCheck::SerialIsConnected) {
        if(!serial.isConnected()) {
            Log::Error("Connect to GRBL before sending commands");
            return -1;
        }
    }
    if(prechecks & PreCheck::NoFileRunning) {
        if(gcList.isFileRunning()) {
            Log::Error("File is already running");
            return -1;
        }
    }
    if(prechecks & PreCheck::GRBLIsIdle) {
        if(!(sys.status.getVals().state == GRBLState::Status_Idle || sys.status.getVals().state == GRBLState::Status_Check)) {
            Log::Error("Machine is not idle");
            return -1;
        }
    }
    return 0;
}

void GRBL::sendUpdateSettings()
{
    // update all settings
    send("$#");
    send("$N");
    send("$G");
    send("$$");
}

// Sends an incremental jog to p
// jogs do not affect the parser state 
// therefore you do not need to set the machine back to G90 after using a G91 command
// and the feedrate is not modal
void GRBL::sendJog(const glm::vec3& p, int feedrate) {
    // cancel with Grbl.SendRT(GRBL_RT_JOG_CANCEL);
    
    if(p == glm::vec3(0,0,0)) {
        Log::Error("Jog requires a distance");
        return;
    }
    // example: $J=G91 X10 F1000
    std::stringstream s;
    s << "$J=G91";
    
    if(p.x) s << "X" << p.x;
    if(p.y) s << "Y" << p.y;
    if(p.z) s << "Z" << p.z;
    
    s << "F" << feedrate;
   
    if(send(s.str(), PreCheck::SerialIsConnected | PreCheck::NoFileRunning)) {
        // cannot send
        return;
    }
}
/* sends a REALTIME COMMAND
 *     - These are not considered as part of the streaming protocol and are executed instantly
 *     - They do not require a line feed or carriage return after them.
 *     - None of these respond with 'ok' or 'error'
 *    see https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands
 */ 
void GRBL::sendRT(char cmd) 
{
    switch (cmd) {
    case GRBL_RT_SOFT_RESET:
        Log::Critical("Use grbl.softReset() instead");
        //Log::Info("Sent: 'Soft Reset'");
        break;
    case GRBL_RT_STATUS_QUERY:
        Log::Info("Sent: 'Status Query'");
        break;
    case GRBL_RT_HOLD:
        Log::Info("Sent: 'Hold'");
        break;
    case GRBL_RT_RESUME:
        Log::Info("Sent: 'Resume'"); 
        break;
    
    case GRBL_RT_DOOR:
        Log::Info("Sent: 'Door'");
        break;
    case GRBL_RT_JOG_CANCEL:
        Log::Info("Sent: 'Cancel Jog'");
        break;
    
    case GRBL_RT_OVERRIDE_FEED_100PERCENT:
        Log::Info("Sent: 'Override Feedrate (Set to 100%)'");
        break;
    case GRBL_RT_OVERRIDE_FEED_ADD_10PERCENT:
        Log::Info("Sent: 'Override Feedrate (+10%)'");
        break;
    case GRBL_RT_OVERRIDE_FEED_MINUS_10PERCENT:
        Log::Info("Sent: 'Override Feedrate (-10%)'");
        break;
    case GRBL_RT_OVERRIDE_FEED_ADD_1PERCENT:
        Log::Info("Sent: 'Override Feedrate (+1%)'");
        break;
    case GRBL_RT_OVERRIDE_FEED_MINUS_1PERCENT:
        Log::Info("Sent: 'Override Feedrate (-1%)'");
        break;
    
    case GRBL_RT_OVERRIDE_RAPIDFEED_100PERCENT:
        Log::Info("Sent: 'Override Rapid Feedrate (Set to 100%)'");
        break;
    case GRBL_RT_OVERRIDE_RAPIDFEED_50PERCENT:
        Log::Info("Sent: 'Override Rapid Feedrate (Set to 50%)'");
        break;
    case GRBL_RT_OVERRIDE_RAPIDFEED_25PERCENT:
        Log::Info("Sent: 'Override Rapid Feedrate (Set to 25%)'");
        break;
    
    case GRBL_RT_OVERRIDE_SPINDLE_100PERCENT:
        Log::Info("Sent: 'Override Spindle Speed (Set to 100%)'");
        break;
    case GRBL_RT_OVERRIDE_SPINDLE_ADD_10PERCENT:
        Log::Info("Sent: 'Override Spindle Speed (+10%)'");
        break;
    case GRBL_RT_OVERRIDE_SPINDLE_MINUS_10PERCENT:
        Log::Info("Sent: 'Override Spindle Speed (-10%)'");
        break;
    case GRBL_RT_OVERRIDE_SPINDLE_ADD_1PERCENT:
        Log::Info("Sent: 'Override Spindle Speed (+1%)'");
        break;
    case GRBL_RT_OVERRIDE_SPINDLE_MINUS_1PERCENT:
        Log::Info("Sent: 'Override Spindle Speed (-1%)'");
        break;
    
    case GRBL_RT_SPINDLE_STOP:
        Log::Info("Sent: 'Stop Spindle'");
        break;
    case GRBL_RT_FLOOD_COOLANT:
        Log::Info("Sent: 'Flood Coolant'");
        break;
    case GRBL_RT_MIST_COOLANT:
        Log::Info("Sent: 'Mist Coolant'");
        break;

    default:
        Log::Error("Realtime command unrecognised: %c", cmd);
        return;
    }
    serial.sendRT(cmd);
}

// soft reset 
void GRBL::softReset() 
{
    auto callback = [this]() {    
        // reset GCode List and serial
        gcList.softReset();
        serial.softReset();
    };
    if(!resetThreads(GRBL_CMD_RESET, callback))
        Log::Info("System has been reset");
    
}

// cancel file transfer
// commands remaining in buffer will still be executed 
void GRBL::cancel() 
{
    auto callback = [this]() {    
        // clear unset items in GCode list
        gcList.clearUnsent();
    };
    if(!resetThreads(GRBL_CMD_CANCEL, callback))
        Log::Info("Cancelled");
}

void GRBL::shutdown() {
    setCommand(GRBL_CMD_SHUTDOWN);
}

void GRBL::setCommand(int cmd) {
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_runCommand = cmd;
    }
    serial.setCommand(cmd);
    gcList.setCommand(cmd);
}

// clears any completed GCodes in the buffer
// used for clearing old commands in log
void GRBL::clearCompleted() {
    return gcList.clearCompleted();
}
int GRBL::getLastItem(GCItem& item) {
    return gcList.getLastItem(item);
}
// returns items in the gcode list for viewing
const GCItem& GRBL::getGCItem(uint index) {
    return gcList.getItem(index);
}
    // returns items in the gcode list for viewing
uint GRBL::getGCListSize() {
    return gcList.getSize();
}

void GRBL::setViewStatusReport(bool isViewable) {
    std::lock_guard<std::mutex> guard(m_mutex);
    viewStatusReport = isViewable;
}
bool GRBL::getViewStatusReport() {
    std::lock_guard<std::mutex> guard(m_mutex);
    return viewStatusReport;
}
// set the interval timer for the status report
// no faster than 5Hz (200ns)
/*void GRBL::setStatusInterval(uint ms) {
    std::lock_guard<std::mutex> guard(m_mutex);
    statusTimerInterval = ms;
}
uint GRBL::getStatusInterval() {
    std::lock_guard<std::mutex> guard(m_mutex);
    return statusTimerInterval;
}
*/
void GRBL::SystemCommands()
{
    int cmd;
    {    // get run flag
        std::lock_guard<std::mutex> guard(m_mutex);
        cmd = m_runCommand;
    }
    // thread has signalled to reset
    // we need to be out of lock before calling this
    if(cmd == GRBL_CMD_RESET)
        softReset();    
    // thread has signalled to cancel
    // we need to be out of lock before calling this
    if(cmd == GRBL_CMD_CANCEL)
        cancel();
}

// returns true if end of execution of gcode from grbl ('ok' or 'error' received)
int GRBL::processResponse(const std::string& msg)
{
    // Response for an 'error'
    auto errorResponse = [](const std::string& msg) 
    {  // retrieve error code
        int errCode = stoi(msg.substr(6));
        // get error description and add to log
        std::string errName, errDesc;
        if(getErrMsg(errCode, &errName, &errDesc)) 
            Log::Error("Error %d: Can't find error code", errCode);
        else
            Log::Response("Error %d: %s (%s)", errCode, errName.c_str(), errDesc.c_str());
        return errCode;
    };
    // Response for an 'alarm'
    auto alarmResponse = [](const std::string& msg) 
    {    // retrieve alarm code
        int alarmCode = stoi(msg.substr(6));
        // add response to log
        std::string alarmName, alarmDesc;
        if(getAlarmMsg(alarmCode, &alarmName, &alarmDesc))
            Log::Error("ALARM %d: Can't find alarm code", alarmCode);
        else    
            Log::Response("ALARM %d: %s (%s)", alarmCode, alarmName.c_str(), alarmDesc.c_str());
    };
    
    // ignore blank responses
    if(!msg.compare(0, 1, "")) {
    }
    // match up an 'ok' or 'error' to the corrosponding sent gcode and set it's status
    else if(!msg.compare("ok")) {
        Log::Response("ok");
        return STATUS_OK;
    } 
    // error messages
    else if(!msg.compare(0, 6, "error:")) {
        return errorResponse(msg);
    }
    // alarm messages
    else if(!msg.compare(0, 6, "ALARM:")) {        
        alarmResponse(msg);
        // status report stops getting sent from grbl so we update state manually
        sys.status.setState(GRBLState::Status_Alarm); 
        // end if file running so that the progress doesn't keep counting
        gcList.setFileEnded();
    }
    else if(!msg.compare(0, 1, "<")) {
        sys.decodeStatus(msg);
        // show the status report if checkbox selected
        bool viewStatus = getViewStatusReport();
        // show the status report if a user sends it through console
        GCItem lastItem;
        if(!getLastItem(lastItem))
            viewStatus |= ((lastItem.str == "?\n") && (lastItem.status == STATUS_SENT));
        
        if(viewStatus)  
            Log::Response(msg);
    }
    // Startup Line Execution    ">G54G20:ok" or ">G54G20:error:X"
    else if(!msg.compare(0, 1, ">")) {    
        sys.checkStartupLine(msg);
        Log::Response(msg);
    }
    // just print out message
    else if(!msg.compare(0, 4, "Grbl") || !msg.compare(0, 4, "[MSG") || !msg.compare(0, 4, "[HLP") || !msg.compare(0, 4, "[echo")) {
    Log::Response(msg);
    }
    // View build info - just print out    
    // This response hasnt been decoded as seen as unnesessary
    // For more details, see: https://github.com/gnea/grbl/wiki/Grbl-v1.1-Interface
    else if(!msg.compare(0, 4, "[VER") || !msg.compare(0, 4, "[OPT")) {    
        Log::Response(msg);
    }

    else if(!msg.compare(0, 3, "[GC")) {
        sys.decodeMode(msg);
        Log::Response(msg);
    }
    else if(!msg.compare(0, 1, "[")) {
        sys.decodeCoords(msg);                
        Log::Response(msg);
        // signal event that coords have been updated
        //Event<Event_SettingsUpdated_Coords>::Dispatch({Event_SettingType::CoordSystems}); 
    }
    // if startup block added, it will look like this on starup: '>G20G54G17:ok' or error
    else if(!msg.compare(0, 2, "$N")) {    
        sys.decodeStartupBlock(msg);
        Log::Response(msg);
    }
    // settings codes
    else if(!msg.compare(0, 1, "$")) {
        std::string setting = sys.decodeSettings(msg);
        Log::Response(msg + setting);
    }
    else {    
        Log::Error("Unsupported GRBL message: %s", msg.c_str());
    }
    return STATUS_MSG;
}


void GRBL::checkGCodeAction(const string& gcode)
{
    auto findAndSend = [&](const string& findStr, const string& trueCond) {
        if(gcode.find(findStr) != string::npos)
            send(trueCond, PreCheck::SerialIsConnected);
    }; 
    // tool length offset
    findAndSend("G43.1", "$#");
    findAndSend("G49", "$#");
    // set coord system
    findAndSend("G10", "$#");
    findAndSend("G28.1", "$#");
    findAndSend("G30.1", "$#");
    findAndSend("G92", "$#");
    
    // change coord system
    findAndSend("G0", "$G");
    findAndSend("G1", "$G");
    findAndSend("G2", "$G");
    findAndSend("G3", "$G");
    findAndSend("G38.2", "$G");
    findAndSend("G38.3", "$G");
    findAndSend("G38.4", "$G");
    findAndSend("G38.5", "$G");
    findAndSend("G80", "$G");
    
    // change feedrate mode
    findAndSend("G93", "$G");
    findAndSend("G94", "$G");
    
    // change coord system
    findAndSend("G54", "$G");
    findAndSend("G55", "$G");
    findAndSend("G56", "$G");
    findAndSend("G57", "$G");
    findAndSend("G58", "$G");
    findAndSend("G59", "$G");
    // change plane
    findAndSend("G17", "$G");
    findAndSend("G18", "$G");
    findAndSend("G19", "$G");
    // absolute / incremental
    findAndSend("G90", "$G");
    findAndSend("G91", "$G");
    
    // change units
    findAndSend("G20", "$G");
    findAndSend("G21", "$G");
    
    // ProgramMode
    findAndSend("M0", "$G");
    findAndSend("M1", "$G");
    findAndSend("M2", "$G");
    findAndSend("M30", "$G");
    // SpindleState
    findAndSend("M3", "$G");
    findAndSend("M4", "$G");
    findAndSend("M5", "$G");
    // CoolantState
    findAndSend("M7", "$G");
    findAndSend("M8", "$G");
    findAndSend("M9", "$G");
    
    // any setting
    findAndSend("$0", "$$");
    findAndSend("$1", "$$");
    findAndSend("$2", "$$");
    findAndSend("$3", "$$");
    findAndSend("$4", "$$");
    findAndSend("$5", "$$");
    findAndSend("$6", "$$");
    findAndSend("$7", "$$");
    findAndSend("$8", "$$");
    findAndSend("$9", "$$");
    
}

#define THREAD_NONE        0b00
#define THREAD_WRITE     0b01
#define THREAD_READ        0b10

/*
    Signalling a shutdown, soft reset or cancel are all done in a similar manner:
    The read and write threads are asked to return to the beginning of their loop.
    If a shutdown is called, all threads will break their loop.
    If a soft reset / cancel is called, they will sit and wait at the beginning of their loop and signal 
    to the main thread that they are stationary. Once both threads have confirmed this, the command will 
    commence. This prevents any possible corruption as threads could be half way through their operation 
    when a command is called. 
    In the event that a soft reset / cancel is called by one either the write or read thread (rather than 
    the main thread) The thread signals for the main thread to call command instead and runs as above. 
*/

// command threads to return to beginning of loop, then call callback, then restart threads
int GRBL::resetThreads(int cmd, std::function<void(void)> callback) 
{
    // set reset flags for grbl, gclist and serial
    setCommand(cmd);
    
    // if calling thread was not main, signal main thread to call this instead
    if(this_thread::get_id() != m_mainThreadID)
        return 1;
    
    {   // lock mutex and wait for threads to reset themselves
        std::unique_lock<std::mutex> locker(m_mutex);
        
        // wait for both read and write threads to be ready
        m_cond_reset.wait(locker, [&]() { 
            Log::Debug(DEBUG_THREAD_BLOCKING, "Inside resetThreads() condtion (cmd = %d)", cmd); 
            // wake condtion
            return m_threadsReady == (THREAD_WRITE | THREAD_READ); 
        }); 
        Log::Debug(DEBUG_THREAD_BLOCKING, "Passed resetThreads() condtion (cmd = %d)", cmd);  
        
        // reset the flag
        m_threadsReady = THREAD_NONE;
    }
    // call specific functions
    callback();
    // reset command flags
    setCommand(GRBL_CMD_RUN);
    // signal threads to continue
    m_cond_threads.notify_all(); 
    return 0;
}

int GRBL::blockThreads(int threadBit)
{
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        
        if(m_runCommand == GRBL_CMD_RUN) 
            return 0; // continue
        if(m_runCommand == GRBL_CMD_SHUTDOWN) 
            return 1; // return so we can break look
        // else we must be resetting / cancelling
        
        // set flag to state this thread is ready to restart
        m_threadsReady |= threadBit;
    }
    // to ensure that write & read threads get there after
    delay(100);
    // notify waitForThreads()
    m_cond_reset.notify_one();
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        // if waiting for other thread, return and loop
        m_cond_threads.wait(locker, [&]() { 
            Log::Debug(DEBUG_THREAD_BLOCKING, "T%d Inside resetThreads() condtion", threadBit);
            // wake condition
            return m_runCommand == GRBL_CMD_RUN; 
        }); 
        Log::Debug(DEBUG_THREAD_BLOCKING, "T%d Passed resetThreads() condtion", threadBit); 
    }
    return 0;
}

// Read from serial
// Write back onto GCode list
void GRBL::thread_read() 
{   
    while(m_runCommand != GRBL_CMD_SHUTDOWN) 
    {
        // if a command has been signalled (reset/cancelled), sit and wait here
        if(blockThreads(THREAD_READ))
            continue;
        // static so we dont have to keep allocating memory for it
        static std::string msg(128, ' ');
        // get data from serial
        if(serial.receive(msg))
            continue; // serial had no data available
        // process data
        int response = processResponse(msg);
        // if message
        if(response == STATUS_MSG) {
            continue;
        }
        // if error
        else if(response > 0) {
            // we have recieved an error in check mode
            if(sys.status.isCheckMode()) {
                gcList.addCheckModeError();
            }  
            else if(gcList.isFileRunning()) {
                Log::Error("Error recieved mid file transfer, machine has been reset for safety");
                Log::Info("Consider using check mode prior to sending file");
                softReset();
                continue;
            }
        }
        
        // reponse was ok or error, therefore grbl 
        // has acknowledged the last sent command
        // we can remove it from our queue
        if(serial.bufferRemove()) {
            softReset();
            continue;
        }
        string completedGCode;
        // set response in gcode list
        if(gcList.setNextResponse(response, completedGCode)) {
            softReset();
            continue;
        }
        // check to see if we should update of our data
        // if a user changes a setting
        if(!gcList.isFileRunning() && response == STATUS_OK) {
            checkGCodeAction(completedGCode);
        }
    }
}

// Read from GCode List
// Write to serial
void GRBL::thread_write() 
{   
    while(m_runCommand != GRBL_CMD_SHUTDOWN) 
    {
        // if a command has been signalled (reset/cancelled), sit and wait here
        if(blockThreads(THREAD_WRITE))
            continue;
        // retrieve next item from GCode list
        GCItem item;
        if(gcList.getNextItem(item))
            continue;
        // send item to serial
        if(serial.send(item.str))
            continue;
        // sets item to pending and increments
        gcList.nextItem();
        // log the gcode
        Log::Info(std::string("Sent: ") + item.str);
    }
}

void GRBL::RequestStatusReport() 
{
    serial.sendRT(GRBL_RT_STATUS_QUERY);
}
/*
void GRBL::thread_RequestStatusReport() 
{
    while(m_runCommand != GRBL_CMD_SHUTDOWN) {
        RequestStatusReport();
        delay(getStatusInterval());
    }
}
*/

// get all grbl values (this is much quicker than getting
// as required as it does require lots of mutexes)
void GRBL::UpdateGRBLVals(GRBLVals& grblVals)
{
    grblVals.coords = sys.coords.getVals();
    grblVals.modal = sys.modal.getVals();
    grblVals.status = sys.status.getVals();
    grblVals.settings = sys.settings.getVals();

    grblVals.isConnected = isConnected();
    grblVals.isCheckMode = (grblVals.status.state == GRBLState::Status_Check);
    grblVals.isFileRunning = isFileRunning();
    getFilePos(grblVals.curLineIndex, grblVals.curLine, grblVals.totalLines);
    
    //grblVals.statusTimerInterval = getStatusInterval();
}

void GRBL::Update(GRBLVals& grblVals)
{
    // read commands flag and call required command
    SystemCommands();
    // send a status report request
    RequestStatusReport();
    // build a structure of all values so we dont have to constantly be locking mutexes throughout program
    UpdateGRBLVals(grblVals);
}

} // end namespace Sqeak

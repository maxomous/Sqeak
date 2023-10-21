
#pragma once

#include <string>
#include <map>
// threads
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <thread>

#include <MaxLib.h> 

#include "grbl_common.h" 
#include "serial.h" 
#include "gclist.h" 
#include "grblcodes.h" 


namespace Sqeak { 
    
using namespace MaxLib;
using namespace MaxLib::Geom;



// *********************** //
//     GRBL Settings        //
// *********************** //

struct GRBLCoords_vals {
    Vec3 workCoords[6];        //    [G54:4.000,0.000,0.000]        work coords        can be changed with     G10 L2 Px or G10 L20 Px
                                    //    [G55:4.000,6.000,7.000]
                                    //    [G56:0.000,0.000,0.000]
                                    //    [G57:0.000,0.000,0.000]
                                    //    [G58:0.000,0.000,0.000]
                                    //    [G59:0.000,0.000,0.000]
    Vec3 homeCoords[2];        //    [G28:1.000,2.000,0.000]        pre-defined positions     can be changed with     G28.1
                                    //    [G30:4.000,6.000,0.000]                                 G30.1
    Vec3 offsetCoords;         //    [G92:0.000,0.000,0.000]        coordinate offset 
    float toolLengthOffset = 0.0f;  //    [TLO:0.000]            tool length offsets
    Vec3 probeOffset;          //    [PRB:0.000,0.000,0.000:0]    probing
    bool probeSuccess = false;      //                   ^
};
    
// GRBL GCode Coords
class GRBLCoords
{
public:
    // returns a copy of vals
    const GRBLCoords_vals getVals();
    
private:
    std::mutex m_mutex;
    GRBLCoords_vals m_vals;
    friend class GRBLSystem;
    
};

 
struct GRBLModal_vals{
    std::string StartupBlock[2];
    float MotionMode            = 0.0f;     // *G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
    uint CoordinateSystem       = 0;        // *G54, G55, G56, G57, G58, G59
    uint Plane                  = 0;        // *G17, G18, G19
    uint DistanceMode           = 0;        // *G90, G91
    float ArcIJKDistanceMode    = 91.1f;    // *G91.1
    uint FeedRateMode           = 0;        // G93, *G94
    uint UnitsMode              = 0;        // G20, *G21
    uint CutterRadCompensation  = 40;       // *G40
    float ToolLengthOffset      = 49.0f;    // G43.1, *G49
    uint ProgramMode            = 0;        // *M0, M1, M2, M30
    uint SpindleState           = 0;        // M3, M4, *M5
    uint CoolantState           = 0;        // M7, M8, *M9

    uint toolNumber             = 0;
    float spindleSpeed          = 0.0f;
    float feedRate              = 0.0f;    
};

// Modal Group
//     These define the last set (modal) values
class GRBLModal
{
public:
    // returns a copy of vals
    const GRBLModal_vals getVals();
    
private:
    std::mutex m_mutex;
    GRBLModal_vals m_vals;
    friend class GRBLSystem;
};

// Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep
//- `Hold:0` Hold complete. Ready to resume.
//- `Hold:1` Hold in-progress. Reset will throw an alarm.
//- `Door:0` Door closed. Ready to resume.
//- `Door:1` Machine stopped. Door still ajar. Can't resume until closed.
//- `Door:2` Door opened. Hold (or parking retract) in-progress. Reset will throw an alarm.
//- `Door:3` Door closed and resuming. Restoring from park, if applicable. Reset will throw an alarm.
enum class GRBLState {
    Status_Idle, 
    Status_Hold0,
    Status_Hold1, 
    Status_Sleep, 
    // ^ colour idle ^
    Status_Run, 
    Status_Jog, 
    Status_Check, 
    Status_Home, 
    // ^ colour motion ^
    Status_Alarm, 
    Status_Door0, 
    Status_Door1, 
    Status_Door2, 
    Status_Door3,
    Status_Unknown
    // ^ colour alert ^
};

struct GRBLStatus_vals
{
    GRBLState state              = GRBLState::Status_Unknown;
    Vec3 MPos; // either MPos if WPos is given (WPos = MPos - WCO)
    Vec3 WPos;
    Vec3 WCO;  // this is recieved every 10 or 30 status messages
    int lineNum                  = 0;
    float feedRate               = 0;    // mm/min or inches/min
    int spindleSpeed             = 0;     // rpm
    
    bool inputPin_LimX           = 0;
    bool inputPin_LimY           = 0;
    bool inputPin_LimZ           = 0;
    bool inputPin_Probe          = 0;
    bool inputPin_Door           = 0;
    bool inputPin_Hold           = 0;
    bool inputPin_SoftReset      = 0;
    bool inputPin_CycleStart     = 0;
    
    int override_Feedrate        = 0;
    int override_RapidFeed       = 0;
    int override_SpindleSpeed    = 0;
    
    int accessory_SpindleDir     = 0;
    int accessory_FloodCoolant   = 0;
    int accessory_MistCoolant    = 0;
    
    int bufferPlannerAvail       = 0;
    int bufferSerialAvail        = 0;
    
};

// Realtime status values
//    These are the values recieved from the real time status report
class GRBLStatus 
{    
public:
    bool isCheckMode();
    // returns a copy of vals
    const GRBLStatus_vals getVals();
    // returns value 
    const std::string stateStr(GRBLState state);
    // for externally setting the state
    void setState(GRBLState state);
    
private:
    std::mutex m_mutex;
    GRBLStatus_vals m_vals;
    friend class GRBLSystem;
};

struct GRBLSettings_vals 
{
    int min_SpindleSpeed;
    int max_SpindleSpeed;
    float max_FeedRateX;
    float max_FeedRateY;
    float max_FeedRateZ;
    float max_FeedRate;
    std::string units_Distance;
    std::string units_Feed;
    
    std::map<int, float> RawValues;
};

class GRBLSettings 
{
public:
    // returns a copy of vals
    const GRBLSettings_vals getVals();
    // sets units to mm or inches
    void setUnitsInches(bool isInches);
    
private:
    std::mutex m_mutex;
    GRBLSettings_vals m_vals;
    friend class GRBLSystem;
};

// a structure to hold a shallow copy of all data in grbl per frame
// it is a lot quicker to get this data all at once rather
// than lock many mutexes as we go
struct GRBLVals {
    bool isConnected;
    bool isCheckMode;
    bool isFileRunning;
    uint curLineIndex;  // position in gcList
    uint curLine;       // position in file
    uint totalLines;
    GRBLCoords_vals coords;
    GRBLModal_vals modal;
    GRBLStatus_vals status;
    GRBLSettings_vals settings;
    
    Vec3 ActiveCoordSys()
    {
        uint coordSys = modal.CoordinateSystem;
        uint index = (coordSys >= 54 && coordSys <= 59) ? coordSys - 54 : 0;
        return coords.workCoords[index];
    }
};
 
class GRBLSystem 
{
public:
    GRBLCoords       coords;
    GRBLModal        modal;
    GRBLStatus       status;
    GRBLSettings     settings;

private:

    // checks Startup Line Execution for error    msg = ">G54G20:ok" or ">G54G20:error:X"
    // it is very unlikely that there will be an error as this is checked before it is saves onto the eeprom
    void checkStartupLine(const std::string& msg);
    // decodes GCode Parameters
    // and stores inside grbl Parameters
    void decodeCoords(const std::string& msg);
    // decodes the startup block
    void decodeStartupBlock(const std::string& msg);
    // decodes modal groups
    // and stores inside grbl Parameters
    void decodeMode(const std::string& msg);
    // decodes status response 
    // and stores inside grbl Parameters
    // The $10 status report mask setting can alter what data is present and certain data fields can be reported intermittently (see descriptions for details.)
    // The $13 report inches settings alters the units of some data values. $13=0 false indicates mm-mode, while $13=1 true indicates inch-mode reporting.
    // "<Idle|WPos:828.000,319.000,49.100|FS:0,0|Pn:PXYZ>"
    void decodeStatus(const std::string& msg);
    // decodes the settings froms grbl
    // just prints them for now
    std::string decodeSettings(const std::string& msg);
    friend class GRBL;
};
 
enum PreCheck {
    SerialIsConnected   = 0x1 << 0,
    NoFileRunning       = 0x1 << 1,
    GRBLIsIdle          = 0x1 << 2
};
// allow bitwise operation
inline PreCheck operator|(PreCheck a, PreCheck b) { return static_cast<PreCheck>(static_cast<int>(a) | static_cast<int>(b)); }

class GRBL 
{
public:
    GRBL();
    ~GRBL();
    GRBLSystem sys;
    
    void connect(std::string device, int baudrate);
    void disconnect();
    bool isConnected();
    // this makes a copy of a const std::string (i.e Send("G90")) 
    // so that we can pass it to and manipulate it in lower 
    // down functions (i.e. removing whitespace etc)
    int send(const std::string& cmd, PreCheck prechecks = (PreCheck::SerialIsConnected | PreCheck::NoFileRunning)); //PreCheck::SerialIsConnected | PreCheck::NoFileRunning | PreCheck::GRBLIsIdle
    // adds to the GCode list, ready to be written when buffer has space
    // sending a pointer is slightly quicker as it wont have to be be copied, 
    // it will however, modify the original std::string to remove whitespace and comments etc
    // returns 0 on success, -1 on failure
    int send(std::string& cmd, PreCheck prechecks = (PreCheck::SerialIsConnected | PreCheck::NoFileRunning));
    int sendFile(const std::string& file);
    int sendArray(const std::vector<std::string>& gcodes);
    bool isFileRunning();
    void getFilePos(uint& posIndex, uint& pos, uint& total);
    // checks to be done prior to sending gcodes
    // this is seperated to allow checks to be done just once
    // if lots of gcodes are to be sent
    // these checks require mutexes to be locked and therefor may slow
    // down transfer if done many times
    int send_preChecks(PreCheck prechecks);
    // update all settings
    void sendUpdateSettings();
    // Sends an incremental jog to p
    // jogs do not affect the parser state 
    // therefore you do not need to set the machine back to G90 after using a G91 command
    // and the feedrate is not modal
    void sendJog(const Vec3& p, int feedrate);
    /* sends a REALTIME COMMAND
     *     - These are not considered as part of the streaming protocol and are executed instantly
     *     - They do not require a line feed or carriage return after them.
     *     - None of these respond with 'ok' or 'error'
     *    see https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands
     */ 
    void sendRT(char cmd);
    // soft reset 
    void softReset();
    // clears any gcodes which havent received a reponse 
    // commands remaining in buffer will still be executed 
    void cancel();
    // triggers a shutdown of all threads
    void shutdown();
    void setCommand(int cmd);
    
    // clears any completed GCodes in the buffer
    // used for clearing old commands in log
    void clearCompleted();
    // return last sent item
    int getLastItem(GCodeItem& item);
    // returns items in the gcode list for viewing
    // use index of -1 for last item sent
    const GCodeItem& getGCodeItem(uint index);
        // returns items in the gcode list for viewing
    uint getGCodeListSize();
    void setViewStatusReport(bool isViewable);
    bool getViewStatusReport();
    // set the interval timer for the status report
    // no faster than 5Hz (200ns)
    //void setStatusInterval(uint ms);
    //uint getStatusInterval();
    // checks whether thread has signalled to reset/cancel
    void SystemCommands();
    // sends a real time status report request to grbl 
    void RequestStatusReport();
    // get all grbl values, this is much quicker than getting 
    // individually as it prevent lots of mutexes
    void UpdateGRBLVals(GRBLVals& grblVals);
    // calls updates for each frame
    void Update(GRBLVals& grblVals);
private:
    GCodeList gcList;
    Serial serial;
    // thread variables
    std::thread t_Read, t_Write;//, t_StatusReport;
    std::atomic<std::thread::id> m_mainThreadID;
    std::mutex m_mutex;
    std::condition_variable m_cond_reset;
    std::condition_variable m_cond_threads;
    // used across threads to determinine whether we need to reset or shutdown
    int m_runCommand;
    // The number of threads ready at beginning of loop, used during a change in command (reset / cancel)
    int m_threadsReady = 0;
    // status report variables
    bool viewStatusReport = false;
    
    // returns true if end of execution of gcode from grbl ('ok' or 'error' received)
    int processResponse(const std::string& msg);
    // checks whether a command should be sent when a gcode has been acknowledged (i.e. update settings if a setting was changed)
    void checkGCodeAction(const std::string& gcode);
    // commands the threads to stop execution,returns them to beginning of loop, where they are blocked.
    // once both threads have got there, callback is called.
    // finally, we restart the threads
    int resetThreads(int cmd, std::function<void(void)> callback); 
    // resets threads when performing soft reset
    int blockThreads(int thread);
    // infinate looping thread
    // read from serial
    // write back onto GCode list
    void thread_read();
    // infinate looping thread
    // read from GCode list
    // write to serial
    void thread_write();
    // infinate looping thread
    // send status report requests
    //void thread_RequestStatusReport();
    //uint statusTimerInterval = 25;    // ms
};

} // end namespace Sqeak


#pragma once

// *********************** //
//     GRBL Settings 	   //
// *********************** //
#define SERIAL_DEVICE 			    "/dev/ttyAMA0"
#define SERIAL_BAUDRATE 		    115200

#define MAX_GRBL_BUFFER 		    128
#define MAX_GRBL_RECEIVE_BUFFER 	128

struct GRBLCoords_vals{
    point3D workCoords[6];		    // 	[G54:4.000,0.000,0.000]		work coords		can be changed with 	G10 L2 Px or G10 L20 Px
                                    //	[G55:4.000,6.000,7.000]
                                    //	[G56:0.000,0.000,0.000]
                                    //	[G57:0.000,0.000,0.000]
                                    //	[G58:0.000,0.000,0.000]
                                    //	[G59:0.000,0.000,0.000]
    point3D homeCoords[2];		    // 	[G28:1.000,2.000,0.000]		pre-defined positions 	can be changed with 	G28.1
                                    //	[G30:4.000,6.000,0.000]						 		G30.1
    point3D offsetCoords;		    //	[G92:0.000,0.000,0.000]		coordinate offset 
    float toolLengthOffset = 0.0f;	//	[TLO:0.000]			tool length offsets
    point3D probeOffset;		    //	[PRB:0.000,0.000,0.000:0]	probing
    bool probeSuccess = false;	    //			       ^
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
    std::string MotionMode;		        // *G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
    std::string CoordinateSystem;	    // *G54, G55, G56, G57, G58, G59
    std::string Plane;	                // *G17, G18, G19
    std::string DistanceMode;	        // *G90, G91
    std::string ArcIJKDistanceMode;	    // *G91.1
    std::string FeedRateMode;	        // G93, *G94
    std::string UnitsMode;	            // G20, *G21
    std::string CutterRadCompensation;	// *G40
    std::string ToolLengthOffset;	    // G43.1, *G49
    std::string ProgramMode;		    // *M0, M1, M2, M30
    std::string SpindleState;		    // M3, M4, *M5
    std::string CoolantState;		    // M7, M8, *M9

    int toolNumber 			    = 0;
    float spindleSpeed 			= 0.0f;
    float feedRate 			    = 0.0f;	
};

// Modal Group
// 	These define the last set (modal) values
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
enum GRBLState {
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
    GRBLState state 		    = GRBLState::Status_Unknown;
    point3D MPos;			    // either MPos if WPos is given (WPos = MPos - WCO)
    point3D WPos;
    point3D WCO;			    // this is recieved every 10 or 30 status messages
    int lineNum			        = 0;
    float feedRate		        = 0;	// mm/min or inches/min
    int spindleSpeed		    = 0; 	// rpm
    
    bool inputPin_LimX		    = 0;
    bool inputPin_LimY		    = 0;
    bool inputPin_LimZ		    = 0;
    bool inputPin_Probe		    = 0;
    bool inputPin_Door		    = 0;
    bool inputPin_Hold		    = 0;
    bool inputPin_SoftReset	    = 0;
    bool inputPin_CycleStart	= 0;
    
    int override_Feedrate	    = 0;
    int override_RapidFeed	    = 0;
    int override_SpindleSpeed	= 0;
    
    int accessory_SpindleDir	= 0;
    int accessory_FloodCoolant	= 0;
    int accessory_MistCoolant	= 0;
    
    int bufferPlannerAvail	    = 0;
    int bufferSerialAvail	    = 0;
    
};

// Realtime status values
//	These are the values recieved from the real time status report
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

struct MainSettings_vals 
{
    int min_SpindleSpeed 	= 0;
    int max_SpindleSpeed	= 24000;
    float max_FeedRateX 	= 6000;
    float max_FeedRateY 	= 6000;
    float max_FeedRateZ		= 6000;
    float max_FeedRate 		= 6000;	// internal use only
    std::string units_Distance 	= "mm";
    std::string units_Feed 	= "mm/min";
};

class MainSettings 
{
public:
    // returns a copy of vals
    const MainSettings_vals getVals();
    // sets units to mm or inches
    void setUnitsInches(bool isInches);
    
private:
    std::mutex m_mutex;
    MainSettings_vals m_vals;
    friend class GRBLSystem;
};

 
class GRBLSystem 
{
public:
    GRBLCoords 		coords;
    GRBLModal 		modal;
    GRBLStatus 		status;
    MainSettings 	settings;

private:

    // This takes a std::string of 3 values seperated by commas (,) and will return a 3DPoint
    // 4.000,0.000,0.000
    point3D stoxyz(const std::string& msg);   
    // checks Startup Line Execution for error	msg = ">G54G20:ok" or ">G54G20:error:X"
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
 

class GRBL 
{
public:
    GRBL();
    GCList gcList;
    Serial serial;
    GRBLSystem sys;
    
    void connect();
    void disconnect();
    bool isConnected();
    // this makes a copy of a const std::string (i.e Send("G90")) 
    // so that we can pass it to and manipulate it in lower 
    // down functions (i.e. removing whitespace etc)
    int send(const std::string& cmd);
    // adds to the GCode list, ready to be written when buffer has space
    // sending a pointer is slightly quicker as it wont have to be be copied, 
    // it will however, modify the original std::string to remove whitespace and comments etc
    // returns 0 on success, -1 on failure
    int send(std::string& cmd);
    int sendFile(const std::string& file);
    bool isFileRunning();
    void getFilePos(uint& pos, uint& total);
    // checks to be done prior to sending gcodes
    // this is seperated to allow checks to be done just once
    // if lots of gcodes are to be sent
    // these checks require mutexes to be locked and therefor may slow
    // down transfer if done many times
    int send_preChecks();
    // Sends an incremental jog to p
    // jogs do not affect the parser state 
    // therefore you do not need to set the machine back to G90 after using a G91 command
    // and the feedrate is not modal
    void sendJog(const point3D& p, int feedrate);
    /* sends a REALTIME COMMAND
     * 	- These are not considered as part of the streaming protocol and are executed instantly
     * 	- They do not require a line feed or carriage return after them.
     * 	- None of these respond with 'ok' or 'error'
     *	see https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands
     */ 
    void sendRT(char cmd);
    

    // soft reset 
    void softReset();
    // triggers a shutdown of all threads
    void shutdown();
    void setCommand(int cmd);
    
    // clears any gcodes which havent received a reponse 
    // (but ones left in grbl's buffer will still exectute)
    void cancel();
    // clears any completed GCodes in the buffer
    // used for clearing old commands in log
    void clearCompleted();
    // return last sent item
    int getLastItem(GCItem& item);
    // returns items in the gcode list for viewing
    // use index of -1 for last item sent
    const GCItem& getGCItem(uint index);
        // returns items in the gcode list for viewing
    uint getGCListSize();
    void setViewStatusReport(bool isViewable);
    bool getViewStatusReport();
    // set the interval timer for the status report
    // no faster than 5Hz (200ns)
    void setStatusInterval(uint ms);
    uint getStatusInterval();
    
private:
    std::mutex m_mutex;
    std::condition_variable m_cond_reset;
    std::condition_variable m_cond_threads;
    int m_runCommand;
    int m_threadsReady = 0;
    bool viewStatusReport = false;
    uint statusTimerInterval = 100;	// ms
    
    // returns true if end of execution of gcode from grbl ('ok' or 'error' received)
    int processResponse(const std::string& msg);
    void thread_statusReport();
    // resets threads when performing soft reset
    int resetThreads(int thread);
    // read from q
    // write to serial
    void thread_write();
    // read from serial
    // write back onto q
    void thread_read();

    friend void thread_statusReport(GRBL& grbl);
    friend void thread_write(GRBL& grbl);
    friend void thread_read(GRBL& grbl);
};


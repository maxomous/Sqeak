/*
 * grbl.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */
#pragma once


#include <queue>
	 
#define SERIAL_DEVICE 		"/dev/ttyAMA0"
#define SERIAL_BAUDRATE 	115200

#define MAX_GRBL_BUFFER 			128
#define MAX_GRBL_RECEIVE_BUFFER 	128

#define STATUS_UNSENT		-2	// not sent yet to grbl
#define STATUS_PENDING 		-1	// sent to grbl but no status received
#define STATUS_OK			0	// 'ok' received by grbl

#define GRBL_STATE_COLOUR_IDLE 		0
#define GRBL_STATE_COLOUR_MOTION 	1
#define GRBL_STATE_COLOUR_ALERT 	2

// ERROR STATUS NOW MATCHES GRBL's 	#define STATUS_ERROR		3	// 'error' received by grbl

// REALTIME COMMANDS
#define GRBL_RT_SOFT_RESET 							(char)0x18
#define GRBL_RT_STATUS_QUERY 						(char)'?'
#define GRBL_RT_RESUME								(char)'~'
#define GRBL_RT_HOLD								(char)'!'

#define GRBL_RT_DOOR								(char)0x84
#define GRBL_RT_JOG_CANCEL							(char)0x85

#define GRBL_RT_OVERRIDE_FEED_100PERCENT			(char)0x90
#define GRBL_RT_OVERRIDE_FEED_ADD_10PERCENT			(char)0x91
#define GRBL_RT_OVERRIDE_FEED_MINUS_10PERCENT		(char)0x92
#define GRBL_RT_OVERRIDE_FEED_ADD_1PERCENT			(char)0x93
#define GRBL_RT_OVERRIDE_FEED_MINUS_1PERCENT		(char)0x94

#define GRBL_RT_OVERRIDE_RAPIDFEED_100PERCENT		(char)0x95
#define GRBL_RT_OVERRIDE_RAPIDFEED_50PERCENT		(char)0x96
#define GRBL_RT_OVERRIDE_RAPIDFEED_25PERCENT		(char)0x97

#define GRBL_RT_OVERRIDE_SPINDLE_100PERCENT			(char)0x99
#define GRBL_RT_OVERRIDE_SPINDLE_ADD_10PERCENT		(char)0x9A
#define GRBL_RT_OVERRIDE_SPINDLE_MINUS_10PERCENT	(char)0x9B
#define GRBL_RT_OVERRIDE_SPINDLE_ADD_1PERCENT		(char)0x9C
#define GRBL_RT_OVERRIDE_SPINDLE_MINUS_1PERCENT		(char)0x9D

#define GRBL_RT_SPINDLE_STOP						(char)0x9E
#define GRBL_RT_FLOOD_COOLANT						(char)0xA0
#define GRBL_RT_MIST_COOLANT						(char)0xA1

typedef struct {
	std::string str;
	int status;
} GCItem_t;

class GCList {
public:
	// retrives the next item to be sent to grbl
	GCItem_t GetNextItem();
	// retrieves an item from the gclist
	GCItem_t GetItem(int n);
	// retrieves size of list
	int GetSize();
	// clears completed gcodes
	// used for 'clear commands' button
	void ClearCompleted();
	// clears any gcodes which have not received a response
	// used for 'cancel' button
	void ClearUnsent();
	// clears any gcodes which have not been sent yet or ones that have not received a response
	// used for soft resetting
	void ClearSent();
	// clears entire gcode list
	//void ClearAll();
	// adds gcode to GClist
	int Add(std::string& str);		
	// sets status of gcode (before grbl has responded
	void SetStatus(int status);
	// grbl has repsonded, this sets the response to the gcode in status
	void SetResponse(std::vector<std::string>* consoleLog, int response);
	// checks if any gcodes are waiting to be sent to grbl
	bool IsWaitingToSend();
	// sets file completed
	void EndOfFile();
	// returns true if we are mid file transfer
	bool IsFileRunning();
	// this triggers that we have sent a file 
	// and not to allow further commands until it complete 
	void FileStart();
	void FileSent();
	uint GetFileLines();
	uint GetFilePos();
private:
	// use gCodeList.size() to see how many have been added to buffer
	uint written = 0;	// how many sent to grbl
	uint read = 0;		// how many recieved a response from grbl
	// GCode List - a store of all gcodes sent (or en route) to grbl
	std::vector<GCItem_t> gCodeList;
	// denotes the end point of a file sent
	// prevents sending file multiple times
	uint fileStart = 0;
	uint fileEnd = 0;
	// function to clean gcode string (remove spaces / comments etc)
	void CleanString(std::string& str);
};

/*		GRBL GCode Parametes
* 
	[G54:4.000,0.000,0.000]		work coords				can be changed with 	G10 L2 Px or G10 L20 Px
	[G55:4.000,6.000,7.000]
	[G56:0.000,0.000,0.000]
	[G57:0.000,0.000,0.000]
	[G58:0.000,0.000,0.000]
	[G59:0.000,0.000,0.000]
	[G28:1.000,2.000,0.000]		pre-defined positions 	can be changed with 	G28.1
	[G30:4.000,6.000,0.000]												 		G30.1
	[G92:0.000,0.000,0.000]		coordinate offset 
	[TLO:0.000]					tool length offsets
	[PRB:0.000,0.000,0.000:0]	probing
*/

typedef struct 
{
public:
	// returns value
	const point3D& workCoords(size_t i) const 	{ if(i < 6) return workCoords_[i]; }	// G54 - G59
	const point3D& homeCoords(size_t i) const 	{ if(i < 2) return homeCoords_[i]; }	// G28 & G30
	const point3D& offsetCoords() const 		{ return offsetCoords_; }						// G92
	float toolLengthOffset() const 				{ return toolLengthOffset_; }
	const point3D& probeOffset() const 			{ return probeOffset_; }
	bool probeSuccess() const 					{ return probeSuccess_; }
	
private:
	point3D workCoords_[6];	// G54 - G59
	point3D homeCoords_[2];	// G28 & G30
	point3D offsetCoords_;	// G92
	float toolLengthOffset_ = 0.0f;
	point3D probeOffset_;
	bool probeSuccess_ = false;
	
	friend class GRBLParams;
	
} gCodeParams_t;


/*												
	Modal Group					Member Words	*default
Motion Mode					*G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
Coordinate System Select	*G54, G55, G56, G57, G58, G59
Plane Select				*G17, G18, G19
Distance Mode				*G90, G91
Arc IJK Distance Mode		*G91.1
Feed Rate Mode				G93, *G94
Units Mode					G20, *G21
Cutter Radius Compensation	*G40
Tool Length Offset			G43.1, *G49
Program Mode				*M0, M1, M2, M30
Spindle State				M3, M4, *M5
Coolant State				M7, M8, *M9

T tool number
S spindle speed
F feed rate,
*/
typedef struct 
{
public:
	// returns value
	const std::string& StartupBlock(size_t i) const 	{ if(i < 2) return StartupBlock_[i]; }
	
	const std::string& MotionMode() const 				{ return MotionMode_; }
	const std::string& CoordinateSystem() const 		{ return CoordinateSystem_; }
	const std::string& Plane() const 					{ return Plane_; }
	const std::string& DistanceMode() const 			{ return DistanceMode_; }
	const std::string& ArcIJKDistanceMode() const		{ return ArcIJKDistanceMode_; }
	const std::string& FeedRateMode() const 			{ return FeedRateMode_; }
	const std::string& UnitsMode() const 				{ return UnitsMode_; }
	const std::string& CutterRadCompensation() const 	{ return CutterRadCompensation_; }
	const std::string& ToolLengthOffset() const 		{ return ToolLengthOffset_; }
	const std::string& ProgramMode() const 				{ return ProgramMode_; }
	const std::string& SpindleState() const 			{ return SpindleState_; }
	const std::string& CoolantState() const				{ return CoolantState_; }

	int toolNumber() 									{ return toolNumber_; }
	float spindleSpeed() 								{ return spindleSpeed_; }
	float feedRate() 									{ return feedRate_; }	
	
private:
	
	std::string StartupBlock_[2];

	std::string MotionMode_ 			= "G0";
	std::string CoordinateSystem_ 		= "G54";
	std::string Plane_ 					= "G17";
	std::string DistanceMode_ 			= "G90";
	std::string ArcIJKDistanceMode_ 	= "G91.1";
	std::string FeedRateMode_ 			= "G94";
	std::string UnitsMode_ 				= "G21";
	std::string CutterRadCompensation_ 	= "G40";
	std::string ToolLengthOffset_ 		= "G49";
	std::string ProgramMode_ 			= "M0";
	std::string SpindleState_ 			= "M5";
	std::string CoolantState_ 			= "M9";

	int toolNumber_ 					= 0;
	float spindleSpeed_ 				= 0.0f;
	float feedRate_ 					= 0.0f;	
	
	friend class GRBLParams;
	// all of these are the last set (modal) values 
} modalGroup_t;

// realtime status values
typedef struct 
{	
public:
	// returns value
	const std::string& state() const 	{ return state_; }
	int stateColour() const 			{ return stateColour_; }
	const point3D& MPos() const 		{ return MPos_; }
	const point3D& WPos() const 		{ return WPos_; }
	const point3D& WCO() const 			{ return WCO_; }
	int lineNum() const 				{ return lineNum_; }
	float feedRate() const				{ return feedRate_; }
	int spindleSpeed() const			{ return spindleSpeed_; }
	
	bool inputPin_LimX() const 			{ return inputPin_LimX_; }
	bool inputPin_LimY() const 			{ return inputPin_LimY_; }
	bool inputPin_LimZ() const 			{ return inputPin_LimZ_; }
	bool inputPin_Probe() const 		{ return inputPin_Probe_; }
	bool inputPin_Door() const 			{ return inputPin_Door_; }
	bool inputPin_Hold() const 			{ return inputPin_Hold_; }
	bool inputPin_SoftReset() const 	{ return inputPin_SoftReset_; }
	bool inputPin_CycleStart() const 	{ return inputPin_CycleStart_; }
	
	int override_Feedrate() const 		{ return override_Feedrate_; }
	int override_RapidFeed() const 		{ return override_RapidFeed_; }
	int override_SpindleSpeed() const 	{ return override_SpindleSpeed_; }
	
	int accessory_SpindleDir() const	{ return accessory_SpindleDir_; }
	int accessory_FloodCoolant() const 	{ return accessory_FloodCoolant_; }
	int accessory_MistCoolant() const 	{ return accessory_MistCoolant_; }
	
private:
	std::string state_;
	int stateColour_ 				= 0;
	// either of these are given (WPos = MPos - WCO)
	point3D MPos_;
	point3D WPos_;
	// this is recieved every 10 or 30 status messages
	point3D WCO_;
	int lineNum_					= 0;
	float feedRate_					= 0;	// mm/min or inches/min
	int spindleSpeed_				= 0; 	// rpm
	
	bool inputPin_LimX_				= 0;
	bool inputPin_LimY_				= 0;
	bool inputPin_LimZ_				= 0;
	bool inputPin_Probe_			= 0;
	bool inputPin_Door_				= 0;
	bool inputPin_Hold_				= 0;
	bool inputPin_SoftReset_		= 0;
	bool inputPin_CycleStart_		= 0;
	
	int override_Feedrate_			= 0;
	int override_RapidFeed_			= 0;
	int override_SpindleSpeed_		= 0;
	
	int accessory_SpindleDir_		= 0;
	int accessory_FloodCoolant_		= 0;
	int accessory_MistCoolant_		= 0;
	
	friend class GRBLParams;
	
} grblStatus_t;

class MainSettings {
public:
	// sets units to mm or inches
	void SetUnitsInches(bool val);
	// returns value
	int min_SpindleSpeed() const 				{ return min_SpindleSpeed_; }
	int max_SpindleSpeed() const 				{ return max_SpindleSpeed_; }
	float max_FeedRateX() const 				{ return max_FeedRateX_; }
	float max_FeedRateY() const 				{ return max_FeedRateY_; }
	float max_FeedRateZ() const 				{ return max_FeedRateZ_; }
	float max_FeedRate() const 					{ return max_FeedRate_; }
	const std::string& units_Distance() const	{ return units_Distance_; }
	const std::string& units_Feed() const		{ return units_Feed_; }
	
private:
	int min_SpindleSpeed_ 			= 0;
	int max_SpindleSpeed_			= 24000;
	float max_FeedRateX_ 			= 6000;
	float max_FeedRateY_ 			= 6000;
	float max_FeedRateZ_			= 6000;
	float max_FeedRate_ 			= 6000;	// internal use only
	std::string units_Distance_ 	= "mm";
	std::string units_Feed_ 		= "mm/min";
	friend class GRBLParams;
};

class GRBLParams {
public:	
	gCodeParams_t gcParam;
	modalGroup_t mode;
	grblStatus_t status;
	MainSettings settings;
	
private:	
	// printAll - for debugging
	void Print();
	point3D stoxyz(const std::string& msg);
	void CheckStartupLine(const std::string& msg);
	void DecodeParameters(const std::string& msg);
	void DecodeStartupBlock(const std::string& msg) ;
	void DecodeMode(const std::string& msg);
	void SetState(const std::string& state);
	void DecodeStatus(const std::string& msg);
	std::string DecodeSettings(const std::string& msg);
	
	friend class GRBL;
};

class GRBL {
	public:
		GRBLParams Param;
		GCList gcList;	
		std::vector<std::string>* consoleLog;
		// flag to show status reponse in console
		bool viewStatusReport = false;
		 
		GRBL();
		~GRBL();
		
		// flushes the serial buffer
		void Flush();
		// initialises connection to the serial port
		int Connect();
		// disconnects connection to the serial port
		void Disconnect();
		// returns connection status
		bool IsConnected() { return connected; }
		// send command to serial port
		int Send(std::string& cmd);
		int Send(const std::string& cmd);
		// send a file to GRBL
		int SendFile(const std::string& file);
		// send jog command to p
		void SendJog(point3D p, int feedrate);
		// Writes realtime command to serial port
		void SendRT(char cmd);
		// stops any more commands being sent to grbl
		// note: any remaining command grbl has in it's buffer will still be executed
		void Cancel();
		// Resets GRBL and clears program running flag
		void SoftReset() ;
		// Something has gone very wrong and we need to reset everything
		void Reset();
		// Writes line to serial port
		void Write();
		// Reads block off serial port
		// returns true if new reponse
		void Read();
		// sets the interval time between status requests
		void SetStatusInterval(uint timems);
		// Sends a request to GRBL for a status report
		void RequestStatus();
		// a blocking function which waits for status to be read from grbl
		// returns 0 when status recieved, -1 on timeout
		int WaitForIdle();
		// returns true when mid file transfer
		bool IsFileRunning();	
		
	private:
		std::queue<int> q;
		bool connected = false;
		// flag to force-wait until we recieve a status repsonse
		bool waitingForStatus = false;
		int fd;
		int grblBufferSize = MAX_GRBL_BUFFER;
		// status report timer
		uint statusTimer = 0;
		uint statusTimerInterval = 100;
		// Reads line of serial port. 
		// Returns string and length in msg
		void ReadLine(std::string& msg);
		int BufferRemove();
		int BufferAdd(int len);
			
};

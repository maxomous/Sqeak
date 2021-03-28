/*
 * grbl.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#ifndef GRBL_HPP
#define GRBL_HPP


#define SERIAL_DEVICE 		"/dev/ttyAMA0"
#define SERIAL_BAUDRATE 	115200

#define MAX_GRBL_BUFFER 	128

#define STATUS_NONE			-2	// not sent yet to grbl
#define STATUS_PENDING 		-1	// sent to grbl but no status received
#define STATUS_OK			0	// 'ok' received by grbl


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



class GCList {
	public:
		// use size() to see how many have been added to buffer
		int written;	// how many sent to grbl
		int read;		// how many recieved a response from grbl
		// GCode List - a store of all gcodes sent (or en route) to grbl
		std::vector<std::string> str;
		std::vector<int> status;
		
		GCList();
		void GetListItem(int n, std::string& str, int& status);
		int GetSize();
		// clears completed gcodes
		// used for 'clear commands' button
		void ClearCompleted();
		// clears any gcodes which have not received a response
		// used for 'cancel' button
		void ClearNoResponses();
		// clears any gcodes which have not been sent yet or ones that have not received a response
		// used for soft resetting
		void ClearSent();
		// clears entire gcode list
		//void ClearAll();
		// adds gcode to GClist
		int Add(std::string* str);		
		// grbl has repsonded, this sets the response to the gcode in status
		void SetResponse(std::vector<std::string>* consoleLog, int response);
		// returns true if we are mid file transfer
		bool IsFileRunning();
		// this triggers that we have sent a file 
		// and not to allow further commands until it complete 
		void FileSent();
	private:
		// denotes the end point of a file sent
		// prevents sending file multiple times
		int fileEnd = 0;
		void CleanString(std::string* str);
		
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

typedef struct {
	point3D workCoords[6];	// G54 - G59
	point3D homeCoords[2];	// G28 & G30
	point3D offsetCoords;	// G92
	float toolLengthOffset;
	point3D probeOffset;
	bool probeSuccess;
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
typedef struct {
	std::string MotionMode;
	std::string CoordinateSystem;
	std::string Plane;
	std::string DistanceMode;
	std::string ArcIJKDistanceMode;
	std::string FeedRateMode;
	std::string UnitsMode;
	std::string CutterRadiusCompensation;
	std::string ToolLengthOffset;
	std::string ProgramMode;
	std::string SpindleState;
	std::string CoolantState;

	int toolNumber;
	float spindleSpeed;
	float feedRate;
} modalGroup_t;

// realtime status values
typedef struct {	
	std::string state;
	int stateColour = 0;
	// either of these are given 
	// WPos = MPos - WCO
	point3D MPos;
	point3D WPos;
	// this is given every 10 or 30 status messages
	point3D WCO;
	int lineNum;
	float feedRate;	// mm/min or inches/min
	int spindleSpeed; // rpm
	
	bool inputPin_LimX;
	bool inputPin_LimY;
	bool inputPin_LimZ;
	bool inputPin_Probe;
	bool inputPin_Door;
	bool inputPin_Hold;
	bool inputPin_SoftReset;
	bool inputPin_CycleStart;
	
	int override_Feedrate;
	int override_RapidFeed;
	int override_SpindleSpeed;
	
	int accessory_SpindleDirection;
	int accessory_FloodCoolant;
	int accessory_MistCoolant;
	
} grblStatus_t;

class MainSettings {
	public:
		float max_FeedRateX 			= 6000;
		float max_FeedRateY 			= 6000;
		float max_FeedRateZ 			= 6000;
		
		std::string units_Distance	= "mm";
		std::string units_Feed		= "mm/min";
		
		void SetUnitsInches(int val);
		
		MainSettings();
};

class GRBLParams {
	public:		
		MainSettings settings;
		std::string startupBlock[2];
		gCodeParams_t gcParam;
		modalGroup_t mode;
		grblStatus_t status;
		
		GRBLParams();
		
		void Print() ;
		point3D stoxyz(const std::string& msg);
		void CheckStartupLine(const std::string& msg);
		void DecodeParameters(const std::string& msg);
		void DecodeMode(const std::string& msg);
		void DecodeStatus(const std::string& msg);
		std::string DecodeSettings(const std::string& msg);
		//printAll
};

class GRBL {
	public:
		std::vector<std::string>* consoleLog;
		GRBLParams Param;
		bool viewStatusReport = false;
		
		GRBL();
		~GRBL();
		
		// flushes the serial buffer
		void Flush();
		// initialises connection to the serial port
		void Connect();
		// send command to serial port
		int Send(std::string* cmd);
		int Send(std::string cmd);
		// send a file to GRBL
		int SendFile(const std::string& file);
		// Writes realtime command to serial port
		void SendRT(char cmd);
		// send jog command
		void SendJog(int axis, int dir, float distance, int feedrate);
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
		Queue *q;
		GCList gcList;		
		
	private:
		// flag to force-wait until we recieve a status repsonse
		bool waitingForStatus = false;
		int fd;
		int grblBufferSize = MAX_GRBL_BUFFER;
		// status report timer
		uint statusTimer;
		uint statusTimerInterval;
		// Reads line of serial port. 
		// Returns string and length in msg
		void ReadLine(std::string* msg);
		int BufferRemove();
		int BufferAdd(int len);
		// callback function which executes a line from FileRun
		//int ExectuteFileLine(const string& cmd);
			
};

#endif

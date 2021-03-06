#ifndef GRBL_HPP
#define GRBL_HPP


#define SERIAL_DEVICE 		"/dev/ttyAMA0"
#define SERIAL_BAUDRATE 	115200

#define STATUS_NONE			-2	// not sent yet to grbl
#define STATUS_PENDING 		-1	// sent to grbl but no status received
#define STATUS_OK			0	// 'ok' received by grbl
// ERROR STATUS MATCHES GRBL's 	#define STATUS_ERROR		3	// 'error' received by grbl

#define MAX_GRBL_BUFFER 	128

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

static int grblBufferSize = MAX_GRBL_BUFFER;

class gcList_t {
	public:
		int count;
		int written;
		int read;
		
		std::vector<std::string> str;
		std::vector<int> status;
		
		gcList_t();
		void add(std::string str);		
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

class grblParams_t {
	public:
		std::string startupBlock[2];
		gCodeParams_t param;
		modalGroup_t mode;
		grblStatus_t status;
		
		
		grblParams_t();
};



// Reads line of serial port. 
// Returns string and length in msg
extern void grblReadLine(int fd, std::string* msg);
// Reads block off serial port
extern void grblRead(grblParams_t* grblParams, int fd, gcList_t* gcList, queue_t* q);
// Writes line to serial port
extern void grblWrite(int fd, gcList_t* gcList, queue_t* q);
// Writes realtime command to serial port
extern void grblRealTime(int fd, char cmd);
#endif

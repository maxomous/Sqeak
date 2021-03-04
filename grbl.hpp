#ifndef GRBL_HPP
#define GRBL_HPP


#define SERIAL_DEVICE 		"/dev/ttyAMA0"
#define SERIAL_BAUDRATE 	115200

#define STATUS_NONE			0	// not sent yet to grbl
#define STATUS_PENDING 		1	// sent to grbl but no status received
#define STATUS_OK			2	// 'ok' received by grbl
#define STATUS_ERROR		3	// 'error' received by grbl

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

class gcList_t {
	public:
		int count;
		int written;
		int read;
		
		std::vector<std::string> str;
		std::vector<int> status;
		
		gcList_t();
		~gcList_t();
		void add(std::string str);		
};

// Reads line of serial port. 
// Returns string and length in msg
extern void grblReadLine(int fd, std::string* msg);
// Reads block off serial port
extern void grblRead(int fd, gcList_t* gcList, queue_t* q);
// Writes line to serial port
extern void grblWrite(int fd, gcList_t* gcList, queue_t* q);
// Writes realtime command to serial port
extern void grblRealTime(int fd, char cmd);
#endif

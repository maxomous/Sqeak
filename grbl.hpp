#ifndef GRBL_HPP
#define GRBL_HPP


#define SERIAL_DEVICE 		"/dev/ttyAMA0"
#define SERIAL_BAUDRATE 	115200

#define MAX_MSG		256

#define STATUS_NONE		0	// not sent yet to grbl
#define STATUS_PENDING 	1	// sent to grbl but no status received
#define STATUS_OK		2	// 'ok' received by grbl
#define STATUS_ERROR	3	// 'error' received by grbl

#define GRBL_ADD_TO_STREAM 	TRUE
#define GRBL_DONT_STREAM 	FALSE

#define MAX_GRBL_BUFFER 128


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
/*
typedef struct{
	char* str;
	int status;
} gc_t;
*/

extern void gcListAdd(const char* str);
extern void gcListFree();	
// Reads line of serial interface. 
// Returns string and length in msg
extern void grblReadLine(int fd, char* msg, int* len);
// Reads block of serial interface until no response received
extern void grblRead(int fd, gcList_t* gcList, queue_t* q, bool stream);
extern void grblWrite(int fd, gcList_t* gcList, queue_t* q);
#endif

/*
 * grbl.c
 */

#include "common.hpp"



//static gc_t** gcList;
static int grblBufferSize = MAX_GRBL_BUFFER;


gcList_t::gcList_t(){
	this->count = 0;
	this->written = 0;
	this->read = 0;
}

gcList_t::~gcList_t(){
	/*for (int i = 0; i < this->count; i++) {
		free(gcList[i]->str);
		free(gcList[i]);
	}
	free(gcList);*/
}
/*
void gcListFree() {
	for (int i = 0; i < gcListCount; i++) {
		free(gcList[i]->str);
		free(gcList[i]);
	}
	free(gcList);
}
*/

void gcList_t::add(std::string str) {
	if(str.length() > MAX_GRBL_BUFFER)
		exitf("ERROR: String is longer than grbl buffer!\n");
	
	this->str.push_back(str);
	this->status.push_back(STATUS_NONE);
	
	this->count++;
}	

/*
void gcList_t::add(const char* str) {
	// allocate memory for a gcode struct
	gc_t* gc = (gc_t*)malloc( sizeof(gc_t) );
	assert(gc);
	memset(gc, '\0', sizeof(gc_t));
	// allocate memory for string inside the struct
	int len = strlen(str) + 1;
	if(len > MAX_GRBL_BUFFER)
		exitf("ERROR: String is longer than grbl buffer!\n");
	gc->str = (char*)malloc(len);
	assert(gc->str);
	// copy data onto struct
	if(gc->str)
		strncpy(gc->str, str, len);
	gc->status = STATUS_NONE;
	// copy struct onto gcList and allocate memory
	gcList = (gc_t**)realloc(gcList, sizeof (gc_t) * (gcListCount+1));
	gcList[gcListCount] = gc;
	gcListCount++;
}	
*/
	

		
// Reads line of serial interface. 
// Returns string and length in msg
void grblReadLine(int fd, std::string* msg) {
	
	msg->clear();
			
	char buf;
	//retrieve line
	do {
		// retrieve letter
		buf = serialGetchar(fd);
		// break if end of line
		if (buf == '\n')
			break;
		// add to buffer - skip non-printable characters
		if(isprint(buf)) 
			*msg += buf;
	} while(1);
}

// Reads block of serial interface until no response received
void grblRead(int fd, gcList_t* gcList, queue_t* q) {
	
	std::string msg;
	// retrieve data upto response 'ok' or 'error'
	do {
		if(!serialDataAvail(fd))
			break;
		
		grblReadLine(fd, &msg);
			
		if(!msg.compare("ok")){	
			int lastInQueue = q->dequeue();		// TODO sort this properly
			if(lastInQueue == 0) {
				printf("ERROR: Queue is empty!\n");
				std::cout << msg << std::endl;
				break;
			}
			grblBufferSize += lastInQueue;
			std::cout << " buf: " << grblBufferSize << "/" << MAX_GRBL_BUFFER << '\t';
			
			gcList->status[gcList->read] = STATUS_OK;
			std::cout << '#' << gcList->read << " " << gcList->str[gcList->read] <<  "status: " << msg << std::endl;
			gcList->read++;
			break; 
		}
		else if(!msg.compare(0, 6, "error:")){		
			int errcode = std::stoi(msg.substr(6));
			std::cout << "error code = " << errcode << std::endl;		
			
			int lastInQueue = q->dequeue();		// TODO sort this properly
			if(lastInQueue == 0) {
				printf("ERROR: Queue is empty!\n");
				std::cout << msg << std::endl;
				break;
			}
			grblBufferSize += lastInQueue;
			std::cout << " buf: " << grblBufferSize << "/" << MAX_GRBL_BUFFER << '\t';
			
			gcList->status[gcList->read] = STATUS_ERROR;
			std::cout << '#' << gcList->read << " " << gcList->str[gcList->read] <<  "status: " << msg << std::endl;
			gcList->read++;
			break;
		}
		else {			
			std::cout << "Unsupported message: " << msg << std::endl;
			
		}
			
	} while(1);	
}

void grblWrite(int fd, gcList_t* gcList, queue_t* q) {
	
	do {
		// exit if nothing new in the gcList
		if (gcList->written >= gcList->count) //gcListWritten >= gcListCount)
			break;
			
		std::string* curStr = &gcList->str[gcList->written];
		int len = curStr->length();
		
		// exit if buffer full
		if(grblBufferSize - len < 0)
			break;
		// reduce buffer size by length of string
		grblBufferSize -= len;
		// add length of string to queue
		if(!q->enqueue(len)) 
			exitf("ERROR: Queue full\n");
		// set status
		gcList->status[gcList->written] = STATUS_PENDING;
		// write to serial port
		serialPuts(fd, curStr->c_str());
		gcList->written++;
		//printf("ADDING #%d: %s", gcListWritten, gc->str.c_str);
	} while (1);
}

/* sends a REALTIME COMMAND
 * 	- These are not considered as part of the streaming protocol and are executed instantly
 * 	- They do not require a line feed or carriage return after them.
 * 	- None of these respond with 'ok' or 'error'
 *	see https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands
 */ 
void grblRealTime(int fd, char cmd) {
	
	switch (cmd) {
		case GRBL_RT_SOFT_RESET:
			printf("Sent: 'Soft Reset'\n");
			break;
		case GRBL_RT_STATUS_QUERY:
			printf("Sent: 'Status Query'\n");
			break;
		case GRBL_RT_HOLD:
			printf("Sent: 'Hold'\n");
			break;
		case GRBL_RT_RESUME:
			printf("Sent: 'Resume'\n");
			break;
			
		case GRBL_RT_DOOR:
			printf("Sent: 'Door'\n");
			break;
		case GRBL_RT_JOG_CANCEL:
			printf("Sent: 'Cancel Jog'\n");
			break;
			
		case GRBL_RT_OVERRIDE_FEED_100PERCENT:
			printf("Sent: 'Override Feedrate (Set to 100%)'\n");
			break;
		case GRBL_RT_OVERRIDE_FEED_ADD_10PERCENT:
			printf("Sent: 'Override Feedrate (+10%)'\n");
			break;
		case GRBL_RT_OVERRIDE_FEED_MINUS_10PERCENT:
			printf("Sent: 'Override Feedrate (-10%)'\n");
			break;
		case GRBL_RT_OVERRIDE_FEED_ADD_1PERCENT:
			printf("Sent: 'Override Feedrate (+1%)'\n");
			break;
		case GRBL_RT_OVERRIDE_FEED_MINUS_1PERCENT:
			printf("Sent: 'Override Feedrate (-1%)'\n");
			break;
			
		case GRBL_RT_OVERRIDE_RAPIDFEED_100PERCENT:
			printf("Sent: 'Override Rapid Feedrate (Set to 100%)'\n");
			break;
		case GRBL_RT_OVERRIDE_RAPIDFEED_50PERCENT:
			printf("Sent: 'Override Rapid Feedrate (Set to 50%)'\n");
			break;
		case GRBL_RT_OVERRIDE_RAPIDFEED_25PERCENT:
			printf("Sent: 'Override Rapid Feedrate (Set to 25%)'\n");
			break;
			
		case GRBL_RT_OVERRIDE_SPINDLE_100PERCENT:
			printf("Sent: 'Override Spindle Speed (Set to 100%)'\n");
			break;
		case GRBL_RT_OVERRIDE_SPINDLE_ADD_10PERCENT:
			printf("Sent: 'Override Spindle Speed (+10%)'\n");
			break;
		case GRBL_RT_OVERRIDE_SPINDLE_MINUS_10PERCENT:
			printf("Sent: 'Override Spindle Speed (-10%)'\n");
			break;
		case GRBL_RT_OVERRIDE_SPINDLE_ADD_1PERCENT:
			printf("Sent: 'Override Spindle Speed (+1%)'\n");
			break;
		case GRBL_RT_OVERRIDE_SPINDLE_MINUS_1PERCENT:
			printf("Sent: 'Override Spindle Speed (-1%)'\n");
			break;
			
		case GRBL_RT_SPINDLE_STOP:
			printf("Sent: 'Stop Spindle'\n");
			break;
		case GRBL_RT_FLOOD_COOLANT:
			printf("Sent: 'Flood Coolant'\n");
			break;
		case GRBL_RT_MIST_COOLANT:
			printf("Sent: 'Mist Coolant'\n");
			break;

			
		default:
			exitf("ERROR: Realtime command not recognised: %c\n", cmd);
	}
	serialPutchar(fd, cmd);
}

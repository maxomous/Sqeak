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
void grblReadLine(int fd, char* msg, int* len) {
	
	memset(msg, '\0', MAX_MSG);	
	*len = 0;
			
	char buf;
	//retrieve line
	do {
		// retrieve letter
		buf = serialGetchar(fd);
		// break if end of line
		if (buf == '\n')
			break;
		// add to buffer - skip non-printable characters
		if(isprint(buf)) {
			msg[(*len)++] = buf;
			if(*len >= MAX_MSG)
				exitf("ERROR: Msg length has been exceeded\n");
		}
	} while(1);
}

// Reads block of serial interface until no response received
void grblRead(int fd, gcList_t* gcList, queue_t* q, bool stream) {
	
	char msg[MAX_MSG];
	int len;
	
	// retrieve data upto response 'ok' or 'error'
	do {
		// if we have just sent a realtime command, wait for it			-  TODO: not sure I like this as it doesn't cover what happens if we receive a response for wrong thing?
		if (stream == GRBL_DONT_STREAM) {
			do {
				if (serialDataAvail(fd))
					break;
			} while (1);
		}
		else {
			if(!serialDataAvail(fd))
				break;
		}
		grblReadLine(fd, msg, &len);
			
		if(!strncmp(msg, "ok", MAX_MSG)){	
			if(stream == GRBL_ADD_TO_STREAM) {					// not sure im happy with this
				int lastInQueue = q->dequeue();
				if(lastInQueue == 0) 
					exitf("ERROR: Queue is empty!");
				grblBufferSize += lastInQueue;
				printf("buffer size = %d\n", grblBufferSize);
				//gcList[gcListRead]->status = STATUS_OK;
				gcList->status[gcList->read] = STATUS_OK;
				//printf("#%d: %s %s\n", gcListRead, gcList[gcListRead]->str.c_str, msg);
				printf("#%d: %s %s\n", gcList->read, gcList->str[gcList->read].c_str(), msg);
				gcList->read++;
			}
			break; 
		}
		else if(!strncmp(msg, "error", MAX_MSG)){			
			printf("error received\n");
			break;
		}
		else {			
			//printf("length = %d\n", len);
			printf("%s\n", msg);
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

/* REALTIME COMMAND
 * 	- These are not considered as part of the streaming protocol and are executed instantly
 * 	- Does not require a line feed or carriage return after them.
	
		0x18 (ctrl-x) : Soft-Reset

			Immediately halts and safely resets Grbl without a power-cycle.
			Accepts and executes this command at any time.
			If reset while in motion, Grbl will throw an alarm to indicate position may be lost from the motion halt.
			If reset while not in motion, position is retained and re-homing is not required.
			An input pin is available to connect a button or switch.
		
		? : Status Report Query

			Immediately generates and sends back runtime data with a status report.
			Accepts and executes this command at any time, except during a homing cycle and when critical alarm (hard/soft limit error) is thrown.
		
		~ : Cycle Start / Resume

			Resumes a feed hold, a safety door/parking state when the door is closed, and the M0 program pause states.
			Command is otherwise ignored.
			If the parking compile-time option is enabled and the safety door state is ready to resume, Grbl will re-enable the spindle and coolant, move back into position, and then resume.
			An input pin is available to connect a button or switch.
		
		! : Feed Hold

			Places Grbl into a suspend or HOLD state. If in motion, the machine will decelerate to a stop and then be suspended.
			Command executes when Grbl is in an IDLE, RUN, or JOG state. It is otherwise ignored.
			If jogging, a feed hold will cancel the jog motion and flush all remaining jog motions in the planner buffer. The state will return from JOG to IDLE or DOOR, if was detected as ajar during the active hold.
			By machine control definition, a feed hold does not disable the spindle or coolant. Only motion.
			An input pin is available to connect a button or switch.
 * 
 * */

/*
 * $$ and $x=val - View and write Grbl settings
 * 
 * 
 * $# - View gcode parameters
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






/*
	void  serialPutchar (int fd, unsigned char c) ;
	Sends the single byte to the serial device identified by the given file descriptor.

	void  serialPuts (int fd, char *s) ;
	Sends the nul-terminated string to the serial device identified by the given file descriptor.

	void  serialPrintf (int fd, char *message, …) ;
	Emulates the system printf function to the serial device.

	int   serialDataAvail (int fd) ;
	Returns the number of characters available for reading, or -1 for any error condition, in which case errno will be set appropriately.

	int serialGetchar (int fd) ;
	Returns the next character available on the serial device. This call will block for up to 10 seconds if no data is available (when it will return -1)

	void serialFlush (int fd) ;
	This discards all data received, or waiting to be send down the given device.
*/

/* **********PROBLEMS FOR LATER************
 * 
 * should recieve this:
		Once connected you should get the Grbl-prompt, which looks like this:
		Grbl 1.1e ['$' for help]
* 
* */

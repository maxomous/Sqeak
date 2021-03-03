/*
 * grbl.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#include "queue.h"

#define SERIAL_DEVICE 		"/dev/ttyAMA0"
#define SERIAL_BAUDRATE 	115200

#define MAX_MSG		256

#define STATUS_NONE		0	// not sent yet to grbl
#define STATUS_PENDING 	1	// sent to grbl but no status received
#define STATUS_OK		2	// 'ok' received by grbl
#define STATUS_ERROR	3	// 'error' received by grbl





typedef struct	{
	char str[MAX_MSG];
	int len;
} msg_t;


void exitf(char* str) {
	printf(str);
	exit(1);
}


#define MAX_GRBL_BUFFER 128
int grblBufferSize = MAX_GRBL_BUFFER;

typedef struct{
	char* str;
	int status;
} gc_t;

gc_t** gcList;
int gcListCount = 0;
int gcListWritten = 0;
int gcListRead = 0;

void gcListAdd(char* str) {
	// allocate memory for a gcode struct
	gc_t* gc = malloc( sizeof(gc_t) );
	assert(gc);
	memset(gc, 0, sizeof(gc_t));
	// allocate memory for string inside the struct
	int len = strlen(str) + 1;
	if(len > MAX_GRBL_BUFFER)
		exitf("ERROR: String is longer than grbl buffer!\n");
	gc->str = (char*)malloc(len);
	// copy data onto struct
	if(gc->str)
		strncpy(gc->str, str, len);
	gc->status = STATUS_NONE;
	// copy struct onto gcList and allocate memory
	gcList = realloc(gcList, sizeof (gc_t) * (gcListCount+1));
	gcList[gcListCount] = gc;
	gcListCount++;
}	

void gcListFree() {
	for (int i = 0; i < gcListCount; i++) {
		free(gcList[i]->str);
		free(gcList[i]);
	}
	free(gcList);
}


void clearmsg(msg_t* msg) {
	memset(msg->str, '\0', MAX_MSG);
	msg->len = 0;
}
	
// Reads line of serial interface. 
// Returns string and length in msg
void grblReadLine(int fd, msg_t* msg) {
	
	clearmsg(msg);
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
			msg->str[msg->len++] = buf;
			if(msg->len >= MAX_MSG)
				exitf("ERROR: Msg length has been exceeded\n");
		}
	} while(1);
}

// Reads block of serial interface until no response received
void grblRead(int fd, queue_t* q) {
	
	
	
	msg_t msg;
	// retrieve data upto response 'ok' or 'error'
	do {
		if(!serialDataAvail(fd))
			return;
			
		grblReadLine(fd, &msg);
			
		if(!strncmp(msg.str, "ok", msg.len)){	
			grblBufferSize += dequeue(q);
			printf("buffer size = %d\n", grblBufferSize);
			
			gcList[gcListRead]->status = STATUS_OK;
			printf("ACKNOWLEDGED: #%d: %s OK\n", gcListRead, gcList[gcListRead]->str);
			gcListRead++;
			break; 
		}
		else if(!strncmp(msg.str, "error", msg.len)){			
			printf("error received\n");
			break;
		}
		else {			
			//printf("length = %d\n", msg.len);
			printf("%s\n", msg.str);
		}
			
	} while(1);	
}

void grblWrite(int fd) {
	(void)fd;
	return;
}

int main(int argc, char **argv)
{
	(void)argc, (void) argv;
		
	int wiringPiSetup(void);

	int fd = serialOpen(SERIAL_DEVICE, SERIAL_BAUDRATE);
	if(fd == -1)
		exitf("ERROR: Could not open serial device\n");
	
	//serialPuts(fd, "\r\n\r\n");
	//delay(2000);
	//serialFlush(fd);	
	
	
	queue_t* q = init_queue(128);
	
	gcListAdd("$X\n");
	gcListAdd("$$\n");
	gcListAdd("G1 X10 Y20 Z50 F1000\n");
	gcListAdd("G1 X20 Y10 Z10\n");
	gcListAdd("G1 X30 Y20 Z50\n");
	gcListAdd("G1 X40 Y10 Z10\n");
	gcListAdd("G1 X10 Y20 Z50\n");
	gcListAdd("G1 X20 Y10 Z10\n");
	gcListAdd("G1 X30 Y20 Z50\n");
	gcListAdd("G1 X40 Y10 Z10\n");
	gcListAdd("G1 X10 Y20 Z50\n");
	gcListAdd("G1 X20 Y10 Z10\n");
	
	
	/*serialPuts(fd, "$\n");
	serialPuts(fd, "$$\n");

	serialPuts(fd, "$X\n");
	
	serialPuts(fd, "$#\n");*/
	//serialPuts(fd, "G00 X5\n");
	//serialPuts(fd, "G00 y5\n");
	
	//uint time = millis() + 1000;
	
	do {
		grblRead(fd, q);
		
		//grblWrite(fd);
		while (1) {
			if (gcListWritten >= gcListCount)
				break;
			gc_t* gc = gcList[gcListWritten];
			int len = strnlen(gc->str, MAX_GRBL_BUFFER); 
			if(grblBufferSize - len < 0)
				break;
			grblBufferSize -= len;
			if(!enqueue(q, len)) 
				exitf("ERROR: Queue full\n");
			gc->status = STATUS_PENDING;
			//printf("ADDING #%d: %s", gcListWritten, gc->str);
			serialPuts(fd, gc->str);
			gcListWritten++;
		}
		
		
		
		/*if(millis() > time) {
			serialPuts(fd, "?\n");
			time += 1000;
		}*/
		
	} while (1);
	
	
	
	// close serial connection
	serialClose(fd);
	// free gcList
	gcListFree();
	
	return 0;
}


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

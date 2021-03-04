
#include "common.hpp"

// - I dont quite understand new - i get that it allocates memory
// 		-is it just used when making a pointer and thats all? if so that makes sense
// - Whats the naming convention for classes? im still using x_t
// - Inlcudes are probably for c std libraries
// - static int grblBufferSize = MAX_GRBL_BUFFER; in grbl.cpp are global, should I do these differently?
// - do i need to destruct gcList_t?
// - dont like how we are handling real time commands in grblRead
// - difference between passing by reference and pointer
int main(int argc, char **argv)
{
	(void)argc, (void) argv;
		
	int wiringPiSetup(void);

	int fd = serialOpen(SERIAL_DEVICE, SERIAL_BAUDRATE);
	if(fd == -1)
		exitf("ERROR: Could not open serial device\n");
	
	serialPuts(fd, "\r\n\r\n"); // what is this doing?
	delay(2000);
	serialFlush(fd);	
	
	grblRealTime(fd, GRBL_RT_SOFT_RESET);	
	grblRealTime(fd, GRBL_RT_STATUS_QUERY);	
	
	
	queue_t* q = new queue_t(128);			
	
	gcList_t* gcList = new gcList_t;
	// add gcodes to the stream
	gcList->add("$X\n");
	gcList->add("$$\n");
	gcList->add("G91\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	/*
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");*/
	/*gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X10 Y20 Z50 F6000\n");
	gcList->add("G4 P1\n");
	gcList->add("G1 X-10 Y-20 Z-50 F6000\n");
	gcList->add("G4 P1\n");
	*/
	/*serialPuts(fd, "$\n");
	serialPuts(fd, "$$\n");

	serialPuts(fd, "$X\n");
	
	serialPuts(fd, "$#\n");*/
	//serialPuts(fd, "G00 X5\n");
	//serialPuts(fd, "G00 y5\n");
	
	uint requestTime = millis() + 1000;
	
	uint requestTimeHold = millis() + 4000;
	uint requestTimeResume = millis() + 7000;
	do {
		grblWrite(fd, gcList, q);
		grblRead(fd, gcList, q);
		
		
		
		
		/*
		if(millis() > requestTime) {
			grblRealTime(fd, GRBL_RT_STATUS_QUERY);			
			requestTime += 1000;
		}
		*/
		/*
		if(millis() > requestTimeHold) {
			grblRealTime(fd, GRBL_RT_OVERRIDE_FEED_100PERCENT);	
			requestTimeHold += 3000;
		}
		*
		/*if(millis() > requestTimeResume) {
			grblRealTime(fd, GRBL_RT_FLOOD_COOLANT);	
			requestTimeResume += 10000;
			
		}*/
	} while (1);
	
	// close serial connection
	serialClose(fd);
	
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

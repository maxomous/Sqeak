
#include "common.hpp"

using namespace std;

// - I dont quite understand new - i get that it allocates memory
// 		-is it just used when making a pointer and thats all? if so that makes sense
// - Whats the naming convention for classes? im still using x_t
// - Inlcudes are probably for c std libraries
// - static int grblBufferSize = MAX_GRBL_BUFFER; in grbl.cpp are global, should I do these differently?
// - do i need to destruct gcList_t?
// - dont like how we are handling real time commands in grblRead
// - difference between passing by reference and pointer
// - is the buffer cleared out when theres an alarm ??
int main(int argc, char **argv)
{
	//cout << *argv << endl;

	
	(void)argc, (void) argv;
		
	int wiringPiSetup(void);

	int fd = serialOpen(SERIAL_DEVICE, SERIAL_BAUDRATE);
	if(fd == -1)
		exitf("ERROR: Could not open serial device\n");
	
	//clear the serial buffer
	serialPuts(fd, "\r\n\r\n"); // not actually sure what this does but it is given in the grbl example
	delay(2000);
	serialFlush(fd);	
	
	grblRealTime(fd, GRBL_RT_SOFT_RESET);	
	grblRealTime(fd, GRBL_RT_STATUS_QUERY);	
	
	grblParams_t* grblParams = new grblParams_t;
	
	queue_t* q = new queue_t(128);			
	
	gcList_t* gcList = new gcList_t;
	// add gcodes to the stream
	gcList->add("$X");
	/*
	
	gcList->add("M3 S1232.0");
	gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	
	gcList->add("G1 X10 Y20 Z50 F6000");
	gcList->add("G4 P1");
	
	gcList->add("$$");
	gcList->add("$#");
	gcList->add("$G");*/
	gcList->add("$I");
	
	/* JOG
	gcList->add("$J=G91 X10 F1000");
	grblRealTime(fd, GRBL_RT_JOG_CANCEL);
	*/
	
	
	/* PROBE 
	gcList->add("G91 G38.2 Z-200 F100\n");
	gcList->add("G91 G38.4 Z1 F100\n");
	*/
	
	/*gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	
	gcList->add("G1 X10 Y20 Z50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X10 Y20 Z50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X10 Y20 Z50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X10 Y20 Z50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X10 Y20 Z50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X10 Y20 Z50 F6000");
	gcList->add("G4 P1");
	gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	*/
	
	uint requestTime = millis() + 1000;
	
	uint requestTimeHold = millis() + 1000;
	uint requestTimeResume = millis() + 7000;
	do {
		grblWrite(fd, gcList, q);
		grblRead(grblParams, fd, gcList, q);
		
		
		
		
		/*
		if(millis() > requestTime) {
			// no more that 5Hz (200ns)
			grblRealTime(fd, GRBL_RT_STATUS_QUERY);			
			requestTime += 1000;
		}
		*/
		
		if(millis() > requestTimeHold) {
			requestTimeHold += 3000;
		/*	grblRealTime(fd, GRBL_RT_OVERRIDE_FEED_100PERCENT);	
			for (int i = 0; i < 6; i++) {
				cout << "Work Coord G" << 54+i << " = " << grblParams->param.workCoords[i].x << ", " << grblParams->param.workCoords[i].y << ", " << grblParams->param.workCoords[i].z << endl;
			}
			
			cout << "Home Coord G28 = " << grblParams->param.homeCoords[0].x << ", " << grblParams->param.homeCoords[0].y << ", " << grblParams->param.homeCoords[0].z << endl;
			cout << "Home Coord G30 = " << grblParams->param.homeCoords[1].x << ", " << grblParams->param.homeCoords[1].y << ", " << grblParams->param.homeCoords[1].z << endl;
			
			cout << "Offset Coord G92 = " << grblParams->param.offsetCoords.x << ", " << grblParams->param.offsetCoords.y << ", " << grblParams->param.offsetCoords.z << endl;
			cout << "TLO = " << grblParams->param.toolLengthOffset << endl;
			
			cout << "Probe = " << grblParams->param.probeOffset.x << ", " << grblParams->param.probeOffset.y << ", " << grblParams->param.probeOffset.z << endl;
			cout << "Probe Success = " << grblParams->param.probeSuccess << endl << endl;
			
			
			cout << "MotionMode = " << grblParams->mode.MotionMode << endl;
			cout << "CoordinateSystem = " << grblParams->mode.CoordinateSystem << endl;
			cout << "Plane = " << grblParams->mode.Plane << endl;
			cout << "DistanceMode = " << grblParams->mode.DistanceMode << endl;
			cout << "ArcIJKDistanceMode = " << grblParams->mode.ArcIJKDistanceMode << endl;
			cout << "FeedRateMode = " << grblParams->mode.FeedRateMode << endl;
			cout << "UnitsMode = " << grblParams->mode.UnitsMode << endl;
			cout << "CutterRadiusCompensation = " << grblParams->mode.CutterRadiusCompensation << endl;
			cout << "ToolLengthOffset = " << grblParams->mode.ToolLengthOffset << endl;
			cout << "ProgramMode = " << grblParams->mode.ProgramMode << endl;
			cout << "SpindleState = " << grblParams->mode.SpindleState << endl;
			cout << "CoolantState = " << grblParams->mode.CoolantState << endl;
			cout << "toolNumber = " << grblParams->mode.toolNumber << endl;
			cout << "spindleSpeed = " << grblParams->mode.spindleSpeed << endl;
			cout << "feedRate = " << grblParams->mode.feedRate << endl;
			*/
			cout << "Startup Block 1 = " << grblParams->startupBlock[0] << endl;
			cout << "Startup Block 2 = " << grblParams->startupBlock[1] << endl;
		}
		
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

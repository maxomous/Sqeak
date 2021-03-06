
#include "common.hpp"

using namespace std;



/* **********PROBLEMS FOR LATER************
 * 
 * should recieve this: - i get this when i send a $X reset?
		Once connected you should get the Grbl-prompt, which looks like this:
		Grbl 1.1e ['$' for help]


// - I dont quite understand new - i get that it allocates memory
// 		-is it just used when making a pointer and thats all? if so that makes sense
// - Whats the naming convention for classes? im still using x_t
// - Inlcudes are probably for c std libraries
// - static int grblBufferSize = MAX_GRBL_BUFFER; in grbl.cpp are global, should I do these differently?
// - do i need to destruct gcList_t?
// - dont like how we are handling real time commands in grblRead
// - difference between passing by reference and pointer
// - is the buffer cleared out when theres an alarm ??
// test all value are coming through ok - in particular coord systems as i modified the code
// - should check if in mm or inches ($13) as everything returned from grbl is based on those units

// on error, halt
// $C on open file?
// to sync gui to grbl use G4 P0.01
// handle errors cleanly
	// i.e.  if(b != string::npos) ... else handle error

/*
	EEPROM Issues
	EEPROM access on the Arduino AVR CPUs turns off all of the interrupts while the CPU writes to EEPROM. This poses a problem for certain features in Grbl, particularly if a user is streaming and running a g-code program, since it can pause the main step generator interrupt from executing on time. Most of the EEPROM access is restricted by Grbl when it's in certain states, but there are some things that developers need to know.

	Settings should not be streamed with the character-counting streaming protocols. Only the simple send-response protocol works. This is because during the EEPROM write, the AVR CPU also shuts-down the serial RX interrupt, which means data can get corrupted or lost. This is safe with the send-response protocol, because it's not sending data after commanding Grbl to save data.
	For reference:

	Grbl's EEPROM write commands: G10 L2, G10 L20, G28.1, G30.1, $x=, $I=, $Nx=, $RST=
	Grbl's EEPROM read commands: G54-G59, G28, G30, $$, $I, $N, $#
*/


int main(int argc, char **argv)
{
	//cout << *argv << endl;
	
	

	




  /*
  
  
	istringstream stream(s); 
	string token; 
	size_t pos = -1; 
  
	while (stream >> token) { 
		// If ',' is found then tokenize 
		// the string token 
		while ((pos = token.rfind('|')) 
			   != std::string::npos) { 
			token.erase(pos, 1); 
		} 
  
		// Print the tokenize string 
		cout << token << '\n'; 
	} */
	

	//return 0;

	
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
	
	// GRBL SETTINGS
	// gcList->add("$$");
	
	// GCODE PARAMETERS (coord systems)
	//gcList->add("$#");
	
	// MODAL STATES
	//gcList->add("$G");
	
	// BUILD INFO
	//gcList->add("$I");
	
	// STARTUP BLOCKS
	//gcList->add("$N");
	
	
	// CHECK MODE (send again to cancel)
	//gcList->add("$C");
	
	// KILL ALARM LOCK
	gcList->add("$X");
	
	// RUN HOMING CYCLE
	//gcList->add("$H");
	
	// JOG
	//gcList->add("$J=G91 X10 F1000");
	//grblRealTime(fd, GRBL_RT_JOG_CANCEL);
	
	// RESET SETTINGS ( USE WITH CATION ) 
	// "$RST=$", "$RST=#", and "$RST=*"
	
	// SLEEP
	//gcList->add("$SLP");
	
	
	
	/* PROBE 
	gcList->add("G91 G38.2 Z-200 F100\n");
	gcList->add("G91 G38.4 Z1 F100\n");
	*/
	/*
	
	gcList->add("M3 S1232.0");
	gcList->add("G1 X-10 Y-20 Z-50 F6000");
	gcList->add("G4 P1");
	
	gcList->add("G1 X10 Y20 Z50 F6000");
	gcList->add("G4 P1");
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
	
	uint timerStatusQuery = millis() + 1000;
	
	uint timer2 = millis() + 2000;
	uint timer3 = millis() + 7000;
	do {
		grblWrite(fd, gcList, q);
		grblRead(grblParams, fd, gcList, q);
		
		
		
		
		
		if(millis() > timerStatusQuery) {
			// no more that 5Hz (200ns)
			grblRealTime(fd, GRBL_RT_STATUS_QUERY);			
			timerStatusQuery = millis() + 1000;
		}
		
		
		if(millis() > timer2) {
			
			
			timer2 = millis() + 2000;
			grblRealTime(fd, GRBL_RT_OVERRIDE_FEED_100PERCENT);	
			for (int i = 0; i < 6; i++) {
				cout << "Work Coord G" << 54+i << " = " << grblParams->param.workCoords[i].x << ", " << grblParams->param.workCoords[i].y << ", " << grblParams->param.workCoords[i].z << endl;
			}
			
			cout << "Home Coord G28 = " << grblParams->param.homeCoords[0].x << ", " << grblParams->param.homeCoords[0].y << ", " << grblParams->param.homeCoords[0].z << endl;
			cout << "Home Coord G30 = " << grblParams->param.homeCoords[1].x << ", " << grblParams->param.homeCoords[1].y << ", " << grblParams->param.homeCoords[1].z << endl;
			
			cout << "Offset Coord G92 = " << grblParams->param.offsetCoords.x << ", " << grblParams->param.offsetCoords.y << ", " << grblParams->param.offsetCoords.z << endl;
			cout << "TLO = " << grblParams->param.toolLengthOffset << endl;
			
			cout << "Probe = " << grblParams->param.probeOffset.x << ", " << grblParams->param.probeOffset.y << ", " << grblParams->param.probeOffset.z << endl;
			cout << "Probe Success = " << grblParams->param.probeSuccess << endl << endl;
			
			/*
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
			
			cout << "Startup Block 1 = " << grblParams->startupBlock[0] << endl;
			cout << "Startup Block 2 = " << grblParams->startupBlock[1] << endl;
			*/
		}
		
		/*if(millis() > timer3) {
			grblRealTime(fd, GRBL_RT_FLOOD_COOLANT);	
			timer3 = millis() + 7000;
			
		}*/
	} while (1);
	
	// close serial connection
	serialClose(fd);
	
	return 0;
}

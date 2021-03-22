/*
 * main.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.hpp" 

 
using namespace std; 
 


/* **********PROBLEMS FOR LATER************
 * 
 * should recieve this: - i get this when i send a $X reset?
		Once connected you should get  the Grbl-prompt, which looks like this:
		Grbl 1.1e ['$' for help]


// - Inlcudes are probably for c std libraries
// - grblBufferSize is a global variable in grbl.cpp are global, should I do these differently?
// - do i need to destruct gcList?
// test all value are coming through ok - in particular coord systems as i modified the code
* 		// maybe a test which sends a tonnes of values and checks they match?
// - should check if in mm or inches ($13) as everything returned from grbl is based on those units
// - check buffer state response in status report matches our buffer (Bf:15,128. number of available blocks in the planner buffer / number of available bytes in the serial RX buffer)  - mask needs to be enabled first $_=_
// handle errors cleanly
	// i.e. better way than this: if(b != string::npos) ... else handle error
	// and exitf(...)
// on error, halt rest of commands
// handle alarms
// - is the buffer cleared out when theres an alarm ?? - i think so

// other notes
* // $C (check) should be called on open file?
* // if we need to sync gui to grbl, use G4 P0.01

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
	
	string workingDir = getWorkingDir(argv);
		
	gui(workingDir);
	return 0;
	
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
	
	GRBLParams* grblParams = new GRBLParams;
	
	Queue* q = new Queue(128);			
	
	GCList* gcList = new GCList;



	
	
	// add gcodes to the stream
	
	// GRBL SETTINGS
	//gcList->add("$$");
	
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
	//gcList->add("$X");
	
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
	
	
	
//	string file = "/home/pi/Desktop/New.nc";
	
//	if(runFile(gcList, file)) {
		/*couldnt open file*/
//	}
	
	
	
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
			/*
			
			timer2 = millis() + 2000;
			grblRealTime(fd, GRBL_RT_OVERRIDE_FEED_100PERCENT);	
			for (int i = 0; i < 6; i++) {
				cout << "Work Coord G" << 54+i << " = " << GRBLParams->param.workCoords[i].x << ", " << GRBLParams->param.workCoords[i].y << ", " << GRBLParams->param.workCoords[i].z << endl;
			}
			
			cout << "Home Coord G28 = " << GRBLParams->param.homeCoords[0].x << ", " << GRBLParams->param.homeCoords[0].y << ", " << GRBLParams->param.homeCoords[0].z << endl;
			cout << "Home Coord G30 = " << GRBLParams->param.homeCoords[1].x << ", " << GRBLParams->param.homeCoords[1].y << ", " << GRBLParams->param.homeCoords[1].z << endl;
			
			cout << "Offset Coord G92 = " << GRBLParams->param.offsetCoords.x << ", " << GRBLParams->param.offsetCoords.y << ", " << GRBLParams->param.offsetCoords.z << endl;
			cout << "TLO = " << GRBLParams->param.toolLengthOffset << endl;
			
			cout << "Probe = " << GRBLParams->param.probeOffset.x << ", " << GRBLParams->param.probeOffset.y << ", " << GRBLParams->param.probeOffset.z << endl;
			cout << "Probe Success = " << GRBLParams->param.probeSuccess << endl << endl;
			*/
			/*
			cout << "MotionMode = " << GRBLParams->mode.MotionMode << endl;
			cout << "CoordinateSystem = " << GRBLParams->mode.CoordinateSystem << endl;
			cout << "Plane = " << GRBLParams->mode.Plane << endl;
			cout << "DistanceMode = " << GRBLParams->mode.DistanceMode << endl;
			cout << "ArcIJKDistanceMode = " << GRBLParams->mode.ArcIJKDistanceMode << endl;
			cout << "FeedRateMode = " << GRBLParams->mode.FeedRateMode << endl;
			cout << "UnitsMode = " << GRBLParams->mode.UnitsMode << endl;
			cout << "CutterRadiusCompensation = " << GRBLParams->mode.CutterRadiusCompensation << endl;
			cout << "ToolLengthOffset = " << GRBLParams->mode.ToolLengthOffset << endl;
			cout << "ProgramMode = " << GRBLParams->mode.ProgramMode << endl;
			cout << "SpindleState = " << GRBLParams->mode.SpindleState << endl;
			cout << "CoolantState = " << GRBLParams->mode.CoolantState << endl;
			cout << "toolNumber = " << GRBLParams->mode.toolNumber << endl;
			cout << "spindleSpeed = " << GRBLParams->mode.spindleSpeed << endl;
			cout << "feedRate = " << GRBLParams->mode.feedRate << endl;
			
			cout << "Startup Block 1 = " << GRBLParams->startupBlock[0] << endl;
			cout << "Startup Block 2 = " << GRBLParams->startupBlock[1] << endl;
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

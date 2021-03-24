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
	
	(void)argc, (void) argv;
	string workingDir = getWorkingDir(argv);
		
	int wiringPiSetup(void);

	GRBL* Grbl = new GRBL();
	
	gui(workingDir, Grbl);
	
	return 0;
}

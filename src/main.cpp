/*
 * main.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.h" 

 
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


	EEPROM Issues
	EEPROM access on the Arduino AVR CPUs turns off all of the interrupts while the CPU writes to EEPROM. This poses a problem for certain features in Grbl, particularly if a user is streaming and running a g-code program, since it can pause the main step generator interrupt from executing on time. Most of the EEPROM access is restricted by Grbl when it's in certain states, but there are some things that developers need to know.

	Settings should not be streamed with the character-counting streaming protocols. Only the simple send-response protocol works. This is because during the EEPROM write, the AVR CPU also shuts-down the serial RX interrupt, which means data can get corrupted or lost. This is safe with the send-response protocol, because it's not sending data after commanding Grbl to save data.
	For reference:

	Grbl's EEPROM write commands: G10 L2, G10 L20, G28.1, G30.1, $x=, $I=, $Nx=, $RST=
	Grbl's EEPROM read commands: G54-G59, G28, G30, $$, $I, $N, $#
*/
		
#ifdef DEBUG_MEMORY_ALLOC
	void* operator new(size_t size)
	{
		s_AllocCount++;
		cout << "Allocating " << size << " bytes\n";
		return malloc(size);
	}
#endif



#define JOYSTICK_X 			ADS1115_PIN_A0
#define JOYSTICK_Y 			ADS1115_PIN_A1
#define JOYSTICK_BUTTON 	ADS1115_PIN_A2
#define JOYSTICK_INVERT 	point2D(1, -1)
#define JOYSTICK_VMAX	 	3.3f	// volts
#define JOYSTICK_VTRIG	 	3.0f	// voltage above this is high


void readJoystick(ADS1115& adc, point2D& return_point, bool& return_press) 
{	// read joystick voltage
	point2D p = point2D(adc.Read(JOYSTICK_X), adc.Read(JOYSTICK_Y));
	// normalise to -1 to 1, and translate from (0 to 2) to (-1 to 1)
	p = (p * 2 / JOYSTICK_VMAX) - 1.0f;
	// invert y axis
	return_point = p * JOYSTICK_INVERT;
	return_press = (adc.Read(JOYSTICK_BUTTON) > JOYSTICK_VTRIG) ? true : false;
}

int main(int argc, char **argv)
{

	/*
	ADS1115 adc(0x48, ADS1115_FSR_4_096V);
	
	while(1) {		
		point2D p;
		bool clicked;
		readJoystick(adc, p, clicked);
		
		polar pol(p);
		float ignoreRadius = 0.1f; // +- this is between 0 and 1
		// Ignore if within small radius
		if(pol.r < ignoreRadius){
			p = point2D(0, 0);
			pol = polar(0, 0);
		}
		
		cout << "\r                                                                             " << flush;
		cout << "\r Joystick = " << p << "  " << pol.r << "V  " << rad2deg(pol.th) << "degs   pressed = " << clicked << flush;
	}

	return 0;
	*/
	(void)argc, (void) argv;
	
	if(wiringPiSetup() == -1) {
		cout << "Error: Could not start wiringPi " << strerror(errno) << endl;
		exit(1);
	}
	    
	GRBL* Grbl = new GRBL();
	
	gui(Grbl);
	
	delete(Grbl);
	
	return 0;
}

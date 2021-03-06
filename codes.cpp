/*
 * codes.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.hpp"

#define NUM_ERROR_MSG 36
//"Error Code in v1.1+", "Error Message in v1.0-", "Error Description"
static std::string errMessages[NUM_ERROR_MSG][3] = {
	{"1","Expected command letter","G-code words consist of a letter and a value. Letter was not found."},
	{"2","Bad number format","Missing the expected G-code word value or numeric value format is not valid."},
	{"3","Invalid statement","Grbl '$' system command was not recognized or supported."},
	{"4","Value < 0","Negative value received for an expected positive value."},
	{"5","Setting disabled","Homing cycle failure. Homing is not enabled via settings."},
	{"6","Value < 3 usec","Minimum step pulse time must be greater than 3usec."},
	{"7","EEPROM read fail. Using defaults","An EEPROM read failed. Auto-restoring affected EEPROM to default values."},
	{"8","Not idle","Grbl '$' command cannot be used unless Grbl is IDLE. Ensures smooth operation during a job."},
	{"9","G-code lock","G-code commands are locked out during alarm or jog state."},
	{"10","Homing not enabled","Soft limits cannot be enabled without homing also enabled."},
	{"11","Line overflow","Max characters per line exceeded. Received command line was not executed."},
	{"12","Step rate > 30kHz","Grbl '$' setting value cause the step rate to exceed the maximum supported."},
	{"13","Check Door","Safety door detected as opened and door state initiated."},
	{"14","Line length exceeded","Build info or startup line exceeded EEPROM line length limit. Line not stored."},
	{"15","Travel exceeded","Jog target exceeds machine travel. Jog command has been ignored."},
	{"16","Invalid jog command","Jog command has no '=' or contains prohibited g-code."},
	{"17","Setting disabled","Laser mode requires PWM output."},
	{"20","Unsupported command","Unsupported or invalid g-code command found in block."},
	{"21","Modal group violation","More than one g-code command from same modal group found in block."},
	{"22","Undefined feed rate","Feed rate has not yet been set or is undefined."},
	{"23","Invalid gcode ID:23","G-code command in block requires an integer value."},
	{"24","Invalid gcode ID:24","More than one g-code command that requires axis words found in block."},
	{"25","Invalid gcode ID:25","Repeated g-code word found in block."},
	{"26","Invalid gcode ID:26","No axis words found in block for g-code command or current modal state which requires them."},
	{"27","Invalid gcode ID:27","Line number value is invalid."},
	{"28","Invalid gcode ID:28","G-code command is missing a required value word."},
	{"29","Invalid gcode ID:29","G59.x work coordinate systems are not supported."},
	{"30","Invalid gcode ID:30","G53 only allowed with G0 and G1 motion modes."},
	{"31","Invalid gcode ID:31","Axis words found in block when no command or current modal state uses them."},
	{"32","Invalid gcode ID:32","G2 and G3 arcs require at least one in-plane axis word."},
	{"33","Invalid gcode ID:33","Motion command target is invalid."},
	{"34","Invalid gcode ID:34","Arc radius value is invalid."},
	{"35","Invalid gcode ID:35","G2 and G3 arcs require at least one in-plane offset word."},
	{"36","Invalid gcode ID:36","Unused value words found in block."},
	{"37","Invalid gcode ID:37","G43.1 dynamic tool length offset is not assigned to configured tool length axis."},
	{"38","Invalid gcode ID:38","Tool number greater than max supported value."}
};

// returns error message and description from given err
int getErrMsg(int num, std::string* name, std::string* desc) {
	// itterate through errors, find err number and return message + description
	for (int i = 0; i < NUM_ERROR_MSG; i++) {
		if(num == std::stoi(errMessages[i][0])) {
			*name = errMessages[i][1];
			*desc = errMessages[i][2];
			return ERR_NONE;
		}
	}
	return ERR_FAIL;
}



#define NUM_SETTINGS_MSG 34
//"$-Code"," Setting"," Units"," Setting Description"
static std::string settingsMessages[NUM_SETTINGS_MSG][4] = {
	{"0","Step pulse time","microseconds","Sets time length per step. Minimum 3usec."},
	{"1","Step idle delay","milliseconds","Sets a short hold delay when stopping to let dynamics settle before disabling steppers. Value 255 keeps motors enabled with no delay."},
	{"2","Step pulse invert","mask","Inverts the step signal. Set axis bit to invert (00000ZYX)."},
	{"3","Step direction invert","mask","Inverts the direction signal. Set axis bit to invert (00000ZYX)."},
	{"4","Invert step enable pin","boolean","Inverts the stepper driver enable pin signal."},
	{"5","Invert limit pins","boolean","Inverts the all of the limit input pins."},
	{"6","Invert probe pin","boolean","Inverts the probe input pin signal."},
	{"10","Status report options","mask","Alters data included in status reports."},
	{"11","Junction deviation","millimeters","Sets how fast Grbl travels through consecutive motions. Lower value slows it down."},
	{"12","Arc tolerance","millimeters","Sets the G2 and G3 arc tracing accuracy based on radial error. Beware: A very small value may effect performance."},
	{"13","Report in inches","boolean","Enables inch units when returning any position and rate value that is not a settings value."},
	{"20","Soft limits enable","boolean","Enables soft limits checks within machine travel and sets alarm when exceeded. Requires homing."},
	{"21","Hard limits enable","boolean","Enables hard limits. Immediately halts motion and throws an alarm when switch is triggered."},
	{"22","Homing cycle enable","boolean","Enables homing cycle. Requires limit switches on all axes."},
	{"23","Homing direction invert","mask","Homing searches for a switch in the positive direction. Set axis bit (00000ZYX) to search in negative direction."},
	{"24","Homing locate feed rate","mm/min","Feed rate to slowly engage limit switch to determine its location accurately."},
	{"25","Homing search seek rate","mm/min","Seek rate to quickly find the limit switch before the slower locating phase."},
	{"26","Homing switch debounce delay","milliseconds","Sets a short delay between phases of homing cycle to let a switch debounce."},
	{"27","Homing switch pull-off distance","millimeters","Retract distance after triggering switch to disengage it. Homing will fail if switch isn't cleared."},
	{"30","Maximum spindle speed","RPM","Maximum spindle speed. Sets PWM to 100% duty cycle."},
	{"31","Minimum spindle speed","RPM","Minimum spindle speed. Sets PWM to 0.4% or lowest duty cycle."},
	{"32","Laser-mode enable","boolean","Enables laser mode. Consecutive G1/2/3 commands will not halt when spindle speed is changed."},
	{"100","X-axis travel resolution","step/mm","X-axis travel resolution in steps per millimeter."},
	{"101","Y-axis travel resolution","step/mm","Y-axis travel resolution in steps per millimeter."},
	{"102","Z-axis travel resolution","step/mm","Z-axis travel resolution in steps per millimeter."},
	{"110","X-axis maximum rate","mm/min","X-axis maximum rate. Used as G0 rapid rate."},
	{"111","Y-axis maximum rate","mm/min","Y-axis maximum rate. Used as G0 rapid rate."},
	{"112","Z-axis maximum rate","mm/min","Z-axis maximum rate. Used as G0 rapid rate."},
	{"120","X-axis acceleration","mm/sec^2","X-axis acceleration. Used for motion planning to not exceed motor torque and lose steps."},
	{"121","Y-axis acceleration","mm/sec^2","Y-axis acceleration. Used for motion planning to not exceed motor torque and lose steps."},
	{"122","Z-axis acceleration","mm/sec^2","Z-axis acceleration. Used for motion planning to not exceed motor torque and lose steps."},
	{"130","X-axis maximum travel","millimeters","Maximum X-axis travel distance from homing switch. Determines valid machine space for soft-limits and homing search distances."},
	{"131","Y-axis maximum travel","millimeters","Maximum Y-axis travel distance from homing switch. Determines valid machine space for soft-limits and homing search distances."},
	{"132","Z-axis maximum travel","millimeters","Maximum Z-axis travel distance from homing switch. Determines valid machine space for soft-limits and homing search distances."}
};

// returns settings message, units and description from given code
int getSettingsMsg(int num, std::string* name, std::string* units, std::string* desc) {
	// itterate through errors, find err number and return message + description
	for (int i = 0; i < NUM_SETTINGS_MSG; i++) {
		if(num == std::stoi(settingsMessages[i][0])) {
			*name = settingsMessages[i][1];
			
			//simplify these to shorter version
			std::string unitStr = settingsMessages[i][2];
			if(unitStr == "microseconds")
				unitStr = "us";
			if(unitStr == "milliseconds")
				unitStr = "ms";
			if(unitStr == "millimeters")
				unitStr = "mm";			
			*units = unitStr;
			
			*desc = settingsMessages[i][3];
			return ERR_NONE;
		}
	}
	return ERR_FAIL;
}


#define NUM_ALARM_MSG 10
// "Alarm Code in v1.1+"," Alarm Message in v1.0-"," Alarm Description"
static std::string alarmMessages[NUM_ALARM_MSG][4] = {
	{"1","Hard limit","Hard limit has been triggered. Machine position is likely lost due to sudden halt. Re-homing is highly recommended."},
	{"2","Soft limit","Soft limit alarm. G-code motion target exceeds machine travel. Machine position retained. Alarm may be safely unlocked."},
	{"3","Abort during cycle","Reset while in motion. Machine position is likely lost due to sudden halt. Re-homing is highly recommended."},
	{"4","Probe fail","Probe fail. Probe is not in the expected initial state before starting probe cycle when G38.2 and G38.3 is not triggered and G38.4 and G38.5 is triggered."},
	{"5","Probe fail","Probe fail. Probe did not contact the workpiece within the programmed travel for G38.2 and G38.4."},
	{"6","Homing fail","Homing fail. The active homing cycle was reset."},
	{"7","Homing fail","Homing fail. Safety door was opened during homing cycle."},
	{"8","Homing fail","Homing fail. Pull off travel failed to clear limit switch. Try increasing pull-off setting or check wiring."},
	{"9","Homing fail","Homing fail. Could not find limit switch within search distances. Try increasing max travel, decreasing pull-off distance, or check wiring."},
	{"10","Homing fail","Homing fail. Second dual axis limit switch failed to trigger within configured search distance after first. Try increasing trigger fail distance or check wiring."}
};

// returns settings message, units and description from given code
int getAlarmMsg(int num, std::string* name, std::string* desc) {
	// itterate through errors, find err number and return message + description
	for (int i = 0; i < NUM_ALARM_MSG; i++) {
		if(num == std::stoi(alarmMessages[i][0])) {
			*name = alarmMessages[i][1];
			*desc = alarmMessages[i][2];
			return ERR_NONE;
		}
	}
	return ERR_FAIL;
}

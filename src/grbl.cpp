/*
 * grbl.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.hpp"
using namespace std;

MainSettings::MainSettings()
{
	SetUnitsInches(false);
}

void MainSettings::SetUnitsInches(int val) 
{
	if(val) {
		units_Distance	= "in";
		units_Feed		= "in/min";
	}
	else /*inches*/ {
		units_Distance	= "mm";
		units_Feed		= "mm/min";
	}	
}
	
GRBLParams::GRBLParams() {
	
// base (GRBLParams)
	startupBlock[0] = "";
	startupBlock[1] = "";
	
// clear all values in object
	point3D defaultP = (point3D){.x=0.0f, .y=0.0f, .z=0.0f};
	
// gCodeParams_t
	gCodeParams_t* g = &(gcParam);
	// G54 - G59
	for (int i = 0; i < 6; i++)
		g->workCoords[i] = defaultP;
	// G28 & G30
	for (int i = 0; i < 2; i++)
		g->homeCoords[i] = defaultP;	
	// G92
	g->offsetCoords = defaultP;
		
	g->toolLengthOffset = 0.0f;
	
	g->probeOffset = defaultP;
	g->probeSuccess = FALSE;
	
// modalGroup_t
	modalGroup_t* m = &(mode);
	m->MotionMode = "G0";
	m->CoordinateSystem = "G54";
	m->Plane = "G17";
	m->DistanceMode = "G90";
	m->ArcIJKDistanceMode = "G91.1";
	m->FeedRateMode = "G94";
	m->UnitsMode = "G21";
	m->CutterRadiusCompensation = "G40";
	m->ToolLengthOffset = "G49";
	m->ProgramMode = "M0";
	m->SpindleState = "M5";
	m->CoolantState = "M9";

	m->toolNumber = 0;
	m->spindleSpeed = 0.0f;
	m->feedRate = 0.0f;

// grblStatus_t
	grblStatus_t* s = &(status);
	s->state = "";
	s->MPos = defaultP;
	s->WPos = defaultP;
	s->WCO = defaultP;
	s->lineNum = 0;
	s->feedRate = 0;
	s->spindleSpeed = 0;
	
	s->inputPin_LimX = 0;
	s->inputPin_LimY = 0;
	s->inputPin_LimZ = 0;
	s->inputPin_Probe = 0;
	s->inputPin_Door = 0;
	s->inputPin_Hold = 0;
	s->inputPin_SoftReset = 0;
	s->inputPin_CycleStart = 0;
	
	s->override_Feedrate = 0;
	s->override_RapidFeed = 0;
	s->override_SpindleSpeed = 0;
	
	s->accessory_SpindleDirection = 0;
	s->accessory_FloodCoolant = 0;
	s->accessory_MistCoolant = 0;
	
}


void GRBLParams::Print() {
	
	cout << "Startup Block 1 = " << startupBlock[0] << endl;
	cout << "Startup Block 2 = " << startupBlock[1] << endl;
	
	// gCodeParams_t
	for (int i = 0; i < 6; i++) {
		cout << "Work Coord G" << 54+i << " = " << gcParam.workCoords[i].x << ", " << gcParam.workCoords[i].y << ", " << gcParam.workCoords[i].z << endl;
	}
	cout << "Home Coord G28 = " << gcParam.homeCoords[0].x << ", " << gcParam.homeCoords[0].y << ", " << gcParam.homeCoords[0].z << endl;
	cout << "Home Coord G30 = " << gcParam.homeCoords[1].x << ", " << gcParam.homeCoords[1].y << ", " << gcParam.homeCoords[1].z << endl;
	cout << "Offset Coord G92 = " << gcParam.offsetCoords.x << ", " << gcParam.offsetCoords.y << ", " << gcParam.offsetCoords.z << endl;
	cout << "TLO = " << gcParam.toolLengthOffset << endl;
	cout << "Probe = " << gcParam.probeOffset.x << ", " << gcParam.probeOffset.y << ", " << gcParam.probeOffset.z << endl;
	cout << "Probe Success = " << gcParam.probeSuccess << endl << endl;

	// modalGroup_t
	cout << "MotionMode = " << mode.MotionMode << endl;
	cout << "CoordinateSystem = " << mode.CoordinateSystem << endl;
	cout << "Plane = " << mode.Plane << endl;
	cout << "DistanceMode = " << mode.DistanceMode << endl;
	cout << "ArcIJKDistanceMode = " << mode.ArcIJKDistanceMode << endl;
	cout << "FeedRateMode = " << mode.FeedRateMode << endl;
	cout << "UnitsMode = " << mode.UnitsMode << endl;
	cout << "CutterRadiusCompensation = " << mode.CutterRadiusCompensation << endl;
	cout << "ToolLengthOffset = " << mode.ToolLengthOffset << endl;
	cout << "ProgramMode = " << mode.ProgramMode << endl;
	cout << "SpindleState = " << mode.SpindleState << endl;
	cout << "CoolantState = " << mode.CoolantState << endl;
	cout << "toolNumber = " << mode.toolNumber << endl;
	cout << "spindleSpeed = " << mode.spindleSpeed << endl;
	cout << "feedRate = " << mode.feedRate << endl;


	// grblStatus_t
	cout << "state = " << status.state << endl;
	cout << "MPos x = " << status.MPos.x << endl;
	cout << "MPos y = " << status.MPos.y << endl;
	cout << "MPos z = " << status.MPos.z << endl;
	cout << "WPos x = " << status.WPos.x << endl;
	cout << "WPos y = " << status.WPos.y << endl;
	cout << "WPos z = " << status.WPos.z << endl;
	cout << "WCO x = " << status.WCO.x << endl;
	cout << "WCO y = " << status.WCO.y << endl;
	cout << "WCO z = " << status.WCO.z << endl;
	cout << "LineNum = " << status.lineNum << endl;
	cout << "FeedRate = " << status.feedRate << endl;
	cout << "SpindleSpeed = " << status.spindleSpeed << endl;
	cout << "Override Feedrate = " << status.override_Feedrate << endl;
	cout << "Override RapidFeed = " << status.override_RapidFeed << endl;
	cout << "Override SpindleSpeed = " << status.override_SpindleSpeed << endl;
	cout << "Pin X = " << status.inputPin_LimX << endl;
	cout << "Pin Y = " << status.inputPin_LimY << endl;
	cout << "Pin Z = " << status.inputPin_LimZ << endl;
	cout << "Probe = " << status.inputPin_Probe << endl;
	cout << "Door = " << status.inputPin_Door << endl;
	cout << "Hold = " << status.inputPin_Hold << endl;
	cout << "SoftReset = " << status.inputPin_SoftReset << endl;
	cout << "CycleStart = " << status.inputPin_CycleStart << endl << endl;
	cout << "Spindle Direction = " << status.accessory_SpindleDirection << endl;
	cout << "Flood Coolant = " << status.accessory_FloodCoolant << endl;
	cout << "Mist Coolant = " << status.accessory_MistCoolant << endl;

}
	

// This takes a string of 3 values seperated by commas (,) and will return a 3DPoint
// 4.000,0.000,0.000
point3D GRBLParams::stoxyz(const string& msg) {
	
	istringstream stream(msg);
	string segment;
	float val[3];
	
	for (int i = 0; i < 3; i++) {
		getline(stream, segment, ',');
		val[i] = stof(segment);
	}
	
	return (point3D) {.x=val[0], .y=val[1], .z=val[2]};
}
	
// checks Startup Line Execution for error	msg = ">G54G20:ok" or ">G54G20:error:X"
// it is very unlikely that there will be an error as this is checked before it is saves onto the eeprom
void GRBLParams::CheckStartupLine(const string& msg) {
	
	// retrieve position of :
	size_t a = msg.find(":");
	// checks to prevent errors
	if(a != string::npos) {
		// retrieve value of response ('ok' or 'error:x')
		string response = msg.substr(a+1, msg.length()-a-1);
		if(response == "ok")
			cout << msg << endl;
		else {
			size_t b = response.find(":");
			if(b != string::npos) {
				int errCode = stoi(response.substr(b+1));
				cout << "Startup Line Execution has encountered an error: " << errCode << endl;						//ERROR NEED TO HALT EVERYTHING
				exit(1);
			}
			else
				exitf("ERROR: Something is not right here\n");
		}
	}
	else
		exitf("ERROR: Something is not right here\n");
}	

// decodes GCode Parameters
// and stores inside grbl Parameters
void GRBLParams::DecodeParameters(const string& msg) {
	
	gCodeParams_t* p = &(gcParam);
	// get name i.e 'G54'
	string param = msg.substr(1, 3);
	// get number string i.e. '4.000,0.000,0.000'
	string num = msg.substr(5, msg.length()-6);
	
	// [G54:4.000,0.000,0.000] - [G59:4.000,0.000,0.000]
	if(!param.compare("G54")) 
		p->workCoords[0] = stoxyz(num);
	else if(!param.compare("G55")) 
		p->workCoords[1] = stoxyz(num);
	else if(!param.compare("G56")) 
		p->workCoords[2] = stoxyz(num);
	else if(!param.compare("G57")) 
		p->workCoords[3] = stoxyz(num);
	else if(!param.compare("G58")) 
		p->workCoords[4] = stoxyz(num);
	else if(!param.compare("G59")) 
		p->workCoords[5] = stoxyz(num);
	// [G28:1.000,2.000,0.000]  / [G30:4.000,6.000,0.000]
	else if(!param.compare("G28")) 
		p->homeCoords[0] = stoxyz(num);
	else if(!param.compare("G30")) 
		p->homeCoords[1] = stoxyz(num);
	// [G92:0.000,0.000,0.000]
	else if(!param.compare("G92")) 
		p->offsetCoords = stoxyz(num);
	// [TLO:0.000]	
	else if(!param.compare("TLO")) 
		p->toolLengthOffset = stof(num);
	// [PRB:0.000,0.000,0.000:0]
	else if(!param.compare("PRB")) {
		p->probeOffset = stoxyz(msg.substr(5, msg.length()-8));
		p->probeSuccess = (bool)stoi(msg.substr(msg.length()-2, 1));
	}
	else {
		cout << "Message unrecognised: " << msg << endl;
		exit(1);
	}
}

// decodes modal groups
// and stores inside grbl Parameters
void GRBLParams::DecodeMode(const string& msg) {
	
	modalGroup_t* m = &(mode);
	
	cout << msg << endl;
	
	string s = msg.substr(4, msg.length()-5);
	istringstream stream(s);
	do {
		string code;
		stream >> code;
		// [GC:G0 G54 G17 G21 G90 G94 M0 M5 M9 T0 S0.0 F500.0]
		if(code == "")
			{/*do nothing*/}
		else if(code == "G0" || code == "G1" || code ==  "G2" || code ==  "G3" || code ==  "G38.2" || code ==  "G38.3" || code ==  "G38.4 "|| code ==  "G38.5" || code ==  "G80")
			m->MotionMode = code;
		else if(code == "G54" || code == "G55" || code == "G56" || code == "G57" || code ==  "G58" || code == "G59")
			m->CoordinateSystem = code;
		else if(code == "G17" || code == "G18" || code == "G19")
			m->Plane = code;
		else if(code == "G90" || code == "G91")
			m->DistanceMode = code;
		else if(code == "G91.1")
			m->ArcIJKDistanceMode = code;
		else if(code == "G93" || code == "G94")
			m->FeedRateMode = code;
		else if(code == "G20" || code == "G21")
			m->UnitsMode = code;
		else if(code == "G40")
			m->CutterRadiusCompensation = code;
		else if(code == "G43.1" || code == "G49")
			m->ToolLengthOffset = code;
		else if(code == "M0" || code == "M1" || code == "M2" || code == "M30")
			m->ProgramMode = code;
		else if(code == "M3" || code == "M4" || code == "M5")
			m->SpindleState = code;
		else if(code == "M7" || code == "M8" || code == "M9")
			m->CoolantState = code;
		else if(code.compare(0, 1, "T"))
			m->toolNumber = stoi(code.substr(1, code.length()-1));
		else if(code.compare(0, 1, "S"))
			m->spindleSpeed = stof(code.substr(1, code.length()-1));
		else if(code.compare(0, 1, "F"))
			m->feedRate = stof(code.substr(1, code.length()-1));
		else {
			cout << "ERROR: Code unrecognised: " << code << endl;
			exit(1);
		}
	} while (stream);
}
// decodes status response 
// and stores inside grbl Parameters
// The $10 status report mask setting can alter what data is present and certain data fields can be reported intermittently (see descriptions for details.)
// The $13 report inches settings alters the units of some data values. $13=0 false indicates mm-mode, while $13=1 true indicates inch-mode reporting.
// "<Idle|WPos:828.000,319.000,49.100|FS:0,0|Pn:PXYZ>"
void GRBLParams::DecodeStatus(const string& msg) {
	
	grblStatus_t* s = &(status);
	
	istringstream stream(msg.substr(1, msg.length()-2));
	string segment;
	vector<string> segs;
	
	while(getline(stream, segment, '|')) 
		segs.push_back(segment);
	
	// itterate backwards through segs[i]s
	for (int i = segs.size()-1; i >= 0; i--) {
		
		#ifdef DEBUG
			cout << "segment " << i << "= " << segs[i] << endl; 
		#endif
		
		// Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep
		//- `Hold:0` Hold complete. Ready to resume.
		//- `Hold:1` Hold in-progress. Reset will throw an alarm.
		//- `Door:0` Door closed. Ready to resume.
		//- `Door:1` Machine stopped. Door still ajar. Can't resume until closed.
		//- `Door:2` Door opened. Hold (or parking retract) in-progress. Reset will throw an alarm.
		//- `Door:3` Door closed and resuming. Restoring from park, if applicable. Reset will throw an alarm.
		
		if(segs[i] == "Idle" || segs[i] == "Run" || segs[i].substr(0, 4) == "Hold" || segs[i] == "Jog" || segs[i] == "Alarm" 
			|| segs[i].substr(0, 4) == "Door" || segs[i] == "Check" || segs[i] == "Home" || segs[i] == "Sleep" ) {
			s->state = segs[i];
			
		}
		// MPos:0.000,-10.000,5.000 machine position  or  WPos:-2.500,0.000,11.000 work position
		// WPos = MPos - WCO
		else if(segs[i].substr(0, 4) == "MPos") {
			s->MPos = stoxyz(segs[i].substr(5));
			s->WPos = minus3p(s->MPos, s->WCO);
		}
		else if(segs[i].substr(0, 4) == "WPos") {
			s->WPos = stoxyz(segs[i].substr(5));
			s->MPos = add3p(s->WPos, s->WCO);
		}
		// work coord offset - shown every 10-30 times
		// the current work coordinate system, G92 offsets, and G43.1 tool length offset
		else if(segs[i].substr(0, 3) == "WCO") {
			s->WCO = stoxyz(segs[i].substr(4));
		}
		
		// Buffer State - mainly used for debugging
		// Bf:15,128. number of available blocks in the planner buffer / number of available bytes in the serial RX buffer.
		else if(segs[i].substr(0, 2) == "Bf") {
		}
		// line number Ln:99999
		else if(segs[i].substr(0, 2) == "Ln") {
			s->lineNum = stoi(segs[i].substr(3)); 
		}
		// feed & speed
		// FS:500,8000 (feed rate / spindle speed)
		else if(segs[i].substr(0, 2) == "FS") {
			size_t a = segs[i].find(",");
			if (a != string::npos) {
				s->feedRate = stof(segs[i].substr(3, a-3)); 
				s->spindleSpeed = stoi(segs[i].substr(a+1)); 
			}
			else
				exitf("ERROR: Can't find ',' in FS\n");
		}
		// feed only
		// F:500 (feed rate only) - when VARIABLE_SPINDLE is disabled in config.h
		else if(segs[i].substr(0, 1) == "F") {
			s->feedRate = stof(segs[i].substr(2)); 
		}
		// Input Pin State
		// Pn:XYZPDHRS - can be any number of letters
		else if(segs[i].substr(0, 2) == "Pn") {
			// set all pins to default
			s->inputPin_LimX = false;
			s->inputPin_LimY = false;
			s->inputPin_LimZ = false;
			s->inputPin_Probe = false;
			s->inputPin_Door = false;
			s->inputPin_Hold = false;
			s->inputPin_SoftReset = false;
			s->inputPin_CycleStart = false;
			
			string str = segs[i].substr(3);
			for (int j = 0; j < str.length(); j++) {
				if(str[j] == 'X')
					s->inputPin_LimX = true;
				else if(str[j] == 'Y')
					s->inputPin_LimY = true;
				else if(str[j] == 'Z')
					s->inputPin_LimZ = true;
				else if(str[j] == 'P')
					s->inputPin_Probe = true;
				else if(str[j] == 'D')
					s->inputPin_Door = true;
				else if(str[j] == 'H')
					s->inputPin_Hold = true;
				else if(str[j] == 'R')
					s->inputPin_SoftReset = true;
				else if(str[j] == 'S')
					s->inputPin_CycleStart = true;
				else
					exitf("ERROR: Input pin unrecognised\n");
			}
			
		}
		//Override Values:
		// Ov:100,100,100 current override values in percent of programmed values for feed, rapids, and spindle speed, respectively.
		else if(segs[i].substr(0, 2) == "Ov") {
			point3D ov = stoxyz(segs[i].substr(3));
			s->override_Feedrate = (int)ov.x;
			s->override_RapidFeed = (int)ov.y;
			s->override_SpindleSpeed = (int)ov.z;
		}
		// Accessory State
		// 	'A:SFM' - can be any number of letters
		// S indicates spindle is enabled in the CW direction. This does not appear with C.
		// C indicates spindle is enabled in the CCW direction. This does not appear with S.
		// F indicates flood coolant is enabled.
		// M indicates mist coolant is enabled.
		else if(segs[i].substr(0, 2) == "A:") {
			// set all pins to default
			s->accessory_SpindleDirection = false;
			s->accessory_FloodCoolant = false;
			s->accessory_MistCoolant = false;
			
			string str = segs[i].substr(2);
			for (int j = 0; j < str.length(); j++) {
				if(str[j] == 'S')
					s->accessory_SpindleDirection = CLOCKWISE;	// (1)
				else if(str[j] == 'C')
					s->accessory_SpindleDirection = ANTICLOCKWISE;	// (-1)
				else if(str[j] == 'F')
					s->accessory_FloodCoolant = true;
				else if(str[j] == 'M')
					s->accessory_MistCoolant = true;
				else
					exitf("ERROR: Input pin unrecognised\n");
			}
		}
	}
}

// decodes the settings froms grbl
// just prints them for now
void GRBLParams::DecodeSettings(ostringstream& outputStream, const string& msg) {
	// retrieve settings code & current value
	size_t a = msg.find("=");
	// checks to prevent errors
	if((a != string::npos) && (a > 0) && msg.length() > a+1) {
		int settingsCode = stoi(msg.substr(1, a-1));
		float value = stof(msg.substr(a+1));
		// retrieve name, desc & unit of setting
		string name, unit, desc;
		if(getSettingsMsg(settingsCode, &name, &unit, &desc)) {	
			cout << "Error: Can't find setting code!" << endl;
			exit(1);
		}
		//determine which units we are using
		if(settingsCode == 13)
			settings.SetUnitsInches((int)value);
			
		if(settingsCode == 110)
			settings.max_FeedRateX = value;
		if(settingsCode == 111)
			settings.max_FeedRateY = value;
		if(settingsCode == 112)
			settings.max_FeedRateZ = value;
		
		// display
		outputStream << "$" << settingsCode << " = " << value << " (";
		// if it's a mask, display as in binary instead of unit
		(unit == "mask") ? (outputStream << bitset<8>(value)) : (outputStream << unit);
		outputStream << ") :\t" << name << endl; //<< " Desc: " << desc << endl;
	}
}


GCList::GCList(){
	count = 0;
	written = 0;
	read = 0;
}

// takes a GCode linbe and cleans up
// (removes spaces, comments, make uppercase, ensures '\n' is present)
void GCList::CleanString(string* str) {
	
	// strip out whitespace - this means we can fit more in the buffer
	str->erase(remove_if(str->begin(), str->end(), ::isspace), str->end());
	// remove comments within () 
	size_t a = str->find("(");
	if(a != string::npos) {
		size_t b = str->find(")", a);
		if(b != string::npos)
			str->erase(a, b-a+1);
	}
	// remove comments after ;
	size_t c = str->find(";");
	if(c != string::npos)
		str->erase(c, str->length()-c );
	// make all uppercase
	transform(str->begin(), str->end(),str->begin(), ::toupper);
	// add a newline character to end
	str-> append("\n");
}

// add a line of GCode to the GCode List
void GCList::Add(string* str) {
	
	CleanString(str);
	// ignore blank lines
	if(*str == "\n")
		return;
		
	if(str->length() > MAX_GRBL_BUFFER)
		exitf("ERROR: String is longer than grbl buffer!\n");
	
	this->str.push_back(*str);
	status.push_back(STATUS_NONE);
	
	count++;
}	
		
// set the response status of the GCode line
void GCList::SetResponse(ostringstream& outputStream, int response) {
	// set reponse to corrosponding gcode
	status[read] = response;
	outputStream << '#' << read << "\t" << str[read].substr(0, str[read].length() -1) <<  "\t\tstatus: ";
	if (response == STATUS_OK) 
		outputStream << "ok" << endl;
	else {
		string errName, errDesc;
		if(getErrMsg(response, &errName, &errDesc)) {	
			outputStream << "Error: Can't find error code!" << endl;
			exit(1);
		}
		outputStream << "Error " << response << ": " << errName << "\tDesc: " << errDesc << endl;
	}
	read++;
}

GRBL::GRBL() {
	q = new Queue(128);
	statusTimerInterval = 500;
	statusTimer = millis() + statusTimerInterval;
}


GRBL::~GRBL() {
	// close serial connection
	serialClose(fd);
}

void GRBL::Connect() {
	
	fd = serialOpen(SERIAL_DEVICE, SERIAL_BAUDRATE);
	if(fd == -1)
		exitf("ERROR: Could not open serial device\n");
	//clear the serial buffer
	serialPuts(fd, "\r\n\r\n"); // not actually sure what this does but it is given in the grbl example
	delay(2000);
	serialFlush(fd);	
}

// adds to the GCode list, ready to be written when buffer has space
// sending a pointer is slightly quicker as it wont have to be be copied, it will however be modified to remove whitespace and comments etc
void GRBL::Send(string* cmd) {
	gcList.Add(cmd); 
}
void GRBL::Send(string cmd) {
	gcList.Add(&cmd); 
}

/* sends a REALTIME COMMAND
 * 	- These are not considered as part of the streaming protocol and are executed instantly
 * 	- They do not require a line feed or carriage return after them.
 * 	- None of these respond with 'ok' or 'error'
 *	see https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands
 */ 
void GRBL::SendRT(char cmd) {
	
	switch (cmd) {
		case GRBL_RT_SOFT_RESET:
			printf("Sent: 'Soft Reset'\n");
			break;
		case GRBL_RT_STATUS_QUERY:
			#ifdef DEBUG
				printf("Sent: 'Status Query'\n");
			#endif
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

void GRBL::SendJog(int axis, int dir, float distance, int feedrate) {
    //Grbl->SendRT(GRBL_RT_JOG_CANCEL);
    
    // example: $J=G91 X10 F1000
    string cmd("$J=G91");
    
    if(axis == X_AXIS)
		cmd += "X";
    if(axis == Y_AXIS)
		cmd += "Y";
    if(axis == Z_AXIS)
		cmd += "Z";
		
	char val[16];
	snprintf(val, 16, "%g", dir*(float)((int)(distance*1000))/1000);
	cmd += val;
    
	cmd += "F";
	snprintf(val, 16, "%d", feedrate);
	cmd += val;
	
	Send(&cmd);
}

void GRBL::Write() {
//void grblWrite(int fd, GCList* gcList, Queue* q) {
	//GCList* gcList = &(gcList);
	do {
		// exit if nothing new in the gcList
		if (gcList.written >= gcList.count)
			break;
			
		string* curStr = &gcList.str[gcList.written];
		
		#ifdef DEBUG
			cout << "Writing to grbl: " << *curStr << " (length = "<< len << ")" << endl;
		#endif
		
		if(BufferAdd(curStr->length()))
			break;
		// set status
		gcList.status[gcList.written] = STATUS_PENDING;
		// write to serial port
		serialPuts(fd, curStr->c_str());
		gcList.written++;
		//printf("ADDING #%d: %s", gcListWritten, gc->str.c_str);
	} while (1);
}

// Reads line of serial interface and returns onto msg
void GRBL::ReadLine(string* msg) {
	
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
int GRBL::Read(string& outputLog) {
	
//void grblRead(GRBLParams* grblParams, int fd, GCList* gcList, Queue* q) {
	GRBLParams* grblParams = &(Param);
	ostringstream outputStream;
	string msg;
	
	bool streamModified = false;
	
	// retrieve data upto response 'ok' or 'error'
	do {
		if(!serialDataAvail(fd))
			break;
		
		ReadLine(&msg);
		
		#ifdef DEBUG
			cout << "Reading from grbl: " << msg << endl;
		#endif
		
		// ignore blank responses
		if(!msg.compare(0, 1, "")) {
		}
		else {
			// we are going to return some message
			streamModified = true;
				
			// match up an 'ok' or 'error' to the corrosponding sent gcode and set it's status
			if(!msg.compare("ok")) {	
				BufferRemove();
				// set response of corrosponding gcode to 'OK'
				gcList.SetResponse(outputStream, STATUS_OK);
				break; 
			}
			// error messages
			else if(!msg.compare(0, 6, "error:")) {			//ERROR NEED TO HALT EVERYTHING
				
				BufferRemove();	
				// retrieve error code
				int errCode = stoi(msg.substr(6));
				// set response of corrosponding gcode to 'ERROR'
				gcList.SetResponse(outputStream, errCode);
				break;
			}
			// alarm messages
			else if(!msg.compare(0, 6, "ALARM:")) {		
				// retrieve error code
				int alarmCode = stoi(msg.substr(6));
					
				string alarmName, alarmDesc;
				if(getAlarmMsg(alarmCode, &alarmName, &alarmDesc)) {	
					cout << "Error: Can't find alarm code!" << endl;
					exit(1);
				}
				
				outputStream << "ALARM " << alarmCode << ": " << alarmName << "\tDesc: " << alarmDesc << endl;
				break;
			}
			else 
			{
				if(!msg.compare(0, 1, "<")) {
					grblParams->DecodeStatus(msg);
					if(verbose)
						outputStream << msg << endl;
					else
						streamModified = false;
				}
				else
				{
					// Startup Line Execution	">G54G20:ok" or ">G54G20:error:X"
					// checks for unlikely event of error and prints to show execution
					if(!msg.compare(0, 1, ">")) {	
						outputStream << msg << endl;
						grblParams->CheckStartupLine(msg);
					}
					// print out messages
					else if(!msg.compare(0, 4, "Grbl") || !msg.compare(0, 4, "[MSG") || !msg.compare(0, 4, "[HLP") || !msg.compare(0, 4, "[echo")) {
						outputStream << msg << endl;
					}
					
					// View build info - just print out	
					// This response hasnt been decoded as seen as unnesessary
					// For more details, see: https://github.com/gnea/grbl/wiki/Grbl-v1.1-Interface
					else if(!msg.compare(0, 4, "[VER") || !msg.compare(0, 4, "[OPT")) {	
						outputStream << msg << endl;
					}
					
					else if(!msg.compare(0, 3, "[GC")) {
						outputStream << msg << endl;
						grblParams->DecodeMode(msg);
					}
					else if(!msg.compare(0, 1, "[")) {
						outputStream << msg << endl;
						grblParams->DecodeParameters(msg);				
					}
					// if startup block added, it will look like this on starup: '>G20G54G17:ok' or error
					else if(!msg.compare(0, 2, "$N")) {	
						outputStream << msg << endl;
						int blockNum = stoi(msg.substr(2, 1));
						if(blockNum == 0 || blockNum == 1)
							grblParams->startupBlock[blockNum] = msg.substr(4);
						else {
							cout << "Startup block number unrecognised: " << msg << endl;
							exit(1);
						}
					}
					
					// settings codes
					else if(!msg.compare(0, 1, "$")) {
						outputStream << msg << endl;
						grblParams->DecodeSettings(outputStream, msg);
					}
					else {			
						outputStream << " ERROR: Unsupported message" << endl;
					}
				}
			}
		}
	} while(1);	
	
	if(streamModified) {
		// copy contents of log to return it
		outputLog = outputStream.str();
		return 1;
	}
	return 0;
}


// grbl has just read a line of GCode
// we then add the number of characters 
// in that line back onto our buffer size variable
void GRBL::BufferRemove() {
	try {
		// add length of string of completed request back onto buffer
		size_t cmdSize = q->dequeue();
		grblBufferSize += cmdSize;
		#ifdef DEBUG
			cout << "(remaining buffer: " << grblBufferSize << "/" << MAX_GRBL_BUFFER << ")\t";
		#endif
	} catch (const char* e) {
		cerr << e << endl;
		exit(1);
	}
}

// we are about to send grbl a line of GCode
// we then remove the number of characters in 
// that line from our buffer size variable
// returns TRUE if full
int GRBL::BufferAdd(int len) {
	// return true if buffer full
	if(grblBufferSize - len < 0)
		return TRUE;
	// reduce buffer size by length of string
	grblBufferSize -= len;
	
	// add length of string to queue
	try{
		q->enqueue(len);
	} catch(const char* e) {
		cerr << e << endl;
		exit(1);
	} 
	return FALSE;
}

// set the interval timer for the status report
// no more that 5Hz (200ns)
void GRBL::SetStatusInterval(uint timems) {
	
	statusTimerInterval = timems;
}

// check status report
void GRBL::Status() {
	if(!statusTimerInterval)
		return;
	if(millis() > statusTimer) {
		SendRT(GRBL_RT_STATUS_QUERY);			
		statusTimer = millis() + statusTimerInterval;
	}
	
}

	
        
/* see: https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands
 
	$$ and $x=val - View and write Grbl settings
	
	
	$# - View gcode parameters
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


	$G - View gcode parser state
		[GC:G0 G54 G17 G21 G90 G94 M0 M5 M9 T0 S0.0 F500.0]

														
			Modal Group					Member Words	*default
		Motion Mode					*G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
		Coordinate System Select	*G54, G55, G56, G57, G58, G59
		Plane Select				*G17, G18, G19
		Distance Mode				*G90, G91
		Arc IJK Distance Mode		*G91.1
		Feed Rate Mode				G93, *G94
		Units Mode					G20, *G21
		Cutter Radius Compensation	*G40
		Tool Length Offset			G43.1, *G49
		Program Mode				*M0, M1, M2, M30
		Spindle State				M3, M4, *M5
		Coolant State				M7, M8, *M9

		T tool number, S spindle speed, and F feed rate,

		NOT INCLUDED: G4, G10 L2, G10 L20, G28, G30, G28.1, G30.1, G53, G92, G92.1
	
	
	$I - View build info
		Optionally, $I can also store a short string to help identify which CNC machine you are communicating with
		To set this string, send Grbl $I=xxx
	
	$N - View startup blocks - (set with $Nx=line)
		GCodes to be ran on startup - set using $N0=xxxx (e.g. '$N0=G21 G54 G17'  metric / work offset 0 / xy plane)
		$N0=
		$N1=
		ok

	$C - Check gcode mode
	
	$X - Kill alarm lock
	
	$H - Run homing cycle
	
	$J=G91 X1 F2000 - Run jogging motion
		Requires at least one X, Y or Z and always an F
		Several jog motions may be queued into the planner buffer, but the jogging can be easily canceled by a jog-cancel or feed-hold real-time command. 
		Grbl will immediately hold the current jog and then automatically purge the buffers of any remaining commands.
		***should check whether we get ok's on cancelled jog commands***
		the following modal commands can be used (these modal commands are only active for THIS jog only, i.e non-modal):
			G20 or G21 - Inch and millimeter mode
			G90 or G91 - Absolute and incremental distances
			G53 - Move in machine coordinates
			
			
	$RST=$, $RST=#, and $RST=*- Restore Grbl settings and data to defaults
		$RST=$ : Erases and restores the $$ Grbl settings back to defaults, which is defined by the default settings file used when compiling Grbl. 
			Often OEMs will build their Grbl firmwares with their machine-specific recommended settings. 
			This provides users and OEMs a quick way to get back to square-one, if something went awry or if a user wants to start over.
		$RST=# : Erases and zeros all G54-G59 work coordinate offsets and G28/30 positions stored in EEPROM. 
			These are generally the values seen in the $# parameters printout. 
			This provides an easy way to clear these without having to do it manually for each set with a G20 L2/20 or G28.1/30.1 command.
		$RST=* : This clears and restores all of the EEPROM data used by Grbl. 
			This includes $$ settings, $# parameters, $N startup lines, and $I build info string. 
			Note that this doesn't wipe the entire EEPROM, only the data areas Grbl uses. To do a complete wipe, please use the Arduino IDE's EEPROM clear example project.
	
	
	$SLP - Enable Sleep Mode
		This command will place Grbl into a de-powered sleep state, shutting down the spindle, coolant, and stepper enable pins and block any commands. 
		It may only be exited by a soft-reset or power-cycle. 
		Once re-initialized, Grbl will automatically enter an ALARM state, because it's not sure where it is due to the steppers being disabled.
*/


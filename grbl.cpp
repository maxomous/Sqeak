/*
 * grbl.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.hpp"
using namespace std;


// This takes a string of 3 values seperated by commas (,) and will return a 3DPoint
// 4.000,0.000,0.000
static point3D stoxyz(string msg) {
	
	istringstream stream(msg);
	string segment;
	float val[3];
	
	for (int i = 0; i < 3; i++) {
		getline(stream, segment, ',');
		val[i] = stof(segment);
	}
	
	return (point3D) {.x=val[0], .y=val[1], .z=val[2]};
}
	
// checks Startup Line Execution for error	">G54G20:ok" or ">G54G20:error:X"
// it is very unlikely that there will be an error as this is checked before it is saves onto the eeprom
void checkStartupLine(string msg) {
	// prints message to show it has executed
	cout << msg << endl;
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
void decodeParameters(gCodeParams_t* p, string msg) {

	cout << msg << endl;
	
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
	else
		cout << "Message unrecognised: " << msg << endl;
}

// decodes modal groups
// and stores inside grbl Parameters
void decodeMode(modalGroup_t* m, string msg) {
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
		else
			cout << "Code unrecognised: " << code << endl;
	} while (stream);
}
// decodes status response 
// and stores inside grbl Parameters
// The $10 status report mask setting can alter what data is present and certain data fields can be reported intermittently (see descriptions for details.)
// The $13 report inches settings alters the units of some data values. $13=0 false indicates mm-mode, while $13=1 true indicates inch-mode reporting.
// "<Idle|WPos:828.000,319.000,49.100|FS:0,0|Pn:PXYZ>"
void decodeStatus(grblStatus_t* s, string msg) {
	
	#ifdef DEBUG
		cout << msg << endl;
	#endif
	
	istringstream stream(msg.substr(1, msg.length()-2));
	string segment;
	vector<string> segs;
	
	while(getline(stream, segment, '|')) 
		segs.push_back(segment);
	
	// itterate backwards through segs[i]s
	for (int i = segs.size()-1; i >= 0; i--) {
		
		cout << segs[i] << endl; 

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
			cout << "state = " << s->state << endl;
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
			cout << segs[i] << endl; 
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
				cout << segs[i].substr(3, a-3) << endl;
				s->feedRate = stof(segs[i].substr(3, a-3)); 
				cout << segs[i].substr(a+1) << endl;
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
			cout << "Pin X = " << s->inputPin_LimX << endl;
			cout << "Pin Y = " << s->inputPin_LimY << endl;
			cout << "Pin Z = " << s->inputPin_LimZ << endl;
			cout << "Probe = " << s->inputPin_Probe << endl;
			cout << "Door = " << s->inputPin_Door << endl;
			cout << "Hold = " << s->inputPin_Hold << endl;
			cout << "SoftReset = " << s->inputPin_SoftReset << endl;
			cout << "CycleStart = " << s->inputPin_CycleStart << endl << endl;
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
			cout << "Spindle Direction = " << s->accessory_SpindleDirection << endl;
			cout << "Flood Coolant = " << s->accessory_FloodCoolant << endl;
			cout << "Mist Coolant = " << s->accessory_MistCoolant << endl;
		}
	}

	cout << "MPos x = " << s->MPos.x << endl;
	cout << "MPos y = " << s->MPos.y << endl;
	cout << "MPos z = " << s->MPos.z << endl;
	cout << "WPos x = " << s->WPos.x << endl;
	cout << "WPos y = " << s->WPos.y << endl;
	cout << "WPos z = " << s->WPos.z << endl;
	cout << "WCO x = " << s->WCO.x << endl;
	cout << "WCO y = " << s->WCO.y << endl;
	cout << "WCO z = " << s->WCO.z << endl;
}

// decodes the settings froms grbl
// just prints them for now
void decodeSettings(string msg) {
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
		// display
		cout << "$" << settingsCode << " = " << value << " (";
		// if it's a mask, display as in binary instead of unit
		(unit == "mask") ? (cout << bitset<8>(value)) : (cout << unit);
		cout << ") :\t" << name << endl; //<< " Desc: " << desc << endl;
	}
}


GRBLParams::GRBLParams() {
	
// base (GRBLParams)
	this->startupBlock[0] = "";
	this->startupBlock[1] = "";
	
// clear all values in object
	point3D defaultP = (point3D){.x=0.0f, .y=0.0f, .z=0.0f};
	
// gCodeParams_t
	gCodeParams_t* g = &(this->param);
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
	modalGroup_t* m = &(this->mode);
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
	grblStatus_t* s = &(this->status);
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
	
	

GCList::GCList(){
	this->count = 0;
	this->written = 0;
	this->read = 0;
}

static void cleanString(string* str) {
	
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

void GCList::add(string str) {
	
	cleanString(&str);
	
	if(str.length() > MAX_GRBL_BUFFER)
		exitf("ERROR: String is longer than grbl buffer!\n");
	
	this->str.push_back(str);
	this->status.push_back(STATUS_NONE);
	
	this->count++;
}	

		
// Reads line of serial interface. 
// Returns string and length in msg
void grblReadLine(int fd, string* msg) {
	
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

static void bufferRemove(Queue* q){
	try {
	// add length of string of completed request back onto buffer
		size_t cmdSize = q->dequeue();
		grblBufferSize += cmdSize;
		//#ifdef DEBUG
			cout << "(remaining buffer: " << grblBufferSize << "/" << MAX_GRBL_BUFFER << ")\t";
		//#endif
	} catch (const char* e) {
		cerr << e << endl;
		exit(1);
	}
}

static int bufferAdd(Queue* q, int len){
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

// set the response status
static void gcListSetResponse(GCList* gcList, int response) {
	// set reponse to corrosponding gcode
	gcList->status[gcList->read] = response;
	cout << '#' << gcList->read << "\t" << gcList->str[gcList->read].substr(0, gcList->str[gcList->read].length() -1) <<  "\t\tstatus: ";
	if (response == STATUS_OK) 
		cout << "ok" << endl;
	else {
		string errName, errDesc;
		if(getErrMsg(response, &errName, &errDesc)) {	
			cout << "Error: Can't find error code!" << endl;
			exit(1);
		}
		cout << "Error " << response << ": " << errName << "\tDesc: " << errDesc << endl;
	}
	gcList->read++;
}
	
// Reads block of serial interface until no response received
void grblRead(GRBLParams* grblParams, int fd, GCList* gcList, Queue* q) {
	
	string msg;
	
	// retrieve data upto response 'ok' or 'error'
	do {
		if(!serialDataAvail(fd))
			break;
		
		grblReadLine(fd, &msg);
		
		#ifdef DEBUG
			cout << "Reading from grbl: " << msg << endl;
		#endif
		
		// ignore blank responses
		if(!msg.compare(0, 1, "")) {
		}
		// match up an 'ok' or 'error' to the corrosponding sent gcode and set it's status
		else if(!msg.compare("ok")) {	
			bufferRemove(q);
			// set response of corrosponding gcode to 'OK'
			gcListSetResponse(gcList, STATUS_OK);
			break; 
		}
		// error messages
		else if(!msg.compare(0, 6, "error:")) {																				//ERROR NEED TO HALT EVERYTHING
			bufferRemove(q);	
			// retrieve error code
			int errCode = stoi(msg.substr(6));
			// set response of corrosponding gcode to 'ERROR'
			gcListSetResponse(gcList, errCode);
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
			cout << "ALARM " << alarmCode << ": " << alarmName << "\tDesc: " << alarmDesc << endl;
			break;
		}
		// Startup Line Execution	">G54G20:ok" or ">G54G20:error:X"
		// checks for unlikely event of error and prints to show execution
		else if(!msg.compare(0, 1, ">")) {	
			checkStartupLine(msg);
		}
		// print out messages
		else if(!msg.compare(0, 4, "Grbl") || !msg.compare(0, 4, "[MSG") || !msg.compare(0, 4, "[HLP") || !msg.compare(0, 4, "[echo")) {
			cout << msg << endl;
		}
		
		// View build info - just print out	
		// This response hasnt been decoded as seen as unnesessary
		// For more details, see: https://github.com/gnea/grbl/wiki/Grbl-v1.1-Interface
		else if(!msg.compare(0, 4, "[VER") || !msg.compare(0, 4, "[OPT")) {	
			cout << msg << endl;
		}
		
		else if(!msg.compare(0, 3, "[GC")) {
			decodeMode(&(grblParams->mode), msg);
		}
		else if(!msg.compare(0, 1, "[")) {
			decodeParameters(&(grblParams->param), msg);				
		}
		else if(!msg.compare(0, 1, "<")) {
			decodeStatus(&(grblParams->status), msg);
		}
		// if startup block added, it will look like this on starup: '>G20G54G17:ok' or error
		else if(!msg.compare(0, 2, "$N")) {	
			cout << msg << endl;
			int blockNum = stoi(msg.substr(2, 1));
			if(blockNum == 0 || blockNum == 1)
				grblParams->startupBlock[blockNum] = msg.substr(4);
			else
				cout << "Startup block number unrecognised: " << msg << endl;
		}
		
		// settings codes
		else if(!msg.compare(0, 1, "$")) {	
			decodeSettings(msg);
		}
		else {			
			cout << "ERROR: Unsupported message: " << msg << endl;
		}
		
	} while(1);	
}

void grblWrite(int fd, GCList* gcList, Queue* q) {
	
	do {
		// exit if nothing new in the gcList
		if (gcList->written >= gcList->count)
			break;
			
		string* curStr = &gcList->str[gcList->written];
		
		#ifdef DEBUG
			cout << "Writing to grbl: " << *curStr << " (length = "<< len << ")" << endl;
		#endif
		
		if(bufferAdd(q, curStr->length()))
			break;
		// set status
		gcList->status[gcList->written] = STATUS_PENDING;
		// write to serial port
		serialPuts(fd, curStr->c_str());
		gcList->written++;
		//printf("ADDING #%d: %s", gcListWritten, gc->str.c_str);
	} while (1);
}

/* sends a REALTIME COMMAND
 * 	- These are not considered as part of the streaming protocol and are executed instantly
 * 	- They do not require a line feed or carriage return after them.
 * 	- None of these respond with 'ok' or 'error'
 *	see https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands
 */ 
void grblRealTime(int fd, char cmd) {
	
	switch (cmd) {
		case GRBL_RT_SOFT_RESET:
			printf("Sent: 'Soft Reset'\n");
			break;
		case GRBL_RT_STATUS_QUERY:
			printf("Sent: 'Status Query'\n");
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

/*
 * grbl.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.h"
using namespace std;

void MainSettings::SetUnitsInches(bool val) 
{
	if(val) {
		units_Distance_	= "in";
		units_Feed_		= "in/min";
	}
	else /*mm*/ {
		units_Distance_	= "mm";
		units_Feed_		= "mm/min";
	}	
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
	
	return (point3D) {val[0], val[1], val[2]};
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
			return;
		else {
			size_t b = response.find(":");
			if(b != string::npos) {
				int errCode = stoi(response.substr(b+1));
				Log::Critical("Startup Line Execution has encountered an error: %d", errCode);
			}
			else
				Log::Critical("Something is not right here, didn't find 2nd ':'");
		}
	}
	else
		Log::Critical("Something is not right here, didn't find ':'");
}	

// decodes GCode Parameters
// and stores inside grbl Parameters
void GRBLParams::DecodeParameters(const string& msg) {
	
	// get name i.e 'G54'
	string param = msg.substr(1, 3);
	// get number string i.e. '4.000,0.000,0.000'
	string num = msg.substr(5, msg.length()-6);
	
	// [G54:4.000,0.000,0.000] - [G59:4.000,0.000,0.000]
	if(!param.compare("G54")) 
		gcParam.workCoords_[0] = stoxyz(num);
	else if(!param.compare("G55")) 
		gcParam.workCoords_[1] = stoxyz(num);
	else if(!param.compare("G56")) 
		gcParam.workCoords_[2] = stoxyz(num);
	else if(!param.compare("G57")) 
		gcParam.workCoords_[3] = stoxyz(num);
	else if(!param.compare("G58")) 
		gcParam.workCoords_[4] = stoxyz(num);
	else if(!param.compare("G59")) 
		gcParam.workCoords_[5] = stoxyz(num);
	// [G28:1.000,2.000,0.000]  / [G30:4.000,6.000,0.000]
	else if(!param.compare("G28")) 
		gcParam.homeCoords_[0] = stoxyz(num);
	else if(!param.compare("G30")) 
		gcParam.homeCoords_[1] = stoxyz(num);
	// [G92:0.000,0.000,0.000]
	else if(!param.compare("G92")) 
		gcParam.offsetCoords_ = stoxyz(num);
	// [TLO:0.000]	
	else if(!param.compare("TLO")) 
		gcParam.toolLengthOffset_ = stof(num);
	// [PRB:0.000,0.000,0.000:0]
	else if(!param.compare("PRB")) {
		gcParam.probeOffset_ = stoxyz(msg.substr(5, msg.length()-8));
		gcParam.probeSuccess_ = (bool)stoi(msg.substr(msg.length()-2, 1));
	}
	else {
		Log::Error("Something's not right... Parameter unrecognised: %s", msg.c_str());
	}
}
// decodes the startup block
void GRBLParams::DecodeStartupBlock(const string& msg) 
{
	int blockNum = stoi(msg.substr(2, 1));
	if(blockNum == 0 || blockNum == 1)
		mode.StartupBlock_[blockNum] = msg.substr(4);
	else {
		Log::Error("Something's not right... Startup block number unrecognised: %s", msg.c_str());
	}
}
// decodes modal groups
// and stores inside grbl Parameters
void GRBLParams::DecodeMode(const string& msg) {
	
	string s = msg.substr(4, msg.length()-5);
	istringstream stream(s);
	do {
		string code;
		stream >> code;
		// [GC:G0 G54 G17 G21 G90 G94 M0 M5 M9 T0 S0.0 F500.0]
		if(code == "")
			{/*do nothing*/}
		else if(code == "G0" || code == "G1" || code ==  "G2" || code ==  "G3" || code ==  "G38.2" || code ==  "G38.3" || code ==  "G38.4 "|| code ==  "G38.5" || code ==  "G80")
			mode.MotionMode_ = code;
		else if(code == "G54" || code == "G55" || code == "G56" || code == "G57" || code ==  "G58" || code == "G59")
			mode.CoordinateSystem_ = code;
		else if(code == "G17" || code == "G18" || code == "G19")
			mode.Plane_ = code;
		else if(code == "G90" || code == "G91")
			mode.DistanceMode_ = code;
		else if(code == "G91.1")
			mode.ArcIJKDistanceMode_ = code;
		else if(code == "G93" || code == "G94")
			mode.FeedRateMode_ = code;
		else if(code == "G20" || code == "G21")
			mode.UnitsMode_ = code;
		else if(code == "G40")
			mode.CutterRadCompensation_ = code;
		else if(code == "G43.1" || code == "G49")
			mode.ToolLengthOffset_ = code;
		else if(code == "M0" || code == "M1" || code == "M2" || code == "M30")
			mode.ProgramMode_ = code;
		else if(code == "M3" || code == "M4" || code == "M5")
			mode.SpindleState_ = code;
		else if(code == "M7" || code == "M8" || code == "M9")
			mode.CoolantState_ = code;
		else if(code.compare(0, 1, "T"))
			mode.toolNumber_ = stoi(code.substr(1, code.length()-1));
		else if(code.compare(0, 1, "S"))
			mode.spindleSpeed_ = stof(code.substr(1, code.length()-1));
		else if(code.compare(0, 1, "F"))
			mode.feedRate_ = stof(code.substr(1, code.length()-1));
		else {
			Log::Error("Something's not right... Mode unrecognised: %s", code.c_str());
		}
	} while (stream);
}

void GRBLParams::SetState(const string& state) {
	
	if(state == "Idle" || state.substr(0, 4) == "Hold"  || state == "Sleep") {
		status.state_ = state;
		status.stateColour_ = GRBL_STATE_COLOUR_IDLE; // for colouring state
	}
	else if(state == "Run" || state == "Jog" || state == "Check" || state == "Home") {
		status.state_ = state;
		status.stateColour_ = GRBL_STATE_COLOUR_MOTION;
	}
	else if(state == "Alarm" || state.substr(0, 4) == "Door") {
		status.state_ = state;
		status.stateColour_ = GRBL_STATE_COLOUR_ALERT;
	}
	else {
		Log::Error("Unknown state: %s", state.c_str());
		status.state_ = "Unknown";
		status.stateColour_ = GRBL_STATE_COLOUR_ALERT;
	}
}

// decodes status response 
// and stores inside grbl Parameters
// The $10 status report mask setting can alter what data is present and certain data fields can be reported intermittently (see descriptions for details.)
// The $13 report inches settings alters the units of some data values. $13=0 false indicates mm-mode, while $13=1 true indicates inch-mode reporting.
// "<Idle|WPos:828.000,319.000,49.100|FS:0,0|Pn:PXYZ>"
void GRBLParams::DecodeStatus(const string& msg) {
		
	istringstream stream(msg.substr(1, msg.length()-2));
	// reserve enough space for all possible segments
	static vector<string> segs;
	segs.reserve(16);
	segs.clear();
	
	static string segment(32, 0);
	// iterate backwards through segs[i]s
	// this is to ensure WCO can be calculated after MPos / WPos
	while(getline(stream, segment, '|')) 
		segs.emplace_back(segment);
	
	// **** ^ 3 allocations above ^ ****
	
	for (int i = segs.size()-1; i >= 0; i--) {
		
		// Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep
		//- `Hold:0` Hold complete. Ready to resume.
		//- `Hold:1` Hold in-progress. Reset will throw an alarm.
		//- `Door:0` Door closed. Ready to resume.
		//- `Door:1` Machine stopped. Door still ajar. Can't resume until closed.
		//- `Door:2` Door opened. Hold (or parking retract) in-progress. Reset will throw an alarm.
		//- `Door:3` Door closed and resuming. Restoring from park, if applicable. Reset will throw an alarm.
		
		if(segs[i].substr(0, 4) == "Hold" || segs[i].substr(0, 4) == "Door" || segs[i] == "Idle" || segs[i] == "Sleep" || 
		segs[i] == "Run" || segs[i] == "Jog" || segs[i] == "Check" || segs[i] == "Home" || segs[i] == "Alarm") {
			SetState(segs[i]);
		}	
	
		
		/*	
		if(segs[i] == "Idle" || segs[i] == "Run" || segs[i].substr(0, 4) == "Hold" || segs[i] == "Jog" || segs[i] == "Alarm" 
			|| segs[i].substr(0, 4) == "Door" || segs[i] == "Check" || segs[i] == "Home" || segs[i] == "Sleep" ) {
			status._state = segs[i];
		}*/
		// MPos:0.000,-10.000,5.000 machine position  or  WPos:-2.500,0.000,11.000 work position
		// WPos = MPos - WCO
		else if(segs[i].substr(0, 4) == "MPos") {
			status.MPos_ = stoxyz(segs[i].substr(5));
			status.WPos_ = status.MPos_ - status.WCO_;
		}
		else if(segs[i].substr(0, 4) == "WPos") {
			status.WPos_ = stoxyz(segs[i].substr(5));
			status.MPos_ = status.WPos_ + status.WCO_;
			
			// **** ^ 2 allocations in here ^ ****
		}
		// work coord offset - shown every 10-30 times
		// the current work coordinate system, G92 offsets, and G43.1 tool length offset
		else if(segs[i].substr(0, 3) == "WCO") {
			status.WCO_ = stoxyz(segs[i].substr(4));
		}
		
		// Buffer State - mainly used for debugging
		// Bf:15,128. number of available blocks in the planner buffer / number of available bytes in the serial RX buffer.
		else if(segs[i].substr(0, 2) == "Bf") {
		}
		// line number Ln:99999
		else if(segs[i].substr(0, 2) == "Ln") {
			status.lineNum_ = stoi(segs[i].substr(3)); 
		}
		// feed & speed
		// FS:500,8000 (feed rate / spindle speed)
		else if(segs[i].substr(0, 2) == "FS") {
			size_t a = segs[i].find(",");
			if (a != string::npos) {
				status.feedRate_ = stof(segs[i].substr(3, a-3)); 
				status.spindleSpeed_ = stoi(segs[i].substr(a+1)); 
			}
			else
				Log::Error("Can't find ',' in FS");
		}
		// feed only
		// F:500 (feed rate only) - when VARIABLE_SPINDLE is disabled in config.h
		else if(segs[i].substr(0, 1) == "F") {
			status.feedRate_ = stof(segs[i].substr(2)); 
			
		}
		// Input Pin State
		// Pn:XYZPDHRS - can be any number of letters
		else if(segs[i].substr(0, 2) == "Pn") {
			// set all pins to default
			status.inputPin_LimX_ = false;
			status.inputPin_LimY_ = false;
			status.inputPin_LimZ_ = false;
			status.inputPin_Probe_ = false;
			status.inputPin_Door_ = false;
			status.inputPin_Hold_ = false;
			status.inputPin_SoftReset_ = false;
			status.inputPin_CycleStart_ = false;
			
			string str = segs[i].substr(3);
			for (size_t j = 0; j < str.length(); j++) {
				if(str[j] == 'X')
					status.inputPin_LimX_ = true;
				else if(str[j] == 'Y')
					status.inputPin_LimY_ = true;
				else if(str[j] == 'Z')
					status.inputPin_LimZ_ = true;
				else if(str[j] == 'P')
					status.inputPin_Probe_ = true;
				else if(str[j] == 'D')
					status.inputPin_Door_ = true;
				else if(str[j] == 'H')
					status.inputPin_Hold_ = true;
				else if(str[j] == 'R')
					status.inputPin_SoftReset_ = true;
				else if(str[j] == 'S')
					status.inputPin_CycleStart_ = true;
				else
					Log::Error("Input pin unrecognised: %c", str[j]);
			}
			
		}
		//Override Values:
		// Ov:100,100,100 current override values in percent of programmed values for feed, rapids, and spindle speed, respectively.
		else if(segs[i].substr(0, 2) == "Ov") {
			point3D ov = stoxyz(segs[i].substr(3));
			status.override_Feedrate_ = (int)ov.x;
			status.override_RapidFeed_ = (int)ov.y;
			status.override_SpindleSpeed_ = (int)ov.z;
		}
		// Accessory State
		// 	'A:SFM' - can be any number of letters
		// S indicates spindle is enabled in the CW direction. This does not appear with C.
		// C indicates spindle is enabled in the CCW direction. This does not appear with S.
		// F indicates flood coolant is enabled.
		// M indicates mist coolant is enabled.
		else if(segs[i].substr(0, 2) == "A:") {
			// set all pins to default
			status.accessory_SpindleDir_ = false;
			status.accessory_FloodCoolant_ = false;
			status.accessory_MistCoolant_ = false;
			
			string str = segs[i].substr(2);
			for (size_t j = 0; j < str.length(); j++) {
				if(str[j] == 'S')
					status.accessory_SpindleDir_ = CLOCKWISE;	// (1)
				else if(str[j] == 'C')
					status.accessory_SpindleDir_ = ANTICLOCKWISE;	// (-1)
				else if(str[j] == 'F')
					status.accessory_FloodCoolant_ = true;
				else if(str[j] == 'M')
					status.accessory_MistCoolant_ = true;
				else
					Log::Error("Accessory pin unrecognised: %c", str[j]);
			}
		}
	}
}
/*	
$0 = 10 (us) : Step pulse time
$1 = 25 (ms) : Step idle delay
$2 = 7 (00000111) : Step pulse invert
$3 = 6 (00000110) : Step direction invert
$4 = 0 (boolean) : Invert step enable pin
$5 = 0 (boolean) : Invert limit pins
$6 = 1 (boolean) : Invert probe pin
$10 = 2 (00000010) : Status report options
$11 = 0.01 (mm) : Junction deviation
$12 = 0.002 (mm) : Arc tolerance
$13 = 0 (boolean) : Report in inches
$20 = 0 (boolean) : Soft limits enable
$21 = 1 (boolean) : Hard limits enable
$22 = 1 (boolean) : Homing cycle enable
$23 = 3 (00000011) : Homing direction invert
$24 = 25 (mm/min) : Homing locate feed rate
$25 = 2500 (mm/min) : Homing search seek rate
$26 = 25 (ms) : Homing switch debounce delay
$27 = 1 (mm) : Homing switch pull-off distance
$30 = 24000 (RPM) : Maximum spindle speed
$31 = 0 (RPM) : Minimum spindle speed
$32 = 0 (boolean) : Laser-mode enable
$100 = 320 (step/mm) : X-axis travel resolution
$101 = 320 (step/mm) : Y-axis travel resolution
$102 = 640 (step/mm) : Z-axis travel resolution
$110 = 6000 (mm/min) : X-axis maximum rate
$111 = 6000 (mm/min) : Y-axis maximum rate
$112 = 3000 (mm/min) : Z-axis maximum rate
$120 = 200 (mm/sec^2) : X-axis acceleration
$121 = 200 (mm/sec^2) : Y-axis acceleration
$122 = 200 (mm/sec^2) : Z-axis acceleration
$130 = 950 (mm) : X-axis maximum travel
$131 = 530 (mm) : Y-axis maximum travel
$132 = 160 (mm) : Z-axis maximum travel
*/
// decodes the settings froms grbl
// just prints them for now
string GRBLParams::DecodeSettings(const string& msg) {
	// retrieve settings code & current value
	size_t a = msg.find("=");
	// checks to prevent errors
	if((a != string::npos) && (a > 0) && msg.length() > a+1) {
		int settingsCode = stoi(msg.substr(1, a-1));
		float value = stof(msg.substr(a+1));
		
		//determine which units we are using
		if(settingsCode == 13)
			settings.SetUnitsInches((int)value);
			
		if(settingsCode == 30)
			settings.max_SpindleSpeed_ = value;
		if(settingsCode == 31)
			settings.min_SpindleSpeed_ = value;
		
		if(settingsCode == 110 || settingsCode == 111 || settingsCode == 112) {
			if(settingsCode == 110)
				settings.max_FeedRateX_ = value;
			if(settingsCode == 111)
				settings.max_FeedRateY_ = value;
			if(settingsCode == 112)
				settings.max_FeedRateZ_ = value;
			// set largest value to max_feedrate
			settings.max_FeedRate_ = max(settings.max_FeedRateX_, settings.max_FeedRateY_);
			settings.max_FeedRate_ = max(settings.max_FeedRate_, settings.max_FeedRateZ_);
		}
		
		// retrieve name, desc & unit of setting
		string name, unit, desc;
		if(getSettingsMsg(settingsCode, &name, &unit, &desc)) {	
			Log::Error("Error %d: Can't find settings code", settingsCode);
		}

		// display unit and name for setting
		ostringstream s;
		s << " (";
		// if it's a mask, display as in binary instead of unit
		(unit == "mask") ? (s << bitset<8>(value)) : (s << unit);
		// append the name
		s << ") : " << name; //<< " (" << desc << ")";
		return s.str();
	}
	return "";
}

GCItem_t GCList::GetNextItem() {
	return GetItem(written);
}

GCItem_t GCList::GetItem(int n) {
	return gCodeList[n];
}

int GCList::GetSize() {
	return gCodeList.size();
}
// takes a GCode linbe and cleans up
// (removes spaces, comments, make uppercase, ensures '\n' is present)
void GCList::CleanString(string& str) {
	
	// strip out whitespace - this means we can fit more in the buffer
	str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
	// remove comments within () 
	size_t a = str.find("(");
	if(a != string::npos) {
		size_t b = str.find(")", a);
		if(b != string::npos)
			str.erase(a, b-a+1);
	}
	// remove comments after ;
	size_t c = str.find(";");
	if(c != string::npos)
		str.erase(c, str.length()-c );
	// make all uppercase
	upperCase(str);
	// add a newline character to end
	str.append("\n");
}

// add a line of GCode to the GCode List
int GCList::Add(string& str) {

	CleanString(str);
	// ignore blank lines
	if(str == "\n") {
		return 0;
	}	
	if(str.length() > MAX_GRBL_BUFFER) {
		Log::Error("Line is longer than the maximum buffer size");
		return -1;
	}
	gCodeList.emplace_back((GCItem_t){ str, STATUS_UNSENT });
	return 0;
}	

void GCList::SetStatus(int status)
{	// set status
	gCodeList[written++].status = status;
}	

// set the response status of the GCode line
void GCList::SetResponse(int response) 
{
	if(read >= gCodeList.size()) {
		Log::Error("We are reading more than we have sent... size = %d", gCodeList.size());
		return;
	}
	// Set reponse to corrosponding gcode
	gCodeList[read++].status = response;
	// to trigger that we have reached end of file
	if(fileEnd != 0 && read >= fileEnd)
		EndOfFile();
}
bool GCList::IsWaitingToSend() {
	return written < gCodeList.size();
}

void GCList::EndOfFile() {
	fileStart = 0;
	fileEnd = 0;
	Log::Info("End of file");
}

bool GCList::IsFileRunning() {
	return fileEnd;
}

void GCList::FileStart() {
	fileStart = gCodeList.size();
}

void GCList::FileSent() {
	fileEnd = gCodeList.size();
}

uint GCList::GetFileLines() {
	return fileEnd - fileStart;
}

uint GCList::GetFilePos() {
	return read - fileStart;
}


// clears any completed GCodes in the buffer
// used for clearing old commands in log
void GCList::ClearCompleted() {
	gCodeList.erase(gCodeList.begin(), gCodeList.begin() + read);
	fileEnd = (fileEnd < read) ? 0 : fileEnd - read;
	if (fileEnd <= 0) {
		fileStart = 0;
		fileEnd = 0;
	}	
	written -= read;
	read = 0;
}

// clears any remaining GCodes in our buffer
// for cancelling any commands that are waiting to be sent
void GCList::ClearUnsent() {
	gCodeList.erase(gCodeList.begin() + written, gCodeList.end());
	fileEnd = gCodeList.size();
}

// clears any remaining GCodes in ours, and GRBLs buffer
// for resetting only
// queue MUST be emptied also + grblBufferSize set to max
void GCList::ClearSent() {
	gCodeList.erase(gCodeList.begin() + read, gCodeList.end());
	fileStart = 0;
	fileEnd = 0;
	written = read;
}

/*
// clears entire GCode List  - unused
void GCList::ClearAll() {
	gCodeList.clear();
	written = 0;
	read = 0;
	fileEnd = 0;
}
*/
 
GRBL::GRBL() { 
	statusTimer = millis() + statusTimerInterval;
}


GRBL::~GRBL() {
	Disconnect();
}

// Flush the serial buffer
void GRBL::Flush() {
	serialPuts(fd, "\r\n\r\n");
	delay(2000);
	serialFlush(fd);
}

int GRBL::Connect() {
	if(!connected) {
		fd = serialOpen(SERIAL_DEVICE, SERIAL_BAUDRATE);
		if(fd < 0) {
			Log::Error("Could not open serial device");
			return -1;
		}
		// clear the serial buffer
		Flush();
		connected = true;
		// request settings
		Send("$$");
	}
	return 0;
}

void GRBL::Disconnect() {
	if(connected) {
		// close serial connection
		serialClose(fd);
		connected = false;
	}
}
// adds to the GCode list, ready to be written when buffer has space
// sending a pointer is slightly quicker as it wont have to be be copied, it will however, modify the original string to remove whitespace and comments etc
// returns 0 on success, -1 on failure
int GRBL::Send(string& cmd) 
{
	if(!connected) {
		Log::Error("Connect to GRBL before sending commands");
		return -2;
	}
	if(gcList.IsFileRunning()) {
		Log::Error("File is already running");
		return -3;
	}
	// add to log
	Log::Info((string)"Sent: " + cmd);
	// add command to GCList
	if(gcList.Add(cmd)) 
		return -1;	// line was longer than GRBL_MAX_BUFFER

	return 0;
}
// this makes a copy of a const string (i.e Send("G90")) 
// so that we can pass it to and manipulate it in lower 
// down functions (i.e. removing whitespace etc)
int GRBL::Send(const string& cmd) {
	string str = cmd;
	return Send(str);
}
	 
int GRBL::SendFile(const string& file) {
	
	auto executeLine = [this](string& str) {
		
		if(Send(str))
			return -1; // cannot send, is probably busy with file running

		return 0;
	}; 
	
	gcList.FileStart();
	
	if(File::Read(file, executeLine)) {
		Log::Error(string("Could not open file ") + file);
		gcList.EndOfFile();
		return -1;
	}
	// state that we have sent a file - this is to prevent sending twice
	gcList.FileSent();
	return 0;
}

// Sends an incremental jog to p
// jogs do not affect the parser state 
// therefore you do not need to set the machine back to G90 after using a G91 command
// and the feedrate is not modal
void GRBL::SendJog(point3D p, int feedrate) {
    // cancel with Grbl->SendRT(GRBL_RT_JOG_CANCEL);
    
    if(p == point3D(0,0,0)) {
		Log::Error("Jog requires a distance");
		return;
	}
    // example: $J=G91 X10 F1000
    ostringstream s;
    s << "$J=G91";
    
   // s << fixed << setprecision(3);
    if(p.x) s << "X" << p.x;
    if(p.y) s << "Y" << p.y;
    if(p.z) s << "Z" << p.z;
    
    s << "F" << feedrate;
   
	if(Send(s.str())) {
		// cannot send, is probably busy with file transfer
	}
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
			Log::Info("Sent: 'Soft Reset'");
			break;
		case GRBL_RT_STATUS_QUERY:
			#ifdef DEBUG
				Log::Info("Sent: 'Status Query'");
			#endif
			break;
		case GRBL_RT_HOLD:
			Log::Info("Sent: 'Hold'");
			break;
		case GRBL_RT_RESUME:
			Log::Info("Sent: 'Resume'");
			break;
			
		case GRBL_RT_DOOR:
			Log::Info("Sent: 'Door'");
			break;
		case GRBL_RT_JOG_CANCEL:
			Log::Info("Sent: 'Cancel Jog'");
			break;
			
		case GRBL_RT_OVERRIDE_FEED_100PERCENT:
			Log::Info("Sent: 'Override Feedrate (Set to 100%)'");
			break;
		case GRBL_RT_OVERRIDE_FEED_ADD_10PERCENT:
			Log::Info("Sent: 'Override Feedrate (+10%)'");
			break;
		case GRBL_RT_OVERRIDE_FEED_MINUS_10PERCENT:
			Log::Info("Sent: 'Override Feedrate (-10%)'");
			break;
		case GRBL_RT_OVERRIDE_FEED_ADD_1PERCENT:
			Log::Info("Sent: 'Override Feedrate (+1%)'");
			break;
		case GRBL_RT_OVERRIDE_FEED_MINUS_1PERCENT:
			Log::Info("Sent: 'Override Feedrate (-1%)'");
			break;
			
		case GRBL_RT_OVERRIDE_RAPIDFEED_100PERCENT:
			Log::Info("Sent: 'Override Rapid Feedrate (Set to 100%)'");
			break;
		case GRBL_RT_OVERRIDE_RAPIDFEED_50PERCENT:
			Log::Info("Sent: 'Override Rapid Feedrate (Set to 50%)'");
			break;
		case GRBL_RT_OVERRIDE_RAPIDFEED_25PERCENT:
			Log::Info("Sent: 'Override Rapid Feedrate (Set to 25%)'");
			break;
			
		case GRBL_RT_OVERRIDE_SPINDLE_100PERCENT:
			Log::Info("Sent: 'Override Spindle Speed (Set to 100%)'");
			break;
		case GRBL_RT_OVERRIDE_SPINDLE_ADD_10PERCENT:
			Log::Info("Sent: 'Override Spindle Speed (+10%)'");
			break;
		case GRBL_RT_OVERRIDE_SPINDLE_MINUS_10PERCENT:
			Log::Info("Sent: 'Override Spindle Speed (-10%)'");
			break;
		case GRBL_RT_OVERRIDE_SPINDLE_ADD_1PERCENT:
			Log::Info("Sent: 'Override Spindle Speed (+1%)'");
			break;
		case GRBL_RT_OVERRIDE_SPINDLE_MINUS_1PERCENT:
			Log::Info("Sent: 'Override Spindle Speed (-1%)'");
			break;
			
		case GRBL_RT_SPINDLE_STOP:
			Log::Info("Sent: 'Stop Spindle'");
			break;
		case GRBL_RT_FLOOD_COOLANT:
			Log::Info("Sent: 'Flood Coolant'");
			break;
		case GRBL_RT_MIST_COOLANT:
			Log::Info("Sent: 'Mist Coolant'");
			break;

		default:
			Log::Error("Realtime command unrecognised: %c", cmd);
			return;
	}
	serialPutchar(fd, cmd);
}
void GRBL::Cancel() {
	// clears any gcodes which havent received a reponse (but ones left in grbl's buffer will still exectute)
	gcList.ClearUnsent();
}

// soft reset 
void GRBL::SoftReset() {
	// send reset to grbl
	SendRT(GRBL_RT_SOFT_RESET);	
	// flush the serial buffer
	Flush();
	// clears any waiting gcodes in the gclist
	gcList.ClearSent();
	// clears the queue
	while(!q.empty())
		q.pop();
	// sets buffer size to max
	grblBufferSize = MAX_GRBL_BUFFER;
}


void GRBL::Write() {
	
	do {
		// return if nothing new in the gcList
		if(!(gcList.IsWaitingToSend()))
			break;
		string gcode = gcList.GetNextItem().str;
		
		#ifdef DEBUG
			Log::Info(string("Sending (raw): ") + gcode);
		#endif
		
		if(BufferAdd(gcode.length()))
			break;
		// write to serial port
		serialPuts(fd, gcode.c_str());
		// set status
		gcList.SetStatus(STATUS_PENDING);
	} while (1);
}

// Reads line of serial interface and returns onto msg
void GRBL::ReadLine(string& msg) {
	
	msg.clear();
	
	char buf;
	//retrieve line
	do {
		// retrieve letter
		buf = serialGetchar(fd);
		// break if end of line
		if (buf == '\n') 
			break;
		if(msg.length() >= MAX_GRBL_RECEIVE_BUFFER) {
			Log::Warning("Serial input length is greater than input buffer, allocating more memory");
			msg.resize(2 * msg.capacity());
		}
		// add to buffer - skip non-printable characters
		if(isprint(buf)) 
			msg += buf;
	} while(1);
}

// Reads block of serial interface until no response received
void GRBL::Read() {
	
	// Response for an 'ok'
	auto okResponse = [this]() 
	{	// remove command from gcbuffer
		if(BufferRemove())
			return;
		// set response of corrosponding gcode to 'OK'
		gcList.SetResponse(STATUS_OK);
		// add response to log
		Log::Response("ok");
	};
	
	// Response for an 'error'
	auto errorResponse = [this](const string& msg) 
	{	// remove command from gcbuffer
		if(BufferRemove())
			return;
		// retrieve error code
		int errCode = stoi(msg.substr(6));
		// set response of corrosponding gcode to 'ERROR'
		gcList.SetResponse(errCode);
		// add response to log
		string errName, errDesc;
		if(getErrMsg(errCode, &errName, &errDesc)) 
			Log::Error("Error %d: Can't find error code", errCode);
		else
			Log::Response("Error %d: %s (%s)", errCode, errName.c_str(), errDesc.c_str());
	};
	
	// Response for an 'alarm'
	auto alarmResponse = [this](const string& msg) 
	{	// retrieve alarm code
		int alarmCode = stoi(msg.substr(6));
		// add response to log
		string alarmName, alarmDesc;
		if(getAlarmMsg(alarmCode, &alarmName, &alarmDesc))
			Log::Error("ALARM %d: Can't find alarm code", alarmCode);
		else	
			Log::Response("ALARM %d: %s (%s)", alarmCode, alarmName.c_str(), alarmDesc.c_str());
	};
	
	static string msg(128, ' ');
	
	// retrieve data upto response 'ok', 'error' or 'alarm'
	do {
		if(!serialDataAvail(fd))
			break;
			
		ReadLine(msg);
		
		#ifdef DEBUG
			Log::Info(string("Recieved (raw): ") + msg);
		#endif
		
		// ignore blank responses
		if(!msg.compare(0, 1, "")) {
		}
		else {				
			// match up an 'ok' or 'error' to the corrosponding sent gcode and set it's status
			if(!msg.compare("ok")) {
				okResponse();
				break; 
			} 
			// error messages
			else if(!msg.compare(0, 6, "error:")) {	
				errorResponse(msg);
				// We have recieved an error, the safest thing to do is stop everything
				if(IsFileRunning()) {
					Log::Error("An error occured mid file transfer. The machine has been reset for safety. Consider using check mode before running file");
					SoftReset();
				}
				break;
			}
			// alarm messages
			else if(!msg.compare(0, 6, "ALARM:")) {		
				alarmResponse(msg);
				// status report stops getting sent from grbl so we update state manually
				Param.SetState("Alarm"); 
				break;
			}
			else 
			{
				if(!msg.compare(0, 1, "<")) {
					// Flag used to make sure we receice status before we carry out next task
					waitingForStatus = false;
					Param.DecodeStatus(msg);
					if(viewStatusReport)
						Log::Response(msg);
				}
				else
				{
					// Startup Line Execution	">G54G20:ok" or ">G54G20:error:X"
					if(!msg.compare(0, 1, ">")) {	
						Param.CheckStartupLine(msg);
						Log::Response(msg);
					}
					// just print out message
					else if(!msg.compare(0, 4, "Grbl") || !msg.compare(0, 4, "[MSG") || !msg.compare(0, 4, "[HLP") || !msg.compare(0, 4, "[echo")) {
						Log::Response(msg);
					}
					// View build info - just print out	
					// This response hasnt been decoded as seen as unnesessary
					// For more details, see: https://github.com/gnea/grbl/wiki/Grbl-v1.1-Interface
					else if(!msg.compare(0, 4, "[VER") || !msg.compare(0, 4, "[OPT")) {	
						Log::Response(msg);
					}
					
					else if(!msg.compare(0, 3, "[GC")) {
						Param.DecodeMode(msg);
						Log::Response(msg);
					}
					else if(!msg.compare(0, 1, "[")) {
						Param.DecodeParameters(msg);				
						Log::Response(msg);
					}
					// if startup block added, it will look like this on starup: '>G20G54G17:ok' or error
					else if(!msg.compare(0, 2, "$N")) {	
						Param.DecodeStartupBlock(msg);
						Log::Response(msg);
					}
					// settings codes
					else if(!msg.compare(0, 1, "$")) {
						string setting = Param.DecodeSettings(msg);
						Log::Response(msg + setting);
					}
					else {	
						Log::Error("Unsupported GRBL message: %s", msg.c_str());
					}
				}
			}
		}
	} while(1);	
}


// grbl has just read a line of GCode
// we then add the number of characters 
// in that line back onto our buffer size variable
int GRBL::BufferRemove() {
	
	if(q.empty()) {
		SoftReset();
		Log::Error("Unexpected response, machine has been reset. (Purhaps there were some commands left in GRBL's buffer?)");
		return -1;
	}
	
	// add length of string of completed request back onto buffer
	size_t cmdSize = q.front();
	q.pop();
	
	grblBufferSize += cmdSize;
	
	#ifdef DEBUG
		Log::Info("Remaining buffer: %d/%d", grblBufferSize, MAX_GRBL_BUFFER);
	#endif
	
	return 0;
}

// we are about to send grbl a line of GCode
// we then remove the number of characters in 
// that line from our buffer size variable
// returns TRUE if full
int GRBL::BufferAdd(int len) {
	// return true if buffer full
	if(grblBufferSize - len < 0) {
		return -1;
	}
	// reduce buffer size by length of string
	grblBufferSize -= len;
	q.push(len);
	
	return 0;
}

// set the interval timer for the status report
// no more that 5Hz (200ns)
void GRBL::SetStatusInterval(uint timems) {
	
	statusTimerInterval = timems;
}

// check status report
void GRBL::RequestStatus() {
	if(!statusTimerInterval)
		return;
	if(millis() > statusTimer) {
		SendRT(GRBL_RT_STATUS_QUERY);	
		statusTimer = millis() + statusTimerInterval;
	}
	 
}

#define WAIT_FOR_STATUS_TIMEOUT 10000 // 10s

// A blocking loop until we recieve next status report (waitingForStatus is set to false)
// returns 0 when recieved and is idle, -1 on timeout or not idle
int GRBL::WaitForIdle() { 
	
	string grblReponse;
	waitingForStatus = true;
	uint timeout = millis() + WAIT_FOR_STATUS_TIMEOUT;
	// send status query
	SendRT(GRBL_RT_STATUS_QUERY);
	do {
		// just in case we receive a message before the status report
		Read();
        // return error if timeout
		if(millis() > timeout) {
			Log::Error("Timeout, no response recieved from grbl");
			return -1;
		}
	} while (waitingForStatus);
	
	if(Param.status.state() != "Idle" && Param.status.state() != "Check") {
	    Log::Error("Grbl is not 'Idle'");
		return -2;
	}
	
	return 0;
}	
	    
bool GRBL::IsFileRunning() {
	return gcList.IsFileRunning();
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


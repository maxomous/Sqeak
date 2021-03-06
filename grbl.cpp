/*
 * grbl.c
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
	

grblParams_t::grblParams_t() {
	// clear all values in object
	point3D defaultP = (point3D){.x=0.0f, .y=0.0f, .z=0.0f};
	
	// 
	// G54 - G59
	for (int i = 0; i < 6; i++)
		this->param.workCoords[i] = defaultP;
	// G28 & G30
	for (int i = 0; i < 2; i++)
		this->param.homeCoords[i] = defaultP;	
	// G92
	this->param.offsetCoords = defaultP;
		
	this->param.toolLengthOffset = 0.0f;
	
	this->param.probeOffset = defaultP;
	this->param.probeSuccess = FALSE;
	
	// modal group
	this->mode.MotionMode = "G0";
	this->mode.CoordinateSystem = "G54";
	this->mode.Plane = "G17";
	this->mode.DistanceMode = "G90";
	this->mode.ArcIJKDistanceMode = "G91.1";
	this->mode.FeedRateMode = "G94";
	this->mode.UnitsMode = "G21";
	this->mode.CutterRadiusCompensation = "G40";
	this->mode.ToolLengthOffset = "G49";
	this->mode.ProgramMode = "M0";
	this->mode.SpindleState = "M5";
	this->mode.CoolantState = "M9";

	this->mode.toolNumber = 0;
	this->mode.spindleSpeed = 0.0f;
	this->mode.feedRate = 0.0f;
	
	this->startupBlock[0] = "";
	this->startupBlock[1] = "";
}



gcList_t::gcList_t(){
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

void gcList_t::add(string str) {
	
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

static void bufferRemove(queue_t* q){
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

static int bufferAdd(queue_t* q, int len){
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
static void gcListSetResponse( gcList_t* gcList, int response) {
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
void grblRead(grblParams_t* grblParams, int fd, gcList_t* gcList, queue_t* q) {
	
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
		else if(!msg.compare(0, 1, ">")) {	
			// Startup Line Execution	">G54G20:ok" or ">G54G20:error:X"
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
					grblParams->mode.MotionMode = code;
				else if(code == "G54" || code == "G55" || code == "G56" || code == "G57" || code ==  "G58" || code == "G59")
					grblParams->mode.CoordinateSystem = code;
				else if(code == "G17" || code == "G18" || code == "G19")
					grblParams->mode.Plane = code;
				else if(code == "G90" || code == "G91")
					grblParams->mode.DistanceMode = code;
				else if(code == "G91.1")
					grblParams->mode.ArcIJKDistanceMode = code;
				else if(code == "G93" || code == "G94")
					grblParams->mode.FeedRateMode = code;
				else if(code == "G20" || code == "G21")
					grblParams->mode.UnitsMode = code;
				else if(code == "G40")
					grblParams->mode.CutterRadiusCompensation = code;
				else if(code == "G43.1" || code == "G49")
					grblParams->mode.ToolLengthOffset = code;
				else if(code == "M0" || code == "M1" || code == "M2" || code == "M30")
					grblParams->mode.ProgramMode = code;
				else if(code == "M3" || code == "M4" || code == "M5")
					grblParams->mode.SpindleState = code;
				else if(code == "M7" || code == "M8" || code == "M9")
					grblParams->mode.CoolantState = code;
				else if(code.compare(0, 1, "T"))
					grblParams->mode.toolNumber = stoi(code.substr(1, code.length()-1));
				else if(code.compare(0, 1, "S"))
					grblParams->mode.spindleSpeed = stof(code.substr(1, code.length()-1));
				else if(code.compare(0, 1, "F"))
					grblParams->mode.feedRate = stof(code.substr(1, code.length()-1));
				else
					cout << "Code unrecognised: " << code << endl;
			} while (stream);
		}
		else if(!msg.compare(0, 1, "[")) {
			cout << msg << endl;
		
			string s = msg.substr(5, msg.length()-6);
			// [G54:4.000,0.000,0.000] - [G59:4.000,0.000,0.000]
			if(!msg.compare(1, 3, "G54")) 
				grblParams->param.workCoords[0] = stoxyz(s);
			else if(!msg.compare(1, 3, "G55")) 
				grblParams->param.workCoords[1] = stoxyz(s);
			else if(!msg.compare(1, 3, "G56")) 
				grblParams->param.workCoords[2] = stoxyz(s);
			else if(!msg.compare(1, 3, "G57")) 
				grblParams->param.workCoords[3] = stoxyz(s);
			else if(!msg.compare(1, 3, "G58")) 
				grblParams->param.workCoords[4] = stoxyz(s);
			else if(!msg.compare(1, 3, "G59")) 
				grblParams->param.workCoords[5] = stoxyz(s);
			// [G28:1.000,2.000,0.000]  / [G30:4.000,6.000,0.000]
			else if(!msg.compare(1, 3, "G28")) 
				grblParams->param.homeCoords[0] = stoxyz(s);
			else if(!msg.compare(1, 3, "G30")) 
				grblParams->param.homeCoords[1] = stoxyz(s);
			// [G92:0.000,0.000,0.000]
			else if(!msg.compare(1, 3, "G92")) 
				grblParams->param.offsetCoords = stoxyz(s);
			// [TLO:0.000]	
			else if(!msg.compare(1, 3, "TLO")) 
				grblParams->param.toolLengthOffset = stof(s);
			// [PRB:0.000,0.000,0.000:0]
			else if(!msg.compare(1, 3, "PRB")) {
				grblParams->param.probeOffset = stoxyz(msg.substr(5, msg.length()-8));
				grblParams->param.probeSuccess = (bool)stoi(msg.substr(msg.length()-2, 1));
			}
			else
				cout << "Message unrecognised: " << msg << endl;				
		}
		else if(!msg.compare(0, 1, "<")) {	
			cout << msg << endl;
			

			// The $10 status report mask setting can alter what data is present and certain data fields can be reported intermittently (see descriptions for details.)
			// The $13 report inches settings alters the units of some data values. $13=0 false indicates mm-mode, while $13=1 true indicates inch-mode reporting.
			// "<Idle|WPos:828.000,319.000,49.100|FS:0,0|Pn:PXYZ>"

			//string msg = "<Idle|WPos:828.000,319.000,49.100|FS:0,0|Pn:PXYZ>"; 


			istringstream stream(msg.substr(1, msg.length()-2));
			string segment;
			vector<string> segs;
			
			while(getline(stream, segment, '|')) 
				segs.push_back(segment);
			
			// itterate backwards through segs[i]s
			for (int i = segs.size()-1; i >= 0; i--) {
				
				grblStatus_t* s = &(grblParams->status);
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
				
				
				
			}

					cout << "MPos x = " << grblParams->status.MPos.x << endl;
					cout << "MPos y = " << grblParams->status.MPos.y << endl;
					cout << "MPos z = " << grblParams->status.MPos.z << endl;
					cout << "WPos x = " << grblParams->status.WPos.x << endl;
					cout << "WPos y = " << grblParams->status.WPos.y << endl;
					cout << "WPos z = " << grblParams->status.WPos.z << endl;
					cout << "WCO x = " << grblParams->status.WCO.x << endl;
					cout << "WCO y = " << grblParams->status.WCO.y << endl;
					cout << "WCO z = " << grblParams->status.WCO.z << endl;
			
			
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
		else {			
			cout << "Unsupported message: " << msg << endl;
		}
/*
	< > : Enclosed chevrons contains status report data.
	Grbl X.Xx ['$' for help] : Welcome message indicates initialization.
	ALARM:x : Indicates an alarm has been thrown. Grbl is now in an alarm state.
	$x=val and $Nx=line indicate a settings printout from a $ and $N user query, respectively.
	[MSG:] : Indicates a non-queried feedback message.
	[GC:] : Indicates a queried $G g-code state message.
	[HLP:] : Indicates the help message.
	[G54:], [G55:], [G56:], [G57:], [G58:], [G59:], [G28:], [G30:], [G92:], [TLO:], and [PRB:] messages indicate the parameter data printout from a $# user query.
	[VER:] : Indicates build info and string from a $I user query.
	[echo:] : Indicates an automated line echo from a pre-parsed string prior to g-code parsing. Enabled by config.h option.
	>G54G20:ok : The open chevron indicates startup line execution. The :ok suffix shows it executed correctly without adding an unmatched ok response on a new line.
*/
		
	} while(1);	
}

void grblWrite(int fd, gcList_t* gcList, queue_t* q) {
	
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

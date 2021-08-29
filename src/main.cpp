
#include "common.h"

using namespace std;
            
/*            
    The below image shows a rough breakdown of the program stucture:
    
    The main class: 
    GRBL: This is the main class which acts as an interface between the gui and grbl itself. 
        It allows the gui to send commands to grbl and stores data recieved from it.
        Within this class there are three main sub-classes and three threads (see below) 
    
    There are three main sub-classes:
        GCode List: This is a list of all gcodes waiting to be sent to grbl, 
            and includes the status of each code before (unsent / sent) and after response from grbl (ok / error code)
        Serial: This contains all functions related to the writing/reading to the serial interface.
            This acts as a barrier between this program and the serial. It limits the number of characters 
            sent to grbl to 128 at any time.
        System: This stores all data recieved from grbl for access by the gui
    
    There are three threads which run in an infinate loop:
        Write Thread: This removes the next available gcode from the GCode list (when available) and 
            writes it to the serial interface (if there is space)
        Read Thread: This reads data from the serial interface (if available) and writes the status 
            into the GCode List and any data into system
        Status Request Thread: This sends realtime status requests at a regular interval to grbl, grbl then sends 
            back its current status which includes for example: state, position, feedrate etc.
        
    There are two types of commands:
        Standard: This uses the GCode list and character counting buffer to store, and regulate the data respectively.
            the reponse is then recieved from grbl and added back on to the GCode list
        Realtime: This bypasses all of the above and directly sends a command to grbl and will not recieve an 'ok' or 
            'error' response. This is used for things like status request, soft reset, overrides etc.
        
        
    For help with grbl, see: https://github.com/gnea/grbl/wiki/
 
             -------------------------      
            |  Status Request Thread  |
             -------------------------  
                         |     
                         |
                         v                                                           
              -----------------------                  
             |  Send Status Request  | -------------------------------------------------------------------
              -----------------------                                                                     |
                                                                                                          |
                      ---------------                                                                     |                     
            ------>  | Send Realtime |  -------------------------------------------------------------     |            
           |          ---------------                                                                |    |                                                                                               |     
           |                                                                                         v    v     
   ------------          ------------          ------------          -------------------          ------------          ------------    
  |            |  --->  |    Send    |  --->  |            |  --->  |   Write Thread    |  --->  |            |  --->  |            |       
  |    GRBL    |         ------------         | GCode List |         -------------------         |   Serial   |        |            |   
  |            |                              |            |                                     |            |        |    GRBL    |   
  |   Class    |         ------------         |   Class    |         -------------------         |   Class    |        |            |   
  |            |  --->  |  Send File |  --->  |            |  <---  |    Read Thread    |  <---  |            |  <---  |            |       
   ------------          ------------          ------------          -------------------          ------------          ------------    
           ^                                                              |                             
           |                        ------------                          |              
           |                       |    GRBL    |                         |
            ---------------------  |            |  <----------------------                      
                                   |   System   |  
                                    ------------                     
                                    

*/
                                                
/*     
 
TODO: 
     No way to have condition variable for serial recieve
        * Workaround: Use a timer
      How best to read items for clipper (reading Log::GetConsoleLog & getGCItem)
         * Workaround: Use a mutex for size, and then another mutex for each element of data
    Scroll to bottom isnt working with always horizontal scroll
       * Workaround: use word wrapping, but this causes problems with scrolling to bottom so had to bodge that a bit

    
    Scrolling the log...
    
    GCReader has a few things to do - e.g. coord systems
    check everything (e.g. g28) works with offset coord
    
    FileDialog - 
        File::OpenFileDialog();
    
    Combine cleanString() from gclist and gcviewer
    
    Sort tabs / spacs...
    
    Positioning of frames on first open
    
    Jogging
         lots of jogs can crash grbl
         combine buttons/keyboards/joystick
    
    Overrides should be sliders
    
    Dont allow jogs to be sent when inside command input box
    
    
Other notes:
    * if we need to sync gui to grbl, use G4 P0.01

    EEPROM Issues
    EEPROM access on the Arduino AVR CPUs turns off all of the interrupts while the CPU writes to EEPROM. This poses a problem for certain features in Grbl, particularly if a user is streaming and running a g-code program, since it can pause the main step generator interrupt from executing on time. Most of the EEPROM access is restricted by Grbl when it's in certain states, but there are some things that developers need to know.

    Settings should not be streamed with the character-counting streaming protocols. Only the simple send-response protocol works. This is because during the EEPROM write, the AVR CPU also shuts-down the serial RX interrupt, which means data can get corrupted or lost. This is safe with the send-response protocol, because it's not sending data after commanding Grbl to save data.
    For reference:

    Grbl's EEPROM write commands: G10 L2, G10 L20, G28.1, G30.1, $x=, $I=, $Nx=, $RST=
    Grbl's EEPROM read commands: G54-G59, G28, G30, $$, $I, $N, $#
*/
    




/*
#define JOYSTICK_X         ADS1115_PIN_A0
#define JOYSTICK_Y         ADS1115_PIN_A1
#define JOYSTICK_BUTTON     ADS1115_PIN_A2
#define JOYSTICK_INVERT     point2D(1, -1)
#define JOYSTICK_VMAX         3.3f    // volts
#define JOYSTICK_VTRIG         3.0f    // voltage above this is high


void readJoystick(ADS1115& adc, point2D& return_point, bool& return_press) 
{    // read joystick voltage
    point2D p = point2D(adc.Read(JOYSTICK_X), adc.Read(JOYSTICK_Y));
    // normalise to -1 to 1, and translate from (0 to 2) to (-1 to 1)
    p = (p * 2 / JOYSTICK_VMAX) - 1.0f;
    // invert y axis
    return_point = p * JOYSTICK_INVERT;
    return_press = (adc.Read(JOYSTICK_BUTTON) > JOYSTICK_VTRIG) ? true : false;
}

void joystickTest()
{
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
}
*/

int main()
{
    
    // initialise WiringPi
    if(wiringPiSetup() == -1)
        Log::Critical("Could not start wiringPi: %s", strerror(errno));
 
    // create GRBL
    GRBL grbl;
    // start gui loop
    gui(grbl);
    
    return 0;
}



// all available sends

// KILL ALARM LOCK
//Grbl.Send("$X");

// GRBL SETTINGS
//Grbl.Send("$$");

// GCODE PARAMETERS (coord systems)
//Grbl.Send("$#");

// MODAL STATES
//Grbl.Send("$G");

// BUILD INFO
//Grbl.Send("$I");

// STARTUP BLOCKS
//Grbl.Send("$N");


// CHECK MODE (send again to cancel)
//Grbl.Send("$C");

// RUN HOMING CYCLE
//Grbl.Send("$H");

// JOG
//Grbl.Send("$J=G91 X10 F1000");
//Grbl.SendRT(GRBL_RT_JOG_CANCEL);

// RESET SETTINGS ( USE WITH CATION ) 
// "$RST=$", "$RST=#", and "$RST=*"

// SLEEP
//Grbl.Send("$SLP");

// Coordinate systems
// Settings should not be streamed with the character-counting streaming protocols. Only the simple send-response protocol works. This is because during the EEPROM write, the AVR CPU also shuts-down the serial RX interrupt, which means data can get corrupted or lost. This is safe with the send-response protocol, because it's not sending data after commanding Grbl to save data.
//  G10 L2, G10 L20, G28.1, G30.1

/* PROBE 
Grbl.Send("G91 G38.2 Z-200 F100\n");
Grbl.Send("G91 G38.4 Z1 F100\n");
*/
/*
string file = "/home/pi/Desktop/New.nc";
Grbl.FileRun(file);
*/


/* see: https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands
 
    $$ and $x=val - View and write Grbl settings
    
    
    $# - View gcode parameters
        [G54:4.000,0.000,0.000]        work coords                can be changed with     G10 L2 Px or G10 L20 Px
        [G55:4.000,6.000,7.000]
        [G56:0.000,0.000,0.000]
        [G57:0.000,0.000,0.000]
        [G58:0.000,0.000,0.000]
        [G59:0.000,0.000,0.000]
        [G28:1.000,2.000,0.000]        pre-defined positions     can be changed with     G28.1
        [G30:4.000,6.000,0.000]                                                         G30.1
        [G92:0.000,0.000,0.000]        coordinate offset 
        [TLO:0.000]                    tool length offsets
        [PRB:0.000,0.000,0.000:0]    probing


    $G - View gcode parser state
        [GC:G0 G54 G17 G21 G90 G94 M0 M5 M9 T0 S0.0 F500.0]

                                                        
            Modal Group                    Member Words    *default
        Motion Mode                    *G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
        Coordinate System Select    *G54, G55, G56, G57, G58, G59
        Plane Select                *G17, G18, G19
        Distance Mode                *G90, G91
        Arc IJK Distance Mode        *G91.1
        Feed Rate Mode                G93, *G94
        Units Mode                    G20, *G21
        Cutter Radius Compensation    *G40
        Tool Length Offset            G43.1, *G49
        Program Mode                *M0, M1, M2, M30
        Spindle State                M3, M4, *M5
        Coolant State                M7, M8, *M9

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
            G90 or G91 - MotionType::Absolute and incremental distances
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

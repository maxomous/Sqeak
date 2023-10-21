#pragma once
#include <MaxLib.h>


// *********************** //
//       Debug Flags       //
// *********************** //       
#define DEBUG_NONE                                  0x0
#define DEBUG_GCLIST_BUILD                          MaxLib::Log::NextDebugBit()
#define DEBUG_CHAR_COUNTING                         MaxLib::Log::NextDebugBit()
#define DEBUG_GCLIST                                MaxLib::Log::NextDebugBit()
#define DEBUG_SERIAL                                MaxLib::Log::NextDebugBit()
#define DEBUG_THREAD_BLOCKING                       MaxLib::Log::NextDebugBit()
#define DEBUG_GCREADER                              MaxLib::Log::NextDebugBit()

// *********************** //
//       GRBL defines      //
// *********************** //
    
#define MAX_GRBL_BUFFER                             128
#define MAX_GRBL_RECEIVE_BUFFER                     128

// used for signalling to threads to stop execution
#define GRBL_CMD_RUN                                0
#define GRBL_CMD_SHUTDOWN                           1
#define GRBL_CMD_RESET                              2
#define GRBL_CMD_CANCEL                             3
                
#define STATUS_MSG                                 -3   // resonse is message, just continue reading serial
#define STATUS_UNSENT                              -2   // not sent yet to grbl
#define STATUS_SENT                                -1   // sent to grbl but no status received
#define STATUS_OK                                   0   // 'ok' received by grbl
                                                    // positive numbers represent errors recieved from grbl

#define GRBL_STATE_COLOUR_IDLE                      0
#define GRBL_STATE_COLOUR_MOTION                    1
#define GRBL_STATE_COLOUR_ALERT                     2

// REALTIME COMMANDS
#define GRBL_RT_SOFT_RESET                          (char)0x18
#define GRBL_RT_STATUS_QUERY                        (char)'?'
#define GRBL_RT_RESUME                              (char)'~'
#define GRBL_RT_HOLD                                (char)'!'
                                                    
#define GRBL_RT_DOOR                                (char)0x84
#define GRBL_RT_JOG_CANCEL                          (char)0x85
                                                    
#define GRBL_RT_OVERRIDE_FEED_100PERCENT            (char)0x90
#define GRBL_RT_OVERRIDE_FEED_ADD_10PERCENT         (char)0x91
#define GRBL_RT_OVERRIDE_FEED_MINUS_10PERCENT       (char)0x92
#define GRBL_RT_OVERRIDE_FEED_ADD_1PERCENT          (char)0x93
#define GRBL_RT_OVERRIDE_FEED_MINUS_1PERCENT        (char)0x94
                                                    
#define GRBL_RT_OVERRIDE_RAPIDFEED_100PERCENT       (char)0x95
#define GRBL_RT_OVERRIDE_RAPIDFEED_50PERCENT        (char)0x96
#define GRBL_RT_OVERRIDE_RAPIDFEED_25PERCENT        (char)0x97
                                                    
#define GRBL_RT_OVERRIDE_SPINDLE_100PERCENT         (char)0x99
#define GRBL_RT_OVERRIDE_SPINDLE_ADD_10PERCENT      (char)0x9A
#define GRBL_RT_OVERRIDE_SPINDLE_MINUS_10PERCENT    (char)0x9B
#define GRBL_RT_OVERRIDE_SPINDLE_ADD_1PERCENT       (char)0x9C
#define GRBL_RT_OVERRIDE_SPINDLE_MINUS_1PERCENT     (char)0x9D

#define GRBL_RT_SPINDLE_STOP                        (char)0x9E
#define GRBL_RT_FLOOD_COOLANT                       (char)0xA0
#define GRBL_RT_MIST_COOLANT                        (char)0xA1

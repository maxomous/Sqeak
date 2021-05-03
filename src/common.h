/*
 * common.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#pragma once

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>
#include <bitset>
#include <algorithm>
#include <assert.h>
#include <functional>
#include <vector>
#include <queue>
#include <optional>
// threads
#include <thread>
#include <pthread.h> // for priority
#include <mutex>
#include <condition_variable>
#include <atomic>
// wiring pi
#include <wiringPi.h>
#include <wiringSerial.h>


// gui
#define IMGUI_DEFINE_MATH_OPERATORS 
#include "gui/imgui/imgui.h"
#include "gui/imgui/imgui_internal.h"
#include "gui/imgui/imgui_impl_glfw.h"
#include "gui/imgui/imgui_impl_opengl3.h"
#include "gui/imgui/imgui_stdlib.h"	// to use string
#include "gui/imgui/imgui_memory_editor.h"	// to use string

// for loading images
#include "gui/stb_image/imgui_stb_image.h"	// wrapper to use stb_image with ImGui buttons
// fonts
#include "gui/fonts/font_geomanist.h"


#include <GL/glew.h>
// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#include "dev/ads1115.h"
#include "libs/geom.h"
#include "libs/file.h"

#include "serial.h"
#include "gclist.h"

#include "grblcodes.h"
#include "grbl.h"

#include "gui/gui.h"
#include "gui/frames.h"


// *********************** //
//       GRBL defines      //
// *********************** //

// used for signalling to threads to stop execution
#define GRBL_CMD_RUN 			 0
#define GRBL_CMD_SHUTDOWN 		 1
#define GRBL_CMD_RESET 			 2
#define GRBL_CMD_CANCEL			 3

#define STATUS_MSG			-3	// resonse is message, just continue reading serial
#define STATUS_UNSENT			-2	// not sent yet to grbl
#define STATUS_SENT 			-1	// sent to grbl but no status received
#define STATUS_OK			 0	// 'ok' received by grbl
// positive numbers represent errors recieved from grbl
 
#define GRBL_STATE_COLOUR_IDLE 		 0
#define GRBL_STATE_COLOUR_MOTION 	 1
#define GRBL_STATE_COLOUR_ALERT 	 2

// REALTIME COMMANDS
#define GRBL_RT_SOFT_RESET 				(char)0x18
#define GRBL_RT_STATUS_QUERY 				(char)'?'
#define GRBL_RT_RESUME					(char)'~'
#define GRBL_RT_HOLD					(char)'!'

#define GRBL_RT_DOOR					(char)0x84
#define GRBL_RT_JOG_CANCEL				(char)0x85

#define GRBL_RT_OVERRIDE_FEED_100PERCENT		(char)0x90
#define GRBL_RT_OVERRIDE_FEED_ADD_10PERCENT		(char)0x91
#define GRBL_RT_OVERRIDE_FEED_MINUS_10PERCENT		(char)0x92
#define GRBL_RT_OVERRIDE_FEED_ADD_1PERCENT		(char)0x93
#define GRBL_RT_OVERRIDE_FEED_MINUS_1PERCENT		(char)0x94

#define GRBL_RT_OVERRIDE_RAPIDFEED_100PERCENT		(char)0x95
#define GRBL_RT_OVERRIDE_RAPIDFEED_50PERCENT		(char)0x96
#define GRBL_RT_OVERRIDE_RAPIDFEED_25PERCENT		(char)0x97

#define GRBL_RT_OVERRIDE_SPINDLE_100PERCENT		(char)0x99
#define GRBL_RT_OVERRIDE_SPINDLE_ADD_10PERCENT		(char)0x9A
#define GRBL_RT_OVERRIDE_SPINDLE_MINUS_10PERCENT	(char)0x9B
#define GRBL_RT_OVERRIDE_SPINDLE_ADD_1PERCENT		(char)0x9C
#define GRBL_RT_OVERRIDE_SPINDLE_MINUS_1PERCENT		(char)0x9D

#define GRBL_RT_SPINDLE_STOP				(char)0x9E
#define GRBL_RT_FLOOD_COOLANT				(char)0xA0
#define GRBL_RT_MIST_COOLANT				(char)0xA1


// *********************** //
// General constants //
// *********************** //
#define MAX_STRING 			255 

#define CLOCKWISE			1
#define ANTICLOCKWISE		       -1

#define FORWARD				1
#define BACKWARD		       -1

#define X_AXIS				1
#define Y_AXIS				2
#define Z_AXIS				3


// debug flags
#define DEBUG_NONE			0x0
#define DEBUG_GCLIST_BUILD		0x1 << 0
#define DEBUG_CHAR_COUNTING		0x1 << 1
#define DEBUG_GCLIST			0x1 << 2
#define DEBUG_SERIAL			0x1 << 3
#define DEBUG_THREAD_BLOCKING		0x1 << 4

// converts variable arguments to a string
std::string va_str(const char* format, ... );
// modifies string to lower case
extern void lowerCase(std::string& str);
// modifies string to upper case
extern void upperCase(std::string& str);
// convert seconds into hours, minutes and seconds
extern void normaliseSecs(uint s, uint& hr, uint& min, uint& sec);

class Log {  
public:
    enum LogLevel{
	LevelInfo, LevelResponse, LevelWarning, LevelError, LevelCritical, LevelDebug
    };
    static void SetLevelTerminal(LogLevel level) { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	get().m_logLevelTerminal = level; 
    }
    static void SetLevelConsole(LogLevel level) { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	get().m_logLevelConsole = level; 
    }
    static void ClearConsoleLog() { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	get().m_consoleLog.clear();
    }
    static const std::string& GetConsoleLog(size_t index) { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	return get().m_consoleLog[index]; 
    } 
    static std::optional<const std::string> GetConsoleLogLast() { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	if(get().m_consoleLog.size() == 0)
	    return {}; 
	return get().m_consoleLog.back(); 
    } 
    static size_t GetConsoleLogSize() { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	return get().m_consoleLog.size(); 
    }
    static void SetDebugFlags(int flags) { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	get().m_debugFlags = flags; 
    }
    static void Debug(int flag, const std::string& msg)			{ get().Print(flag, LevelDebug, msg.c_str()); }
    static void Critical(const std::string& msg) 			{ get().Print(LevelCritical, 	msg.c_str()); }
    static void Error(const std::string& msg) 				{ get().Print(LevelError, 	msg.c_str()); }
    static void Warning(const std::string& msg) 			{ get().Print(LevelWarning,  	msg.c_str()); }
    static void Response(const std::string& msg) 			{ get().Print(LevelResponse, 	msg.c_str()); }
    static void Info(const std::string& msg)				{ get().Print(LevelInfo, 	msg.c_str()); }

    template <typename ... Args>
    static void Debug	(int flag, const char* msg, Args... args)	{ get().Print(flag, LevelDebug, msg, args...); }
    template <typename ... Args>
    static void Critical(const char* msg, Args... args)			{ get().Print(LevelCritical, 	msg, args...); }
    template <typename ... Args>
    static void Error	(const char* msg, Args... args) 		{ get().Print(LevelError, 	msg, args...); }
    template <typename ... Args>
    static void Warning	(const char* msg, Args... args) 		{ get().Print(LevelWarning, 	msg, args...); }
    template <typename ... Args>
    static void Response(const char* msg, Args... args)			{ get().Print(LevelResponse, 	msg, args...); }
    template <typename ... Args>
    static void Info	(const char* msg, Args... args)			{ get().Print(LevelInfo, 	msg, args...); }
    
private:
    std::vector<std::string> m_consoleLog;
    LogLevel m_logLevelTerminal = LevelInfo; // default show all
    LogLevel m_logLevelConsole = LevelInfo; // default show all
    int m_debugFlags = false; // default show none
    std::mutex m_mutex;

    bool PrintDebug(int flag) {
	std::lock_guard<std::mutex> guard(m_mutex);
	return m_debugFlags & flag;
    }
    template <typename ... Args>
    void Print(int debugFlag, LogLevel level, const char* msg, Args... args) 
    {
	if(!PrintDebug(debugFlag))
	    return;
	Print(level, msg, args...);
    }
    
    template <typename ... Args>
    void Print(LogLevel level, const char* msg, Args... args) 
    {
	// lock the mutex
        std::lock_guard<std::mutex> guard(m_mutex);

	PrintToTerminal(level, msg, args...);
	PrintToConsole(level, msg, args...);
	// stop program execution
	if(level == LevelCritical)
	    exit(1);
    }

    std::string levelPrefix(LogLevel level) 
    {		
	if(level == LevelInfo)
	    return "[Info] ";
	else if(level == LevelResponse)
	    return "";
	else if(level == LevelWarning)
	    return "[Warning] ";
	else if(level == LevelError)
	    return "[Error] ";
	else if(level == LevelCritical)
	    return "[Critical] ";
	else if(level == LevelDebug)
	    return "[Debug] ";
	return "";
    }

    template <typename ... Args>
    void PrintToTerminal(LogLevel level, const char* msg, Args... args)
    { 
	if(m_logLevelTerminal <= level) {
	    char dateStr[32];
	    time_t t = time(NULL);
	    strftime(dateStr, 32, "%H:%M:%S", localtime(&t));
	    printf("[%s]",dateStr);
	    printf(levelPrefix(level).c_str());
	    printf(msg, args...);
	    printf("\n"); 
	}
    }

    template <typename ... Args>
    void PrintToConsole(LogLevel level, const char* msg, Args... args) 
    { 
	if(m_logLevelConsole <= level) {
	    std::string str = levelPrefix(level);
	    str += va_str(msg, args...);
	    m_consoleLog.emplace_back(move(str)); 
	}
    }

    static Log& get() {
	static Log log;
	return log;
    }

    Log() {}
    Log(const Log&) = delete;
    Log& operator= (const Log&) = delete;
};

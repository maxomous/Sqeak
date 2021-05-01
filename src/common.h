/*
 * common.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#pragma once

//#define DEBUG
//#define DEBUG_SERIAL

 

// pre compiled headers
//#include "pch.hpp"
#include <iostream>
#include <time.h>
#include <bitset>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string>
#include <string.h>
#include <string_view>
#include <sstream>
#include <algorithm>
#include <ctype.h>
#include <assert.h>
#include <vector>
#include <functional>
// threads
#include <thread>
#include <pthread.h> // for priority
#include <mutex>
#include <condition_variable>
#include <queue>
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
// GRBL specific constants //
// *********************** //

// used for signalling to threads what to do
#define GRBL_CMD_RUN 			0
#define GRBL_CMD_SHUTDOWN 		1
#define GRBL_CMD_RESET 			2

#define STATUS_MSG			-3	// resonse is message, just continue reading serial
#define STATUS_UNSENT			-2	// not sent yet to grbl
#define STATUS_PENDING 			-1	// sent to grbl but no status received
#define STATUS_OK			0	// 'ok' received by grbl
// ERROR STATUS NOW MATCHES GRBL's 	#define STATUS_ERROR		3	// 'error' received by grbl

#define GRBL_STATE_COLOUR_IDLE 		0
#define GRBL_STATE_COLOUR_MOTION 	1
#define GRBL_STATE_COLOUR_ALERT 	2

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
#define MAX_STRING 		255 

#define CLOCKWISE		1
#define ANTICLOCKWISE		-1

#define FORWARD			1
#define BACKWARD		-1

#define X_AXIS			1
#define Y_AXIS			2
#define Z_AXIS			3



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
	LevelInfo, LevelResponse, LevelWarning, LevelError, LevelCritical
    };
    static void SetLevelTerminal(LogLevel level) { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	get().logLevelTerminal = level; 
    }
    static void SetLevelConsole(LogLevel level) { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	get().logLevelConsole = level; 
    }
    static void ClearConsoleLog() { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	get().consoleLog.clear();
    }
    static const std::string& GetConsoleLog(size_t index) { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	return get().consoleLog[index]; 
    } 
    static size_t GetConsoleLogSize() { 
	// lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
	return get().consoleLog.size(); 
    }
    static void Critical(const std::string& msg) 		{ get().Print(LevelCritical, 	msg.c_str()); }
    static void Error(const std::string& msg) 			{ get().Print(LevelError, 	msg.c_str()); }
    static void Warning(const std::string& msg) 		{ get().Print(LevelWarning,  	msg.c_str()); }
    static void Response(const std::string& msg) 		{ get().Print(LevelResponse, 	msg.c_str()); }
    static void Info(const std::string& msg)			{ get().Print(LevelInfo, 	msg.c_str()); }

    template <typename ... Args>
    static void Critical(const char* msg, Args... args)		{ get().Print(LevelCritical, 	msg, args...); }
    template <typename ... Args>
    static void Error	(const char* msg, Args... args) 	{ get().Print(LevelError, 	msg, args...); }
    template <typename ... Args>
    static void Warning	(const char* msg, Args... args) 	{ get().Print(LevelWarning, 	msg, args...); }
    template <typename ... Args>
    static void Response(const char* msg, Args... args)		{ get().Print(LevelResponse, 	msg, args...); }
    template <typename ... Args>
    static void Info	(const char* msg, Args... args)		{ get().Print(LevelInfo, 	msg, args...); }
    
private:
    std::vector<std::string> consoleLog;
    LogLevel logLevelTerminal = LevelInfo; // default show all
    LogLevel logLevelConsole = LevelInfo; // default show all
    std::mutex m_mutex;

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
	return "";
    }

    template <typename ... Args>
    void PrintToTerminal(LogLevel level, const char* msg, Args... args)
    { 
	if(logLevelTerminal <= level) {
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
	if(logLevelConsole <= level) {
	    std::string str = levelPrefix(level);
	    str += va_str(msg, args...);
	    consoleLog.emplace_back(move(str)); 
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

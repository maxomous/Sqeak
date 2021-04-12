/*
 * common.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#pragma once

//#define DEBUG_MEMORY_ALLOC

 

// pre compiled headers
//#include "pch.hpp"
#include <iostream>
#include <time.h>
#include <bitset>
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

#include <wiringPi.h>
#include <wiringSerial.h>

// gui
#define IMGUI_DEFINE_MATH_OPERATORS 
#include "gui/imgui/imgui.h"
#include "gui/imgui/imgui_internal.h"
#include "gui/imgui/imgui_impl_glfw.h"
#include "gui/imgui/imgui_impl_opengl3.h"
#include "gui/imgui/imgui_stdlib.h"	// to use string
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
#include "grbl.h"
#include "grblcodes.h"
#include "gui/gui.h"
#include "gui/frames.h"



#define ERR_NONE		0
#define ERR_FAIL		1

#define MAX_STRING 		255 // general maximum string length 

#define CLOCKWISE		1
#define ANTICLOCKWISE	-1

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

extern std::string lowerCase(const std::string& str);

// convert seconds into hours, minutes and seconds
extern void normaliseSecs(uint s, uint& hr, uint& min, uint& sec);


 
class Log {  
public:
    enum LogLevel{
		LevelInfo, LevelResponse, LevelWarning, LevelError, LevelCritical
    };
    static void SetLevelTerminal(LogLevel level) 			{ get().logLevelTerminal = level; }
    static void SetLevelConsole(LogLevel level) 			{ get().logLevelConsole = level; }
	static void ClearConsoleLog() 							{ get().consoleLog.clear(); }
    static const std::vector<std::string>& GetConsoleLog() 	{ return get().consoleLog; }	
   /* // append to the last 
	static void AppendToLast(const std::string& str)		{ get().consoleLog.back().append(str); }
						consoleLog->back().append(setting);
						Log::AppendToLast(setting);
						*/
    static void Critical(const std::string& msg) 	{ get().Print(LevelCritical, msg.c_str()); }
    static void Error(const std::string& msg) 		{ get().Print(LevelError, 	 msg.c_str()); }
    static void Warning(const std::string& msg) 	{ get().Print(LevelWarning,  msg.c_str()); }
    static void Response(const std::string& msg) 	{ get().Print(LevelResponse, msg.c_str()); }
    static void Info(const std::string& msg)		{ get().Print(LevelInfo, 	 msg.c_str()); }
    
	template <typename ... Args>
    static void Critical(const char* msg, Args... args)		{ get().Print(LevelCritical, 	msg, args...); }
	template <typename ... Args>
    static void Error	(const char* msg, Args... args) 	{ get().Print(LevelError, 		msg, args...); }
	template <typename ... Args>
	static void Warning	(const char* msg, Args... args) 	{ get().Print(LevelWarning, 	msg, args...); }
	template <typename ... Args>
    static void Response(const char* msg, Args... args)		{ get().Print(LevelResponse, 	msg, args...); }
	template <typename ... Args>
    static void Info	(const char* msg, Args... args)		{ get().Print(LevelInfo, 		msg, args...); }
    
private:
	std::vector<std::string> consoleLog;
	LogLevel logLevelTerminal = LevelInfo; // default show all
	LogLevel logLevelConsole = LevelInfo; // default show all
	
	template <typename ... Args>
    void Print(LogLevel level, const char* msg, Args... args) 
    {
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
			consoleLog.emplace_back(str); 
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

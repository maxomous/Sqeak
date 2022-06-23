
#pragma once

#include <algorithm>
#include <assert.h>
#include <bitset>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <variant>
#include <memory>
// threads
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <pthread.h>
#include <thread>
// wiring pi
#include <wiringPi.h>
#include <wiringSerial.h>
// OpenGL / ImGui
#include "glcore/glcore.h"

#include "libs/geos.h"

// allows bitwise operations on enums
template<class T> inline T operator~ (T a) { return (T)~(int)a; }
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<class T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }


// *********************** //
//       Events            //
// *********************** //


struct InputEvent {
    Event_KeyInput* keyboard = nullptr;
    Event_MouseButton* mouseClick = nullptr;
    Event_MouseMove* mouseMove = nullptr;
};


#include <MaxLib.h>

#include "dev/joystick.h"

#include "gclist.h"
#include "serial.h"

#include "grbl.h"
#include "grblcodes.h"

#include "settings.h"

#include "gcreader.h"


#include "gui/imgui_custommodules.h"

#include "gui/filebrowser.h"
#include "gui/viewer.h"
#include "sketch/gcodebuilder.h"
#include "sketch/toolsettings.h"
#include "sketch/sketch.h"

#include "sketcher/sketch.h"

#include "gui/frames.h"
#include "gui/gui.h"



namespace Sqeak {
    
struct Event_Update3DModelFromFile      { std::string filename; };
struct Event_Update3DModelFromVector    { std::vector<std::string> gcodes; };
struct Event_Viewer_AddLineLists        { std::vector<DynamicBuffer::DynamicVertexList>* dynamicLineLists; };
struct Event_Viewer_AddPointLists       { std::vector<DynamicBuffer::DynamicVertexList>* dynamicPointLists; };

struct Event_PopupMessage               { std::string msg; };
struct Event_ResetFileTimer             {};
struct Event_ConsoleScrollToBottom      {};
struct Event_SaveSettings               {};
struct Event_UpdateSettingsFromFile     {};
struct Event_Set2DMode                  { bool isTrue; };
struct Event_Set2DCursor                { bool isValid; glm::vec2 worldCoords; };
struct Event_GetCursorWorldCoords       { bool& isValid; glm::vec3& returnCoords; };


#define GUI_WINDOW_NAME     "Sqeak"
#define GUI_WINDOW_W        1280
#define GUI_WINDOW_H        720
#define GUI_WINDOW_WMIN     200
#define GUI_WINDOW_HMIN     200

#define CONFIG_FILE         "config.ini"        // both created in the directory of the executable
#define GUI_CONFIG_FILE     "uiconfig.ini"

#define GUI_IMG_ICON        "/img/icon.png"



// *********************** //
//       GRBL defines      //
// *********************** //
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

// *********************** //
// General constants //
// *********************** //
#define MAX_STRING                                  255
                    
#define FORWARD                                     1
#define BACKWARD                                   -1
                    
#define X_AXIS                                      1
#define Y_AXIS                                      2
#define Z_AXIS                                      3
                    
// debug flags                     
#define DEBUG_NONE                                  0x0
#define DEBUG_GCLIST_BUILD                          0x1 << 0
#define DEBUG_CHAR_COUNTING                         0x1 << 1
#define DEBUG_GCLIST                                0x1 << 2
#define DEBUG_SERIAL                                0x1 << 3
#define DEBUG_THREAD_BLOCKING                       0x1 << 4
#define DEBUG_GCREADER                              0x1 << 5
#define DEBUG_SKETCH_REFERENCES                     0x1 << 6


// Normalises seconds into hours, minutes & seconds
class Time {
public:
    Time(uint seconds);
    uint Hours() { return m_hr; }
    uint Mins() { return m_min; }
    uint Secs() { return m_sec; }
    std::string TimeString() { return MaxLib::String::va_str("%u:%.2u:%.2u", m_hr, m_min, m_sec); }
private:
    uint m_hr;
    uint m_min;
    uint m_sec;
};
 // This takes a std::string of 3 values seperated by commas (,) and will return a 3DPoint
// 4.000,0.000,0.000
glm::vec2 stoVec2(const std::string& msg);
// This takes a std::string of 3 values seperated by commas (,) and will return a 3DPoint
// 4.000,0.000,0.000
glm::vec3 stoVec3(const std::string& msg);



// returns value of input and switches input to false if true 
bool trigger(bool& input);

class Log {
public:
    enum LogLevel {
        LevelInfo,
        LevelResponse,
        LevelWarning,
        LevelError,
        LevelCritical,
        LevelDebug
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
    static const std::string &GetConsoleLog(size_t index) {
        // lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
        return get().m_consoleLog[index];
    }
    static std::optional<const std::string> GetConsoleLogLast() {
        // lock the mutex
        std::lock_guard<std::mutex> guard(get().m_mutex);
        if (get().m_consoleLog.size() == 0)
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
    static void Debug(int flag, const std::string &msg) { get().Print(flag, LevelDebug, msg.c_str()); }
    static void Critical(const std::string &msg) { get().Print(LevelCritical, msg.c_str()); }
    static void Error(const std::string &msg) { get().Print(LevelError, msg.c_str()); }
    static void Warning(const std::string &msg) { get().Print(LevelWarning, msg.c_str()); }
    static void Response(const std::string &msg) { get().Print(LevelResponse, msg.c_str()); }
    static void Info(const std::string &msg) { get().Print(LevelInfo, msg.c_str()); }

    template <typename... Args>
    static void Debug(int flag, const char *msg, Args... args) { get().Print(flag, LevelDebug, msg, args...); }
    template <typename... Args>
    static void Critical(const char *msg, Args... args) { get().Print(LevelCritical, msg, args...); }
    template <typename... Args>
    static void Error(const char *msg, Args... args) { get().Print(LevelError, msg, args...); }
    template <typename... Args>
    static void Warning(const char *msg, Args... args) { get().Print(LevelWarning, msg, args...); }
    template <typename... Args>
    static void Response(const char *msg, Args... args) { get().Print(LevelResponse, msg, args...); }
    template <typename... Args>
    static void Info(const char *msg, Args... args) { get().Print(LevelInfo, msg, args...); }

private:
    std::vector<std::string> m_consoleLog;
    LogLevel m_logLevelTerminal = LevelInfo; // default show all
    LogLevel m_logLevelConsole = LevelInfo;  // default show all
    int m_debugFlags = false;                // default show none
    std::mutex m_mutex;

    bool PrintDebug(int flag) {
        std::lock_guard<std::mutex> guard(m_mutex);
        return m_debugFlags & flag;
    }
    template <typename... Args>
    void Print(int debugFlag, LogLevel level, const char *msg, Args... args) {
        if (!PrintDebug(debugFlag))
            return;
        Print(level, msg, args...);
    }

    template <typename... Args>
    void Print(LogLevel level, const char *msg, Args... args) {
        // lock the mutex
        std::lock_guard<std::mutex> guard(m_mutex);

        PrintToTerminal(level, msg, args...);
        PrintToGUI(level, msg, args...);
        // stop program execution
        if (level == LevelCritical)
            exit(1);
    }

    std::string levelPrefix(LogLevel level) {
        if (level == LevelInfo)
            return "[Info] ";
        else if (level == LevelResponse)
            return "";
        else if (level == LevelWarning)
            return "[Warning] ";
        else if (level == LevelError)
            return "[Error] ";
        else if (level == LevelCritical)
            return "[Critical] ";
        else if (level == LevelDebug)
            return "[Debug] ";
        return "";
    }

    template <typename... Args>
    void PrintToTerminal(LogLevel level, const char *msg, Args... args) {
        if (m_logLevelTerminal <= level) {
            char dateStr[32];
            time_t t = time(NULL);
            strftime(dateStr, 32, "%H:%M:%S", localtime(&t));
            printf("[%s]", dateStr);
            printf(levelPrefix(level).c_str());
            printf(msg, args...);
            printf("\n");
        }
    }

    template <typename... Args>
    void PrintToGUI(LogLevel level, const char *msg, Args... args) {
        if (m_logLevelConsole <= level) {
            std::string str = levelPrefix(level);
            str += MaxLib::String::va_str(msg, args...);
            // print to console
            m_consoleLog.emplace_back(str);
            Event<Event_ConsoleScrollToBottom>::Dispatch({});
            // print to message popup
            Event<Event_PopupMessage>::Dispatch({ str });
        }
    }
    
    static Log &get() {
        static Log log;
        return log;
    }

    Log() {}                                // delete the constructor
    Log(const Log &) = delete;              // delete the copy constructor
    Log &operator=(const Log &) = delete;   // delete the copy assignment operatory
};

} // end namespace Sqeak

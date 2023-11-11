#pragma once

#include <algorithm>
#include <assert.h>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <bitset>
// threads
//#include <atomic>
//#include <condition_variable>
//#include <mutex>
//#include <pthread.h>
//#include <thread>
//// wiring pi
//#include <wiringPi.h>
//#include <wiringSerial.h>
#include <MaxLib.h>
// OpenGL / ImGui
#include "glcore/glcore.h"

//#include "libs/geos.h" // old c version
#include "libs/Geos.h"
#include "dev/joystick.h"
#include "grbl/grbl.h"




// contains a list of points/lines + a colour
struct ColouredVertexList {
    ColouredVertexList() {}
    ColouredVertexList(const glm::vec3& col) : colour(col) {}
    std::vector<glm::vec3> position; 
    glm::vec3 colour; 
};


namespace Sqeak {

struct InputEvent {
    Event_KeyInput* keyboard = nullptr;
    Event_MouseButton* mouseClick = nullptr;
    Event_MouseMove* mouseMove = nullptr;
};

struct Event_Get2DMode                  { bool& is2DMode; };
struct Event_Set2DMode                  { bool isTrue; };
struct Event_Set2DCursor                { bool isValid; glm::vec2 worldCoords; };
struct Event_GetCursorWorldCoords       { bool& isValid; glm::vec3& returnCoords; };

struct Event_Update3DModelFromFile      { std::string filename; };
struct Event_Update3DModelFromVector    { std::vector<std::string> gcodes; };
struct Event_Viewer_AddLineLists        { std::vector<ColouredVertexList>* dynamicLineLists; };
struct Event_Viewer_AddPointLists       { std::vector<ColouredVertexList>* dynamicPointLists; };
    
struct Event_PopupMessage               { std::string msg; };
struct Event_ResetFileTimer             {};
struct Event_SaveSettings               {};
struct Event_UpdateSettingsFromFile     {};

}






#include "sketch/toolsettings.h"
#include "settings.h"

#include "gcreader.h"


#include "gui/imgui_custommodules.h"


#include "gui/viewer.h"



#include "sketch/sketch.h"

#include "sketcher/sketch.h"

#include "functions/functions.h"


#include "gui/toolbar.h"
#include "gui/frames.h"
#include "gui/gui.h"



namespace Sqeak {


#define GUI_WINDOW_NAME     "Sqeak"
#define GUI_WINDOW_W        1280
#define GUI_WINDOW_H        720
#define GUI_WINDOW_WMIN     200
#define GUI_WINDOW_HMIN     200

#define CONFIG_FILE         "config.ini"        // both created in the directory of the executable
#define GUI_CONFIG_FILE     "uiconfig.ini"

#define GUI_IMG_ICON        "/img/icon.png"



// *********************** //
// General constants //
// *********************** //
#define MAX_STRING                                  255
                    
#define FORWARD                                     1
#define BACKWARD                                   -1
                    
#define X_AXIS                                      1
#define Y_AXIS                                      2
#define Z_AXIS                                      3
       
       
} // end namespace Sqeak


// allows bitwise operations on enums
template<class T> inline T operator~ (T a) { return (T)~(int)a; }
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<class T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }


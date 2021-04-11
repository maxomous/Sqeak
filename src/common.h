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

extern void exitf(const char* format, ... );

// modifies string to lower case
extern void lowerCase(std::string& str);
// modifies string to upper case
extern void upperCase(std::string& str);

extern std::string lowerCase(const std::string& str);

// convert seconds into hours, minutes and seconds
extern void normaliseSecs(uint s, uint& hr, uint& min, uint& sec);


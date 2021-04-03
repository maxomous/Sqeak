/*
 * common.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#ifndef COMMON_HPP
#define COMMON_HPP

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
typedef struct {
	float x;
	float y;
	float z;
} point3D;

#include "grbl.hpp"
#include "grblcodes.hpp"
#include "file.hpp"
#include "gui/gui.hpp"
#include "gui/frames.hpp"



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

// returns p1 + p2
extern point3D add3p(point3D p1, point3D p2);
// return p1 - p2
extern point3D minus3p(point3D p1, point3D p2);

// returns lower case version of str
extern std::string lowerCase(const std::string& str);

// convert seconds into hours, minutes and seconds
extern void normaliseSecs(uint s, uint& hr, uint& min, uint& sec);
#endif

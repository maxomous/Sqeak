/*
 * common.hpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string>
#include <string.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctype.h>
#include <assert.h>
#include <vector>

#include <wiringPi.h>
#include <wiringSerial.h>


typedef struct {
	float x;
	float y;
	float z;
} point3D;

#include "gui/gui.hpp"
#include "queue.hpp"
#include "grbl.hpp"
#include "grblcodes.hpp"
#include "file.hpp"



#define ERR_NONE		0
#define ERR_FAIL		1

#define MAX_STRING 		255 // general maximum string length 

#define CLOCKWISE		1
#define ANTICLOCKWISE	-1



extern void exitf(const char* format, ... );

// returns p1 + p2
extern point3D add3p(point3D p1, point3D p2);
// return p1 - p2
extern point3D minus3p(point3D p1, point3D p2);

#endif

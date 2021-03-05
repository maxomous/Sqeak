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


#include "queue.hpp"
#include "grbl.hpp"
#include "codes.hpp"



#define ERR_NONE		0
#define ERR_FAIL		1

#define MAX_STRING 		255 // general maximum string length 


extern void exitf(const char* format, ... );

#endif

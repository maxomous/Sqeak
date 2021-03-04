#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <vector>

#include <wiringPi.h>
#include <wiringSerial.h>

#include "queue.hpp"
#include "grbl.hpp"


#define ERR_SUCCESS		0
#define ERR_FAIL		1

#define MAX_STRING 		255 // general maximum string length 

extern void exitf(const char* format, ... );

#endif

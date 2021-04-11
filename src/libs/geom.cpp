/*
 * geom.cpp
 * 
 */

#include <iostream>
#include <cmath>
using namespace std;

#include "geom.h"


// Returns Angle between 0 - 2PI
double cleanAngle(double Angle)
{
    double th = Angle;
    while ( (th >= 2*M_PI)  ||  (th < 0) ) {
		if (th < 0) 
			th += 2*M_PI;
		else if (th >= 2*M_PI) 
			th -= 2*M_PI;
    }
    return th;
}
  

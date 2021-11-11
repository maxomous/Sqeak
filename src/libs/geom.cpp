/*
 * geom.cpp
 * 
 */

#include <iostream>
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
void cleanAngles(double& StartAngle, double& EndAngle, int Direction)
{
    double start, end;
    
    // swap start and end for anticlockwise
    if(Direction == 1) { // CW
        start = StartAngle;
        end = EndAngle;
    }
    else if(Direction == -1) {//ACW
        start = EndAngle;
        end = StartAngle;
    }
    
    // if values are equal, make out of phase by 2M_PI
    if (start == end) {  // special condition required due to rounding errir -    after adding 2M_PI to end, (end - start) !>= 2*M_PI)
        end += 2*M_PI;
    }
    else {
        // make start and end within 360degs
        while ( ((end - start) > 2*M_PI)  ||  ((end - start) < 0) )
        {
            if (((end - start) < 0))
                end += 2*M_PI;
            else if ((end - start) > 2*M_PI) 
                end -= 2*M_PI;
        }
    }
    // phase shift both by -2*M_PI until one is less than zero (for vals greater than 4M_PI)
    while((start>0) && (end>0)) {
        start -= 2*M_PI;
        end -= 2*M_PI;
    }
    // phase shift both by +2*M_PI so they are both positive (for values < 0)
    while((start<0) || (end<0)) {
        start += 2*M_PI;
        end += 2*M_PI;
    }
    // swap and return start and end
    if(Direction == 1) { //CW
        StartAngle = start;
        EndAngle = end;
    }
    else if(Direction == -1) { //CCW
        EndAngle = start;
        StartAngle = end;
    }
}

/*
 * geom.cpp
 * 
 */

#include <iostream>
#include <assert.h>
using namespace std;

#include "geom.h"

polar::polar(float R, double Th) { 
    r = R; th = Th; 
}
polar::polar(point2D p) { 
    r = hypotf(p.x, p.y); 
    th = Geom::CleanAngle(atan2(p.x, p.y)); 
}
point2D polar::Cartesian() { 
    return point2D (r*cos(th - M_PI_2),  r*-sin(th - M_PI_2)); 
}

// Returns Angle between 0 - 2PI
double Geom::CleanAngle(double angle)
{
    double th = angle;
    while ( (th >= 2*M_PI)  ||  (th < 0) ) 
    {
        if (th < 0) 
            th += 2*M_PI;
        else if (th >= 2*M_PI) 
            th -= 2*M_PI;
    }
    return th;
}

void Geom::CleanAngles(double& startAngle, double& endAngle, int direction)
{
    assert(direction == CLOCKWISE || direction == ANTICLOCKWISE);
    // swap start and end for anticlockwise
    double start = (direction == CLOCKWISE) ? startAngle : endAngle;
    double end = (direction == CLOCKWISE) ? endAngle : startAngle;
    
    // if values are equal, make out of phase by 2M_PI
    if (start == end) {  // special condition required due to rounding error -    after adding 2M_PI to end, (end - start) !>= 2*M_PI)
        end += 2*M_PI;
    }
    else { // make start and end within 360degs
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
    startAngle = (direction == CLOCKWISE) ? start : end;
    endAngle = (direction == CLOCKWISE) ? end : start;
    
}

point2D Geom::ArcCentreFromRadius(point2D p0, point2D p1, float r, int direction)
{			
    assert(direction == CLOCKWISE || direction == ANTICLOCKWISE);
    point2D dif = p1 - p0;
	// midpoint of start to end	
    point2D pMid = (p0 + p1) / 2.0f;
    
    // length between start and end points
	float 	L = sqrt(dif.x * dif.x + dif.y * dif.y);
    
	//	angle between x axis & line from start to end
	float theta_G = fabs(atan(dif.y / dif.x));
	float h = sqrt(r*r - (L/2.0f)*(L/2.0f));
	
    h = direction * h;
	// 2nd version of the curve (when the centrepoint is past the midway line between start and end) 
	if(r < 0.0f)	
		h = -h;
    
    point2D pCentre;
    point2D invert = { ((p0.y > p1.y) ? -1.0f : 1.0f), ((p1.x > p0.x) ? -1.0f : 1.0f) };
        
	// if start to end is vertical
	if(dif.x == 0.0f) {
        pCentre.x = p0.x + invert.y * h;
		pCentre.y = pMid.y;
	}	
	// if start to end is horizontal
	else if(dif.y == 0.0f) { 
		pCentre.x = pMid.x;
        pCentre.y = p0.y + invert.x * h;
	}
	else  {        
        point2D hyp = { h*sin(theta_G), h*cos(theta_G) };
        pCentre = pMid + invert * hyp;
    }
    return pCentre;
};

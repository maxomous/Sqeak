/*
 * geom.cpp
 * 
 */

#include <iostream>
#include <tuple>
#include <assert.h>
using namespace std;

#include "geom.h"

polar::polar(double R, double Th) { 
    r = R; th = Th; 
}
polar::polar(point2D p) { 
    r = hypot(p.x, p.y); 
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
 

// returns angle from positive x axis in CW direction based on centre point and end point
std::optional<double> Geom::AngleBetween(point2D p0, point2D p1)
{
    // error if points are the same
    if( p0 == p1 ) { return {}; }
    return polar(p1-p0).th;
}

// calculates angle between 3 points		p1 is start, p2 is centre, p3 is end
std::optional<double> Geom::AngleBetween(point2D p1, point2D p2, point2D p3, int direction)
{
    // error if any points are the same
    if( p1 == p2 || p2 == p3 || p3 == p1 ) { return {}; }
    // get angles of lines (p1 -> p2) & (p2 -> p3)
    double startAngle = polar(p1-p2).th;
    double endAngle = polar(p3-p2).th;
    CleanAngles(startAngle, endAngle, direction);
    // clockwise / anticlockwise
    double output = (direction == CLOCKWISE) ? (endAngle - startAngle) : (startAngle - endAngle); 
    return CleanAngle(output);
}

point2D Geom::ArcCentreFromRadius(const point2D& p0, const point2D& p1, double r, int direction)
{			
    assert(direction == CLOCKWISE || direction == ANTICLOCKWISE);
    point2D dif = p1 - p0;
	// midpoint of start to end	
    point2D pMid = (p0 + p1) / 2.0;
    
    // length between start and end points
	double 	L = sqrt(dif.x * dif.x + dif.y * dif.y);
    
	//	angle between x axis & line from start to end
	double theta_G = fabs(atan(dif.y / dif.x));
	double h = sqrt(r*r - (L/2.0)*(L/2.0));
	
    h = direction * h;
	// 2nd version of the curve (when the centrepoint is past the midway line between start and end) 
	if(r < 0.0)	
		h = -h;
    
    point2D pCentre;
    point2D invert = { ((p0.y > p1.y) ? -1.0 : 1.0), ((p1.x > p0.x) ? -1.0 : 1.0) };
        
	// if start to end is vertical
	if(dif.x == 0.0) {
        pCentre.x = p0.x + invert.y * h;
		pCentre.y = pMid.y;
	}	
	// if start to end is horizontal
	else if(dif.y == 0.0) { 
		pCentre.x = pMid.x;
        pCentre.y = p0.y + invert.x * h;
	}
	else  {        
        point2D hyp = { h*sin(theta_G), h*cos(theta_G) };
        pCentre = pMid + invert * hyp;
    }
    return pCentre;
};


/** Calculate determinant of matrix:
	[a b]
	[c d] */
double Geom::Determinant(double a, double b, double c, double d)
{
    return a*d - b*c;
} 

// returns true if point ios
bool Geom::LeftOfLine(const point2D& p1, const point2D& p2, const point2D& pt)
{
    return ((p2.x-p1.x)*(pt.y-p1.y) - (p2.y-p1.y)*(pt.x-p1.x)) > 0.0;
}

// returns tangent point of circle to point p0, (circle has centre pC, and radius r) 	side: 1 is left, -1 is right
point2D Geom::TangentOfCircle(const point2D& p0, const point2D& pC, double r, int side)
{
    point2D line = p0 - pC;
    polar pol = polar(line);
    // length of line
    float L = pol.r;
    // angle of line
    double th = asin(r / L);
    return p0 + polar(sqrt(L*L-r*r), (pol.th - (double)side*th)).Cartesian();		// TODO ***** CHECK DIRECITON IS CORRECT******
}


// calculates the intersection points between 2 circles and passes back in the 2 return pointers
// return 0 if no intersect,   1 if 1 intersect point(tangent)   and 2 if 2 intersect points
std::tuple<int, point2D, point2D> Geom::IntersectTwoCircles(const point2D& c1, double r1, const point2D& c2, double r2)
{
    auto err = make_tuple(0, point2D(), point2D());
    // error if centre is s
    if(c1.x==c2.x && c1.y==c2.y) { return err; }
    double xdif = c2.x - c1.x;
    double ydif = c2.y - c1.y;
    double L = hypot(xdif, ydif);

    if ((L <= r1 + r2) && L >= fabs(r2 - r1)) {
        double ex = (c2.x - c1.x) / L;
        double ey = (c2.y - c1.y) / L;
        double x = (r1*r1 - r2*r2 + L*L) / (2*L);
        double y = sqrt(r1*r1 - x*x);
        point2D p1, p2;
        p1.x = c1.x + x*ex - y*ey;
        p1.y = c1.y + x*ey + y*ex;
        p2.x = c1.x + x*ex + y*ey;
        p2.y = c1.y + x*ey - y*ex;
        if(p1 == p2) { return make_tuple(1, p1, p2); }
        else         { return make_tuple(2, p1, p2); }
    }
    // No Intersection, far outside or one circle within the other
    else { return err; }
}

// calculates the intersection points between a line and a circle and passes back in the 2 return pointers
// return 0 if no intersect,   1 if 1 intersect point(tangent)   and 2 if 2 intersect points
std::tuple<int, point2D, point2D> Geom::IntersectLineCircle(const point2D& p1, const point2D& p2, const point2D& c, double r)
{
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double L = hypot(dx, dy);
    double D = Determinant(p1.x-c.x, p1.y-c.y, p2.x-c.x, p2.y-c.y);
    double q = sqrt(r*r*L*L - D*D);
    
    double disc = r*r*L*L - D*D;
    // no intersection
    if(disc < 0.0) { return make_tuple(0, point2D(), point2D()); }
    // calculate intersection point
	point2D pIntersect1 = point2D(c.x + ((D*dy  + sign(dy)*dx*q) / (L*L)),   c.y + ((-D*dx + fabs(dy)*q) / (L*L)));
    point2D pIntersect2 = point2D(c.x + ((D*dy  - sign(dy)*dx*q) / (L*L)),   c.y + ((-D*dx - fabs(dy)*q) / (L*L)));
    // 1 intersection (tangent)
    if (disc == 0.0) { return make_tuple(1, pIntersect1, point2D()); }
    // 2 intersections
    else if (disc > 0.0) { return make_tuple(2, pIntersect1, pIntersect2); }
    // error
    else { assert(0 && "Invalid number of points in Geom::IntersectLineCircle"); }
    return make_tuple(0, point2D(), point2D()); 
}

// returns whether there is an intersect or not (faster alternative to Geom::IntersectLines however does not return location
bool Geom::IntersectLinesFast(const point2D& p1, const point2D& p2, const point2D& p3, const point2D& p4)
{
    double test1 = (p2.x - p1.x)*(p3.y - p2.y) - (p2.y - p1.y)*(p3.x - p2.x);
    double test2 = (p2.x - p1.x)*(p4.y - p2.y) - (p2.y - p1.y)*(p4.x - p2.x);
    return sign(test1) != sign(test2);
}
// returns 0 on success
///Calculate intersection of two lines.
std::optional<point2D> Geom::IntersectLines(const point2D& p1, const point2D& p2, const point2D& p3, const point2D& p4)
{
    // http://mathworld.wolfram.com/Line-LineIntersection.html
    double detL1 = Determinant(p1.x, p1.y, p2.x, p2.y);
    double detL2 = Determinant(p3.x, p3.y, p4.x, p4.y);
    double x1mx2 = p1.x - p2.x;
    double x3mx4 = p3.x - p4.x;
    double y1my2 = p1.y - p2.y;
    double y3my4 = p3.y - p4.y;

    double xnom = Determinant(detL1, x1mx2, detL2, x3mx4);
    double ynom = Determinant(detL1, y1my2, detL2, y3my4);
    double denom = Determinant(x1mx2, y1my2, x3mx4, y3my4);
    
	// Intersection lines do not cross
    if(denom == 0.0) { return {}; }

    double ixOut = xnom / denom;	
    double iyOut = ynom / denom;
    // intersection is not finite, probably a numerical issue
    if(!isfinite(ixOut) || !isfinite(iyOut)) { return {}; }
	    
    return point2D(ixOut, iyOut);
}

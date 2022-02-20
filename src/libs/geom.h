/*
 * geom.h
 * 
 */
#pragma once

#include <cmath>
#include <optional>
#include <iostream>
#include "../glcore/glm.h"

#define CLOCKWISE                                   1
#define ANTICLOCKWISE                              -1

// returns the sign (1. 0 or -1) of a number
// 0 can be set to 1 or -1 with zeroValue
template <typename T> 
int sign(T val, int zeroValue = 0) { 
    int a = (zeroValue == 1) ? (T(0) <= val) : (T(0) < val);
    int b = (zeroValue == -1) ? (val <= T(0)) : (val < T(0));
    return a-b;
}

#define deg2rad(deg) ((deg) * M_PI / 180.0) 
#define rad2deg(rad) ((rad) * 180.0 / M_PI)

#define Sin(deg) sin(deg2rad(deg))
#define Cos(deg) cos(deg2rad(deg))
#define Tan(deg) tan(deg2rad(deg))


static inline float roundto(float input, float roundto) {
    double x = (double)input / (double)roundto;
    return round(x) * (double)roundto;
}

class point2D {
public:
    // ensures the inhertited destructor is called
    virtual ~point2D() {}
	// Variables
	double x;
	double y; 
	// Constructors
	point2D()                   { x = 0; y = 0; }
	point2D(double X, double Y) { x = X; y = Y; }
	point2D(const glm::vec2& p) { x = p.x; y = p.y; }
    glm::vec2 glm() { return { x, y }; }
};

// Overload operators (can even be printed!)
static inline point2D operator+(const point2D& a, const point2D& b) { return point2D(a.x + b.x, a.y + b.y); }
static inline point2D operator-(const point2D& a, const point2D& b) { return point2D(a.x - b.x, a.y - b.y); }
static inline point2D operator*(const point2D& a, const point2D& b) { return point2D(a.x * b.x, a.y * b.y); }
static inline point2D operator/(const point2D& a, const point2D& b) { return point2D(a.x / b.x, a.y / b.y); }
static inline point2D operator+(const point2D& a, const double b) { return point2D(a.x + b, a.y + b); }
static inline point2D operator-(const point2D& a, const double b) { return point2D(a.x - b, a.y - b); }
static inline point2D operator*(const point2D& a, const double b) { return point2D(a.x * b, a.y * b); }
static inline point2D operator/(const point2D& a, const double b) { return point2D(a.x / b, a.y / b); }
static inline point2D& operator+=(point2D& a, const point2D& b) { a.x += b.x; a.y += b.y; return a;}
static inline point2D& operator-=(point2D& a, const point2D& b) { a.x -= b.x; a.y -= b.y; return a;}
static inline point2D& operator+=(point2D& a, const double b) { a.x += b; a.y += b; return a;}
static inline point2D& operator-=(point2D& a, const double b) { a.x -= b; a.y -= b; return a;}
static inline bool operator==(const point2D& a, const point2D& b) { return (a.x == b.x && a.y == b.y); }
static inline bool operator!=(const point2D& a, const point2D& b) { return (a.x != b.x || a.y != b.y); }
static inline std::ostream& operator<<(std::ostream& os, const point2D& p) { os << "(" << p.x << ", " << p.y << ")"; return os; }
 
class point3D : public point2D {
public:
	// Variables
	double z;
	// Constructors
	point3D()                          { x = 0; y = 0; z = 0; }
	point3D(double X, double Y, double Z) { x = X; y = Y; z = Z; }
	point3D(const glm::vec3& p)        { x = p.x; y = p.y; z = p.z; }
    glm::vec3 glm() { return { x, y, z }; }
};
// Overload operators (can even be printed!)
static inline point3D operator+(const point3D& a, const point3D& b) { return point3D(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline point3D operator-(const point3D& a, const point3D& b) { return point3D(a.x - b.x, a.y - b.y, a.z - b.z); }
static inline point3D operator*(const point3D& a, const point3D& b) { return point3D(a.x * b.x, a.y * b.y, a.z * b.z); }
static inline point3D operator/(const point3D& a, const point3D& b) { return point3D(a.x / b.x, a.y / b.y, a.z / b.z); }
static inline point3D operator+(const point3D& a, const double b) { return point3D(a.x + b, a.y + b, a.z + b); }
static inline point3D operator-(const point3D& a, const double b) { return point3D(a.x - b, a.y - b, a.z - b); }
static inline point3D operator*(const point3D& a, const double b) { return point3D(a.x * b, a.y * b, a.z * b); }
static inline point3D operator/(const point3D& a, const double b) { return point3D(a.x / b, a.y / b, a.z / b); }
static inline point3D& operator+=(point3D& a, const point3D& b) { a.x += b.x; a.y += b.y; a.z += b.z; return a;}
static inline point3D& operator-=(point3D& a, const point3D& b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a;}
static inline point3D& operator+=(point3D& a, const double b) { a.x += b; a.y += b; a.z += b; return a;}
static inline point3D& operator-=(point3D& a, const double b) { a.x -= b; a.y -= b; a.z -= b; return a;}
static inline bool operator==(const point3D& a, const point3D& b) { return (a.x == b.x && a.y == b.y && a.z == b.z); }
static inline bool operator!=(const point3D& a, const point3D& b) { return (a.x != b.x || a.y != b.y || a.z != b.z); }
static inline std::ostream& operator<<(std::ostream& os, const point3D& p) { os << "(" << p.x << ", " << p.y << ", " << p.z << ")"; return os; }

static inline double hypot(const point2D& p) { return sqrt(p.x*p.x + p.y*p.y); }
static inline double hypot(const point3D& p) { return sqrt(p.x*p.x + p.y*p.y + p.z*p.z); }

// 0 is at the top
// positive is clockwise
class polar {
public:
	// Variables
	double r;	// Length
	double th; 	// Angle
	// Constructor
	polar(double R = 0.0, double Th = 0.0);
	polar(point2D p);
    point2D Cartesian();
};
// Overload operators for cout
static inline std::ostream& operator<<(std::ostream& os, const polar& pol) { os << "(" << pol.r << ", " << rad2deg(pol.th) << "degs)"; return os; }

class Geom {
public:
    // Returns Angle between 0 - 2PI
    static double CleanAngle(double angle);
    // Modifies start and end angles to be in correct order based on direction 
    // Makes both >= 0 	
    // Output will produce <= 4*PI difference between angles	(anticlockwise curve could be start:710degs to end:359degs
    // If both angles are identical, they will be set to 2*PI out of phase
    // Direction:  1 CW   -1 CCW
    static void CleanAngles(double& startAngle, double& endAngle, int direction);
        
    // returns angle from positive x axis in CW direction based on centre point and end point
    static std::optional<double> AngleBetween(point2D centre, point2D end);

    // calculates angle between 3 points		p1 is start, p2 is centre, p3 is end
    static std::optional<double> AngleBetween(point2D p1, point2D p2, point2D p3, int direction = CLOCKWISE);

    
    // calculates centre from radius, start & end points (-r will return the second possible arc)
    static point2D ArcCentreFromRadius(const point2D& p0, const point2D& p1, double r, int direction);
    
    /** Calculate determinant of matrix:
        [a b]
        [c d] */
    static double Determinant(double a, double b, double c, double d);
    // returns true if point ios
    static bool LeftOfLine(const point2D& p1, const point2D& p2, const point2D& pt);
    // returns tangent point of circle to point p0, (circle has centre pC, and radius r) 	side: 1 is left, -1 is right
    static point2D TangentOfCircle(const point2D& p0, const point2D& pC, double r, int side);
    // calculates the intersection points between 2 circles and passes back in the 2 return pointers
    // return 0 if no intersect,   1 if 1 intersect point(tangent)   and 2 if 2 intersect points
    static std::tuple<int, point2D, point2D> IntersectTwoCircles(const point2D& c1, double r1, const point2D& c2, double r2);
    // calculates the intersection points between a line and a circle and passes back in the 2 return pointers
    // return 0 if no intersect,   1 if 1 intersect point(tangent)   and 2 if 2 intersect points
    static std::tuple<int, point2D, point2D> IntersectLineCircle(const point2D& p1, const point2D& p2, const point2D& c, double r);
    // returns whether there is an intersect or not (faster alternative to Geom::IntersectLines however does not return location
    static bool IntersectLinesFast(const point2D& p1, const point2D& p2, const point2D& p3, const point2D& p4);
    // returns 0 on success
    ///Calculate intersection of two lines.
    static std::optional<point2D> IntersectLines(const point2D& p1, const point2D& p2, const point2D& p3, const point2D& p4);

private:
    Geom() {}
    Geom(const Geom &) = delete;
    Geom &operator=(const Geom &) = delete;
};

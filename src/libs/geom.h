/*
 * geom.h
 * 
 */
#pragma once

#include <iostream>

// returns the sign (1. 0 or -1) of a number
template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

#define deg2rad(deg) (deg * M_PI / 180.0) 
#define rad2deg(rad) (rad * 180.0f / M_PI)

// Returns Angle between 0 - 2PI
extern double cleanAngle(double Angle);


class point2D {
public:
	// Variables
	float x;
	float y;
	// Constructor
	point2D() { x = 0; y = 0; }
	point2D(float X, float Y) { x = X; y = Y; }
};

// Overload operators (can even be printed!)
static inline point2D operator+(const point2D& a, const point2D& b) { return point2D(a.x + b.x, a.y + b.y); }
static inline point2D operator-(const point2D& a, const point2D& b) { return point2D(a.x - b.x, a.y - b.y); }
static inline point2D operator*(const point2D& a, const point2D& b) { return point2D(a.x * b.x, a.y * b.y); }
static inline point2D operator/(const point2D& a, const point2D& b) { return point2D(a.x / b.x, a.y / b.y); }
static inline point2D operator+(const point2D& a, const float b) { return point2D(a.x + b, a.y + b); }
static inline point2D operator-(const point2D& a, const float b) { return point2D(a.x - b, a.y - b); }
static inline point2D operator*(const point2D& a, const float b) { return point2D(a.x * b, a.y * b); }
static inline point2D operator/(const point2D& a, const float b) { return point2D(a.x / b, a.y / b); }
static inline point2D& operator+=(point2D& a, const point2D& b) { a.x += b.x; a.y += b.y; return a;}
static inline point2D& operator-=(point2D& a, const point2D& b) { a.x -= b.x; a.y -= b.y; return a;}
static inline point2D& operator+=(point2D& a, const float b) { a.x += b; a.y += b; return a;}
static inline point2D& operator-=(point2D& a, const float b) { a.x -= b; a.y -= b; return a;}
static inline bool operator==(const point2D& a, const point2D& b) { return (a.x == b.x && a.y == b.y); }
static inline bool operator!=(const point2D& a, const point2D& b) { return (a.x != b.x || a.y != b.y); }
static inline std::ostream& operator<<(std::ostream& os, const point2D& p) { os << "(" << p.x << ", " << p.y << ")"; return os; }
 
class point3D : public point2D {
public:
	// Variables
	float z;
	// Constructor
	point3D() { x = 0; y = 0; z = 0; }
	point3D(float X, float Y, float Z) { x = X; y = Y; z = Z;}
};

// Overload operators (can even be printed!)
static inline point3D operator+(const point3D& a, const point3D& b) { return point3D(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline point3D operator-(const point3D& a, const point3D& b) { return point3D(a.x - b.x, a.y - b.y, a.z - b.z); }
static inline point3D operator*(const point3D& a, const point3D& b) { return point3D(a.x * b.x, a.y * b.y, a.z * b.z); }
static inline point3D operator/(const point3D& a, const point3D& b) { return point3D(a.x / b.x, a.y / b.y, a.z / b.z); }
static inline point3D operator+(const point3D& a, const float b) { return point3D(a.x + b, a.y + b, a.z + b); }
static inline point3D operator-(const point3D& a, const float b) { return point3D(a.x - b, a.y - b, a.z - b); }
static inline point3D operator*(const point3D& a, const float b) { return point3D(a.x * b, a.y * b, a.z * b); }
static inline point3D operator/(const point3D& a, const float b) { return point3D(a.x / b, a.y / b, a.z / b); }
static inline point3D& operator+=(point3D& a, const point3D& b) { a.x += b.x; a.y += b.y; a.z += b.z; return a;}
static inline point3D& operator-=(point3D& a, const point3D& b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a;}
static inline point3D& operator+=(point3D& a, const float b) { a.x += b; a.y += b; a.z += b; return a;}
static inline point3D& operator-=(point3D& a, const float b) { a.x -= b; a.y -= b; a.z -= b; return a;}
static inline bool operator==(const point3D& a, const point3D& b) { return (a.x == b.x && a.y == b.y && a.z == b.z); }
static inline bool operator!=(const point3D& a, const point3D& b) { return (a.x != b.x || a.y != b.y || a.z != b.z); }
static inline std::ostream& operator<<(std::ostream& os, const point3D& p) { os << "(" << p.x << ", " << p.y << ", " << p.z << ")"; return os; }


// 0 is at the top
// positive is clockwise
class polar{
public:
	// Variables
	float r;	// Length
	double th; 	// Angle
	// Constructor
	polar() { r = 0; th = 0; }
	polar(float R, double Th) { r = R; th = Th; }
	polar(point2D p) { r = hypotf(p.x, p.y); th = cleanAngle(atan2(p.x, p.y)); }
	
	point2D Cartesian() { return point2D (r*cos(th - M_PI_2),  r*-sin(th - M_PI_2)); }
	
};


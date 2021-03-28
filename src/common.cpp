/*
 * common.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.hpp"

using namespace std;

void exitf(const char* format, ... ) 
{
    va_list arglist;
    va_start( arglist, format );
    
	char str[MAX_STRING];
	vsnprintf(str, MAX_STRING, format, arglist);
    va_end( arglist );
    
    printf("%s",str);
	exit(1);
}

// returns p1 + p2
point3D add3p(point3D p1, point3D p2)
{
    return (point3D){ .x = p1.x+p2.x, .y = p1.y+p2.y, .z = p1.z+p2.z };
}
// return p1 - p2
point3D minus3p(point3D p1, point3D p2)
{
    return (point3D){ .x = p1.x-p2.x, .y = p1.y-p2.y, .z = p1.z-p2.z };
}

string lowerCase(const string& str) {
    string s = str;
    for (int i = 0; i < s.length(); i++)
	s[i] = tolower(s[i]);
    
    return s;
}

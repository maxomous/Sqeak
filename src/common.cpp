/*
 * common.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.h"
using namespace std; 

string va_str(const char* format, ... )
{ 
    va_list arglist;
    char buf[255];
    va_start( arglist, format );
    vsnprintf(buf, sizeof(buf), format, arglist);
    va_end( arglist );
    
    return string(buf);
}

void lowerCase(string& str) {
    transform(str.begin(), str.end(),str.begin(), ::tolower);
}

void upperCase(string& str) {
    transform(str.begin(), str.end(),str.begin(), ::toupper);
}

// convert seconds into hours, minutes and seconds
void normaliseSecs(uint s, uint& hr, uint& min, uint& sec)
{
    hr = s / 3600;
    s %= 3600;
    min = s / 60 ;
    s %= 60;
    sec = s; 
}
 

 // This takes a std::string of 3 values seperated by commas (,) and will return a 3DPoint
// 4.000,0.000,0.000
glm::vec2 stoVec2(const std::string& msg) 
{
    std::stringstream stream(msg);
    std::string segment;
    glm::vec2 p;
    
    for (int i = 0; i < 2; i++) {
        getline(stream, segment, ',');
        p[i] = stof(segment);
    }
    return p;
}
 
// This takes a std::string of 3 values seperated by commas (,) and will return a 3DPoint
// 4.000,0.000,0.000
glm::vec3 stoVec3(const std::string& msg) 
{
    std::stringstream stream(msg);
    std::string segment;
    glm::vec3 p;

    for (int i = 0; i < 3; i++) {
        getline(stream, segment, ',');
        p[i] = stof(segment);
    }
    return p;
}
  

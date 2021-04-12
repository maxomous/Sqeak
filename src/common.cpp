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

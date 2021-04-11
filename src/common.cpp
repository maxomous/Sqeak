/*
 * common.cpp
 *  Max Peglar-Willis & Luke Mitchell 2021
 */

#include "common.h"

using namespace std;
/*

class Log {
public:
    enum LogLevel{
	LevelError, LevelWarning, LevelInfo
    };
private:
    LogLevel logLevel = LevelInfo; // default show all errors
    std::vector<std::string> consoleLog;
public:
    //log.SetLogLevel(Log::LevelError);
    void SetLogLevel(LogLevel level) { logLevel = level; }

    void PrintToConsole(string msg) { consoleLog.emplace_back(output); }
    void PrintToTerminal(string msg) { cout << output << endl; }

    void Error(const string& msg)
    {
	string errMsg = "Error: " + msg;
	PrintToConsole(errMsg);
	PrintToTerminal(errMsg);
    }
    void Warning(const string& msg)
    {
	string warningMsg = "Warning: " + msg;
	PrintToConsole(warningMsg);
	PrintToTerminal(warningMsg);
    }
    void Info(const string& msg)
    {
	string infoMsg = "Info: " + msg;
	PrintToConsole(infoMsg);
	PrintToTerminal(infoMsg);
    }
    
public:
    void Add(const char* str);
    void Add(const char* format, ... );
    void Add(const string& str);
    
private:
    
    std::vector<std::string>* consoleLog;
    void ShowAbove();
};



string va_str(const char* format, ... )
{
    va_list arglist;
    char buf[MAX_SIZE];
    va_start( arglist, format );
    vsnprintf(buf, sizeof(buf), format, arglist);
    va_end( arglist );
    
    return buf;
}
*/
void exitf(const char* format, ... ) 
{
    va_list arglist;
    char buf[MAX_STRING];
    va_start( arglist, format );
    vsnprintf(buf, sizeof(buf), format, arglist);
    va_end( arglist );
    
    cout << buf << endl;
    exit(1);
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

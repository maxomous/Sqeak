/*
 * gclist.h
 *  Max Peglar-Willis 2021
 */

#pragma once

#include <string>
#include <vector>
// threads
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "grbl_common.h" 

namespace Sqeak { 

typedef struct {
    std::string str;
    int status;
} GCodeItem;


class GCodeList {
public:
    GCodeList();
    // add a single gcode to list
    int add(const std::string& gcode);
    // allows many gcodes to be sent without relocking mutex
    // IT IS CRITICAL THAT addManyEnd() IS CALLED AFTER TO UNLOCK MUTEX
    int addMany(const std::string& gcode);
    // this must be called after addMany(gcode)'s are called
    void addManyEnd();
    // returns the next item from the list to be sent to grbl
    int getNextItem(GCodeItem& item) ;
    // sets the item which was just send to serial to pending and increments written
    void nextItem();
    // sets the item which was just recived fron serial to response and increments read
    int setNextResponse(int response, std::string& gcode);
    // set file as ended
    void setFileEnded();
    // adds an error at current position
    void addCheckModeError();
    // returns the last sent item
    int getLastItem(GCodeItem& item);
    // Returns n items starting from index
    // use index of -1 for last item sent
    const GCodeItem& getItem(uint index);
    // returns total size of list
    uint getSize();
    // returns true if file is running
    bool isFileRunning();
    // gets position and number of lines in file running
    void getFilePos(uint& posIndex, uint& pos, uint& total);
    // clears any completed GCodes in the buffer
    // used for clearing old commands in log
    void clearCompleted();
    // clears any remaining GCodes in our buffer
    // for cancelling any commands that are waiting to be sent
    void clearUnsent();
    // clears any remaining GCodes in ours, and GRBLs buffer
    // for resetting only
    // queue MUST be emptied also + grblBufferSize set to max
    void softReset();
    // sets the command flag for shutdown / soft reset / 
    void setCommand(int cmd);
private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::unique_lock<std::mutex>* addManyLocker = nullptr; // use lock and unlock
    int m_runCommand;
    
    std::vector<GCodeItem> m_list;    // a list is of all gcodes waiting to be sent to grbl
    uint m_written = 0;        // how many sent to grbl
    uint m_read = 0;        // how many received a response from grbl
    
    uint m_fileStart = 0;
    uint m_fileEnd = 0;
    
    std::vector<int> m_errors;  // a list of line numbers for error checking
    int m_blankLines = 0;       // a tally of any blank lines during file
    
    // adds an entry to GCode List (called by add / addmany only)
    // mutex should already be locked before running this function
    int addEntry(const std::string& gcode);
    // takes a GCode linbe and cleans up
    // (removes spaces, comments, make uppercase, ensures '\n' is present)
    void cleanString(std::string& str);
};


} // end namespace Sqeak

/*
 * gclist.h
 *  Max Peglar-Willis 2021
 */

#pragma once


typedef struct {
    std::string str;
    int status;
} GCItem;


class GCList {
    GCList();
public:
    void debug(bool showDebug);
private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::unique_lock<std::mutex>* addManyLocker = nullptr; // use lock and unlock
    std::unique_lock<std::mutex>* unboundLocker = nullptr; // use lock and unlock
    int m_runCommand;
    bool m_debug_serial = false;
    
    std::vector<GCItem> m_list;	// a list is of all gcodes waiting to be sent to grbl
    uint m_written = 0;		// how many sent to grbl
    uint m_read = 0;		// how many received a response from grbl
    
    uint m_fileStart = 0;
    uint m_fileEnd = 0;
    
    std::vector<int> m_checkModeErrors; // a list of line numbers for error checking
    int m_checkModeBlankLines = 0;
    
    int add(const std::string& gcode);
    // allows many gcodes to be sent without relocking mutex
    // IT IS CRITICAL THAT addManyEnd() IS CALLED AFTER TO UNLOCK MUTEX
    int addMany(const std::string& gcode);
    // this must be called after addMany(gcode)'s are called
    void addManyEnd();
    int getNextItem(GCItem& item) ;
    // sets the item which was just send to serial to pending and increments written
    void nextItem();
    // sets the item which was just recived fron serial to response and increments read
    void setNextResponse(int response);
    void setFileEnded();
    // returns the last sent item
    int getLastItem(GCItem& item);
    // Returns n items starting from index
    // use index of -1 for last item sent
    const GCItem& getItem(uint index);
    // returns total size of list
    uint getSize();
    bool isFileRunning();
    void getFilePos(uint& pos, uint& total);
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
    void setCommand(int cmd);
    
    void addCheckModeError();
    // takes a GCode linbe and cleans up
    // (removes spaces, comments, make uppercase, ensures '\n' is present)
    void cleanString(std::string& str);
    friend class GRBL;
};


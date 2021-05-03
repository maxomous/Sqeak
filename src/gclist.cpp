/*
 * gclist.cpp
 *  Max Peglar-Willis 2021
 */

#include "common.h"  
using namespace std;  

GCList::GCList() 
{
    // lock the mutex (unlikely to be needed on init)
    std::lock_guard<std::mutex> guard(m_mutex);
    m_runCommand = GRBL_CMD_RUN;
}

int GCList::add(const std::string& gcode)
{	
    {   // lock the mutex
		std::lock_guard<std::mutex> gaurd(m_mutex);
		int err = addEntry(gcode);
		if(err) return err;
    }
    // notify since we have added to the queue
    m_cond.notify_one();
    return 0;
}
// allows many gcodes to be sent without relocking mutex
// IT IS CRITICAL THAT addManyEnd() IS CALLED AFTER TO UNLOCK MUTEX
int GCList::addMany(const std::string& gcode) 
{
    if(addManyLocker == nullptr) {
		// lock the mutex
		addManyLocker = new std::unique_lock<std::mutex>(m_mutex);
		// make sure addManyLocker has been set
		assert(addManyLocker != nullptr);
		m_fileStart = m_list.size();
    }
    
    return addEntry(gcode);
}
// this must be called after addMany(gcode)'s are called
void GCList::addManyEnd() 
{
    if(addManyLocker == nullptr) {
		Log::Error("Nothing has been added to addMany() yet");
		return;
    }
    // set end of file
    m_fileEnd = m_list.size();
    // unlock mutex
    addManyLocker->unlock();
    // delete
    delete(addManyLocker);
    addManyLocker = nullptr;
    // notify since we have added to the queue
    m_cond.notify_one();
}

int GCList::getNextItem(GCItem& item) 
{	
    // lock the mutex
    std::unique_lock<std::mutex> locker(m_mutex);
    // unlock the mutex and wait here until notify is called
    // a condition to prevent spurious wakes
    // but we want to wake up if m_runCommand is set to shutdown or reset
    
    if(m_runCommand)
		return 1;
    
    m_cond.wait(locker, [&]() { 
		Log::Debug(DEBUG_THREAD_BLOCKING, "Inside getNextItem() condtion");  
		// wake condition
		return ((m_written < m_list.size()) || m_runCommand); 
    });  
    Log::Debug(DEBUG_THREAD_BLOCKING, "Passed getNextItem() condtion (runCmd = %d)", m_runCommand);  
    
    if(m_runCommand)
		return 1;
	
    // return next item in gcList
    item = std::ref(m_list[m_written]);
    
    Log::Debug(DEBUG_GCLIST, "GCode List: retrieved item = %s", item.str.c_str());
    return 0;
}

// sets the item which was just send to serial to pending and increments written
void GCList::nextItem() 
{	// lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    // set next item status
    m_list[m_written++].status = STATUS_SENT;
}

// sets the item which was just recived fron serial to response and increments read
int GCList::setNextResponse(int response) 
{ 	// lock the mutex
    std::unique_lock<std::mutex> locker(m_mutex);

    if(m_read >= m_list.size()) { // dump data as this could be a bad bug
		Log::Error("We are reading more than we have sent... Resetting for safety");
		Log::Error("read = %u   gcList size = %u", m_read, m_list.size());
		Log::Error("trying to add = %d\n", response);
		Log::Error("Last 10 items of gcList: ");
		for (int i = max((int)m_list.size() - 10, 0); i < (int)m_list.size(); i++)
			Log::Error("item %d  :  %s  :  %d", i, m_list[i].str.c_str(), m_list[i].status);
		
		return 1;
    }
    Log::Debug(DEBUG_GCLIST, "GCode List: Setting response %d to %s", m_list[m_read].status, m_list[m_read].str.c_str());
    
    // Set reponse to corrosponding gcode
    m_list[m_read++].status = response;
    // to trigger that we have reached end of file
    if(m_fileEnd != 0 && m_read >= m_fileEnd) {
		locker.unlock();
		setFileEnded();
    }	
    return 0;
}

void GCList::setFileEnded() 
{	// lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    // return if file running
    if(!m_fileEnd)
		return;
    m_fileStart = 0;
    m_fileEnd = 0;
    Log::Info("End of file");

    // if in check mode, display any errors found
    if(m_errors.size() == 0) 
		return;
    // display any errors found
    Log::Error("Errors were found in this file");
    for(int line : m_errors)
		Log::Error("Error recieved at line: %d", line);
    m_errors.clear();
    m_blankLines = 0;
}

void GCList::addCheckModeError()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    uint filePos = m_read - m_fileStart;
    m_errors.push_back(filePos + m_blankLines);
}

// Returns n items starting from index
int GCList::getLastItem(GCItem& item) 
{   // lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    if(m_written == 0)
		return -1;
    item = m_list[m_written-1];
    return 0;
}

// Returns n items starting from index
const GCItem& GCList::getItem(uint index) 
{   // lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    if(index > m_list.size()) {
		Log::Critical("Cannot access item %d in gcList", index);
    }
    return m_list[index];
}

// returns total size of list
uint GCList::getSize() 
{   // lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    // return total size
    return m_list.size();
}

bool GCList::isFileRunning() {
    // lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    return m_fileEnd;
}

void GCList::getFilePos(uint& pos, uint& total) {
    // lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    // if file is running
    if(m_fileEnd) {
		pos = m_read - m_fileStart;
		total = m_fileEnd - m_fileStart;
    } else {
		pos = 0;
		total = 0;
    }
}

// clears any completed GCodes in the buffer
// used for clearing old commands in log
void GCList::clearCompleted() {
    // lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    m_list.erase(m_list.begin(), m_list.begin() + m_read);
    m_fileEnd = (m_fileEnd < m_read) ? 0 : m_fileEnd - m_read;
    if (m_fileEnd <= 0) {
		m_fileStart = 0;
		m_fileEnd = 0;
    }	
    m_written -= m_read;
    m_read = 0;
}

// clears any remaining GCodes in our buffer
// for cancelling any commands that are waiting to be sent
void GCList::clearUnsent() {
    // lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    m_list.erase(m_list.begin() + m_written, m_list.end());
    m_fileEnd = m_list.size();
}
// MUST BE LOCKED FIRST!
// SHOULD ONLY BE RUN BY GRBL::softReset()
// clears any remaining GCodes in ours, and GRBLs buffer
// for resetting only
void GCList::softReset() 
{
    // lock the mutex
    std::lock_guard<std::mutex> guard(m_mutex);
    m_list.erase(m_list.begin() + m_read, m_list.end());
    m_fileStart = 0;
    m_fileEnd = 0;
    m_written = m_read;
}

void GCList::setCommand(int cmd) {
    {  // lock the mutex
		std::lock_guard<std::mutex> gaurd(m_mutex);
		// set values to ensure condition variable is met
		m_runCommand = cmd;
    }
    // notify so that the thread stops waiting
    m_cond.notify_all();
}



//********************************** PRIVATE *****************************************//
//           ALL FUNCTIONS BELOW SHOULD ALREADY HAVE MUTEX LOCKED!


// this should already be locked with mutex!
int GCList::addEntry(const std::string& gcode)
{
    GCItem item = { gcode, STATUS_UNSENT };
    cleanString(item.str);
    // ignore blank lines
    if(item.str == "\n") {
		m_blankLines++;
		return 0;
    }	
		if(item.str.length() > MAX_GRBL_BUFFER) {
		Log::Error("Line is longer than the maximum buffer size");
		return -1;
    }
    // add to list
    m_list.emplace_back(std::move(item));
    return 0;
}

// takes a GCode linbe and cleans up
// (removes spaces, comments, make uppercase, ensures '\n' is present)
void GCList::cleanString(std::string& str) 
{
    // strip out whitespace - this means we can fit more in the buffer
    str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
    // remove comments within () 
    size_t a = str.find("(");
    if(a != std::string::npos) {
	size_t b = str.find(")", a);
	if(b != std::string::npos)
		str.erase(a, b-a+1);
    }
    // remove comments after ;
    size_t c = str.find(";");
    if(c != std::string::npos)
	str.erase(c, str.length()-c );
    // make all uppercase
    upperCase(str);
    // add a newline character to end
    str.append("\n");
}


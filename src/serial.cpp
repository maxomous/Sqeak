/*
 * serial.cpp
 *  Max Peglar-Willis 2021
 */

#include "common.h" 
using namespace std;



Serial::Serial() {
    m_runCommand 	= GRBL_CMD_RUN;
    m_available 	= MAX_GRBL_BUFFER;
    m_connected 	= false;
}
Serial::~Serial() {
    disconnect();
}
// controlled by grbl
int Serial::connect() 
{
    // lock the mutex
    std::lock_guard<std::mutex> gaurd(m_mutex);
    if(m_connected)
	return -1;
    // open serial connection
    m_fd = serialOpen(SERIAL_DEVICE, SERIAL_BAUDRATE);
    if(m_fd < 0) {
	Log::Error("Could not open serial device");
	return -1;
    }
    m_connected = true;  
    return 0;
}

void Serial::disconnect() 
{
    // lock the mutex
    std::lock_guard<std::mutex> gaurd(m_mutex);
    if(!m_connected)
	return;
    // close serial connection
    serialClose(m_fd);
    m_connected = false;
    
}

bool Serial::isConnected() {
    return m_connected;
}

// special version of send which bypasses the character counting buffer for realtime commands
void Serial::sendRT(char cmd) 
{
    // lock the mutex
    std::lock_guard<std::mutex> gaurd(m_mutex);
    // write to serial port
    serialPutchar(m_fd, cmd);
}

int Serial::send(const std::string& gcode)
{	
    uint len = gcode.length();
    // lock the mutex
    std::unique_lock<std::mutex> locker(m_mutex);
    // unlock the mutex and wait here until notify is called
    // a condition to prevent spurious wakes
    // but we want to wake up if m_runCommand is set to shutdown or reset
    
    if(m_runCommand)
	return 1;
    
    m_cond.wait(locker, [&](){ return (((m_available >= len) && m_connected) || m_runCommand); });
    
    if(m_runCommand)
	return 1;
    // write to serial port
    serialPuts(m_fd, gcode.c_str());
    // update character counter
    m_available -= len;
    m_used.push(len);
    
    if(m_debug_serial)
	Log::Info("Serial: Sending = %s\t(%u/%u)", gcode.c_str(), m_available, MAX_GRBL_BUFFER);
    
    return 0;
}

    
int Serial::receive(std::string& msg)
{	
    msg.clear();
    // lock the mutex
    std::unique_lock<std::mutex> locker(m_mutex);
    
    if(serialDataAvail(m_fd) && m_connected) {
	// read serial
	readLine(msg);
	// unlock mutex
	locker.unlock();
    } else {
	// unlock mutex
	locker.unlock();
	delay(5);
	return 1;
    }
    // 1 is show, 2 is show but ignore status
    if(m_debug_serial == 1 || (m_debug_serial == 2 && msg[0] != '<'))
	Log::Info("Serial: Reading = %s", msg.c_str());

    return 0;
}

int Serial::bufferRemove()
{	
   {    // lock the mutex
        std::lock_guard<std::mutex> gaurd(m_mutex);
	
	// unlikely error which can be caused by data left in buffer from previous session
	if(m_used.empty()) {
	    Log::Error("Unexpected response, machine has been reset. (Purhaps there were some commands left in GRBL's buffer?)");
	    return -1;
	}
	// add length of std::string of completed request back onto buffer
	m_available += m_used.front();
	m_used.pop();
    
	#ifdef DEBUG
	    Log::Info("Remaining buffer: %d/%d", m_available, MAX_GRBL_BUFFER);
	#endif
   }
    
    // notify as there is more space availble now
    m_cond.notify_one();
    return 0;
}


// Reads line of serial interface and returns onto msg
void Serial::readLine(std::string& msg) 
{
    char buf;
    //retrieve line
    do {
	// retrieve letter
	buf = serialGetchar(m_fd);
	// break if end of line
	if (buf == '\n') 
	    break;
	if(msg.length() >= MAX_GRBL_BUFFER) {
	    Log::Warning("Serial input length is greater than input buffer, allocating more memory");
	    msg.resize(2 * msg.capacity());
	}
	// add to buffer - skip non-printable characters
	if(isprint(buf)) 
	    msg += buf;
    } while(1);
}

// MUST BE LOCKED FIRST!
// SHOULD ONLY BE RUN BY GRBL::softReset()
void Serial::softReset() 
{	
    // lock the mutex
    std::lock_guard<std::mutex> gaurd(m_mutex);
    // send reset to grbl
    serialPutchar(m_fd, GRBL_RT_SOFT_RESET);
    // flush the serial buffer
    flush();
    // clear the queue
    clearQueue();   
}

void Serial::setCommand(int cmd) {
    {  // lock the mutex
	std::lock_guard<std::mutex> gaurd(m_mutex);
	// set values to ensure condition variable is met
	m_runCommand = cmd;
    }
    // trigger thread to reset if waiting to send serial
    m_cond.notify_all();
}


// Flush the serial buffer, used for soft reset
// mutex MUST already be locked before using this
void Serial::flush() 
{
    if(!m_connected) {
	Log::Error("Cannot reset when not connected");
	return;
    }
    // flush grbl
    serialPuts(m_fd, "\r\n\r\n");
    delay(2000);
    // flush serial
    serialFlush(m_fd);
}
// clears queue, used for soft reset
// mutex MUST already be locked before using this
void Serial::clearQueue() {
    while(!m_used.empty())
	m_used.pop();
    // sets buffer size to max
    m_available = MAX_GRBL_BUFFER;
}

void Serial::debug(int show) {
    // lock the mutex
    std::lock_guard<std::mutex> gaurd(m_mutex);
    // set values to ensure condition variable is met
    m_debug_serial = show;
    
}


/*
 * serial.h
 *  Max Peglar-Willis 2021
 */

#pragma once


class Serial 
{
public:
    Serial() {
	m_connected = false;
    }
    ~Serial() {
	disconnect();
    }

private:
     
    std::mutex m_mutex;
    std::condition_variable m_cond;

    int m_fd 		= -1;
    uint m_available 	= MAX_GRBL_BUFFER;
    std::atomic<bool> m_connected;
    std::queue<int> m_used;
    
    // controlled by grbl
    int connect() 
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
    
    void disconnect() 
    {
	// lock the mutex
	std::lock_guard<std::mutex> gaurd(m_mutex);
	if(!m_connected)
	    return;
	// close serial connection
	serialClose(m_fd);
	m_connected = false;
	
    }
    
    bool isConnected() {
	return m_connected;
    }
    
    // special version of send which bypasses the character counting buffer for realtime commands
    void sendRT(char cmd) 
    {
	// lock the mutex
	std::lock_guard<std::mutex> gaurd(m_mutex);
	// write to serial port
	serialPutchar(m_fd, cmd);
    }
    
    void send(const std::string& gcode)
    {	
	uint len = gcode.length();
	// lock the mutex
	std::unique_lock<std::mutex> locker(m_mutex);
	// unlock the mutex and wait here until notify is called
	m_cond.wait(locker, [&](){ return (m_available >= len && m_connected); });  // a condition to prevent spurious wakes
	    
	    m_available -= len;
	    m_used.push(len);
	       
	    // write to serial port
	    serialPuts(m_fd, gcode.c_str());
	    //cout << m_available << "/128\tWriting to serial: " << gcode << endl;
	
	// unlock mutex
        locker.unlock();
	
	#ifdef DEBUG_SERIAL
	    Log::Info(std::string("Sent to serial: ") + gcode);
	#endif
    }
    
	
    int receive(std::string& msg)
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
	
	#ifdef DEBUG_SERIAL
	    Log::Info(std::string("Read from serial: ") + msg);
	#endif
	
	return 0;
    }
    
    int bufferRemove()
    {	
	// lock the mutex
	std::unique_lock<std::mutex> locker(m_mutex);
	
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
	
	// unlock mutex
        locker.unlock();
	// notify as there is more space availble now
	m_cond.notify_one();
	return 0;
    }
    
    
    // Reads line of serial interface and returns onto msg
    void readLine(std::string& msg) 
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
    
    // controlled by grbl
    void softReset() 
    {	// lock the mutex
	std::unique_lock<std::mutex> locker(m_mutex);
	// send reset to grbl
	serialPutchar(m_fd, GRBL_RT_SOFT_RESET);
	// flush the serial buffer
	flush();
	// clear the queue
	clearQueue();
	// notify because send and receive are waiting until they receive one
	locker.unlock();
	m_cond.notify_all();
    }
    // Flush the serial buffer, used for soft reset
    // mutex MUST already be locked before using this
    void flush() 
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
    void clearQueue() {
	while(!m_used.empty())
	    m_used.pop();
	// sets buffer size to max
	m_available = MAX_GRBL_BUFFER;
    }
    
    void shutdown() {
	// notify since we have added to the queue
	m_cond.notify_all();
    }
    friend class GRBL;
};



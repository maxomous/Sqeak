
/*
 * serial.h
 *  Max Peglar-Willis 2021
 */

#pragma once

class Serial 
{
public:
    Serial();
    ~Serial();
    // 0 for dont show, 1 for show, 2 for show but ignore status
    void debug(int show);
private:
     
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::unique_lock<std::mutex>* unboundLocker = nullptr; // use lock and unlock
    int m_runCommand;
    int m_debug_serial = false;
    bool m_debug_serial_IgnoreStatus = false;

    int m_fd = -1;
    uint m_available;
    std::atomic<bool> m_connected;
    std::queue<int> m_used;
    
    // controlled by grbl
    int connect();
    void disconnect();
    bool isConnected();
    // special version of send which bypasses the character counting buffer for realtime commands
    void sendRT(char cmd);
    int send(const std::string& gcode);
    int receive(std::string& msg);
    int bufferRemove();
    // Reads line of serial interface and returns onto msg
    void readLine(std::string& msg);

    void softReset();
    void setCommand(int cmd);
    // Flush the serial buffer, used for soft reset
    // mutex MUST already be locked before using this
    void flush();
    // clears queue, used for soft reset
    // mutex MUST already be locked before using this
    void clearQueue();
    
    friend class GRBL;
};


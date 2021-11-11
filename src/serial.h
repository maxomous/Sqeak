
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
    // connects to grbl
    int connect(std::string device, int baudrate);
    // disconnects from grbl
    void disconnect();
    // returns true if connected
    bool isConnected();
    // special version of send which bypasses the character counting buffer for realtime commands
    void sendRT(char cmd);
    // send a string to serial
    int send(const std::string& gcode);
    // recieves a string from serial
    int receive(std::string& msg);
    // takes data off the character counting buffer
    int bufferRemove();
    // sends reset command to grbl, flushes serial and clears character counting buffer
    void softReset();
    // sets the command flag for shutdown / soft reset / 
    void setCommand(int cmd);
private:
     
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::unique_lock<std::mutex>* unboundLocker = nullptr; // use lock and unlock
    int m_runCommand;

    int m_fd = -1;
    uint m_available;
    std::atomic<bool> m_connected;
    std::queue<int> m_used;
    
    // Reads line of serial interface and returns onto msg
    void readLine(std::string& msg);
    // Flush the serial buffer, used for soft reset
    // mutex MUST already be locked before using this
    void flush();
    // clears queue, used for soft reset
    // mutex MUST already be locked before using this
    void clearQueue();
};


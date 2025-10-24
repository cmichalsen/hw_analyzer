#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>

namespace hw_analyzer {

struct SerialPortInfo {
    std::string name;
    std::string description;
    std::string manufacturer;
};

class SerialInterface {
public:
    using DataCallback = std::function<void(const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    SerialInterface();
    ~SerialInterface();

    // Port management
    static std::vector<SerialPortInfo> listPorts();
    
    bool open(const std::string& portName, int baudRate = 115200);
    void close();
    bool isOpen() const;

    // Data operations
    bool write(const std::string& data);
    std::string read(size_t maxBytes = 256);
    
    // Async operations
    void setDataCallback(DataCallback cb);
    void setErrorCallback(ErrorCallback cb);
    void startReadLoop();
    void stopReadLoop();

    // Configuration
    bool setBaudRate(int baudRate);
    bool setDataBits(int bits);
    bool setParity(char parity); // 'N', 'E', 'O'
    bool setStopBits(int bits);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace hw_analyzer

#include "serial_interface.hpp"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dirent.h>
#endif

namespace hw_analyzer {

class SerialInterface::Impl {
public:
    Impl() = default;
    ~Impl() { close(); }

#ifdef _WIN32
    HANDLE handle = INVALID_HANDLE_VALUE;
#else
    int fd = -1;
#endif
    
    std::thread readThread;
    std::atomic<bool> running{false};
    DataCallback onData;
    ErrorCallback onError;
    
    int currentBaud = 115200;
    std::string portName;

    void close() {
        running = false;
        if (readThread.joinable()) {
            readThread.join();
        }
#ifdef _WIN32
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
        }
#else
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
#endif
    }
};

SerialInterface::SerialInterface() : pImpl(std::make_unique<Impl>()) {}
SerialInterface::~SerialInterface() = default;

std::vector<SerialPortInfo> SerialInterface::listPorts() {
    std::vector<SerialPortInfo> ports;
    
#ifdef _WIN32
    // Windows: Check COM1-COM256
    for (int i = 1; i <= 256; i++) {
        std::string portName = "COM" + std::to_string(i);
        std::string devicePath = "\\\\.\\" + portName;
        HANDLE h = CreateFileA(
            devicePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, 0, NULL
        );
        
        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            SerialPortInfo info;
            info.name = portName;
            info.description = "Serial Port";
            ports.push_back(info);
        }
    }
#else
    // Linux/macOS: Check /dev/tty* and /dev/cu*
    DIR* dir = opendir("/dev");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find("ttyUSB") == 0 || name.find("ttyACM") == 0 ||
                name.find("ttyS") == 0 || name.find("cu.") == 0) {
                SerialPortInfo info;
                info.name = "/dev/" + name;
                info.description = "Serial Port";
                ports.push_back(info);
            }
        }
        closedir(dir);
    }
#endif
    
    return ports;
}

bool SerialInterface::open(const std::string& portName, int baudRate) {
    pImpl->close();
    pImpl->portName = portName;
    pImpl->currentBaud = baudRate;

#ifdef _WIN32
    std::string fullName = "\\\\.\\" + portName;
    pImpl->handle = CreateFileA(
        fullName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL
    );
    
    if (pImpl->handle == INVALID_HANDLE_VALUE) {
        if (pImpl->onError) {
            pImpl->onError("Failed to open port: " + portName);
        }
        return false;
    }
    
    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(pImpl->handle, &dcb)) {
        close();
        return false;
    }
    
    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    
    if (!SetCommState(pImpl->handle, &dcb)) {
        close();
        return false;
    }
    
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(pImpl->handle, &timeouts);
    
#else
    pImpl->fd = ::open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (pImpl->fd < 0) {
        if (pImpl->onError) {
            pImpl->onError("Failed to open port: " + portName);
        }
        return false;
    }
    
    struct termios tty;
    if (tcgetattr(pImpl->fd, &tty) != 0) {
        close();
        return false;
    }
    
    // Set baud rate
    speed_t speed = B115200;
    switch(baudRate) {
        case 9600: speed = B9600; break;
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
    }
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
    
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    
    if (tcsetattr(pImpl->fd, TCSANOW, &tty) != 0) {
        close();
        return false;
    }
#endif
    
    return true;
}

void SerialInterface::close() {
    pImpl->close();
}

bool SerialInterface::isOpen() const {
#ifdef _WIN32
    return pImpl->handle != INVALID_HANDLE_VALUE;
#else
    return pImpl->fd >= 0;
#endif
}

bool SerialInterface::write(const std::string& data) {
    if (!isOpen()) return false;
    
#ifdef _WIN32
    DWORD written;
    return WriteFile(pImpl->handle, data.c_str(), data.size(), &written, NULL);
#else
    ssize_t n = ::write(pImpl->fd, data.c_str(), data.size());
    return n == static_cast<ssize_t>(data.size());
#endif
}

std::string SerialInterface::read(size_t maxBytes) {
    if (!isOpen()) return "";
    
    std::vector<char> buffer(maxBytes);
    
#ifdef _WIN32
    DWORD bytesRead;
    if (ReadFile(pImpl->handle, buffer.data(), maxBytes, &bytesRead, NULL)) {
        return std::string(buffer.data(), bytesRead);
    }
#else
    ssize_t n = ::read(pImpl->fd, buffer.data(), maxBytes);
    if (n > 0) {
        return std::string(buffer.data(), n);
    }
#endif
    
    return "";
}

void SerialInterface::setDataCallback(DataCallback cb) {
    pImpl->onData = std::move(cb);
}

void SerialInterface::setErrorCallback(ErrorCallback cb) {
    pImpl->onError = std::move(cb);
}

void SerialInterface::startReadLoop() {
    if (pImpl->running || !isOpen()) return;
    
    pImpl->running = true;
    pImpl->readThread = std::thread([this]() {
        while (pImpl->running) {
            std::string data = read(256);
            if (!data.empty() && pImpl->onData) {
                pImpl->onData(data);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}

void SerialInterface::stopReadLoop() {
    pImpl->running = false;
    if (pImpl->readThread.joinable()) {
        pImpl->readThread.join();
    }
}

bool SerialInterface::setBaudRate(int baudRate) {
    pImpl->currentBaud = baudRate;
    if (isOpen()) {
        std::string port = pImpl->portName;
        close();
        return open(port, baudRate);
    }
    return true;
}

bool SerialInterface::setDataBits(int bits) {
    // Implementation for data bits configuration
    return true;
}

bool SerialInterface::setParity(char parity) {
    // Implementation for parity configuration
    return true;
}

bool SerialInterface::setStopBits(int bits) {
    // Implementation for stop bits configuration
    return true;
}

} // namespace hw_analyzer

# HW Analyzer Backend

Cross-platform C++ backend for serial/I²C/SPI communication.

## Building

### Windows
```powershell
.\scripts\build_backend.bat
```

### Linux/macOS
```bash
chmod +x scripts/build_backend.sh
./scripts/build_backend.sh
```

## Requirements

- CMake >= 3.15
- C++17 compatible compiler
- Windows: Visual Studio 2019+ or MinGW
- Linux: GCC 7+ or Clang 5+
- macOS: Xcode Command Line Tools

## Running

```bash
./backend/build/hw_analyzer_backend
```

The backend will start a WebSocket server on `ws://localhost:9001`.

## Architecture

- `serial_interface.cpp/hpp` - Cross-platform serial port communication
- `websocket_server.hpp` - Lightweight WebSocket server for IPC
- `main.cpp` - Server entry point and message routing

## API

### WebSocket Messages

**List Ports:**
```json
{"cmd": "list"}
```

**Open Port:**
```json
{"cmd": "open", "port": "COM3", "baud": 115200}
```

**Write Data:**
```json
{"cmd": "write", "data": "Hello\\n"}
```

**Close Port:**
```json
{"cmd": "close"}
```

### Responses

**Port List:**
```json
{"type": "ports", "data": [{"name": "COM3", "desc": "Serial Port"}]}
```

**Received Data:**
```json
{"type": "rx", "data": "received text"}
```

**Status:**
```json
{"type": "status", "message": "Port opened successfully"}
```

**Error:**
```json
{"type": "error", "message": "Failed to open port"}
```

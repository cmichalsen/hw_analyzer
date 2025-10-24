# 🚀 Quick Start Guide

## First Time Setup

### 1. Build the C++ Backend

**Windows:**
```powershell
.\scripts\build_backend.bat
```

**Expected output:**
```
Building HW Analyzer Backend...
...
Backend built successfully!
Binary location: backend\build\Release\hw_analyzer_backend.exe
```

### 2. Test the Backend

Start the backend manually to verify it works:

```powershell
.\backend\build\Release\hw_analyzer_backend.exe
```

You should see:
```
HW Analyzer Backend Starting...
WebSocket server will listen on ws://localhost:9001
Backend ready. Waiting for connections...
WebSocket server listening on port 9001
```

**Leave this running** and open a new terminal for the next step.

### 3. Run the Electron App

In a new terminal:

```bash
bun run dev
```

This starts three processes:
- **BE (Backend)** - C++ WebSocket server
- **SK (SvelteKit)** - Frontend dev server  
- **EL (Electron)** - Desktop app

### 4. Use the App

1. The app window opens automatically
2. Select a serial port from the dropdown
3. Choose baud rate (default: 115200)
4. Click **Connect**
5. Switch between **Console** and **Live Graph** tabs

## Common Issues

### ❌ Backend not built

**Error:** `Backend not built. Run: npm run backend:build`

**Solution:**
```bash
npm run backend:build
```

### ❌ Failed to connect to backend

**Error:** `Failed to connect to backend. Make sure the backend is running.`

**Solutions:**
1. Check if port 9001 is available
2. Manually start backend: `.\backend\build\Release\hw_analyzer_backend.exe`
3. Check firewall isn't blocking localhost:9001

### ❌ No ports listed

**Possible causes:**
- No serial devices connected
- Drivers not installed
- Permissions issue (Linux: add user to `dialout` group)

**Linux fix:**
```bash
sudo usermod -a -G dialout $USER
# Then log out and back in
```

### ❌ CMake not found

**Windows:** Install via Visual Studio Installer or download from cmake.org

**Linux:**
```bash
sudo apt install cmake
```

**macOS:**
```bash
brew install cmake
```

## Development Workflow

### Normal Development

```bash
bun run dev
```

Everything starts automatically with hot reload enabled.

### Backend-Only Changes

If you only modified C++ code:

1. Stop the dev server (Ctrl+C)
2. Rebuild backend: `npm run backend:build`
3. Restart: `bun run dev`

### Frontend-Only Changes

Changes to Svelte/TypeScript files auto-reload - no restart needed!

## Debugging

### Debug Everything at Once

1. Press **F5** in VS Code
2. Wait for all processes to start (~5 seconds)
3. Set breakpoints in any `.ts` or `.svelte` file
4. Interact with the app to trigger breakpoints

### Debug Backend (C++)

Use your C++ debugger:

**Visual Studio:**
1. Open `backend/` in Visual Studio
2. Set breakpoints in `.cpp` files
3. Debug > Attach to Process
4. Select `hw_analyzer_backend.exe`

**VS Code (C++):**
1. Install C/C++ extension
2. Add launch config for native debugging
3. Attach to running process

## Next Steps

- Connect a serial device and test communication
- Try the live graph with sensor data
- Explore the codebase in `src/renderer/src/`
- Check out [Notes.md](Notes.md) for the full roadmap

## Testing Without Hardware

### Create Virtual Serial Ports

**Windows (com0com):**
1. Download from sourceforge
2. Install and create COM3-COM4 pair
3. Use one for HW Analyzer, test with the other

**Linux (socat):**
```bash
# Terminal 1: Create virtual ports
socat -d -d pty,raw,echo=0 pty,raw,echo=0
# Note the paths like /dev/pts/2 and /dev/pts/3

# Terminal 2: Send test data
echo "temp:42.5" > /dev/pts/3

# Connect to /dev/pts/2 in HW Analyzer
```

**macOS:**
```bash
# Similar to Linux using socat from Homebrew
brew install socat
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

## Performance Tips

- **Large datasets:** Increase max data points in Live Graph settings
- **High baud rates:** Use Release build of backend for better performance
- **Multiple devices:** Each connection needs a separate backend instance (future feature)

---

**Need help?** Open an issue on GitHub!

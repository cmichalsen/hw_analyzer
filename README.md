# 🧭 HW Analyzer

A modern, cross-platform Serial / I²C / SPI debugging tool built with **Electron**, **SvelteKit**, and **C++**.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## ✨ Features

- ✅ **Serial Port Terminal** - Connect, read, and write to serial devices
- ✅ **Live Console** - Real-time data display with timestamps
- ✅ **Data Visualization** - Live graphing of numeric serial data
- ✅ **Cross-Platform** - Windows, macOS, and Linux support
- ✅ **Modern UI** - Built with SvelteKit and TailwindCSS
- ✅ **Full Debugging Support** - Breakpoints work in all layers
- 🚧 **I²C Support** - Coming soon
- 🚧 **SPI Support** - Coming soon

## 🚀 Quick Start

### Prerequisites

**Windows:**
- Visual Studio 2019+ with C++ Desktop Development workload, OR MinGW-w64
- CMake 3.15+
- Node.js 18+ or Bun
- Git

**Linux:**
```bash
sudo apt install build-essential cmake git
```

**macOS:**
```bash
xcode-select --install
brew install cmake
```

### Installation

```bash
# Clone and install
git clone https://github.com/cmichalsen/hw_analyzer.git
cd hw_analyzer

# Install dependencies (using bun or npm)
bun install
# or: npm install

# Build C++ backend
.\scripts\build_backend.bat  # Windows
# or: ./scripts/build_backend.sh  # Linux/macOS
```

### Running

```bash
bun run dev
# or: npm run dev
```

The app will automatically:
1. Start the C++ backend on `ws://localhost:9001`
2. Start SvelteKit dev server on `http://localhost:5173`
3. Launch Electron window

## 📦 Building for Production

```bash
bun run build        # Build everything
bun run build:win    # Package for Windows
bun run build:mac    # Package for macOS  
bun run build:linux  # Package for Linux
```

## 🏗️ Architecture

```
hw_analyzer/
├── backend/              # C++ backend (WebSocket + serial I/O)
├── src/main/             # Electron main process
├── src/preload/          # Electron preload
├── src/renderer/         # SvelteKit frontend
└── scripts/              # Build scripts
```

| Layer | Technology |
|-------|------------|
| Frontend | SvelteKit + Svelte 5 + TailwindCSS |
| Desktop | Electron |
| Backend | C++17 (cross-platform serial) |
| IPC | WebSocket |
| Charts | Chart.js |

## 🔌 Using the App

1. **Connect Backend** - App auto-connects to `ws://localhost:9001`
2. **Select Port** - Choose from available serial ports
3. **Set Baud Rate** - Default is 115200
4. **Connect** - Click Connect button
5. **View Data** - Switch between Console and Live Graph tabs
6. **Send Commands** - Type in input field and press Enter

### Live Graphing

The Live Graph tab automatically parses numeric values from incoming serial data:
- Extracts numbers from text (e.g., "temp: 42.5" → plots 42.5)
- Auto-scrolling graph with configurable data points
- Pause/Resume and Clear controls

## 🛠️ Development

### Debugging (All Configured! ✅)

Press **F5** in VS Code to start debugging with:
- SvelteKit dev server breakpoints
- Electron main process debugging  
- Chrome DevTools for renderer

### Project Structure

```
backend/src/
  ├── main.cpp                 # WebSocket server + routing
  └── serial_interface.cpp     # Cross-platform serial I/O

src/renderer/src/lib/
  ├── backend.ts               # WebSocket client
  └── components/
      ├── PortSelector.svelte  # Port selection UI
      ├── SerialConsole.svelte # Terminal console
      └── LiveChart.svelte     # Data visualization
```

## 🧪 Testing

Use virtual serial ports for testing:

**Windows:** `com0com`  
**Linux:** 
```bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

Send test data:
```bash
echo "42.5" > /dev/ttyUSB0
```

## 🗺️ Roadmap

- [x] Serial port support ✅
- [x] Live console ✅
- [x] Data visualization ✅
- [ ] I²C scanning 🚧
- [ ] SPI support 🚧
- [ ] Protocol decoding 🔮
- [ ] Session logging 🔮

See [Notes.md](Notes.md) for detailed roadmap.

## 📄 License

MIT License - see LICENSE file

## 🙏 Acknowledgments

Inspired by Saleae Logic and CoolTerm - built to be open, modern, and extensible.

---

Made with ❤️ by [cmichalsen](https://github.com/cmichalsen)

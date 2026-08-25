<p align="center">
  <img width="120" height="120" alt="TimeEngine Logo" src="https://github.com/user-attachments/assets/97592994-22ae-4371-8024-3b3c35344666" />
</p>

<h1 align="center">TimeEngine</h1>

<p align="center">
  <strong>An Integrated Game Development Environment (IGDE) — eliminating multi-app workflow bloat by unifying engine runtime, asset design, code IDE, and profiling into one performant workspace.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Architecture-IGDE-purple.svg" alt="Integrated Game Development Environment">
  <img src="https://img.shields.io/badge/Language-C%2B%2B%20Latest-blue.svg" alt="C++ Latest">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg" alt="Windows | Linux | macOS">
  <img src="https://img.shields.io/badge/Graphics-OpenGL%20%7C%20Vulkan%20%7C%20Metal%20%7C%20DirectX%2011%20%7C%20OpenGL%20ES-orange.svg" alt="Graphics">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License MIT">
</p>

---

## 📌 Overview

**TimeEngine** is a cross-platform **Integrated Game Development Environment (IGDE)** designed for 2D game development and deterministic simulation research.

### 💡 The IGDE Vision
Traditional game development forces creators to constantly context-switch between fragmented external software — digital art suites (Photoshop, Inkscape), external code IDEs, particle editors, and separate engine runtimes. 

**TimeEngine aims to eliminate this multi-app overhead** by building an all-in-one, high-performance in-editor ecosystem where designers and programmers can author code, craft procedural 2D assets, profile memory/frame timings, and execute game logic inside a single native workspace.

> [!NOTE]  
> **Development Status:** The full IGDE multi-app replacement vision (in-engine vector/raster art creation) and deterministic time manipulation (snapshot rewind & time dilation) are core architectural goals currently under active development. See [ROADMAP.md](ROADMAP.md).

---

## 🖼️ Editor Showcase

<p align="center">
  <img width="100%" alt="Time Engine Editor Workspace" src="https://github.com/user-attachments/assets/d48599c2-300e-4cfb-9022-af682d60252f" />
</p>

<p align="center">
  <img width="49%" alt="Editor Viewport & UI" src="https://github.com/user-attachments/assets/8e2ae8d9-fd04-4680-a0cb-19b1c76fbfdd" />
  <img width="49%" alt="Performance Profiler Dashboard" src="https://github.com/user-attachments/assets/e794acb5-7ada-4c5e-a673-c88d68d6be7c" />
</p>

<p align="center">
  <img width="49%" alt="Sprite IDE View 1" src="https://github.com/user-attachments/assets/ee8db0e9-4f17-4b33-b2a4-bfd60151338a" />
  <img width="49%" alt="Sprite IDE View 2" src="https://github.com/user-attachments/assets/0bbb0594-019a-4173-8ade-cfdd333836f4" />
</p>
<p align="center">
  <img width="49%" alt="Sprite IDE View 3" src="https://github.com/user-attachments/assets/73eb1567-28cc-407b-bec5-3bb0841fad17" />
  <img width="49%" alt="Sprite IDE View 4" src="https://github.com/user-attachments/assets/9d5f8098-c56d-4514-9f8f-f80dec346490" />
</p>

---

## 🚀 Getting Started

### Prerequisites
* **C++ Compiler**: Modern C++ compiler with `C++latest` (`C++20`/`C++23`) support:
  * **Windows**: MSVC (Visual Studio 2022+), Clang, or MinGW-w64
  * **macOS**: Xcode (Apple Clang) or Makefiles / LLVM
  * **Linux**: GCC (`g++` 11+) or Clang
* **Git** (with submodule support)

### Setup & Building from Source

1. **Clone the repository**:
   ```bash
   git clone --recursive https://github.com/1SHAMAY1/TimeEngine.git
   cd TimeEngine
   ```

2. **Initialize Submodules & Build Tools** (Required for all platforms):
   ```bash
   ./Scripts/SetupSubmodules.sh
   ```

3. **Generate Workspace / Project Files**:
   * **Windows (MSVC)**: `Scripts\Windows\MSVC\GenerateProjectFiles.bat`
   * **Windows (MinGW)**: `Scripts\Windows\MinGW\GenerateProjectFiles.bat`
   * **Windows (Clang)**: `Scripts\Windows\Clang\GenerateProjectFiles.bat`
   * **Linux**: `Scripts/Linux/GenerateProjectFiles.sh`
   * **macOS**: `Scripts/Mac/GenerateProjectFiles.sh`

4. **Build & Launch**:
   Open the generated project solution/workspace or run the respective platform build script (e.g., `Scripts/Windows/MSVC/BuildDebug.bat` or `Scripts/Linux/BuildDebug.sh`). Launch `TimeEditor` executable to enter the workspace.

---

## 📚 Documentation & Developer Guides

* 🏛️ **[Docs/ARCHITECTURE.md](Docs/ARCHITECTURE.md)** — Core subsystems, ECS components, rendering backends, vendor wrappers, and MCP automation specs.
* 📄 **[Docs/ROADMAP.md](Docs/ROADMAP.md)** — Feature milestones and time-manipulation architecture plans.
* 🤖 **[llms.txt](llms.txt)** / **[llms.md](llms.md)** — LLM architecture summary, coding rules, vendor wrapping guidelines, and MCP server tools.
* 🤝 **[.github/CONTRIBUTING.md](.github/CONTRIBUTING.md)** — Guidelines for code style, submodules, and PR submissions.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

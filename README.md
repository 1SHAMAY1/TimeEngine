<p align="center">
  <img width="120" height="120" alt="TimeEngine Logo" src="https://github.com/user-attachments/assets/97592994-22ae-4371-8024-3b3c35344666" />
</p>

<h1 align="center">TimeEngine</h1>

<p align="center">
  <strong>An Integrated Game Development Environment (IGDE) — eliminating multi-app workflow bloat by unifying engine runtime, asset design, code IDE, and profiling into one performant workspace.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Architecture-IGDE-purple.svg" alt="Integrated Game Development Environment">
  <img src="https://img.shields.io/badge/Language-C%2B%2B20-blue.svg" alt="C++20">
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
> **Development Status:** The full IGDE multi-app replacement vision (in-engine vector/raster art creation) and deterministic time manipulation (snapshot rewind & time dilation) are core architectural goals currently under active development. See [ROADMAP.md](Docs/ROADMAP.md).

---

## 🖼️ Editor Showcase

| Project Launcher & Hub | Integrated Workspace |
| :---: | :---: |
| <img src="https://github.com/user-attachments/assets/3726c10f-d31a-4d97-a73b-795d96537a81" alt="Project Launcher Hub" width="100%" /> | <img src="https://github.com/user-attachments/assets/4b1bc364-4672-4a9e-9f8f-5c657f8c2806" alt="Editor Workspace" width="100%" /> |
| **Viewport & Retained UI** | **Performance & Profiler Dashboard** |
| <img src="https://github.com/user-attachments/assets/83025fbb-cf6e-426f-9283-a2ef6a475574" alt="Viewport and UI" width="100%" /> | <img src="https://github.com/user-attachments/assets/27e7108a-7ee5-4650-9cb2-da97b523afef" alt="Performance Profiler Dashboard" width="100%" /> |

---

## 🚀 Getting Started

### Prerequisites
* **C++20 Compliant Compiler**:
  * **Windows**: MSVC (Visual Studio 2022 v143+), MinGW-w64 (GCC 13+), or LLVM Clang-cl (17+)
  * **macOS**: Apple Clang / Xcode 15+ (macOS 13+)
  * **Linux**: GCC 13+ or Clang 17+
* **Build Tools**: CMake 3.20+ and Git

---

### Setup & Building from Source

1. **Clone the repository**:
   ```bash
   git clone --recursive https://github.com/1SHAMAY1/TimeEngine.git
   cd TimeEngine
   ```

2. **Initialize Submodules** (Required for all platforms):
   ```bash
   bash Scripts/SetupSubmodules.sh
   ```

3. **Generate Workspace & Project Files**:
   * **Windows (Visual Studio / MSVC)**:
     ```cmd
     Scripts\Windows\MSVC\GenerateProjectFiles.bat
     ```
   * **Windows (MinGW / GCC)**:
     ```cmd
     Scripts\Windows\MinGW\GenerateProjectFiles.bat
     ```
   * **Windows (LLVM Clang-cl)**:
     ```cmd
     Scripts\Windows\Clang\GenerateProjectFiles.bat
     ```
   * **macOS (Xcode)**:
     ```bash
     bash Scripts/Mac/Xcode/GenerateProjectFiles.sh
     ```
   * **macOS (Makefiles / Apple Clang)**:
     ```bash
     bash Scripts/Mac/Makefiles/GenerateProjectFiles.sh
     ```
   * **Linux (Makefiles / GCC & Clang)**:
     ```bash
     bash Scripts/Linux/GenerateProjectFiles.sh
     ```

4. **Build the Engine & Editor**:
   * **Windows (MSVC)**:
     * Debug: `Scripts\Windows\MSVC\BuildDebug.bat`
     * Commercial Release: `Scripts\Windows\MSVC\BuildCommercialRelease.bat`
   * **Windows (MinGW)**:
     * Debug: `Scripts\Windows\MinGW\BuildDebug.bat`
     * Commercial Release: `Scripts\Windows\MinGW\BuildCommercialRelease.bat`
   * **Windows (Clang)**:
     * Debug: `Scripts\Windows\Clang\BuildDebug.bat`
     * Commercial Release: `Scripts\Windows\Clang\BuildCommercialRelease.bat`
   * **macOS (Xcode)**:
     * Debug: `bash Scripts/Mac/Xcode/BuildDebug.sh`
     * Commercial Release: `bash Scripts/Mac/Xcode/BuildCommercialRelease.sh`
   * **macOS (Makefiles)**:
     * Debug: `bash Scripts/Mac/Makefiles/BuildDebug.sh`
     * Commercial Release: `bash Scripts/Mac/Makefiles/BuildCommercialRelease.sh`
   * **Linux (Makefiles)**:
     * Debug: `bash Scripts/Linux/BuildDebug.sh`
     * Commercial Release: `bash Scripts/Linux/BuildCommercialRelease.sh`

5. **Clean & Maintenance Scripts**:
   * **Windows Clean**: `Scripts\Windows\CleanProjectFiles.bat`
   * **macOS Clean**: `bash Scripts/Mac/Xcode/CleanProjectFiles.sh` or `bash Scripts/Mac/Makefiles/CleanProjectFiles.sh`
   * **Linux Clean**: `bash Scripts/Linux/CleanProjectFiles.sh`
   * **OS File Associations (`.teproj` / `.tescene`)**:
     * Windows: `Scripts\Windows\RegisterFileExtension.bat`
     * macOS: `bash Scripts/Mac/Xcode/RegisterFileExtension.sh`
     * Linux: `bash Scripts/Linux/RegisterFileExtension.sh`

---

## 💬 Community & Discussions

* 💬 **[GitHub Discussions](https://github.com/1SHAMAY1/TimeEngine/discussions)** — Join our community to ask questions, share game showcases, pitch RFCs, and discuss engine architecture.
* 🤝 **[.github/CODE_OF_CONDUCT.md](.github/CODE_OF_CONDUCT.md)** — Our pledge and standards for a welcoming, inclusive community.

---

## 📚 Documentation & Developer Guides

* 🏛️ **[Docs/ARCHITECTURE.md](Docs/ARCHITECTURE.md)** — Core subsystems, ECS components, rendering backends, vendor wrappers, and MCP automation specs.
* 📄 **[Docs/ROADMAP.md](Docs/ROADMAP.md)** — Feature milestones and time-manipulation architecture plans.
* 🤖 **[llms.txt](llms.txt)** / **[llms.md](llms.md)** — LLM architecture summary, coding rules, vendor wrapping guidelines, and MCP server tools.
* 🤝 **[.github/CONTRIBUTING.md](.github/CONTRIBUTING.md)** — Guidelines for code style, submodules, and PR submissions.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

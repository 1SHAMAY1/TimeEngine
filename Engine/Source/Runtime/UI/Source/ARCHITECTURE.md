# UI Engine & UIAPI Subsystem Architecture

The **UI Engine** subsystem provides a modular, multi-backend abstract architecture for immediate-mode graphical user interfaces in TimeEngine, modeled closely after the engine's \RendererAPI\ pattern.

> [!NOTE]
> In short, the **UI Engine** subsystem decouples all game code, editor panels, and plugins from any specific third-party GUI library. Applications and editor layers interact with the static [\TimeGUI\](../../Include/Utils/TimeGUI.hpp) facade, which delegates lifecycle, render backend, and frame rendering to [\UIEngine\](../../Include/UI/UIEngine.hpp) and its active [\UIAPI\](../../Include/UI/UIAPI.hpp) backend (e.g. \ForgeUI\ or \DearImGui\).

---

## 🏛️ Architecture Overview

\\\mermaid
graph TD
    A[Editor Panels / Game Layers / Plugins] -->|Calls TimeGUI::*| B[TimeGUI Facade]
    B -->|Delegates Lifecycle & Render| C[UIEngine (Singleton Subsystem)]
    C -->|Owns & Dispatches to| D[UIAPI (Abstract Base)]
    D -->|Concrete Backend| E[ForgeUIAPI (Default Backend)]
    D -->|Concrete Backend| F[ImGuiUIAPI (Fallback Backend)]
    E -->|Vendor| G[Vendor/ForgeUI]
    F -->|Vendor| H[Vendor/IMGUI]
\\\

---

## 🔑 Core Components

### 1. \UIBackendType\ Enum
Defined in [\Engine/Include/UI/UIBackendType.hpp\](file:///e:/TimeEngine/Engine/Include/UI/UIBackendType.hpp):
- \UIBackendType::DearImGui\: Dear ImGui backend.
- \UIBackendType::ForgeUI\: ForgeUI backend (**Default**).

### 2. \UIAPI\ (Abstract Base Class)
Defined in [\Engine/Include/UI/UIAPI.hpp\](file:///e:/TimeEngine/Engine/Include/UI/UIAPI.hpp):
- **Lifecycle**: \Init(void* window)\, \Shutdown()\, \InitRenderBackend()\, \ShutdownRenderBackend()\.
- **Multithreading**: \BindThreadContext()\ binds thread-local GUI contexts so UI layout worker threads and dedicated render threads always retain their valid context.
- **Frame Pipeline**: \PrepareFrame()\, \BeginFrame(w, h)\, \EndFrame(w, h)\, \RenderDrawData(void* drawData)\.
- **Factory**: \static TEScope<UIAPI> Create(UIBackendType type)\ instantiates the requested backend.

### 3. Concrete Backends (Zero Header Leakage)
- **\ForgeUIAPI\** (\Engine/src/UI/ForgeUI/\): Encapsulates all \<ForgeUI/ForgeUI.h>\ calls.
- **\ImGuiUIAPI\** (\Engine/src/UI/ImGui/\): Encapsulates all \<imgui.h>\, \<backends/imgui_impl_glfw.h>\, and \<backends/imgui_impl_opengl3.h>\ calls.
- Backends **never** call \TimeGUI\ and are strictly isolated by Premake rules.

### 4. \UIEngine\ (Subsystem Singleton)
Defined in [\Engine/Include/UI/UIEngine.hpp\](file:///e:/TimeEngine/Engine/Include/UI/UIEngine.hpp) and [\Engine/src/UI/UIEngine.cpp\](file:///e:/TimeEngine/Engine/src/UI/UIEngine.cpp):
- Owns \TEScope<UIAPI> m_ActiveUIAPI\.
- Initialized once during \TimeGUILayer::OnAttach(window)\.

### 5. \ProjectUISettings\ (Project Settings Integration)
Defined in [\Engine/Include/Editor/Settings/ProjectUISettings.hpp\](file:///e:/TimeEngine/Engine/Include/Editor/Settings/ProjectUISettings.hpp):
- Exposes UI Backend selection dropdown in Project Settings ("User Interface\ $\rightarrow$ "UI Engine & Backend\).

---

## 🧵 Multithreading Safety
Because TimeEngine renders on a dedicated thread and executes UI tasks across worker thread pools:
1. \UIAPI::BindThreadContext()\ ensures \orge::SetCurrentContext\ / \ImGui::SetCurrentContext\ is invoked on the active executing thread.
2. Frame data produced by \EndFrame()\ is safely passed to \RenderDrawData()\ on the render thread.

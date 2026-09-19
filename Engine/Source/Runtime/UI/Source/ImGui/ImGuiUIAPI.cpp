#include "PreRequisites.h"
#include "ImGuiUIAPI.hpp"
#include "Log.h"
#include "Utils/TEFileSystem.hpp"
#include "RendererContext.hpp"
#include "GraphicsAPI.hpp"
#include "IWindow.hpp"
#include "backends/imgui_impl_glfw.h"
#ifdef TE_SUPPORT_OPENGL
#include "backends/imgui_impl_opengl3.h"
#endif
#ifdef TE_SUPPORT_METAL
#include "ImGuiMetalBackend.hpp"
#endif
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
#include "backends/imgui_impl_dx11.h"
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#endif
#include "imgui.h"
#include "imgui_internal.h"
#include <GLFW/glfw3.h>

// Guarantee clean enum alignment with ImGui native color indices
static_assert((int)UIStyleCol_COUNT == (int)ImGuiCol_COUNT, "UIStyleCol_ enum must match ImGuiCol_ enum size exactly!");

// -------------------------------------------------------------------------
// Helper Conversions (Strictly Local)
// -------------------------------------------------------------------------
static inline ImVec2 ToImVec2(const TEVector2 &v) { return ImVec2(v.x, v.y); }
static inline TEVector2 FromImVec2(const ImVec2 &v) { return TEVector2(v.x, v.y); }
static inline ImVec4 ToImVec4(const TEVector4 &v) { return ImVec4(v.x, v.y, v.z, v.w); }
static inline ImVec4 ToImVec4(const TEColor &c) { return ImVec4(c.r, c.g, c.b, c.a); }

bool ImGuiUIAPI::Init(void *nativeWindow)
{
    if (m_Initialized)
    {
        TE_CORE_WARN("ImGuiUIAPI: already initialized.");
        return true;
    }

    IMGUI_CHECKVERSION();
    m_Context = static_cast<void *>(ImGui::CreateContext());
    ImGui::SetCurrentContext(static_cast<ImGuiContext *>(m_Context));

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Multi-viewports disabled for OpenGL dedicated render thread stability
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigInputTextCursorBlink = true;

    // Direct layout to Config/EngineLayout.ini rather than root imgui.ini
    static TEString s_EngineLayoutPath = "Config/EngineLayout.ini";
    TEFileSystem::CreateDirectories("Config");
    io.IniFilename = s_EngineLayoutPath.c_str();

    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_InputTextCursor] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    GLFWwindow *window = static_cast<GLFWwindow *>(nativeWindow);
    if (!window)
    {
        TE_CORE_ERROR("ImGuiUIAPI: null GLFWwindow pointer.");
        return false;
    }

    GraphicsAPI currentAPI = RendererContext::GetAPI();
    bool glfwInitSuccess = false;

    if (currentAPI == GraphicsAPI::OpenGL || currentAPI == GraphicsAPI::OpenGLES)
    {
#ifdef TE_SUPPORT_OPENGL
        glfwInitSuccess = ImGui_ImplGlfw_InitForOpenGL(window, true);
#else
        glfwInitSuccess = ImGui_ImplGlfw_InitForOther(window, true);
#endif
    }
    else
    {
        glfwInitSuccess = ImGui_ImplGlfw_InitForOther(window, true);
    }

    if (!glfwInitSuccess)
    {
        TE_CORE_ERROR("ImGuiUIAPI: ImGui_ImplGlfw initialization failed.");
        return false;
    }

#ifdef TE_SUPPORT_METAL
    if (currentAPI == GraphicsAPI::Metal)
    {
        if (!ImGui_InitMetalBackend(window))
        {
            TE_CORE_ERROR("ImGuiUIAPI: ImGui_InitMetalBackend failed.");
            return false;
        }
    }
#endif

    m_Initialized = true;
    SyncStyleFromImGui();
    SyncIOFromImGui();
    TE_CORE_INFO("ImGuiUIAPI: initialized.");
    return true;
}

void ImGuiUIAPI::SyncIOFromImGui()
{
    if (!m_Initialized)
        return;
    ImGuiIO &io = ImGui::GetIO();
    m_IO.DeltaTime = io.DeltaTime;
    m_IO.MouseWheel = io.MouseWheel;
    m_IO.MouseDelta = TEVector2(io.MouseDelta.x, io.MouseDelta.y);
    m_IO.DisplaySize = TEVector2(io.DisplaySize.x, io.DisplaySize.y);
    m_IO.KeyShift = io.KeyShift;
    m_IO.KeyCtrl = io.KeyCtrl;
    m_IO.KeyAlt = io.KeyAlt;
    m_IO.KeySuper = io.KeySuper;
    m_IO.WantTextInput = io.WantTextInput;
    m_IO.WantCaptureKeyboard = io.WantCaptureKeyboard;
    m_IO.WantCaptureMouse = io.WantCaptureMouse;
    m_IO.ConfigFlags = (int)io.ConfigFlags;
    m_IO.DefaultFont.nativeFont = io.FontDefault;
}

void ImGuiUIAPI::SyncStyleToImGui()
{
    if (!m_Initialized)
        return;
    ImGuiStyle &imStyle = ImGui::GetStyle();
    imStyle.WindowRounding = m_Style.WindowRounding;
    imStyle.ChildRounding = m_Style.ChildRounding;
    imStyle.FrameRounding = m_Style.FrameRounding;
    imStyle.PopupRounding = m_Style.PopupRounding;
    imStyle.TabRounding = m_Style.TabRounding;
    imStyle.GrabRounding = m_Style.GrabRounding;
    imStyle.ScrollbarRounding = m_Style.ScrollbarRounding;
    imStyle.WindowBorderSize = m_Style.WindowBorderSize;
    imStyle.FrameBorderSize = m_Style.FrameBorderSize;
    imStyle.PopupBorderSize = m_Style.PopupBorderSize;
    imStyle.IndentSpacing = m_Style.IndentSpacing;
    imStyle.ItemSpacing = ToImVec2(m_Style.ItemSpacing);
    imStyle.FramePadding = ToImVec2(m_Style.FramePadding);
    imStyle.WindowPadding = ToImVec2(m_Style.WindowPadding);
    imStyle.AntiAliasedLines = m_Style.AntiAliasedLines;
    imStyle.AntiAliasedFill = m_Style.AntiAliasedFill;

    for (int i = 0; i < ImGuiCol_COUNT && i < UIStyleCol_COUNT; ++i)
    {
        const auto &col = m_Style.Colors[i];
        imStyle.Colors[i] = ImVec4(col.r, col.g, col.b, col.a);
    }
}

void ImGuiUIAPI::SyncStyleFromImGui()
{
    if (!m_Initialized)
        return;
    ImGuiStyle &imStyle = ImGui::GetStyle();
    m_Style.WindowRounding = imStyle.WindowRounding;
    m_Style.ChildRounding = imStyle.ChildRounding;
    m_Style.FrameRounding = imStyle.FrameRounding;
    m_Style.PopupRounding = imStyle.PopupRounding;
    m_Style.TabRounding = imStyle.TabRounding;
    m_Style.GrabRounding = imStyle.GrabRounding;
    m_Style.ScrollbarRounding = imStyle.ScrollbarRounding;
    m_Style.WindowBorderSize = imStyle.WindowBorderSize;
    m_Style.FrameBorderSize = imStyle.FrameBorderSize;
    m_Style.PopupBorderSize = imStyle.PopupBorderSize;
    m_Style.IndentSpacing = imStyle.IndentSpacing;
    m_Style.ItemSpacing = FromImVec2(imStyle.ItemSpacing);
    m_Style.FramePadding = FromImVec2(imStyle.FramePadding);
    m_Style.WindowPadding = FromImVec2(imStyle.WindowPadding);
    m_Style.AntiAliasedLines = imStyle.AntiAliasedLines;
    m_Style.AntiAliasedFill = imStyle.AntiAliasedFill;

    for (int i = 0; i < ImGuiCol_COUNT && i < UIStyleCol_COUNT; ++i)
    {
        const auto &col = imStyle.Colors[i];
        m_Style.Colors[i] = TEColor(col.x, col.y, col.z, col.w);
    }
}

UIIO &ImGuiUIAPI::GetIO()
{
    SyncIOFromImGui();
    return m_IO;
}

UIStyle &ImGuiUIAPI::GetStyle() { return m_Style; }

void ImGuiUIAPI::SetStyle(const UIStyle &style)
{
    m_Style = style;
    SyncStyleToImGui();
}

void ImGuiUIAPI::Shutdown()
{
    if (!m_Initialized)
        return;

    if (m_RenderInitialized)
        ShutdownRenderBackend();

#ifdef TE_SUPPORT_METAL
    ImGui_ShutdownMetalBackend();
#endif
    ImGui_ImplGlfw_Shutdown();
    if (m_Context)
    {
        ImGui::DestroyContext(static_cast<ImGuiContext *>(m_Context));
        m_Context = nullptr;
    }

    m_Initialized = false;
    TE_CORE_INFO("ImGuiUIAPI: shut down.");
}

bool ImGuiUIAPI::InitRenderBackend()
{
    if (m_RenderInitialized)
        return true;

    GraphicsAPI currentAPI = RendererContext::GetAPI();

    if (currentAPI == GraphicsAPI::DirectX11)
    {
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
        DX11Context &ctx = DX11Context::Get();
        if (!ctx.Device || !ctx.DeviceContext)
        {
            TE_CORE_ERROR("ImGuiUIAPI: DX11 device not ready for ImGui render backend init.");
            return false;
        }
        if (!ImGui_ImplDX11_Init(ctx.Device, ctx.DeviceContext))
        {
            TE_CORE_ERROR("ImGuiUIAPI: ImGui_ImplDX11_Init failed.");
            return false;
        }
        m_RenderInitialized = ImGui_ImplDX11_CreateDeviceObjects();
        if (m_RenderInitialized)
            TE_CORE_INFO("ImGuiUIAPI: DirectX11 render backend initialized.");
        return m_RenderInitialized;
#else
        TE_CORE_ERROR("ImGuiUIAPI: DirectX 11 is not supported on this platform build.");
        return false;
#endif
    }
    else if (currentAPI == GraphicsAPI::OpenGL || currentAPI == GraphicsAPI::OpenGLES)
    {
#ifdef TE_SUPPORT_OPENGL
        const char *glsl_version = (currentAPI == GraphicsAPI::OpenGLES) ? "#version 300 es" : "#version 410";
        if (!ImGui_ImplOpenGL3_Init(glsl_version))
        {
            TE_CORE_ERROR("ImGuiUIAPI: ImGui_ImplOpenGL3_Init failed.");
            return false;
        }

        m_RenderInitialized = ImGui_ImplOpenGL3_CreateDeviceObjects();
        if (m_RenderInitialized)
            TE_CORE_INFO("ImGuiUIAPI: OpenGL render backend initialized.");
        return m_RenderInitialized;
#else
        TE_CORE_ERROR("ImGuiUIAPI: OpenGL is not supported on this platform build.");
        return false;
#endif
    }
    else if (currentAPI == GraphicsAPI::Metal)
    {
#ifdef TE_SUPPORT_METAL
        m_RenderInitialized = true;
        return true;
#else
        return false;
#endif
    }

    return false;
}

void ImGuiUIAPI::ShutdownRenderBackend()
{
    if (!m_RenderInitialized)
        return;

    GraphicsAPI currentAPI = RendererContext::GetAPI();
    if (currentAPI == GraphicsAPI::DirectX11)
    {
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
        ImGui_ImplDX11_Shutdown();
#endif
    }
    else if (currentAPI == GraphicsAPI::OpenGL || currentAPI == GraphicsAPI::OpenGLES)
    {
#ifdef TE_SUPPORT_OPENGL
        ImGui_ImplOpenGL3_Shutdown();
#endif
    }
    else if (currentAPI == GraphicsAPI::Metal)
    {
#ifdef TE_SUPPORT_METAL
        ImGui_ShutdownMetalBackend();
#endif
    }
    m_RenderInitialized = false;
}

void ImGuiUIAPI::BindThreadContext()
{
    if (m_Context)
        ImGui::SetCurrentContext(static_cast<ImGuiContext *>(m_Context));
}

void ImGuiUIAPI::PrepareFrame()
{
    ImGui_ImplGlfw_NewFrame();
    GraphicsAPI currentAPI = RendererContext::GetAPI();
    if (currentAPI == GraphicsAPI::DirectX11)
    {
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
        ImGui_ImplDX11_NewFrame();
#endif
    }
    else if (currentAPI == GraphicsAPI::OpenGL || currentAPI == GraphicsAPI::OpenGLES)
    {
#ifdef TE_SUPPORT_OPENGL
        ImGui_ImplOpenGL3_NewFrame();
#endif
    }
    else if (currentAPI == GraphicsAPI::Metal)
    {
#ifdef TE_SUPPORT_METAL
        ImGui_PrepareMetalFrame();
#endif
    }
}

void ImGuiUIAPI::BeginFrame(uint32_t width, uint32_t height)
{
    BindThreadContext();

    ImGuiIO &io = ImGui::GetIO();
    if (width > 0 && height > 0)
    {
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    }

    SyncStyleToImGui();
    SyncIOFromImGui();

    ImGui::NewFrame();
}

void *ImGuiUIAPI::EndFrame(uint32_t width, uint32_t height)
{
    ImGuiIO &io = ImGui::GetIO();
    if (width > 0 && height > 0)
    {
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    }

    ImGui::Render();
    return static_cast<void *>(ImGui::GetDrawData());
}

void ImGuiUIAPI::RenderDrawData(void *drawData)
{
    if (!drawData)
        return;

    BindThreadContext();
    auto *data = static_cast<ImDrawData *>(drawData);
    GraphicsAPI currentAPI = RendererContext::GetAPI();

    if (currentAPI == GraphicsAPI::DirectX11)
    {
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
        ImGui_ImplDX11_RenderDrawData(data);
#endif
    }
    else if (currentAPI == GraphicsAPI::OpenGL || currentAPI == GraphicsAPI::OpenGLES)
    {
#ifdef TE_SUPPORT_OPENGL
        ImGui_ImplOpenGL3_RenderDrawData(data);
#endif
    }
    else if (currentAPI == GraphicsAPI::Metal)
    {
#ifdef TE_SUPPORT_METAL
        ImGui_RenderMetalDrawData(data);
#endif
    }

    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        void *backup_current_context = IWindow::GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        IWindow::MakeContextCurrent(backup_current_context);
    }
}

TEVector2 ImGuiUIAPI::GetMainViewportPos()
{
    if (auto *vp = ImGui::GetMainViewport())
        return TEVector2(vp->Pos.x, vp->Pos.y);
    return TEVector2(0.0f, 0.0f);
}

TEVector2 ImGuiUIAPI::GetMainViewportSize()
{
    if (auto *vp = ImGui::GetMainViewport())
        return TEVector2(vp->Size.x, vp->Size.y);
    return TEVector2(0.0f, 0.0f);
}

uint32_t ImGuiUIAPI::GetMainViewportID()
{
    if (auto *vp = ImGui::GetMainViewport())
        return vp->ID;
    return 0;
}

void *ImGuiUIAPI::GetBackgroundDrawList() { return static_cast<void *>(ImGui::GetBackgroundDrawList()); }
void *ImGuiUIAPI::GetForegroundDrawList() { return static_cast<void *>(ImGui::GetForegroundDrawList()); }
void *ImGuiUIAPI::GetWindowDrawList() { return static_cast<void *>(ImGui::GetWindowDrawList()); }

// -------------------------------------------------------------------------
// Windows & Containers
// -------------------------------------------------------------------------
bool ImGuiUIAPI::Begin(const TEString &name, bool *open, int flags)
{
    return ImGui::Begin(name.c_str(), open, (ImGuiWindowFlags)flags);
}

void ImGuiUIAPI::End() { ImGui::End(); }

bool ImGuiUIAPI::BeginChild(const TEString &strId, const TEVector2 &size, bool border, int flags)
{
    return ImGui::BeginChild(strId.c_str(), ToImVec2(size), border, (ImGuiWindowFlags)flags);
}

void ImGuiUIAPI::EndChild() { ImGui::EndChild(); }

bool ImGuiUIAPI::BeginMenuBar() { return ImGui::BeginMenuBar(); }

void ImGuiUIAPI::EndMenuBar() { ImGui::EndMenuBar(); }

bool ImGuiUIAPI::BeginMenu(const TEString &label, bool enabled) { return ImGui::BeginMenu(label.c_str(), enabled); }

void ImGuiUIAPI::EndMenu() { ImGui::EndMenu(); }

bool ImGuiUIAPI::MenuItem(const TEString &label, const TEString &shortcut, bool selected, bool enabled)
{
    return ImGui::MenuItem(label.c_str(), shortcut.IsEmpty() ? nullptr : shortcut.c_str(), selected, enabled);
}

bool ImGuiUIAPI::MenuItem(const TEString &label, const TEString &shortcut, bool *p_selected, bool enabled)
{
    return ImGui::MenuItem(label.c_str(), shortcut.IsEmpty() ? nullptr : shortcut.c_str(), p_selected, enabled);
}

// -------------------------------------------------------------------------
// Basic Widgets
// -------------------------------------------------------------------------
bool ImGuiUIAPI::Button(const TEString &label, const TEVector2 &size)
{
    return ImGui::Button(label.c_str(), ToImVec2(size));
}

bool ImGuiUIAPI::SmallButton(const TEString &label) { return ImGui::SmallButton(label.c_str()); }

bool ImGuiUIAPI::InvisibleButton(const TEString &strId, const TEVector2 &size, int flags)
{
    return ImGui::InvisibleButton(strId.c_str(), ToImVec2(size), (ImGuiButtonFlags)flags);
}

void ImGuiUIAPI::Text(const TEString &text) { ImGui::Text("%s", text.c_str()); }

void ImGuiUIAPI::TextUnformatted(const TEString &text) { ImGui::TextUnformatted(text.c_str()); }

void ImGuiUIAPI::TextColored(const TEColor &color, const TEString &text)
{
    ImGui::TextColored(ToImVec4(color), "%s", text.c_str());
}

void ImGuiUIAPI::TextDisabled(const TEString &text) { ImGui::TextDisabled("%s", text.c_str()); }

void ImGuiUIAPI::TextWrapped(const TEString &text) { ImGui::TextWrapped("%s", text.c_str()); }

bool ImGuiUIAPI::Checkbox(const TEString &label, bool *checked) { return ImGui::Checkbox(label.c_str(), checked); }

bool ImGuiUIAPI::RadioButton(const TEString &label, bool active) { return ImGui::RadioButton(label.c_str(), active); }

bool ImGuiUIAPI::RadioButton(const TEString &label, int *v, int v_button)
{
    return ImGui::RadioButton(label.c_str(), v, v_button);
}

// -------------------------------------------------------------------------
// Sliders & Drags
// -------------------------------------------------------------------------
bool ImGuiUIAPI::SliderFloat(const TEString &label, float *v, float v_min, float v_max, const TEString &format,
                             int flags)
{
    return ImGui::SliderFloat(label.c_str(), v, v_min, v_max, format.c_str(), (ImGuiSliderFlags)flags);
}

bool ImGuiUIAPI::SliderInt(const TEString &label, int *v, int v_min, int v_max, const TEString &format, int flags)
{
    return ImGui::SliderInt(label.c_str(), v, v_min, v_max, format.c_str(), (ImGuiSliderFlags)flags);
}

bool ImGuiUIAPI::DragFloat(const TEString &label, float *value, float speed, float min, float max,
                           const TEString &format, int flags)
{
    return ImGui::DragFloat(label.c_str(), value, speed, min, max, format.c_str(), (ImGuiSliderFlags)flags);
}

bool ImGuiUIAPI::DragFloat2(const TEString &label, float *v, float speed, float min, float max, const TEString &format,
                            int flags)
{
    return ImGui::DragFloat2(label.c_str(), v, speed, min, max, format.c_str(), (ImGuiSliderFlags)flags);
}

bool ImGuiUIAPI::DragFloat3(const TEString &label, float *v, float speed, float min, float max, const TEString &format,
                            int flags)
{
    return ImGui::DragFloat3(label.c_str(), v, speed, min, max, format.c_str(), (ImGuiSliderFlags)flags);
}

bool ImGuiUIAPI::DragFloat4(const TEString &label, float *v, float speed, float min, float max, const TEString &format,
                            int flags)
{
    return ImGui::DragFloat4(label.c_str(), v, speed, min, max, format.c_str(), (ImGuiSliderFlags)flags);
}

bool ImGuiUIAPI::DragInt(const TEString &label, int *v, float speed, int min, int max)
{
    return ImGui::DragInt(label.c_str(), v, speed, min, max);
}

static ImGuiInputTextFlags ConvertInputTextFlags(int flags)
{
    ImGuiInputTextFlags imFlags = ImGuiInputTextFlags_None;
    if (flags & (1 << 0))
        imFlags |= ImGuiInputTextFlags_CharsDecimal;
    if (flags & (1 << 1))
        imFlags |= ImGuiInputTextFlags_CharsHexadecimal;
    if (flags & (1 << 2))
        imFlags |= ImGuiInputTextFlags_CharsUppercase;
    if (flags & (1 << 3))
        imFlags |= ImGuiInputTextFlags_CharsNoBlank;
    if (flags & (1 << 4))
        imFlags |= ImGuiInputTextFlags_AutoSelectAll;
    if (flags & (1 << 5))
        imFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
    if (flags & (1 << 6))
        imFlags |= ImGuiInputTextFlags_CallbackCompletion;
    if (flags & (1 << 7))
        imFlags |= ImGuiInputTextFlags_CallbackHistory;
    if (flags & (1 << 8))
        imFlags |= ImGuiInputTextFlags_CallbackAlways;
    if (flags & (1 << 9))
        imFlags |= ImGuiInputTextFlags_CallbackCharFilter;
    if (flags & (1 << 10))
        imFlags |= ImGuiInputTextFlags_AllowTabInput;
    if (flags & (1 << 11))
        imFlags |= ImGuiInputTextFlags_CtrlEnterForNewLine;
    if (flags & (1 << 12))
        imFlags |= ImGuiInputTextFlags_NoHorizontalScroll;
    if (flags & (1 << 13))
        imFlags |= ImGuiInputTextFlags_AlwaysOverwrite;
    if (flags & (1 << 14))
        imFlags |= ImGuiInputTextFlags_ReadOnly;
    if (flags & (1 << 15))
        imFlags |= ImGuiInputTextFlags_Password;
    if (flags & (1 << 16))
        imFlags |= ImGuiInputTextFlags_NoUndoRedo;
    if (flags & (1 << 17))
        imFlags |= ImGuiInputTextFlags_CharsScientific;
    if (flags & (1 << 18))
        imFlags |= ImGuiInputTextFlags_CallbackResize;
    if (flags & (1 << 19))
        imFlags |= ImGuiInputTextFlags_CallbackEdit;
    if (flags & (1 << 20))
        imFlags |= ImGuiInputTextFlags_EscapeClearsAll;
    return imFlags;
}

// -------------------------------------------------------------------------
// Input Fields
// -------------------------------------------------------------------------
bool ImGuiUIAPI::InputText(const TEString &label, TEString &value, int flags)
{
    TEString temp = value;
    temp.Reserve(1024);
    if (ImGui::InputText(label.c_str(), temp.Data(), temp.Capacity(), ConvertInputTextFlags(flags)))
    {
        temp.SyncFromBuffer();
        value = temp;
        return true;
    }
    return false;
}

bool ImGuiUIAPI::InputText(const TEString &label, char *buf, size_t bufSize, int flags)
{
    return ImGui::InputText(label.c_str(), buf, bufSize, ConvertInputTextFlags(flags));
}

bool ImGuiUIAPI::InputTextWithHint(const TEString &label, const TEString &hint, TEString &value, int flags)
{
    TEString temp = value;
    temp.Reserve(1024);
    bool result = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), temp.Data(), temp.Capacity(),
                                           ConvertInputTextFlags(flags));
    temp.SyncFromBuffer();
    value = temp;
    return result;
}

bool ImGuiUIAPI::InputTextWithHint(const TEString &label, const char *hint, char *buf, size_t bufSize, int flags)
{
    return ImGui::InputTextWithHint(label.c_str(), hint, buf, bufSize, ConvertInputTextFlags(flags));
}

bool ImGuiUIAPI::InputTextMultiline(const TEString &label, TEString &value, const TEVector2 &size, int flags)
{
    TEString temp = value;
    temp.Reserve(4096);
    if (ImGui::InputTextMultiline(label.c_str(), temp.Data(), temp.Capacity(), ToImVec2(size),
                                  ConvertInputTextFlags(flags)))
    {
        temp.SyncFromBuffer();
        value = temp;
        return true;
    }
    return false;
}

bool ImGuiUIAPI::InputTextMultiline(const TEString &label, char *buf, size_t bufSize, const TEVector2 &size, int flags)
{
    return ImGui::InputTextMultiline(label.c_str(), buf, bufSize, ToImVec2(size), ConvertInputTextFlags(flags));
}

bool ImGuiUIAPI::InputInt(const TEString &label, int *v, int step, int step_fast, int flags)
{
    return ImGui::InputInt(label.c_str(), v, step, step_fast, ConvertInputTextFlags(flags));
}

// -------------------------------------------------------------------------
// Color Pickers & Edits
// -------------------------------------------------------------------------
bool ImGuiUIAPI::ColorEdit3(const TEString &label, float *col) { return ImGui::ColorEdit3(label.c_str(), col); }

bool ImGuiUIAPI::ColorEdit4(const TEString &label, float *col, int flags)
{
    return ImGui::ColorEdit4(label.c_str(), col, (ImGuiColorEditFlags)flags);
}

bool ImGuiUIAPI::ColorPicker4(const TEString &label, float *col, int flags)
{
    return ImGui::ColorPicker4(label.c_str(), col, (ImGuiColorEditFlags)flags);
}

bool ImGuiUIAPI::ColorButton(const TEString &desc_id, const TEVector4 &col, int flags, const TEVector2 &size)
{
    return ImGui::ColorButton(desc_id.c_str(), ToImVec4(col), (ImGuiColorEditFlags)flags, ToImVec2(size));
}

// -------------------------------------------------------------------------
// Combos & Selectables
// -------------------------------------------------------------------------
bool ImGuiUIAPI::Combo(const TEString &label, int *currentItem, const char *const items[], int itemsCount,
                       int popupMaxHeightInItems)
{
    return ImGui::Combo(label.c_str(), currentItem, items, itemsCount, popupMaxHeightInItems);
}

bool ImGuiUIAPI::BeginCombo(const TEString &label, const TEString &previewValue, int flags)
{
    return ImGui::BeginCombo(label.c_str(), previewValue.c_str(), (ImGuiComboFlags)flags);
}

void ImGuiUIAPI::EndCombo() { ImGui::EndCombo(); }

bool ImGuiUIAPI::Selectable(const TEString &label, bool selected, int flags, const TEVector2 &size)
{
    return ImGui::Selectable(label.c_str(), selected, (ImGuiSelectableFlags)flags, ToImVec2(size));
}

bool ImGuiUIAPI::Selectable(const TEString &label, bool *selected, int flags, const TEVector2 &size)
{
    return ImGui::Selectable(label.c_str(), selected, (ImGuiSelectableFlags)flags, ToImVec2(size));
}

// -------------------------------------------------------------------------
// Images
// -------------------------------------------------------------------------
void ImGuiUIAPI::Image(void *userTextureId, const TEVector2 &size, const TEVector2 &uv0, const TEVector2 &uv1,
                       const TEVector4 &tintCol)
{
    ImGui::ImageWithBg((ImTextureID)(size_t)userTextureId, ToImVec2(size), ToImVec2(uv0), ToImVec2(uv1),
                       ImVec4(0, 0, 0, 0), ToImVec4(tintCol));
}

bool ImGuiUIAPI::ImageButton(const TEString &strId, void *userTextureId, const TEVector2 &size, const TEVector2 &uv0,
                             const TEVector2 &uv1)
{
    return ImGui::ImageButton(strId.c_str(), (ImTextureID)(size_t)userTextureId, ToImVec2(size), ToImVec2(uv0),
                              ToImVec2(uv1));
}

// -------------------------------------------------------------------------
// Layout, Groups, Spacing
// -------------------------------------------------------------------------
void ImGuiUIAPI::Separator() { ImGui::Separator(); }
void ImGuiUIAPI::SeparatorEx(int flags) { ImGui::SeparatorEx((ImGuiSeparatorFlags)flags); }
void ImGuiUIAPI::Spacing() { ImGui::Spacing(); }
void ImGuiUIAPI::NewLine() { ImGui::NewLine(); }
void ImGuiUIAPI::SameLine(float offset_from_start_x, float spacing) { ImGui::SameLine(offset_from_start_x, spacing); }
void ImGuiUIAPI::Dummy(const TEVector2 &size) { ImGui::Dummy(ToImVec2(size)); }
void ImGuiUIAPI::BeginGroup() { ImGui::BeginGroup(); }
void ImGuiUIAPI::EndGroup() { ImGui::EndGroup(); }
void ImGuiUIAPI::Indent(float indentW) { ImGui::Indent(indentW); }
void ImGuiUIAPI::Unindent(float indentW) { ImGui::Unindent(indentW); }
void ImGuiUIAPI::Columns(int count, const char *id, bool border) { ImGui::Columns(count, id, border); }
void ImGuiUIAPI::NextColumn() { ImGui::NextColumn(); }
void ImGuiUIAPI::SetColumnWidth(int columnIndex, float width) { ImGui::SetColumnWidth(columnIndex, width); }
void ImGuiUIAPI::AlignTextToFramePadding() { ImGui::AlignTextToFramePadding(); }

// -------------------------------------------------------------------------
// ID & Item Stack
// -------------------------------------------------------------------------
void ImGuiUIAPI::PushID(const TEString &strId) { ImGui::PushID(strId.c_str()); }
void ImGuiUIAPI::PushID(int intId) { ImGui::PushID(intId); }
void ImGuiUIAPI::PopID() { ImGui::PopID(); }
unsigned int ImGuiUIAPI::GetID(const TEString &strId) { return ImGui::GetID(strId.c_str()); }
void ImGuiUIAPI::PushItemWidth(float itemWidth) { ImGui::PushItemWidth(itemWidth); }
void ImGuiUIAPI::PopItemWidth() { ImGui::PopItemWidth(); }
void ImGuiUIAPI::PushMultiItemsWidths(int components, float width_full)
{
    ImGui::PushMultiItemsWidths(components, width_full);
}
float ImGuiUIAPI::CalcItemWidth() { return ImGui::CalcItemWidth(); }
void ImGuiUIAPI::BeginDisabled(bool disabled) { ImGui::BeginDisabled(disabled); }
void ImGuiUIAPI::EndDisabled() { ImGui::EndDisabled(); }

// -------------------------------------------------------------------------
// Styling & Fonts
// -------------------------------------------------------------------------
void ImGuiUIAPI::PushStyleColor(int idx, const TEColor &color)
{
    ImGui::PushStyleColor((ImGuiCol)idx, ToImVec4(color));
}
void ImGuiUIAPI::PopStyleColor(int count) { ImGui::PopStyleColor(count); }
void ImGuiUIAPI::PushStyleVar(int idx, float val) { ImGui::PushStyleVar((ImGuiStyleVar)idx, val); }
void ImGuiUIAPI::PushStyleVar(int idx, const TEVector2 &val) { ImGui::PushStyleVar((ImGuiStyleVar)idx, ToImVec2(val)); }
void ImGuiUIAPI::PopStyleVar(int count) { ImGui::PopStyleVar(count); }
void ImGuiUIAPI::PushFont(void *font) { ImGui::PushFont((ImFont *)font); }
void ImGuiUIAPI::PopFont() { ImGui::PopFont(); }
void *ImGuiUIAPI::GetDefaultFont() { return ImGui::GetFont(); }
void *ImGuiUIAPI::GetFontAtlasTextureID() { return (void *)(uintptr_t)ImGui::GetIO().Fonts->TexID.GetTexID(); }

// -------------------------------------------------------------------------
// Tables
// -------------------------------------------------------------------------
bool ImGuiUIAPI::BeginTable(const TEString &strId, int column, int flags, const TEVector2 &outerSize, float innerWidth)
{
    return ImGui::BeginTable(strId.c_str(), column, (ImGuiTableFlags)flags, ToImVec2(outerSize), innerWidth);
}
void ImGuiUIAPI::EndTable() { ImGui::EndTable(); }
void ImGuiUIAPI::TableSetupColumn(const TEString &label, int flags, float initWidthOrWeight, unsigned int userId)
{
    ImGui::TableSetupColumn(label.c_str(), (ImGuiTableColumnFlags)flags, initWidthOrWeight, userId);
}
void ImGuiUIAPI::TableHeadersRow() { ImGui::TableHeadersRow(); }
bool ImGuiUIAPI::TableNextColumn() { return ImGui::TableNextColumn(); }
void ImGuiUIAPI::TableNextRow(float rowMinHeight, int rowFlags)
{
    ImGui::TableNextRow((ImGuiTableRowFlags)rowFlags, rowMinHeight);
}

// -------------------------------------------------------------------------
// Trees & Collapsing Headers
// -------------------------------------------------------------------------
bool ImGuiUIAPI::TreeNodeEx(const TEString &label, int flags)
{
    return ImGui::TreeNodeEx(label.c_str(), (ImGuiTreeNodeFlags)flags);
}
bool ImGuiUIAPI::TreeNodeEx(void *ptrId, int flags, const TEString &text)
{
    return ImGui::TreeNodeEx(ptrId, (ImGuiTreeNodeFlags)flags, "%s", text.c_str());
}
void ImGuiUIAPI::TreePop() { ImGui::TreePop(); }
bool ImGuiUIAPI::CollapsingHeader(const TEString &label, int flags)
{
    return ImGui::CollapsingHeader(label.c_str(), (ImGuiTreeNodeFlags)flags);
}
void ImGuiUIAPI::SetNextItemOpen(bool isOpen) { ImGui::SetNextItemOpen(isOpen); }

// -------------------------------------------------------------------------
// Popups, Tooltips, Focus & Cursor
// -------------------------------------------------------------------------
void ImGuiUIAPI::OpenPopup(const TEString &strId) { ImGui::OpenPopup(strId.c_str()); }
bool ImGuiUIAPI::BeginPopup(const TEString &strId, int flags)
{
    return ImGui::BeginPopup(strId.c_str(), (ImGuiWindowFlags)flags);
}
bool ImGuiUIAPI::BeginPopupContextWindow(const TEString &strId, int mouseButton, bool alsoOverItems)
{
    return ImGui::BeginPopupContextWindow(strId.IsEmpty() ? nullptr : strId.c_str(),
                                          (ImGuiPopupFlags)mouseButton |
                                              (alsoOverItems ? 0 : ImGuiPopupFlags_NoOpenOverItems));
}
bool ImGuiUIAPI::BeginPopupContextItem(const TEString &strId, int mouseButton)
{
    return ImGui::BeginPopupContextItem(strId.IsEmpty() ? nullptr : strId.c_str(), (ImGuiPopupFlags)mouseButton);
}
bool ImGuiUIAPI::BeginPopupModal(const TEString &name, bool *open, int flags)
{
    return ImGui::BeginPopupModal(name.c_str(), open, (ImGuiWindowFlags)flags);
}
void ImGuiUIAPI::EndPopup() { ImGui::EndPopup(); }
void ImGuiUIAPI::CloseCurrentPopup() { ImGui::CloseCurrentPopup(); }
bool ImGuiUIAPI::IsPopupOpen(const TEString &strId) { return ImGui::IsPopupOpen(strId.c_str()); }
void ImGuiUIAPI::BeginTooltip() { ImGui::BeginTooltip(); }
void ImGuiUIAPI::EndTooltip() { ImGui::EndTooltip(); }
void ImGuiUIAPI::SetTooltip(const TEString &text) { ImGui::SetTooltip("%s", text.c_str()); }
void ImGuiUIAPI::SetItemTooltip(const TEString &text) { ImGui::SetItemTooltip("%s", text.c_str()); }
void ImGuiUIAPI::SetItemDefaultFocus() { ImGui::SetItemDefaultFocus(); }
void ImGuiUIAPI::SetMouseCursor(int cursorType) { ImGui::SetMouseCursor((ImGuiMouseCursor)cursorType); }

// -------------------------------------------------------------------------
// Drag & Drop
// -------------------------------------------------------------------------
bool ImGuiUIAPI::BeginDragDropSource(int flags) { return ImGui::BeginDragDropSource((ImGuiDragDropFlags)flags); }
bool ImGuiUIAPI::SetDragDropPayload(const TEString &type, const void *data, size_t size, int cond)
{
    return ImGui::SetDragDropPayload(type.c_str(), data, size, (ImGuiCond)cond);
}
void ImGuiUIAPI::EndDragDropSource() { ImGui::EndDragDropSource(); }

bool ImGuiUIAPI::BeginDragDropTarget() { return ImGui::BeginDragDropTarget(); }

const TimeGUIPayload *ImGuiUIAPI::AcceptDragDropPayload(const TEString &type, int flags)
{
    const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type.c_str(), (ImGuiDragDropFlags)flags);
    if (!payload)
        return nullptr;

    m_CurrentPayload.Data = payload->Data;
    m_CurrentPayload.DataSize = payload->DataSize;
    m_CurrentPayload.DataType = payload->DataType;
    m_CurrentPayload.IsPreview = payload->IsPreview();
    m_CurrentPayload.IsDelivery = payload->IsDelivery();
    return &m_CurrentPayload;
}

void ImGuiUIAPI::EndDragDropTarget() { ImGui::EndDragDropTarget(); }

const TimeGUIPayload *ImGuiUIAPI::GetDragDropPayload()
{
    const ImGuiPayload *payload = ImGui::GetDragDropPayload();
    if (!payload)
        return nullptr;

    m_CurrentPayload.Data = payload->Data;
    m_CurrentPayload.DataSize = payload->DataSize;
    m_CurrentPayload.DataType = payload->DataType;
    m_CurrentPayload.IsPreview = payload->IsPreview();
    m_CurrentPayload.IsDelivery = payload->IsDelivery();
    return &m_CurrentPayload;
}

// -------------------------------------------------------------------------
// Docking
// -------------------------------------------------------------------------
unsigned int ImGuiUIAPI::DockSpace(unsigned int id, const TEVector2 &size, int flags)
{
    return ImGui::DockSpace(id, ToImVec2(size), (ImGuiDockNodeFlags)flags);
}
void ImGuiUIAPI::DockBuilderRemoveNode(unsigned int nodeId) { ImGui::DockBuilderRemoveNode(nodeId); }
void ImGuiUIAPI::DockBuilderAddNode(unsigned int nodeId, int flags)
{
    ImGui::DockBuilderAddNode(nodeId, (ImGuiDockNodeFlags)flags);
}
void ImGuiUIAPI::DockBuilderSetNodeSize(unsigned int nodeId, const TEVector2 &size)
{
    ImGui::DockBuilderSetNodeSize(nodeId, ToImVec2(size));
}
unsigned int ImGuiUIAPI::DockBuilderSplitNode(unsigned int nodeId, int splitDir, float sizeRatio,
                                              unsigned int *outIdDir1, unsigned int *outIdDir2)
{
    return ImGui::DockBuilderSplitNode(nodeId, (ImGuiDir)splitDir, sizeRatio, (ImGuiID *)outIdDir1,
                                       (ImGuiID *)outIdDir2);
}
void ImGuiUIAPI::DockBuilderDockWindow(const TEString &windowName, unsigned int nodeId)
{
    ImGui::DockBuilderDockWindow(windowName.c_str(), nodeId);
}
void ImGuiUIAPI::DockBuilderFinish(unsigned int nodeId) { ImGui::DockBuilderFinish(nodeId); }

// -------------------------------------------------------------------------
// Tabs & Cards
// -------------------------------------------------------------------------
bool ImGuiUIAPI::BeginTabItem(const TEString &label, bool *open, int flags)
{
    return ImGui::BeginTabItem(label.c_str(), open, (ImGuiTabItemFlags)flags);
}
void ImGuiUIAPI::EndTabItem() { ImGui::EndTabItem(); }
bool ImGuiUIAPI::BeginTabBar(const TEString &strId, int flags)
{
    return ImGui::BeginTabBar(strId.c_str(), (ImGuiTabBarFlags)flags);
}
void ImGuiUIAPI::EndTabBar() { ImGui::EndTabBar(); }
bool ImGuiUIAPI::BeginProjectCard(const TEString &id, const TEVector2 &size, bool &hovered)
{
    ImGui::PushID(id.c_str());
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, ImGui::GetID(id.c_str())))
        return false;

    bool held = false;
    bool pressed = ImGui::ButtonBehavior(bb, ImGui::GetID(id.c_str()), &hovered, &held);

    ImVec4 normalBg = ImVec4(0.12f, 0.13f, 0.16f, 0.85f);
    ImVec4 hoveredBg = ImVec4(0.18f, 0.20f, 0.26f, 0.95f);
    ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(hovered ? hoveredBg : normalBg);

    ImU32 borderCol = hovered ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.40f, 0.50f, 0.70f, 0.90f))
                              : ImGui::ColorConvertFloat4ToU32(ImVec4(0.24f, 0.26f, 0.32f, 0.50f));

    window->DrawList->AddRectFilled(pos, bb.Max, bgCol, 8.0f);
    window->DrawList->AddRect(pos, bb.Max, borderCol, 8.0f, 0, hovered ? 1.8f : 1.0f);

    return pressed;
}
void ImGuiUIAPI::EndProjectCard() { ImGui::PopID(); }

// -------------------------------------------------------------------------
// Next Window & Item Controls
// -------------------------------------------------------------------------
void ImGuiUIAPI::SetNextWindowPos(const TEVector2 &pos, int cond, const TEVector2 &pivot)
{
    ImGui::SetNextWindowPos(ToImVec2(pos), cond, ToImVec2(pivot));
}
void ImGuiUIAPI::SetNextWindowSize(const TEVector2 &size, int cond) { ImGui::SetNextWindowSize(ToImVec2(size), cond); }
void ImGuiUIAPI::SetNextWindowContentSize(const TEVector2 &size) { ImGui::SetNextWindowContentSize(ToImVec2(size)); }
void ImGuiUIAPI::SetNextWindowDockID(unsigned int dockId, int cond) { ImGui::SetNextWindowDockID(dockId, cond); }
void ImGuiUIAPI::SetNextWindowViewport(unsigned int viewportId) { ImGui::SetNextWindowViewport(viewportId); }
void ImGuiUIAPI::SetNextItemWidth(float itemWidth) { ImGui::SetNextItemWidth(itemWidth); }
void ImGuiUIAPI::SetNextItemAllowOverlap() { ImGui::SetNextItemAllowOverlap(); }

// -------------------------------------------------------------------------
// State & Queries
// -------------------------------------------------------------------------
TEVector2 ImGuiUIAPI::GetCursorPos() { return FromImVec2(ImGui::GetCursorPos()); }
void ImGuiUIAPI::SetCursorPos(const TEVector2 &pos) { ImGui::SetCursorPos(ToImVec2(pos)); }
float ImGuiUIAPI::GetCursorPosX() { return ImGui::GetCursorPosX(); }
float ImGuiUIAPI::GetCursorPosY() { return ImGui::GetCursorPosY(); }
void ImGuiUIAPI::SetCursorPosX(float x) { ImGui::SetCursorPosX(x); }
void ImGuiUIAPI::SetCursorPosY(float y) { ImGui::SetCursorPosY(y); }
TEVector2 ImGuiUIAPI::GetCursorScreenPos() { return FromImVec2(ImGui::GetCursorScreenPos()); }
void ImGuiUIAPI::SetCursorScreenPos(const TEVector2 &pos) { ImGui::SetCursorScreenPos(ToImVec2(pos)); }
TEVector2 ImGuiUIAPI::GetWindowSize() { return FromImVec2(ImGui::GetWindowSize()); }
TEVector2 ImGuiUIAPI::GetWindowPos() { return FromImVec2(ImGui::GetWindowPos()); }
float ImGuiUIAPI::GetWindowWidth() { return ImGui::GetWindowWidth(); }
float ImGuiUIAPI::GetWindowHeight() { return ImGui::GetWindowHeight(); }
TEVector2 ImGuiUIAPI::GetContentRegionAvail() { return FromImVec2(ImGui::GetContentRegionAvail()); }
float ImGuiUIAPI::GetFrameHeight() { return ImGui::GetFrameHeight(); }
TEVector2 ImGuiUIAPI::GetWindowContentRegionMin() { return FromImVec2(ImGui::GetWindowContentRegionMin()); }
TEVector2 ImGuiUIAPI::GetWindowContentRegionMax() { return FromImVec2(ImGui::GetWindowContentRegionMax()); }
void ImGuiUIAPI::SetScrollHereY(float centerYRatio) { ImGui::SetScrollHereY(centerYRatio); }
float ImGuiUIAPI::GetScrollY() { return ImGui::GetScrollY(); }
float ImGuiUIAPI::GetScrollMaxY() { return ImGui::GetScrollMaxY(); }
void ImGuiUIAPI::SetScrollY(float scrollY) { ImGui::SetScrollY(scrollY); }
TEArray<unsigned int> ImGuiUIAPI::GetInputQueueCharacters()
{
    TEArray<unsigned int> chars;
    for (auto c : ImGui::GetIO().InputQueueCharacters)
        chars.push_back((unsigned int)c);
    return chars;
}
void ImGuiUIAPI::ClearInputQueueCharacters() { ImGui::GetIO().InputQueueCharacters.resize(0); }
TEVector2 ImGuiUIAPI::GetMousePos() { return FromImVec2(ImGui::GetMousePos()); }
bool ImGuiUIAPI::IsMouseDown(int button) { return ImGui::IsMouseDown((ImGuiMouseButton)button); }
bool ImGuiUIAPI::IsMouseReleased(int button) { return ImGui::IsMouseReleased((ImGuiMouseButton)button); }
bool ImGuiUIAPI::IsMouseClicked(int button, bool repeat)
{
    return ImGui::IsMouseClicked((ImGuiMouseButton)button, repeat);
}
bool ImGuiUIAPI::IsMouseDoubleClicked(int button) { return ImGui::IsMouseDoubleClicked((ImGuiMouseButton)button); }
bool ImGuiUIAPI::IsMouseDragging(int button, float lock_threshold)
{
    return ImGui::IsMouseDragging((ImGuiMouseButton)button, lock_threshold);
}
TEVector2 ImGuiUIAPI::GetMouseDragDelta(int button, float lock_threshold)
{
    return FromImVec2(ImGui::GetMouseDragDelta((ImGuiMouseButton)button, lock_threshold));
}
void ImGuiUIAPI::ResetMouseDragDelta(int button) { ImGui::ResetMouseDragDelta((ImGuiMouseButton)button); }
bool ImGuiUIAPI::IsAnyItemHovered() { return ImGui::IsAnyItemHovered(); }
bool ImGuiUIAPI::IsKeyPressed(int key) { return ImGui::IsKeyPressed((ImGuiKey)key); }
double ImGuiUIAPI::GetTime() { return ImGui::GetTime(); }
TEVector2 ImGuiUIAPI::CalcTextSize(const TEString &text) { return FromImVec2(ImGui::CalcTextSize(text.c_str())); }
bool ImGuiUIAPI::IsItemDeactivatedAfterEdit() { return ImGui::IsItemDeactivatedAfterEdit(); }
bool ImGuiUIAPI::IsItemHovered(int flags) { return ImGui::IsItemHovered((ImGuiHoveredFlags)flags); }
bool ImGuiUIAPI::IsItemActive() { return ImGui::IsItemActive(); }
bool ImGuiUIAPI::IsItemFocused() { return ImGui::IsItemFocused(); }
bool ImGuiUIAPI::IsItemClicked(int mouse_button) { return ImGui::IsItemClicked(mouse_button); }
bool ImGuiUIAPI::IsWindowHovered(int flags) { return ImGui::IsWindowHovered((ImGuiHoveredFlags)flags); }
bool ImGuiUIAPI::IsWindowFocused(int flags) { return ImGui::IsWindowFocused((ImGuiFocusedFlags)flags); }
bool ImGuiUIAPI::IsWindowAppearing() { return ImGui::IsWindowAppearing(); }
TEVector2 ImGuiUIAPI::GetItemRectMin() { return FromImVec2(ImGui::GetItemRectMin()); }
TEVector2 ImGuiUIAPI::GetItemRectSize() { return FromImVec2(ImGui::GetItemRectSize()); }
TEVector2 ImGuiUIAPI::GetItemRectMax() { return FromImVec2(ImGui::GetItemRectMax()); }
void ImGuiUIAPI::SetKeyboardFocusHere(int offset) { ImGui::SetKeyboardFocusHere(offset); }
void ImGuiUIAPI::SetClipboardText(const TEString &text) { ImGui::SetClipboardText(text.c_str()); }
TEString ImGuiUIAPI::GetClipboardText()
{
    const char *text = ImGui::GetClipboardText();
    return text ? TEString(text) : TEString("");
}
void ImGuiUIAPI::SetCaretColor(const TEVector4 &color)
{
    ImGui::GetStyle().Colors[ImGuiCol_InputTextCursor] = ToImVec4(color);
}
TEVector4 ImGuiUIAPI::GetCaretColor()
{
    const ImVec4 &c = ImGui::GetStyle().Colors[ImGuiCol_InputTextCursor];
    return TEVector4(c.x, c.y, c.z, c.w);
}
void ImGuiUIAPI::SaveIniSettingsToDisk(const char *ini_filename) { ImGui::SaveIniSettingsToDisk(ini_filename); }
void ImGuiUIAPI::LoadIniSettingsFromDisk(const char *ini_filename) { ImGui::LoadIniSettingsFromDisk(ini_filename); }

// -------------------------------------------------------------------------
// DrawList Commands
// -------------------------------------------------------------------------
void ImGuiUIAPI::DrawListAddLine(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                 float thickness)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddLine(ToImVec2(p1), ToImVec2(p2), (ImU32)color, thickness);
}

void ImGuiUIAPI::DrawListAddRect(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                 float rounding, int flags, float thickness)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddRect(ToImVec2(p1), ToImVec2(p2), (ImU32)color, rounding, thickness, (ImDrawFlags)(flags & ~0x0F));
}

void ImGuiUIAPI::DrawListAddRectFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                       float rounding)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddRectFilled(ToImVec2(p1), ToImVec2(p2), (ImU32)color, rounding);
}

void ImGuiUIAPI::DrawListAddCircle(void *drawList, const TEVector2 &center, float radius, unsigned int color,
                                   int num_segments, float thickness)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddCircle(ToImVec2(center), radius, (ImU32)color, num_segments, thickness);
}

void ImGuiUIAPI::DrawListAddCircleFilled(void *drawList, const TEVector2 &center, float radius, unsigned int color,
                                         int num_segments)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddCircleFilled(ToImVec2(center), radius, (ImU32)color, num_segments);
}

void ImGuiUIAPI::DrawListAddText(void *drawList, const TEVector2 &pos, unsigned int color, const TEString &text)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddText(ToImVec2(pos), (ImU32)color, text.c_str());
}

void ImGuiUIAPI::DrawListAddText(void *drawList, void *font, float fontSize, const TEVector2 &pos, unsigned int color,
                                 const TEString &text)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddText((ImFont *)font, fontSize, ToImVec2(pos), (ImU32)color, text.c_str());
}

void ImGuiUIAPI::DrawListAddImage(void *drawList, void *texture_id, const TEVector2 &p_min, const TEVector2 &p_max,
                                  const TEVector2 &uv_min, const TEVector2 &uv_max, unsigned int col)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddImage((ImTextureID)texture_id, ToImVec2(p_min), ToImVec2(p_max), ToImVec2(uv_min), ToImVec2(uv_max),
                     (ImU32)col);
}

void ImGuiUIAPI::DrawListAddBezierCubic(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                        const TEVector2 &p4, unsigned int color, float thickness, int num_segments)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddBezierCubic(ToImVec2(p1), ToImVec2(p2), ToImVec2(p3), ToImVec2(p4), (ImU32)color, thickness,
                           num_segments);
}

void ImGuiUIAPI::DrawListPushClipRect(void *drawList, const TEVector2 &clip_rect_min, const TEVector2 &clip_rect_max,
                                      bool intersect_with_current_clip_rect)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->PushClipRect(ToImVec2(clip_rect_min), ToImVec2(clip_rect_max), intersect_with_current_clip_rect);
}

void ImGuiUIAPI::DrawListPopClipRect(void *drawList)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->PopClipRect();
}

void ImGuiUIAPI::DrawListAddPolyline(void *drawList, const TEVector2 *points, int num_points, unsigned int color,
                                     int flags, float thickness)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (!dl)
        return;
    ImDrawFlags imFlags = 0;
    if ((flags & 1) || (flags & (1 << 9)))
        imFlags |= ImDrawFlags_Closed;
    dl->AddPolyline((const ImVec2 *)points, num_points, (ImU32)color, thickness, imFlags);
}

void ImGuiUIAPI::DrawListAddConvexPolyFilled(void *drawList, const TEVector2 *points, int num_points,
                                             unsigned int color)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddConvexPolyFilled((const ImVec2 *)points, num_points, (ImU32)color);
}

void ImGuiUIAPI::DrawListAddTriangleFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2,
                                           const TEVector2 &p3, unsigned int color)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddTriangleFilled(ToImVec2(p1), ToImVec2(p2), ToImVec2(p3), (ImU32)color);
}

void ImGuiUIAPI::DrawListAddQuadFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                       const TEVector2 &p4, unsigned int color)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->AddQuadFilled(ToImVec2(p1), ToImVec2(p2), ToImVec2(p3), ToImVec2(p4), (ImU32)color);
}

void ImGuiUIAPI::DrawListPushTextureID(void *drawList, void *texture_id)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    if (dl)
        dl->PushTextureID(ImTextureRef((ImTextureID)(uintptr_t)texture_id));
}

void *ImGuiUIAPI::DrawListCreate()
{
    ImDrawList *native = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
    if (native)
        native->_ResetForNewFrame();
    return native;
}

void ImGuiUIAPI::DrawListDestroy(void *drawList)
{
    if (drawList)
        IM_DELETE((ImDrawList *)drawList);
}

void ImGuiUIAPI::DrawListRender(void *drawList, const TEVector2 &displaySize)
{
    if (!drawList)
        return;
    ImDrawData drawData;
    drawData.Valid = true;
    drawData.AddDrawList((ImDrawList *)drawList);
    drawData.DisplayPos = ImVec2(0.0f, 0.0f);
    drawData.DisplaySize = ToImVec2(displaySize);
    drawData.FramebufferScale = ImVec2(1.0f, 1.0f);
#ifdef TE_SUPPORT_OPENGL
    ImGui_ImplOpenGL3_RenderDrawData(&drawData);
#endif
#ifdef TE_SUPPORT_METAL
    ImGui_RenderMetalDrawData(&drawData);
#endif
}

int ImGuiUIAPI::DrawListGetVertexCount(void *drawList)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    return dl ? dl->VtxBuffer.Size : 0;
}

int ImGuiUIAPI::DrawListGetCommandCount(void *drawList)
{
    auto *dl = static_cast<ImDrawList *>(drawList);
    return dl ? dl->CmdBuffer.Size : 0;
}

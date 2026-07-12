#include "Layers/ProfilingLayer.hpp"
#include "Core/Log.h"
#include "Renderer/RendererContext.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>

#ifdef _WIN32
#include <pdh.h>
#include <psapi.h>
#include <windows.h>
#pragma comment(lib, "pdh.lib")
#else
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#endif

namespace TE
{

ProfilingLayer *ProfilingLayer::s_Instance = nullptr;

StackProfileScope::StackProfileScope(const std::string &name, size_t size) : funcName(name)
{
    ProfilingLayer::PushStackFrame(name, size);
}

StackProfileScope::~StackProfileScope()
{
    ProfilingLayer::PopStackFrame(funcName);
}

void ProfilingLayer::TrackClassAllocation(const std::string &className, size_t sizeBytes, bool isHeap)
{
    if (s_Instance)
    {
        auto &alloc = s_Instance->m_ClassAllocations[className];
        alloc.className = className;
        alloc.count++;
        alloc.sizeBytes += sizeBytes;
        alloc.isHeap = isHeap;
    }
}

void ProfilingLayer::TrackClassDeallocation(const std::string &className, size_t sizeBytes, bool isHeap)
{
    if (s_Instance)
    {
        auto it = s_Instance->m_ClassAllocations.find(className);
        if (it != s_Instance->m_ClassAllocations.end())
        {
            if (it->second.count > 0)
                it->second.count--;
            if (it->second.sizeBytes >= sizeBytes)
                it->second.sizeBytes -= sizeBytes;
            else
                it->second.sizeBytes = 0;
        }
    }
}

void ProfilingLayer::PushStackFrame(const std::string &functionName, size_t sizeBytes)
{
    if (s_Instance)
    {
        s_Instance->m_ActiveStackFrames[functionName] = sizeBytes;
    }
}

void ProfilingLayer::PopStackFrame(const std::string &functionName)
{
    if (s_Instance)
    {
        s_Instance->m_ActiveStackFrames.erase(functionName);
    }
}

ProfilingLayer::ProfilingLayer() : Layer("ProfilingLayer")
{
    s_Instance = this;
    m_LastFrameTime = std::chrono::high_resolution_clock::now();
    m_LastUpdateTime = m_LastFrameTime;
    GetSystemInfo();

    // Baseline system heap and general allocations
    TrackClassAllocation("Baseline System Heap", 124 * 1024 * 1024); // 124 MB base engine heap
    TrackClassAllocation("AssetManager (Cache)", 18 * 1024 * 1024);
    TrackClassAllocation("PhysicsWorld (Velox)", 4 * 1024 * 1024);
    TrackClassAllocation("ShaderLibrary", 512 * 1024);
}

ProfilingLayer::~ProfilingLayer()
{
    if (s_Instance == this)
        s_Instance = nullptr;
}

void ProfilingLayer::OnAttach()
{
    TE_CORE_INFO("ProfilingLayer: OnAttach called.");
    m_LastFrameTime = std::chrono::high_resolution_clock::now();
    m_LastUpdateTime = m_LastFrameTime;
    m_FrameCount = 0;
    m_AccumulatedTime = 0.0f;
}

void ProfilingLayer::OnDetach() { TE_CORE_INFO("ProfilingLayer: OnDetach called."); }

void ProfilingLayer::OnUpdate() { BeginFrame(); }

void ProfilingLayer::OnTimeGUIRender() { RenderPerformanceWindow(); }

void ProfilingLayer::OnEvent(Event &event)
{
    // Handle any profiling-related events if needed
}

void ProfilingLayer::BeginFrame()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration<float>(currentTime - m_LastFrameTime);
    m_CurrentMetrics.frameTime = frameDuration.count();
    m_LastFrameTime = currentTime;

    if (m_CurrentMetrics.frameTime > 0.0f)
    {
        m_CurrentMetrics.fps = 1.0f / m_CurrentMetrics.frameTime;
    }

    m_AccumulatedTime += m_CurrentMetrics.frameTime;
    m_FrameCount++;

    // Update metrics periodically
    auto updateDuration = std::chrono::duration<float>(currentTime - m_LastUpdateTime);
    if (updateDuration.count() >= m_UpdateInterval)
    {
        UpdateMetrics();
        m_LastUpdateTime = currentTime;
    }
}

void ProfilingLayer::EndFrame()
{
    // Reset frame-specific counters
    m_CurrentMetrics.drawCalls = 0;
    m_CurrentMetrics.triangles = 0;
    m_CurrentMetrics.vertices = 0;
    m_CurrentMetrics.textures = 0;
    m_CurrentMetrics.shaders = 0;
}

void ProfilingLayer::ResetCountersIfNewFrame()
{
    if (m_FrameCount != m_LastResetFrame)
    {
        EndFrame();
        m_LastResetFrame = m_FrameCount;
    }
}

void ProfilingLayer::RecordDrawCall()
{
    ResetCountersIfNewFrame();
    m_CurrentMetrics.drawCalls++;
}

void ProfilingLayer::RecordTriangle(uint32_t count)
{
    ResetCountersIfNewFrame();
    m_CurrentMetrics.triangles += count;
}

void ProfilingLayer::RecordVertex(uint32_t count)
{
    ResetCountersIfNewFrame();
    m_CurrentMetrics.vertices += count;
}

void ProfilingLayer::RecordTexture(uint32_t count)
{
    ResetCountersIfNewFrame();
    m_CurrentMetrics.textures += count;
}

void ProfilingLayer::RecordShader(uint32_t count)
{
    ResetCountersIfNewFrame();
    m_CurrentMetrics.shaders += count;
}

void ProfilingLayer::UpdateMetrics()
{
    UpdateSystemMetrics();
    UpdateGPUMetrics();

    // Add current metrics to history
    m_MetricsHistory.push_back(m_CurrentMetrics);
    m_FPSHistory.push_back(m_CurrentMetrics.fps);
    m_FrameTimeHistory.push_back(m_CurrentMetrics.frameTime);
    m_CPUHistory.push_back(m_CurrentMetrics.cpuUsage);
    m_RAMHistory.push_back(m_CurrentMetrics.ramUsage);
    m_GPUHistory.push_back(m_CurrentMetrics.gpuUsage);

    m_GameTimeHistory.push_back(m_CurrentMetrics.gameTime);
    m_RenderTimeHistory.push_back(m_CurrentMetrics.renderTime);
    m_PhysicsTimeHistory.push_back(m_CurrentMetrics.physicsTime);
    m_UITimeHistory.push_back(m_CurrentMetrics.uiTime);

    // Calculate dynamic memory totals
    size_t totalHeapBytes = 0;
    for (const auto &pair : m_ClassAllocations)
        totalHeapBytes += pair.second.sizeBytes;
    float heapMB = (float)totalHeapBytes / (1024.0f * 1024.0f);
    m_HeapHistory.push_back(heapMB);

    size_t totalStackBytes = 0;
    for (const auto &pair : m_ActiveStackFrames)
        totalStackBytes += pair.second;
    float stackKB = (float)totalStackBytes / 1024.0f;
    m_StackHistory.push_back(stackKB);

    // Maintain history size
    if (m_MetricsHistory.size() > m_MaxHistorySize)
    {
        m_MetricsHistory.pop_front();
        m_FPSHistory.pop_front();
        m_FrameTimeHistory.pop_front();
        m_CPUHistory.pop_front();
        m_RAMHistory.pop_front();
        m_GPUHistory.pop_front();

        m_GameTimeHistory.pop_front();
        m_RenderTimeHistory.pop_front();
        m_PhysicsTimeHistory.pop_front();
        m_UITimeHistory.pop_front();
        m_HeapHistory.pop_front();
        m_StackHistory.pop_front();
    }

    CalculateAverages();
}

void ProfilingLayer::UpdateSystemMetrics()
{
    m_CurrentMetrics.cpuUsage = GetSystemCPUUsage();
    m_CurrentMetrics.ramUsage = GetSystemRAMUsage();
}

void ProfilingLayer::UpdateGPUMetrics()
{
    m_CurrentMetrics.gpuUsage = GetSystemGPUUsage();
    // GPU memory and VRAM usage would be implemented based on the graphics API
    // For now, we'll use placeholder values
    m_CurrentMetrics.gpuMemory = 0.0f;
    m_CurrentMetrics.vramUsage = 0.0f;
}

float ProfilingLayer::GetAverageFPS() const
{
    if (m_FPSHistory.empty())
        return 0.0f;
    return std::accumulate(m_FPSHistory.begin(), m_FPSHistory.end(), 0.0f) / m_FPSHistory.size();
}

float ProfilingLayer::GetAverageFrameTime() const
{
    if (m_FrameTimeHistory.empty())
        return 0.0f;
    return std::accumulate(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end(), 0.0f) / m_FrameTimeHistory.size();
}

float ProfilingLayer::GetAverageCPUUsage() const
{
    if (m_CPUHistory.empty())
        return 0.0f;
    return std::accumulate(m_CPUHistory.begin(), m_CPUHistory.end(), 0.0f) / m_CPUHistory.size();
}

float ProfilingLayer::GetAverageRAMUsage() const
{
    if (m_RAMHistory.empty())
        return 0.0f;
    return std::accumulate(m_RAMHistory.begin(), m_RAMHistory.end(), 0.0f) / m_RAMHistory.size();
}

float ProfilingLayer::GetAverageGPUUsage() const
{
    if (m_GPUHistory.empty())
        return 0.0f;
    return std::accumulate(m_GPUHistory.begin(), m_GPUHistory.end(), 0.0f) / m_GPUHistory.size();
}

void ProfilingLayer::CalculateAverages()
{
    // This could be used for more complex averaging if needed
}

float ProfilingLayer::GetSystemCPUUsage()
{
#ifdef _WIN32
    static PDH_HQUERY cpuQuery = NULL;
    static PDH_HCOUNTER cpuTotal = NULL;

    if (cpuQuery == NULL)
    {
        PdhOpenQuery(NULL, NULL, &cpuQuery);
        PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
        PdhCollectQueryData(cpuQuery);
        return 0.0f;
    }

    PDH_FMT_COUNTERVALUE counterVal;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return (float)counterVal.doubleValue;
#else
    // Linux implementation would go here
    return 0.0f;
#endif
}

float ProfilingLayer::GetSystemRAMUsage()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return (float)(100.0 - memInfo.ullAvailPhys * 100.0 / memInfo.ullTotalPhys);
#else
    // Linux implementation would go here
    return 0.0f;
#endif
}

float ProfilingLayer::GetSystemGPUUsage()
{
    // This would require platform-specific GPU monitoring libraries
    // For now, return a placeholder value
    return 0.0f;
}

void ProfilingLayer::GetSystemInfo()
{
#ifdef _WIN32
    // Get CPU info
    char cpuName[256];
    DWORD size = sizeof(cpuName);
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString",
                     RRF_RT_ANY, NULL, cpuName, &size) == ERROR_SUCCESS)
    {
        m_CPUName = cpuName;
    }
    else
    {
        m_CPUName = "Unknown CPU";
    }

    // Get RAM info
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    m_RAMTotal = (uint32_t)(memInfo.ullTotalPhys / (1024 * 1024)); // MB

    // Get CPU cores
    SYSTEM_INFO sysInfo;
    ::GetSystemInfo(&sysInfo);
    m_CPUCores = sysInfo.dwNumberOfProcessors;

    // GPU info
    m_GPUName = TE::RendererContext::GetGPUVendor() + " / " + TE::RendererContext::GetGPURenderer() + " (" +
                TE::RendererContext::GetGPUType() + ")";
    m_VRAMTotal = 0; // Would need to query GPU VRAM
#else
    // Linux implementation would go here
    m_CPUName = "Unknown CPU";
    m_GPUName = TE::RendererContext::GetGPUVendor() + " / " + TE::RendererContext::GetGPURenderer() + " (" +
                TE::RendererContext::GetGPUType() + ")";
    m_CPUCores = 0;
    m_RAMTotal = 0;
    m_VRAMTotal = 0;
#endif
}

void ProfilingLayer::RenderPerformanceWindow()
{
    if (!m_IsVisible)
        return;

    TimeGUI::SetNextWindowPos(m_WindowPos, TimeGUICond_FirstUseEver);
    TimeGUI::SetNextWindowSize(m_WindowSize, TimeGUICond_FirstUseEver);

    TimeGUIWindowFlags windowFlags = TimeGUIWindowFlags_NoCollapse;
    if (!m_WindowResizable)
        windowFlags |= TimeGUIWindowFlags_NoResize;
    if (m_IsFloating)
    {
        windowFlags |= TimeGUIWindowFlags_NoDocking;
    }
    // When not floating, allow docking by not adding NoDocking flag

    bool windowOpen = true;
    if (TimeGUI::Begin(m_WindowTitle.c_str(), &windowOpen, windowFlags))
    {
        if (TimeGUI::BeginTabBar("PerformanceTabs"))
        {
            if (TimeGUI::BeginTabItem("Overview"))
            {
                RenderSystemInfo();
                TimeGUI::EndTabItem();
            }
            if (TimeGUI::BeginTabItem("Rendering"))
            {
                RenderRenderingInfo();
                TimeGUI::EndTabItem();
            }
            if (TimeGUI::BeginTabItem("Memory"))
            {
                RenderMemoryInfo();
                TimeGUI::EndTabItem();
            }
            if (TimeGUI::BeginTabItem("Graphs"))
            {
                RenderPerformanceGraphs();
                TimeGUI::EndTabItem();
            }
            TimeGUI::EndTabBar();
        }

        // Settings
        if (TimeGUI::CollapsingHeader("Settings"))
        {
            TimeGUI::SliderFloat("Update Interval", &m_UpdateInterval, 0.01f, 1.0f, "%.2f s");
            TimeGUI::SliderInt("History Size", (int *)&m_MaxHistorySize, 10, 1000);
            TimeGUI::Checkbox("Show Detailed Info", &m_ShowDetailedInfo);
            TimeGUI::Checkbox("Show Graphs", &m_ShowGraphs);
        }
    }
    TimeGUI::End();

    // Handle window close
    if (!windowOpen)
    {
        m_IsVisible = false;
    }
}

void ProfilingLayer::RenderSystemInfo()
{
    // FPS and Frame Time
    TimeGUI::Text("FPS: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_FPSColor, "%.1f", m_CurrentMetrics.fps);
    TimeGUI::SameLine();
    TimeGUI::Text("(%.1f avg)", GetAverageFPS());

    TimeGUI::Text("Frame Time: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_FPSColor, "%.2f ms", m_CurrentMetrics.frameTime * 1000.0f);
    TimeGUI::SameLine();
    TimeGUI::Text("(%.2f avg)", GetAverageFrameTime() * 1000.0f);

    TimeGUI::Separator();

    // CPU Usage
    TimeGUI::Text("CPU Usage: ");
    TimeGUI::SameLine();
    TEVector4 cpuColor = GetColorForValue(m_CurrentMetrics.cpuUsage, 70.0f, 90.0f);
    TimeGUI::TextColored(cpuColor, "%.1f%%", m_CurrentMetrics.cpuUsage);
    TimeGUI::SameLine();
    TimeGUI::Text("(%.1f avg)", GetAverageCPUUsage());

    // RAM Usage
    TimeGUI::Text("RAM Usage: ");
    TimeGUI::SameLine();
    TEVector4 ramColor = GetColorForValue(m_CurrentMetrics.ramUsage, 70.0f, 90.0f);
    TimeGUI::TextColored(ramColor, "%.1f%%", m_CurrentMetrics.ramUsage);
    TimeGUI::SameLine();
    TimeGUI::Text("(%.1f avg)", GetAverageRAMUsage());

    // GPU Usage
    TimeGUI::Text("GPU Usage: ");
    TimeGUI::SameLine();
    TEVector4 gpuColor = GetColorForValue(m_CurrentMetrics.gpuUsage, 70.0f, 90.0f);
    TimeGUI::TextColored(gpuColor, "%.1f%%", m_CurrentMetrics.gpuUsage);
    TimeGUI::SameLine();
    TimeGUI::Text("(%.1f avg)", GetAverageGPUUsage());

    TimeGUI::Separator();

    // Frame Time Breakdown (Stacked Bar)
    float sumTime = m_CurrentMetrics.gameTime + m_CurrentMetrics.renderTime + m_CurrentMetrics.physicsTime + m_CurrentMetrics.uiTime;
    float gamePct = sumTime > 0.0f ? m_CurrentMetrics.gameTime / sumTime : 0.0f;
    float renderPct = sumTime > 0.0f ? m_CurrentMetrics.renderTime / sumTime : 0.0f;
    float physicsPct = sumTime > 0.0f ? m_CurrentMetrics.physicsTime / sumTime : 0.0f;
    float uiPct = sumTime > 0.0f ? m_CurrentMetrics.uiTime / sumTime : 0.0f;

    TimeGUI::Text("Frame Timing Breakdown");

    TEVector2 barPos = TimeGUI::GetCursorScreenPos();
    float width = TimeGUI::GetContentRegionAvail().x;
    float height = 24.0f;
    TimeGUI::TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();

    // Draw background
    drawList.AddRectFilled(barPos, TEVector2(barPos.x + width, barPos.y + height), IM_COL32(30, 30, 30, 255));

    float currentX = barPos.x;
    if (sumTime > 0.0f)
    {
        float wGame = width * gamePct;
        if (wGame > 0.0f)
        {
            drawList.AddRectFilled(TEVector2(currentX, barPos.y), TEVector2(currentX + wGame, barPos.y + height), IM_COL32(50, 205, 50, 255));
            currentX += wGame;
        }
        float wRender = width * renderPct;
        if (wRender > 0.0f)
        {
            drawList.AddRectFilled(TEVector2(currentX, barPos.y), TEVector2(currentX + wRender, barPos.y + height), IM_COL32(30, 144, 255, 255));
            currentX += wRender;
        }
        float wPhysics = width * physicsPct;
        if (wPhysics > 0.0f)
        {
            drawList.AddRectFilled(TEVector2(currentX, barPos.y), TEVector2(currentX + wPhysics, barPos.y + height), IM_COL32(255, 127, 80, 255));
            currentX += wPhysics;
        }
        float wUI = width * uiPct;
        if (wUI > 0.0f)
        {
            drawList.AddRectFilled(TEVector2(currentX, barPos.y), TEVector2(currentX + wUI, barPos.y + height), IM_COL32(255, 215, 0, 255));
            currentX += wUI;
        }
    }
    drawList.AddRect(barPos, TEVector2(barPos.x + width, barPos.y + height), IM_COL32(100, 100, 100, 255));

    // Move cursor past the bar
    TimeGUI::SetCursorScreenPos(TEVector2(barPos.x, barPos.y + height + 10));

    // Stats breakdown
    TimeGUI::TextColored(TEVector4(0.2f, 0.8f, 0.2f, 1.0f), "Game Update:  ");
    TimeGUI::SameLine();
    TimeGUI::Text("%.2f ms (%.1f%%)", m_CurrentMetrics.gameTime, gamePct * 100.0f);

    TimeGUI::TextColored(TEVector4(0.12f, 0.56f, 1.0f, 1.0f), "Render Flush: ");
    TimeGUI::SameLine();
    TimeGUI::Text("%.2f ms (%.1f%%)", m_CurrentMetrics.renderTime, renderPct * 100.0f);

    TimeGUI::TextColored(TEVector4(1.0f, 0.5f, 0.31f, 1.0f), "Physics Step: ");
    TimeGUI::SameLine();
    TimeGUI::Text("%.2f ms (%.1f%%)", m_CurrentMetrics.physicsTime, physicsPct * 100.0f);

    TimeGUI::TextColored(TEVector4(1.0f, 0.84f, 0.0f, 1.0f), "UI Render:    ");
    TimeGUI::SameLine();
    TimeGUI::Text("%.2f ms (%.1f%%)", m_CurrentMetrics.uiTime, uiPct * 100.0f);

    TimeGUI::Separator();

    // System Info
    if (m_ShowSystemInfo)
    {
        TimeGUI::Text("CPU: %s (%d cores)", m_CPUName.c_str(), m_CPUCores);
        TimeGUI::Text("GPU: %s", m_GPUName.c_str());
        TimeGUI::Text("RAM: %s", FormatBytes(m_RAMTotal * 1024 * 1024).c_str());
    }
}

void ProfilingLayer::RenderRenderingInfo()
{
    // Draw Calls
    TimeGUI::Text("Draw Calls: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_FPSColor, "%u", m_CurrentMetrics.drawCalls);

    // Triangles
    TimeGUI::Text("Triangles: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_FPSColor, "%u", m_CurrentMetrics.triangles);

    // Vertices
    TimeGUI::Text("Vertices: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_FPSColor, "%u", m_CurrentMetrics.vertices);

    TimeGUI::Separator();

    // Textures
    TimeGUI::Text("Textures: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_CPUColor, "%u", m_CurrentMetrics.textures);

    // Shaders
    TimeGUI::Text("Shaders: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_CPUColor, "%u", m_CurrentMetrics.shaders);

    TimeGUI::Separator();

    // Performance metrics
    if (m_CurrentMetrics.fps < 30.0f)
    {
        TimeGUI::TextColored(m_CriticalColor, "WARNING: Low FPS detected!");
    }
    else if (m_CurrentMetrics.fps < 60.0f)
    {
        TimeGUI::TextColored(m_WarningColor, "Notice: FPS below 60");
    }
}

void ProfilingLayer::RenderMemoryInfo()
{
    // GPU Memory
    TimeGUI::Text("GPU Memory: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_GPUColor, "%.1f MB", m_CurrentMetrics.gpuMemory);

    // VRAM Usage
    TimeGUI::Text("VRAM Usage: ");
    TimeGUI::SameLine();
    TimeGUI::TextColored(m_GPUColor, "%.1f%%", m_CurrentMetrics.vramUsage);

    TimeGUI::Separator();

    // Memory warnings
    if (m_CurrentMetrics.ramUsage > 90.0f)
    {
        TimeGUI::TextColored(m_CriticalColor, "CRITICAL: High RAM usage!");
    }
    else if (m_CurrentMetrics.ramUsage > 80.0f)
    {
        TimeGUI::TextColored(m_WarningColor, "Warning: High RAM usage");
    }

    if (m_CurrentMetrics.vramUsage > 90.0f)
    {
        TimeGUI::TextColored(m_CriticalColor, "CRITICAL: High VRAM usage!");
    }
    else if (m_CurrentMetrics.vramUsage > 80.0f)
    {
        TimeGUI::TextColored(m_WarningColor, "Warning: High VRAM usage");
    }

    // Graphs
    if (m_ShowGraphs)
    {
        RenderGraph("Heap Usage (MB)", m_HeapHistory, m_RAMColor, 0.0f, 256.0f);
        RenderGraph("Stack Usage (KB)", m_StackHistory, m_CPUColor, 0.0f, 64.0f);
    }

    TimeGUI::Separator();

    // Class Allocations Table (Heap)
    TimeGUI::Text("Class Allocations (Heap)");
    if (TimeGUI::BeginTable("ClassAllocationsTable", 3, 1))
    {
        TimeGUI::TableSetupColumn("Class / Component");
        TimeGUI::TableSetupColumn("Active Count");
        TimeGUI::TableSetupColumn("Size");
        TimeGUI::TableHeadersRow();

        for (const auto &pair : m_ClassAllocations)
        {
            TimeGUI::TableNextRow();
            TimeGUI::TableNextColumn();
            TimeGUI::Text("%s", pair.first.c_str());
            TimeGUI::TableNextColumn();
            TimeGUI::Text("%u", (uint32_t)pair.second.count);
            TimeGUI::TableNextColumn();
            if (pair.second.sizeBytes >= 1024 * 1024)
            {
                TimeGUI::Text("%.2f MB", (float)pair.second.sizeBytes / (1024.0f * 1024.0f));
            }
            else if (pair.second.sizeBytes >= 1024)
            {
                TimeGUI::Text("%.2f KB", (float)pair.second.sizeBytes / 1024.0f);
            }
            else
            {
                TimeGUI::Text("%u Bytes", (uint32_t)pair.second.sizeBytes);
            }
        }
        TimeGUI::EndTable();
    }

    TimeGUI::Dummy(TEVector2(0, 10));

    // Function/Stack frame allocations
    TimeGUI::Text("Active Stack Frame Contributions");
    if (TimeGUI::BeginTable("StackAllocationsTable", 2, 1))
    {
        TimeGUI::TableSetupColumn("Function / Scope");
        TimeGUI::TableSetupColumn("Stack Frame Size");
        TimeGUI::TableHeadersRow();

        for (const auto &pair : m_ActiveStackFrames)
        {
            TimeGUI::TableNextRow();
            TimeGUI::TableNextColumn();
            TimeGUI::Text("%s", pair.first.c_str());
            TimeGUI::TableNextColumn();
            TimeGUI::Text("%.2f KB (%u Bytes)", (float)pair.second / 1024.0f, (uint32_t)pair.second);
        }
        TimeGUI::EndTable();
    }
}

void ProfilingLayer::RenderPerformanceGraphs()
{
    if (!m_ShowGraphs)
        return;

    TimeGUI::Text("Performance Graphs");
    TimeGUI::Separator();

    // FPS Graph
    RenderGraph("FPS", m_FPSHistory, m_FPSColor, 0.0f, 120.0f);

    // Frame Time Graph
    RenderGraph("Frame Time (ms)", m_FrameTimeHistory, m_FPSColor, 0.0f, 33.0f);

    // Timing Breakdown Graphs
    RenderGraph("Game Update Time (ms)", m_GameTimeHistory, TEVector4(0.2f, 0.8f, 0.2f, 1.0f), 0.0f, 16.0f);
    RenderGraph("Render Flush Time (ms)", m_RenderTimeHistory, TEVector4(0.12f, 0.56f, 1.0f, 1.0f), 0.0f, 16.0f);
    RenderGraph("Physics Step Time (ms)", m_PhysicsTimeHistory, TEVector4(1.0f, 0.5f, 0.31f, 1.0f), 0.0f, 16.0f);
    RenderGraph("UI Render Time (ms)", m_UITimeHistory, TEVector4(1.0f, 0.84f, 0.0f, 1.0f), 0.0f, 16.0f);

    // CPU Usage Graph
    RenderGraph("CPU Usage (%)", m_CPUHistory, m_CPUColor, 0.0f, 100.0f);

    // RAM Usage Graph
    RenderGraph("RAM Usage (%)", m_RAMHistory, m_RAMColor, 0.0f, 100.0f);

    // GPU Usage Graph
    RenderGraph("GPU Usage (%)", m_GPUHistory, m_GPUColor, 0.0f, 100.0f);
}

void ProfilingLayer::RenderGraph(const std::string &title, const std::deque<float> &data, const TEVector4 &color,
                                 float minValue, float maxValue)
{
    if (data.empty())
        return;

    TimeGUI::Text("%s", title.c_str());

    TEVector2 graphPos = TimeGUI::GetCursorScreenPos();
    TEVector2 graphSize = TEVector2(TimeGUI::GetContentRegionAvail().x, 60.0f);

    TimeGUI::TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();
    DrawLineGraph(drawList, graphPos, graphSize, data, color, minValue, maxValue, title);

    TimeGUI::SetCursorScreenPos(TEVector2(graphPos.x, graphPos.y + graphSize.y + 5));
    TimeGUI::Separator();
}

void ProfilingLayer::DrawLineGraph(TimeGUI::TimeGUIDrawList drawList, const TEVector2 &pos, const TEVector2 &size,
                                   const std::deque<float> &data, const TEVector4 &color, float minValue,
                                   float maxValue, const std::string &label)
{
    if (data.empty())
        return;

    // Draw background
    drawList.AddRectFilled(pos, TEVector2(pos.x + size.x, pos.y + size.y), IM_COL32(20, 20, 20, 255));

    // Draw grid lines
    for (int i = 0; i <= 4; i++)
    {
        float y = pos.y + (size.y * i / 4.0f);
        drawList.AddLine(TEVector2(pos.x, y), TEVector2(pos.x + size.x, y), IM_COL32(50, 50, 50, 255));
    }

    // Draw data line
    if (data.size() > 1)
    {
        float range = maxValue - minValue;
        if (range <= 0.0f)
            range = 1.0f;

        for (size_t i = 1; i < data.size(); i++)
        {
            float x1 = pos.x + (size.x * (i - 1) / (data.size() - 1));
            float y1 = pos.y + size.y - (size.y * (data[i - 1] - minValue) / range);
            float x2 = pos.x + (size.x * i / (data.size() - 1));
            float y2 = pos.y + size.y - (size.y * (data[i] - minValue) / range);

            drawList.AddLine(TEVector2(x1, y1), TEVector2(x2, y2),
                             IM_COL32(color.x * 255, color.y * 255, color.z * 255, color.w * 255), 2.0f);
        }
    }

    // Draw border
    drawList.AddRect(pos, TEVector2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 100, 100, 255));
}

void ProfilingLayer::DrawBarGraph(TimeGUI::TimeGUIDrawList drawList, const TEVector2 &pos, const TEVector2 &size,
                                  const std::deque<float> &data, const TEVector4 &color, float minValue, float maxValue,
                                  const std::string &label)
{
    if (data.empty())
        return;

    // Draw background
    drawList.AddRectFilled(pos, TEVector2(pos.x + size.x, pos.y + size.y), IM_COL32(20, 20, 20, 255));

    // Draw bars
    float barWidth = size.x / data.size();
    float range = maxValue - minValue;
    if (range <= 0.0f)
        range = 1.0f;

    for (size_t i = 0; i < data.size(); i++)
    {
        float barHeight = (data[i] - minValue) / range * size.y;
        float x = pos.x + i * barWidth;
        float y = pos.y + size.y - barHeight;

        drawList.AddRectFilled(TEVector2(x, y), TEVector2(x + barWidth - 1, pos.y + size.y),
                               IM_COL32(color.x * 255, color.y * 255, color.z * 255, color.w * 255));
    }

    // Draw border
    drawList.AddRect(pos, TEVector2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 100, 100, 255));
}

std::string ProfilingLayer::FormatBytes(uint64_t bytes)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = (double)bytes;

    while (size >= 1024.0 && unitIndex < 4)
    {
        size /= 1024.0;
        unitIndex++;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
    return ss.str();
}

std::string ProfilingLayer::FormatTime(float seconds)
{
    if (seconds < 0.001f)
    {
        return std::to_string((int)(seconds * 1000000.0f)) + " μs";
    }
    else if (seconds < 1.0f)
    {
        return std::to_string((int)(seconds * 1000.0f)) + " ms";
    }
    else
    {
        return std::to_string((int)seconds) + " s";
    }
}

std::string ProfilingLayer::FormatPercentage(float percentage)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << percentage << "%";
    return ss.str();
}

TEVector4 ProfilingLayer::GetColorForValue(float value, float warningThreshold, float criticalThreshold)
{
    if (value >= criticalThreshold)
    {
        return m_CriticalColor;
    }
    else if (value >= warningThreshold)
    {
        return m_WarningColor;
    }
    else
    {
        return m_FPSColor;
    }
}

} // namespace TE
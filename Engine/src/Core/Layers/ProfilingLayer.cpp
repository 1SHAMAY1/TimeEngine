#include "Layers/ProfilingLayer.hpp"
#include "Core/Log.h"
#include "imgui.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")
#else
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#endif

namespace TE {

    ProfilingLayer::ProfilingLayer()
        : Layer("ProfilingLayer") {
        m_LastFrameTime = std::chrono::high_resolution_clock::now();
        m_LastUpdateTime = m_LastFrameTime;
        GetSystemInfo();
    }

    ProfilingLayer::~ProfilingLayer() {
    }

    void ProfilingLayer::OnAttach() {
        TE_CORE_INFO("ProfilingLayer: OnAttach called.");
        m_LastFrameTime = std::chrono::high_resolution_clock::now();
        m_LastUpdateTime = m_LastFrameTime;
        m_FrameCount = 0;
        m_AccumulatedTime = 0.0f;
    }

    void ProfilingLayer::OnDetach() {
        TE_CORE_INFO("ProfilingLayer: OnDetach called.");
    }

    void ProfilingLayer::OnUpdate() {
        BeginFrame();
    }

    void ProfilingLayer::OnImGuiRender() {
        RenderPerformanceWindow();
    }

    void ProfilingLayer::OnEvent(Event& event) {
        // Handle any profiling-related events if needed
    }

    void ProfilingLayer::BeginFrame() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto frameDuration = std::chrono::duration<float>(currentTime - m_LastFrameTime);
        m_CurrentMetrics.frameTime = frameDuration.count();
        m_LastFrameTime = currentTime;

        if (m_CurrentMetrics.frameTime > 0.0f) {
            m_CurrentMetrics.fps = 1.0f / m_CurrentMetrics.frameTime;
        }

        m_AccumulatedTime += m_CurrentMetrics.frameTime;
        m_FrameCount++;

        // Update metrics periodically
        auto updateDuration = std::chrono::duration<float>(currentTime - m_LastUpdateTime);
        if (updateDuration.count() >= m_UpdateInterval) {
            UpdateMetrics();
            m_LastUpdateTime = currentTime;
        }
    }

    void ProfilingLayer::EndFrame() {
        // Reset frame-specific counters
        m_CurrentMetrics.drawCalls = 0;
        m_CurrentMetrics.triangles = 0;
        m_CurrentMetrics.vertices = 0;
        m_CurrentMetrics.textures = 0;
        m_CurrentMetrics.shaders = 0;
    }

    void ProfilingLayer::RecordDrawCall() {
        m_CurrentMetrics.drawCalls++;
    }

    void ProfilingLayer::RecordTriangle(uint32_t count) {
        m_CurrentMetrics.triangles += count;
    }

    void ProfilingLayer::RecordVertex(uint32_t count) {
        m_CurrentMetrics.vertices += count;
    }

    void ProfilingLayer::RecordTexture(uint32_t count) {
        m_CurrentMetrics.textures += count;
    }

    void ProfilingLayer::RecordShader(uint32_t count) {
        m_CurrentMetrics.shaders += count;
    }

    void ProfilingLayer::UpdateMetrics() {
        UpdateSystemMetrics();
        UpdateGPUMetrics();

        // Add current metrics to history
        m_MetricsHistory.push_back(m_CurrentMetrics);
        m_FPSHistory.push_back(m_CurrentMetrics.fps);
        m_FrameTimeHistory.push_back(m_CurrentMetrics.frameTime);
        m_CPUHistory.push_back(m_CurrentMetrics.cpuUsage);
        m_RAMHistory.push_back(m_CurrentMetrics.ramUsage);
        m_GPUHistory.push_back(m_CurrentMetrics.gpuUsage);

        // Maintain history size
        if (m_MetricsHistory.size() > m_MaxHistorySize) {
            m_MetricsHistory.pop_front();
            m_FPSHistory.pop_front();
            m_FrameTimeHistory.pop_front();
            m_CPUHistory.pop_front();
            m_RAMHistory.pop_front();
            m_GPUHistory.pop_front();
        }

        CalculateAverages();
    }

    void ProfilingLayer::UpdateSystemMetrics() {
        m_CurrentMetrics.cpuUsage = GetSystemCPUUsage();
        m_CurrentMetrics.ramUsage = GetSystemRAMUsage();
    }

    void ProfilingLayer::UpdateGPUMetrics() {
        m_CurrentMetrics.gpuUsage = GetSystemGPUUsage();
        // GPU memory and VRAM usage would be implemented based on the graphics API
        // For now, we'll use placeholder values
        m_CurrentMetrics.gpuMemory = 0.0f;
        m_CurrentMetrics.vramUsage = 0.0f;
    }

    float ProfilingLayer::GetAverageFPS() const {
        if (m_FPSHistory.empty()) return 0.0f;
        return std::accumulate(m_FPSHistory.begin(), m_FPSHistory.end(), 0.0f) / m_FPSHistory.size();
    }

    float ProfilingLayer::GetAverageFrameTime() const {
        if (m_FrameTimeHistory.empty()) return 0.0f;
        return std::accumulate(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end(), 0.0f) / m_FrameTimeHistory.size();
    }

    float ProfilingLayer::GetAverageCPUUsage() const {
        if (m_CPUHistory.empty()) return 0.0f;
        return std::accumulate(m_CPUHistory.begin(), m_CPUHistory.end(), 0.0f) / m_CPUHistory.size();
    }

    float ProfilingLayer::GetAverageRAMUsage() const {
        if (m_RAMHistory.empty()) return 0.0f;
        return std::accumulate(m_RAMHistory.begin(), m_RAMHistory.end(), 0.0f) / m_RAMHistory.size();
    }

    float ProfilingLayer::GetAverageGPUUsage() const {
        if (m_GPUHistory.empty()) return 0.0f;
        return std::accumulate(m_GPUHistory.begin(), m_GPUHistory.end(), 0.0f) / m_GPUHistory.size();
    }

    void ProfilingLayer::CalculateAverages() {
        // This could be used for more complex averaging if needed
    }

    float ProfilingLayer::GetSystemCPUUsage() {
#ifdef _WIN32
        static PDH_HQUERY cpuQuery = NULL;
        static PDH_HCOUNTER cpuTotal = NULL;
        
        if (cpuQuery == NULL) {
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

    float ProfilingLayer::GetSystemRAMUsage() {
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

    float ProfilingLayer::GetSystemGPUUsage() {
        // This would require platform-specific GPU monitoring libraries
        // For now, return a placeholder value
        return 0.0f;
    }

    void ProfilingLayer::GetSystemInfo() {
#ifdef _WIN32
        // Get CPU info
        char cpuName[256];
        DWORD size = sizeof(cpuName);
        if (RegGetValueA(HKEY_LOCAL_MACHINE, 
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
            "ProcessorNameString", RRF_RT_ANY, NULL, cpuName, &size) == ERROR_SUCCESS) {
            m_CPUName = cpuName;
        } else {
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

        m_GPUName = "Unknown GPU"; // Would need to query GPU info
        m_VRAMTotal = 0; // Would need to query GPU VRAM
#else
        // Linux implementation would go here
        m_CPUName = "Unknown CPU";
        m_GPUName = "Unknown GPU";
        m_CPUCores = 0;
        m_RAMTotal = 0;
        m_VRAMTotal = 0;
#endif
    }

    void ProfilingLayer::RenderPerformanceWindow() {
        if (!m_IsVisible) return;
        
        ImGui::SetNextWindowPos(m_WindowPos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(m_WindowSize, ImGuiCond_FirstUseEver);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
        if (!m_WindowResizable) windowFlags |= ImGuiWindowFlags_NoResize;
        if (m_IsFloating) {
            windowFlags |= ImGuiWindowFlags_NoDocking;
        }
        // When not floating, allow docking by not adding NoDocking flag

        bool windowOpen = true;
        if (ImGui::Begin(m_WindowTitle.c_str(), &windowOpen, windowFlags)) {
            if (ImGui::BeginTabBar("PerformanceTabs")) {
                if (ImGui::BeginTabItem("Overview")) {
                    RenderSystemInfo();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Rendering")) {
                    RenderRenderingInfo();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Memory")) {
                    RenderMemoryInfo();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Graphs")) {
                    RenderPerformanceGraphs();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            // Settings
            if (ImGui::CollapsingHeader("Settings")) {
                ImGui::SliderFloat("Update Interval", &m_UpdateInterval, 0.01f, 1.0f, "%.2f s");
                ImGui::SliderInt("History Size", (int*)&m_MaxHistorySize, 10, 1000);
                ImGui::Checkbox("Show Detailed Info", &m_ShowDetailedInfo);
                ImGui::Checkbox("Show Graphs", &m_ShowGraphs);
            }
        }
        ImGui::End();
        
        // Handle window close
        if (!windowOpen) {
            m_IsVisible = false;
        }
    }

    void ProfilingLayer::RenderSystemInfo() {
        // FPS and Frame Time
        ImGui::Text("FPS: ");
        ImGui::SameLine();
        ImGui::TextColored(m_FPSColor, "%.1f", m_CurrentMetrics.fps);
        ImGui::SameLine();
        ImGui::Text("(%.1f avg)", GetAverageFPS());

        ImGui::Text("Frame Time: ");
        ImGui::SameLine();
        ImGui::TextColored(m_FPSColor, "%.2f ms", m_CurrentMetrics.frameTime * 1000.0f);
        ImGui::SameLine();
        ImGui::Text("(%.2f avg)", GetAverageFrameTime() * 1000.0f);

        ImGui::Separator();

        // CPU Usage
        ImGui::Text("CPU Usage: ");
        ImGui::SameLine();
        ImVec4 cpuColor = GetColorForValue(m_CurrentMetrics.cpuUsage, 70.0f, 90.0f);
        ImGui::TextColored(cpuColor, "%.1f%%", m_CurrentMetrics.cpuUsage);
        ImGui::SameLine();
        ImGui::Text("(%.1f avg)", GetAverageCPUUsage());

        // RAM Usage
        ImGui::Text("RAM Usage: ");
        ImGui::SameLine();
        ImVec4 ramColor = GetColorForValue(m_CurrentMetrics.ramUsage, 70.0f, 90.0f);
        ImGui::TextColored(ramColor, "%.1f%%", m_CurrentMetrics.ramUsage);
        ImGui::SameLine();
        ImGui::Text("(%.1f avg)", GetAverageRAMUsage());

        // GPU Usage
        ImGui::Text("GPU Usage: ");
        ImGui::SameLine();
        ImVec4 gpuColor = GetColorForValue(m_CurrentMetrics.gpuUsage, 70.0f, 90.0f);
        ImGui::TextColored(gpuColor, "%.1f%%", m_CurrentMetrics.gpuUsage);
        ImGui::SameLine();
        ImGui::Text("(%.1f avg)", GetAverageGPUUsage());

        ImGui::Separator();

        // System Info
        if (m_ShowSystemInfo) {
            ImGui::Text("CPU: %s (%d cores)", m_CPUName.c_str(), m_CPUCores);
            ImGui::Text("GPU: %s", m_GPUName.c_str());
            ImGui::Text("RAM: %s", FormatBytes(m_RAMTotal * 1024 * 1024).c_str());
        }
    }

    void ProfilingLayer::RenderRenderingInfo() {
        // Draw Calls
        ImGui::Text("Draw Calls: ");
        ImGui::SameLine();
        ImGui::TextColored(m_FPSColor, "%u", m_CurrentMetrics.drawCalls);

        // Triangles
        ImGui::Text("Triangles: ");
        ImGui::SameLine();
        ImGui::TextColored(m_FPSColor, "%u", m_CurrentMetrics.triangles);

        // Vertices
        ImGui::Text("Vertices: ");
        ImGui::SameLine();
        ImGui::TextColored(m_FPSColor, "%u", m_CurrentMetrics.vertices);

        ImGui::Separator();

        // Textures
        ImGui::Text("Textures: ");
        ImGui::SameLine();
        ImGui::TextColored(m_CPUColor, "%u", m_CurrentMetrics.textures);

        // Shaders
        ImGui::Text("Shaders: ");
        ImGui::SameLine();
        ImGui::TextColored(m_CPUColor, "%u", m_CurrentMetrics.shaders);

        ImGui::Separator();

        // Performance metrics
        if (m_CurrentMetrics.fps < 30.0f) {
            ImGui::TextColored(m_CriticalColor, "WARNING: Low FPS detected!");
        } else if (m_CurrentMetrics.fps < 60.0f) {
            ImGui::TextColored(m_WarningColor, "Notice: FPS below 60");
        }
    }

    void ProfilingLayer::RenderMemoryInfo() {
        // GPU Memory
        ImGui::Text("GPU Memory: ");
        ImGui::SameLine();
        ImGui::TextColored(m_GPUColor, "%.1f MB", m_CurrentMetrics.gpuMemory);

        // VRAM Usage
        ImGui::Text("VRAM Usage: ");
        ImGui::SameLine();
        ImGui::TextColored(m_GPUColor, "%.1f%%", m_CurrentMetrics.vramUsage);

        ImGui::Separator();

        // Memory warnings
        if (m_CurrentMetrics.ramUsage > 90.0f) {
            ImGui::TextColored(m_CriticalColor, "CRITICAL: High RAM usage!");
        } else if (m_CurrentMetrics.ramUsage > 80.0f) {
            ImGui::TextColored(m_WarningColor, "Warning: High RAM usage");
        }

        if (m_CurrentMetrics.vramUsage > 90.0f) {
            ImGui::TextColored(m_CriticalColor, "CRITICAL: High VRAM usage!");
        } else if (m_CurrentMetrics.vramUsage > 80.0f) {
            ImGui::TextColored(m_WarningColor, "Warning: High VRAM usage");
        }
    }

    void ProfilingLayer::RenderPerformanceGraphs() {
        if (!m_ShowGraphs) return;

        ImGui::Text("Performance Graphs");
        ImGui::Separator();

        // FPS Graph
        RenderGraph("FPS", m_FPSHistory, m_FPSColor, 0.0f, 120.0f);

        // Frame Time Graph
        RenderGraph("Frame Time (ms)", m_FrameTimeHistory, m_FPSColor, 0.0f, 33.0f);

        // CPU Usage Graph
        RenderGraph("CPU Usage (%)", m_CPUHistory, m_CPUColor, 0.0f, 100.0f);

        // RAM Usage Graph
        RenderGraph("RAM Usage (%)", m_RAMHistory, m_RAMColor, 0.0f, 100.0f);

        // GPU Usage Graph
        RenderGraph("GPU Usage (%)", m_GPUHistory, m_GPUColor, 0.0f, 100.0f);
    }

    void ProfilingLayer::RenderGraph(const std::string& title, const std::deque<float>& data, const ImVec4& color, float minValue, float maxValue) {
        if (data.empty()) return;

        ImGui::Text("%s", title.c_str());
        
        ImVec2 graphPos = ImGui::GetCursorScreenPos();
        ImVec2 graphSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        DrawLineGraph(drawList, graphPos, graphSize, data, color, minValue, maxValue, title);
        
        ImGui::SetCursorScreenPos(ImVec2(graphPos.x, graphPos.y + graphSize.y + 5));
        ImGui::Separator();
    }

    void ProfilingLayer::DrawLineGraph(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size, 
                                      const std::deque<float>& data, const ImVec4& color, 
                                      float minValue, float maxValue, const std::string& label) {
        if (data.empty()) return;

        // Draw background
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                               IM_COL32(20, 20, 20, 255));

        // Draw grid lines
        for (int i = 0; i <= 4; i++) {
            float y = pos.y + (size.y * i / 4.0f);
            drawList->AddLine(ImVec2(pos.x, y), ImVec2(pos.x + size.x, y), 
                             IM_COL32(50, 50, 50, 255));
        }

        // Draw data line
        if (data.size() > 1) {
            float range = maxValue - minValue;
            if (range <= 0.0f) range = 1.0f;

            for (size_t i = 1; i < data.size(); i++) {
                float x1 = pos.x + (size.x * (i - 1) / (data.size() - 1));
                float y1 = pos.y + size.y - (size.y * (data[i - 1] - minValue) / range);
                float x2 = pos.x + (size.x * i / (data.size() - 1));
                float y2 = pos.y + size.y - (size.y * (data[i] - minValue) / range);

                drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), 
                                 IM_COL32(color.x * 255, color.y * 255, color.z * 255, color.w * 255), 2.0f);
            }
        }

        // Draw border
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                         IM_COL32(100, 100, 100, 255));
    }

    void ProfilingLayer::DrawBarGraph(ImDrawList* drawList, const ImVec2& pos, const ImVec2& size,
                                     const std::deque<float>& data, const ImVec4& color,
                                     float minValue, float maxValue, const std::string& label) {
        if (data.empty()) return;

        // Draw background
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                               IM_COL32(20, 20, 20, 255));

        // Draw bars
        float barWidth = size.x / data.size();
        float range = maxValue - minValue;
        if (range <= 0.0f) range = 1.0f;

        for (size_t i = 0; i < data.size(); i++) {
            float barHeight = (data[i] - minValue) / range * size.y;
            float x = pos.x + i * barWidth;
            float y = pos.y + size.y - barHeight;

            drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + barWidth - 1, pos.y + size.y), 
                                   IM_COL32(color.x * 255, color.y * 255, color.z * 255, color.w * 255));
        }

        // Draw border
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                         IM_COL32(100, 100, 100, 255));
    }

    std::string ProfilingLayer::FormatBytes(uint64_t bytes) {
        const char* units[] = { "B", "KB", "MB", "GB", "TB" };
        int unitIndex = 0;
        double size = (double)bytes;

        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
        return ss.str();
    }

    std::string ProfilingLayer::FormatTime(float seconds) {
        if (seconds < 0.001f) {
            return std::to_string((int)(seconds * 1000000.0f)) + " μs";
        } else if (seconds < 1.0f) {
            return std::to_string((int)(seconds * 1000.0f)) + " ms";
        } else {
            return std::to_string((int)seconds) + " s";
        }
    }

    std::string ProfilingLayer::FormatPercentage(float percentage) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << percentage << "%";
        return ss.str();
    }

    ImVec4 ProfilingLayer::GetColorForValue(float value, float warningThreshold, float criticalThreshold) {
        if (value >= criticalThreshold) {
            return m_CriticalColor;
        } else if (value >= warningThreshold) {
            return m_WarningColor;
        } else {
            return m_FPSColor;
        }
    }

} 
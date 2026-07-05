#pragma once
#include "Layers/Layer.hpp"
#include "Utils/TimeGUI.hpp"
#include <chrono>
#include <deque>
#include <string>
#include <vector>

namespace TE {

    struct PerformanceMetrics {
        float fps = 0.0f;
        float frameTime = 0.0f;
        float cpuUsage = 0.0f;
        float ramUsage = 0.0f;
        float gpuUsage = 0.0f;
        uint32_t drawCalls = 0;
        uint32_t triangles = 0;
        uint32_t vertices = 0;
        uint32_t textures = 0;
        uint32_t shaders = 0;
        float gpuMemory = 0.0f;
        float vramUsage = 0.0f;
    };

    class ProfilingLayer : public Layer {
    public:
        ProfilingLayer();
        virtual ~ProfilingLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnTimeGUIRender() override;
        virtual void OnEvent(Event& event) override;

        // ===== Performance Tracking =====
        void BeginFrame();
        void EndFrame();
        void RecordDrawCall();
        void RecordTriangle(uint32_t count);
        void RecordVertex(uint32_t count);
        void RecordTexture(uint32_t count);
        void RecordShader(uint32_t count);

        // ===== System Info =====
        void UpdateSystemMetrics();
        void UpdateGPUMetrics();

        // ===== Getters =====
        const PerformanceMetrics& GetCurrentMetrics() const { return m_CurrentMetrics; }
        float GetAverageFPS() const;
        float GetAverageFrameTime() const;
        float GetAverageCPUUsage() const;
        float GetAverageRAMUsage() const;
        float GetAverageGPUUsage() const;

        // ===== Settings =====
        void SetMaxHistorySize(size_t size) { m_MaxHistorySize = size; }
        void SetUpdateInterval(float interval) { m_UpdateInterval = interval; }
        void SetShowDetailedInfo(bool show) { m_ShowDetailedInfo = show; }
        void SetShowGraphs(bool show) { m_ShowGraphs = show; }
        void SetFloating(bool floating) { m_IsFloating = floating; }
        void SetVisible(bool visible) { m_IsVisible = visible; }
        void SetWindowTitle(const std::string& title) { m_WindowTitle = title; }
        void SetWindowPosition(const TEVector2& pos) { m_WindowPos = pos; }
        void SetWindowSize(const TEVector2& size) { m_WindowSize = size; }
        bool IsVisible() const { return m_IsVisible; }
        void ToggleVisibility() { m_IsVisible = !m_IsVisible; }

    private:
        // ===== Performance Data =====
        PerformanceMetrics m_CurrentMetrics;
        std::deque<PerformanceMetrics> m_MetricsHistory;
        std::deque<float> m_FPSHistory;
        std::deque<float> m_FrameTimeHistory;
        std::deque<float> m_CPUHistory;
        std::deque<float> m_RAMHistory;
        std::deque<float> m_GPUHistory;

        // ===== Timing =====
        std::chrono::high_resolution_clock::time_point m_LastFrameTime;
        std::chrono::high_resolution_clock::time_point m_LastUpdateTime;
        float m_UpdateInterval = 0.1f; // Update every 100ms
        float m_AccumulatedTime = 0.0f;
        uint32_t m_FrameCount = 0;

        // ===== Display Settings =====
        size_t m_MaxHistorySize = 100;
        bool m_ShowDetailedInfo = true;
        bool m_ShowGraphs = true;
        bool m_ShowSystemInfo = true;
        bool m_ShowRenderingInfo = true;
        bool m_ShowMemoryInfo = true;
        bool m_ShowPerformanceGraphs = true;

        // ===== Window Settings =====
        TEVector2 m_WindowSize = TEVector2(300, 200);
        TEVector2 m_WindowPos = TEVector2(10, 10);
        bool m_WindowCollapsed = false;
        bool m_WindowResizable = true;
        bool m_IsFloating = false;
        bool m_IsVisible = true;
        std::string m_WindowTitle = "Performance Monitor";

        // ===== Colors =====
        TEVector4 m_FPSColor = TEVector4(0.2f, 1.0f, 0.2f, 1.0f);
        TEVector4 m_CPUColor = TEVector4(1.0f, 0.8f, 0.2f, 1.0f);
        TEVector4 m_RAMColor = TEVector4(0.2f, 0.8f, 1.0f, 1.0f);
        TEVector4 m_GPUColor = TEVector4(1.0f, 0.4f, 0.8f, 1.0f);
        TEVector4 m_WarningColor = TEVector4(1.0f, 0.5f, 0.0f, 1.0f);
        TEVector4 m_CriticalColor = TEVector4(1.0f, 0.0f, 0.0f, 1.0f);

        // ===== System Info =====
        std::string m_CPUName;
        std::string m_GPUName;
        uint32_t m_CPUCores = 0;
        uint32_t m_RAMTotal = 0;
        uint32_t m_VRAMTotal = 0;

        // ===== Helper Functions =====
        void RenderPerformanceWindow();
        void RenderSystemInfo();
        void RenderRenderingInfo();
        void RenderMemoryInfo();
        void RenderPerformanceGraphs();
        void RenderGraph(const std::string& title, const std::deque<float>& data, const TEVector4& color, float minValue = 0.0f, float maxValue = 100.0f);
        void UpdateMetrics();
        void CalculateAverages();
        float GetSystemCPUUsage();
        float GetSystemRAMUsage();
        float GetSystemGPUUsage();
        void GetSystemInfo();

        // ===== Graph Rendering =====
        void DrawLineGraph(TimeGUI::TimeGUIDrawList drawList, const TEVector2& pos, const TEVector2& size, 
                          const std::deque<float>& data, const TEVector4& color, 
                          float minValue, float maxValue, const std::string& label);
        void DrawBarGraph(TimeGUI::TimeGUIDrawList drawList, const TEVector2& pos, const TEVector2& size,
                         const std::deque<float>& data, const TEVector4& color,
                         float minValue, float maxValue, const std::string& label);

        // ===== Utility Functions =====
        std::string FormatBytes(uint64_t bytes);
        std::string FormatTime(float seconds);
        std::string FormatPercentage(float percentage);
        TEVector4 GetColorForValue(float value, float warningThreshold, float criticalThreshold);
    };

} 
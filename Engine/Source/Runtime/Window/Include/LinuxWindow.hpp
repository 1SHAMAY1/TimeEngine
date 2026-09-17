#pragma once
#include "IWindow.hpp"

struct GLFWwindow;

class LinuxWindow : public IWindow
{
public:
    LinuxWindow(const WindowProps &props);
    virtual ~LinuxWindow();

    void OnUpdate() override;

    unsigned int GetWidth() const override { return m_Data.Width; }
    unsigned int GetHeight() const override { return m_Data.Height; }

    void SetEventCallback(const EventCallbackFn &callback) override { m_Data.EventCallback = callback; }
    void SetVSync(bool enabled) override;
    bool IsVSync() const override;

    void *GetNativeWindow() const override { return m_Window; }

    void ShowWindow() override;
    bool IsVisible() const override;

    void *GetGLLoaderFunction() const;

private:
    virtual void Init(const WindowProps &props);
    virtual void Shutdown();

    GLFWwindow *m_Window;

    struct WindowData
    {
        TEString Title;
        unsigned int Width, Height;
        bool VSync;

        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};

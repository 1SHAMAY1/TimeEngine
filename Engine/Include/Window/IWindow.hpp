#pragma once

#include "Core/PreRequisites.h"
#include "Core/Events/Event.h"

struct WindowProps {
    std::string Title;
    unsigned int Width;
    unsigned int Height;

    WindowProps(const std::string& title = "Time Engine",
                unsigned int width = 1280,
                unsigned int height = 720)
        : Title(title), Width(width), Height(height) {}
};

class IWindow {
public:
    using EventCallbackFn = std::function<void(TE::Event&)>;

    virtual ~IWindow() = default;

    virtual void OnUpdate() = 0;

    virtual unsigned int GetWidth() const = 0;
    virtual unsigned int GetHeight() const = 0;

    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;

    virtual void* GetNativeWindow() const = 0;

    static IWindow* Create(const WindowProps& props = WindowProps());
};

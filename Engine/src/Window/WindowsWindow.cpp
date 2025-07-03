#include "Window/WindowsWindow.hpp"
#include <GLFW/glfw3.h>

#include "Core/Log.h"
#include "Core/Events/ApplicationEvent.h"

static bool s_GLFWInitialized = false;

WindowsWindow::WindowsWindow(const WindowProps& props) {
    m_Window = nullptr; // ✅ Always null-init pointer
    Init(props);
}

WindowsWindow::~WindowsWindow() {
    Shutdown();
}

void WindowsWindow::Init(const WindowProps& props) {
    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;

    if (!s_GLFWInitialized) {
        int success = glfwInit();
        if (!success) {
            TE_CORE_ERROR("Could not initialize GLFW!");
            return;
        }
        TE_CORE_INFO("GLFW initialized successfully.");
        s_GLFWInitialized = true;
    }

    m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        TE_CORE_ERROR("Failed to create GLFW window!");
        return;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSetWindowUserPointer(m_Window, &m_Data);

    SetVSync(true);

    // === EVENT CALLBACKS ===

    // Resize
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.Width = width;
        data.Height = height;

        TE::WindowResizeEvent event(width, height);
        data.EventCallback(event);
    });

    // Close
    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        TE::WindowCloseEvent event;
        data.EventCallback(event);
    });

    // Minimize (via focus lost/gain)
    glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        if (iconified) {
            TE::WindowLostFocusEvent event;
            data.EventCallback(event);
        } else {
            TE::WindowFocusEvent event;
            data.EventCallback(event);
        }
    });

    TE_CORE_INFO("Window successfully created: {0} ({1}x{2})", m_Data.Title, m_Data.Width, m_Data.Height);
}

void WindowsWindow::Shutdown() {
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        TE_CORE_INFO("GLFW window destroyed.");
    }
}

void WindowsWindow::OnUpdate() {
    if (!m_Window) {
        TE_CORE_ERROR("OnUpdate() called on null window!");
        return;
    }

    glfwPollEvents();
    glfwSwapBuffers(m_Window);
}

void WindowsWindow::SetVSync(bool enabled) {
    if (m_Window)
        glfwSwapInterval(enabled ? 1 : 0);
    m_Data.VSync = enabled;
}

bool WindowsWindow::IsVSync() const {
    return m_Data.VSync;
}

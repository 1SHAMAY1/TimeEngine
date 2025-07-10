#include "Window/WindowsWindow.hpp"
#include <GLFW/glfw3.h>

#include "Core/Log.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Input/Input.hpp"
#include "Core/EngineSettings.hpp"
#include "Renderer/RendererContext.hpp"

static bool s_GLFWInitialized = false;

WindowsWindow::WindowsWindow(const WindowProps& props) {
    m_Window = nullptr;
    Init(props);
}

WindowsWindow::~WindowsWindow() {
    Shutdown();
}

void* WindowsWindow::GetGLLoaderFunction() const {
    return (void*)glfwGetProcAddress;
}

void WindowsWindow::Init(const WindowProps& props) {
    TE::RendererContext::EnableBestGPU();
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

    TE::Input::Init(m_Window); // Register window with input system

    SetVSync(true);

    // === EVENT CALLBACKS ===
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.Width = width;
        data.Height = height;
        TE::WindowResizeEvent event(width, height);
        TE_CORE_DEBUG(event.ToString());
        data.EventCallback(event);
    });

    // === FRAMEBUFFER SIZE CALLBACK ===
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        TE::WindowCloseEvent event;
        TE_CORE_DEBUG(event.ToString());
        data.EventCallback(event);
    });

    glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        if (iconified) {
            TE::WindowLostFocusEvent event;
            TE_CORE_DEBUG(event.ToString());
            data.EventCallback(event);
        } else {
            TE::WindowFocusEvent event;
            TE_CORE_DEBUG(event.ToString());
            data.EventCallback(event);
        }
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action) {
            case GLFW_PRESS: {
                TE::KeyPressedEvent event((TE::KeyCode)key, false);
                TE_CORE_DEBUG(event.ToString());
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                TE::KeyReleasedEvent event((TE::KeyCode)key);
                TE_CORE_DEBUG(event.ToString());
                data.EventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
                TE::KeyPressedEvent event((TE::KeyCode)key, true);
                TE_CORE_DEBUG(event.ToString());
                data.EventCallback(event);
                break;
            }
        }
    });

    glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        TE::KeyTypedEvent event((TE::KeyCode)codepoint);
        TE_CORE_DEBUG(event.ToString());
        data.EventCallback(event);
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        if (action == GLFW_PRESS) {
            TE::MouseButtonPressedEvent event((TE::MouseCode)button);
            TE_CORE_DEBUG(event.ToString());
            data.EventCallback(event);
        } else if (action == GLFW_RELEASE) {
            TE::MouseButtonReleasedEvent event((TE::MouseCode)button);
            TE_CORE_DEBUG(event.ToString());
            data.EventCallback(event);
        }
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        TE::Input::SetMouseScrollDelta((float)xOffset, (float)yOffset);
        TE::MouseScrolledEvent event((float)xOffset, (float)yOffset);
        TE_CORE_DEBUG(event.ToString());
        data.EventCallback(event);
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        TE::MouseMovedEvent event((float)xpos, (float)ypos);
        TE_CORE_DEBUG(event.ToString());
        data.EventCallback(event);
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
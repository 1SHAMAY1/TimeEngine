// Windows/DXGI headers MUST be first.
// glfw3native.h (GLFW_EXPOSE_NATIVE_WIN32) pulls in windows.h.
// If windows.h arrives after engine headers its macros (near, far, min, max, ERROR...)
// corrupt already-parsed engine declarations. Defining the guards here prevents that.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <dxgi.h>

// Undefine polluting Windows.h macros
#ifdef ERROR
#undef ERROR
#endif

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

// Engine headers (windows.h already in scope — no macro surprise)
#include "Core/Asset/AssetManager.hpp"
#include "Core/EngineSettings.hpp"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Log.h"
#include "Input/Input.hpp"
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/RendererContext.hpp"
#include "Window/WindowsWindow.hpp"
#include <filesystem>

static bool s_GLFWInitialized = false;

WindowsWindow::WindowsWindow(const WindowProps &props)
{
    m_Window = nullptr;
    Init(props);
}

WindowsWindow::~WindowsWindow() { Shutdown(); }

void *WindowsWindow::GetGLLoaderFunction() const { return (void *)glfwGetProcAddress; }

void WindowsWindow::Init(const WindowProps &props)
{
    TE::RendererContext::EnableBestGPU();
    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;

    if (!s_GLFWInitialized)
    {
        int success = glfwInit();
        if (!success)
        {
            TE_CORE_ERROR("Could not initialize GLFW!");
            return;
        }
        TE_CORE_INFO("GLFW initialized successfully.");
        s_GLFWInitialized = true;
    }

    switch (TE::RendererContext::GetAPI())
    {
    case TE::GraphicsAPI::OpenGL:
        break;
    case TE::GraphicsAPI::Vulkan:
    case TE::GraphicsAPI::DirectX11:
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        break;
    default:
        break;
    }

    m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);
    if (!m_Window)
    {
        TE_CORE_ERROR("Failed to create GLFW window!");
        return;
    }

    if (TE::RendererContext::GetAPI() == TE::GraphicsAPI::OpenGL)
    {
        glfwMakeContextCurrent(m_Window);
    }
    else if (TE::RendererContext::GetAPI() == TE::GraphicsAPI::DirectX11)
    {
        HWND hwnd = glfwGetWin32Window(m_Window);
        auto *apiInstance = TE::RenderCommand::GetAPIInstance();
        auto *dx11API = dynamic_cast<TE::DirectX11RendererAPI *>(apiInstance);
        if (dx11API)
        {
            dx11API->InitWithWindow(hwnd, props.Width, props.Height);
        }
        else
        {
            TE_CORE_ERROR("RendererAPI is not DirectX11RendererAPI!");
        }
    }
    glfwSetWindowUserPointer(m_Window, &m_Data);

    TE::Input::Init(m_Window); // Register window with input system

    SetVSync(true);

    // Set Window Icon
    {
        GLFWimage images[1];
        std::string iconPath = "Resources/Branding/TimeEngineIcon.png";
        if (!std::filesystem::exists(iconPath))
            iconPath = "e:/TimeEngine/Resources/Branding/TimeEngineIcon.png";

        if (std::filesystem::exists(iconPath))
        {
            TE::ImageData img = TE::AssetManager::ImportImage(iconPath, 4);
            if (img.Data)
            {
                images[0].width = img.Width;
                images[0].height = img.Height;
                images[0].pixels = img.Data;
                glfwSetWindowIcon(m_Window, 1, images);
                TE::AssetManager::FreeImage(img.Data);
            }
            else
            {
                TE_CORE_WARN("Failed to load window icon: {0}", iconPath);
            }
        }
    }

    // === EVENT CALLBACKS ===
    glfwSetWindowSizeCallback(m_Window,
                              [](GLFWwindow *window, int width, int height)
                              {
                                  WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                                  data.Width = width;
                                  data.Height = height;
                                  TE::WindowResizeEvent event(width, height);
                                  TE_CORE_DEBUG(event.ToString());
                                  data.EventCallback(event);
                              });

    // === FRAMEBUFFER SIZE CALLBACK ===
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
                                   { TE::RenderCommand::SetViewport(0, 0, width, height); });

    glfwSetWindowCloseCallback(m_Window,
                               [](GLFWwindow *window)
                               {
                                   WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                                   TE::WindowCloseEvent event;
                                   TE_CORE_DEBUG(event.ToString());
                                   data.EventCallback(event);
                               });

    glfwSetWindowIconifyCallback(m_Window,
                                 [](GLFWwindow *window, int iconified)
                                 {
                                     WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                                     if (iconified)
                                     {
                                         TE::WindowLostFocusEvent event;
                                         TE_CORE_DEBUG(event.ToString());
                                         data.EventCallback(event);
                                     }
                                     else
                                     {
                                         TE::WindowFocusEvent event;
                                         TE_CORE_DEBUG(event.ToString());
                                         data.EventCallback(event);
                                     }
                                 });

    glfwSetKeyCallback(m_Window,
                       [](GLFWwindow *window, int key, int scancode, int action, int mods)
                       {
                           WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

                           switch (action)
                           {
                           case GLFW_PRESS:
                           {
                               TE::KeyPressedEvent event((TE::KeyCode)key, false);
                               TE_CORE_DEBUG(event.ToString());
                               data.EventCallback(event);
                               break;
                           }
                           case GLFW_RELEASE:
                           {
                               TE::KeyReleasedEvent event((TE::KeyCode)key);
                               TE_CORE_DEBUG(event.ToString());
                               data.EventCallback(event);
                               break;
                           }
                           case GLFW_REPEAT:
                           {
                               TE::KeyPressedEvent event((TE::KeyCode)key, true);
                               TE_CORE_DEBUG(event.ToString());
                               data.EventCallback(event);
                               break;
                           }
                           }
                       });

    glfwSetCharCallback(m_Window,
                        [](GLFWwindow *window, unsigned int codepoint)
                        {
                            WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                            TE::KeyTypedEvent event((TE::KeyCode)codepoint);
                            TE_CORE_DEBUG(event.ToString());
                            data.EventCallback(event);
                        });

    glfwSetMouseButtonCallback(m_Window,
                               [](GLFWwindow *window, int button, int action, int mods)
                               {
                                   WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

                                   if (action == GLFW_PRESS)
                                   {
                                       TE::MouseButtonPressedEvent event((TE::MouseCode)button);
                                       TE_CORE_DEBUG(event.ToString());
                                       data.EventCallback(event);
                                   }
                                   else if (action == GLFW_RELEASE)
                                   {
                                       TE::MouseButtonReleasedEvent event((TE::MouseCode)button);
                                       TE_CORE_DEBUG(event.ToString());
                                       data.EventCallback(event);
                                   }
                               });

    glfwSetScrollCallback(m_Window,
                          [](GLFWwindow *window, double xOffset, double yOffset)
                          {
                              WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                              TE::Input::SetMouseScrollDelta((float)xOffset, (float)yOffset);
                              TE::MouseScrolledEvent event((float)xOffset, (float)yOffset);
                              TE_CORE_DEBUG(event.ToString());
                              data.EventCallback(event);
                          });

    glfwSetCursorPosCallback(m_Window,
                             [](GLFWwindow *window, double xpos, double ypos)
                             {
                                 WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                                 TE::MouseMovedEvent event((float)xpos, (float)ypos);
                                 TE_CORE_DEBUG(event.ToString());
                                 data.EventCallback(event);
                             });

    TE_CORE_INFO("Window successfully created: {0} ({1}x{2})", m_Data.Title, m_Data.Width, m_Data.Height);
}

void WindowsWindow::Shutdown()
{
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        TE_CORE_INFO("GLFW window destroyed.");
    }
}

void WindowsWindow::OnUpdate()
{
    if (!m_Window)
    {
        TE_CORE_ERROR("OnUpdate() called on null window!");
        return;
    }

    glfwPollEvents();

    if (TE::RendererContext::GetAPI() == TE::GraphicsAPI::DirectX11)
    {
        TE::DX11Context &ctx = TE::DX11Context::Get();
        if (ctx.SwapChain)
        {
            ctx.SwapChain->Present(m_Data.VSync ? 1 : 0, 0);
        }
    }
    else
    {
        glfwSwapBuffers(m_Window);
    }
}

void WindowsWindow::SetVSync(bool enabled)
{
    if (m_Window)
        glfwSwapInterval(enabled ? 1 : 0);
    m_Data.VSync = enabled;
}

bool WindowsWindow::IsVSync() const { return m_Data.VSync; }

void IWindow::Terminate() { glfwTerminate(); }

void *IWindow::GetCurrentContext() { return glfwGetCurrentContext(); }

void IWindow::MakeContextCurrent(void *context) { glfwMakeContextCurrent(static_cast<GLFWwindow *>(context)); }
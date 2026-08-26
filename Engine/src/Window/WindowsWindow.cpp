#include "Core/PreRequisites.h"
#ifdef TE_PLATFORM_WINDOWS
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
#endif

#include <GLFW/glfw3.h>

#ifdef TE_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

// Engine headers (windows.h already in scope — no macro surprise)
#include "Core/Asset/AssetManager.hpp"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Log.h"
#include "Core/Settings/EngineSettings.hpp"
#include "Input/Input.hpp"
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/RendererContext.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Window/WindowsWindow.hpp"

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
    RendererContext::EnableBestGPU();
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

    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::OpenGL:
#ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
        break;
    case GraphicsAPI::Vulkan:
    case GraphicsAPI::DirectX11:
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        break;
    default:
        break;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);
    if (!m_Window)
    {
        TE_CORE_ERROR("Failed to create GLFW window!");
        return;
    }

    // Set Window Icon before showing the window
    {
#ifdef TE_PLATFORM_WINDOWS
        HWND hWnd = glfwGetWin32Window(m_Window);
        if (hWnd)
        {
            HICON hIconBig =
                (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(101), IMAGE_ICON, 48, 48, LR_SHARED);
            HICON hIconSmall =
                (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(101), IMAGE_ICON, 16, 16, LR_SHARED);

            if (hIconBig || hIconSmall)
            {
                SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
                SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
                SetClassLongPtrW(hWnd, GCLP_HICON, (LONG_PTR)hIconBig);
                SetClassLongPtrW(hWnd, GCLP_HICONSM, (LONG_PTR)hIconSmall);
            }
        }
#endif

        GLFWimage images[1];
        TEString iconPath;
        TEString currDir = TEFileSystem::GetCurrentWorkingDirectory();
        for (int i = 0; i < 5; ++i)
        {
            TEString checkPath = currDir / "Resources/Branding/TimeEngineIcon.png";
            if (TEFileSystem::Exists(checkPath))
            {
                iconPath = checkPath;
                break;
            }
            if (!currDir.HasParentPath() || currDir.GetParentPath() == currDir)
                break;
            currDir = currDir.GetParentPath();
        }

        if (!iconPath.IsEmpty())
        {
            ImageData img = AssetManager::ImportImage(iconPath, 4);
            if (img.IsValid())
            {
                images[0].width = img.Width;
                images[0].height = img.Height;
                images[0].pixels = img.Pixels.GetData();
                glfwSetWindowIcon(m_Window, 1, images);
            }
        }
    }

    // Now show the window with icon already attached
    glfwShowWindow(m_Window);

    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        glfwMakeContextCurrent(m_Window);
    }
#ifdef TE_PLATFORM_WINDOWS
    else if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
    {
        HWND hwnd = glfwGetWin32Window(m_Window);
        auto *apiInstance = RenderCommand::GetAPIInstance();
        auto *dx11API = dynamic_cast<DirectX11RendererAPI *>(apiInstance);
        if (dx11API)
        {
            dx11API->InitWithWindow(hwnd, props.Width, props.Height);
        }
        else
        {
            TE_CORE_ERROR("RendererAPI is not DirectX11RendererAPI!");
        }
    }
#endif
    glfwSetWindowUserPointer(m_Window, &m_Data);

    Input::Init(m_Window); // Register window with input system

    SetVSync(true);

    // === EVENT CALLBACKS ===
    glfwSetWindowSizeCallback(m_Window,
                              [](GLFWwindow *window, int width, int height)
                              {
                                  WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                                  data.Width = width;
                                  data.Height = height;
                                  WindowResizeEvent event(width, height);
                                  TE_INPUT_DEBUG(event.ToString());
                                  data.EventCallback(event);
                              });

    // === FRAMEBUFFER SIZE CALLBACK ===
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
                                   { RenderCommand::SetViewport(0, 0, width, height); });

    glfwSetWindowCloseCallback(m_Window,
                               [](GLFWwindow *window)
                               {
                                   WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                                   WindowCloseEvent event;
                                   TE_INPUT_DEBUG(event.ToString());
                                   data.EventCallback(event);
                                   if (event.Handled())
                                   {
                                       glfwSetWindowShouldClose(window, GLFW_FALSE);
                                   }
                               });

    glfwSetWindowIconifyCallback(m_Window,
                                 [](GLFWwindow *window, int iconified)
                                 {
                                     WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                                     if (iconified)
                                     {
                                         WindowLostFocusEvent event;
                                         TE_INPUT_DEBUG(event.ToString());
                                         data.EventCallback(event);
                                     }
                                     else
                                     {
                                         WindowFocusEvent event;
                                         TE_INPUT_DEBUG(event.ToString());
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
                               KeyPressedEvent event((KeyCode)key, false);
                               TE_INPUT_DEBUG(event.ToString());
                               data.EventCallback(event);
                               break;
                           }
                           case GLFW_RELEASE:
                           {
                               KeyReleasedEvent event((KeyCode)key);
                               TE_INPUT_DEBUG(event.ToString());
                               data.EventCallback(event);
                               break;
                           }
                           case GLFW_REPEAT:
                           {
                               KeyPressedEvent event((KeyCode)key, true);
                               TE_INPUT_DEBUG(event.ToString());
                               data.EventCallback(event);
                               break;
                           }
                           }
                       });

    glfwSetCharCallback(m_Window,
                        [](GLFWwindow *window, unsigned int codepoint)
                        {
                            WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                            KeyTypedEvent event((KeyCode)codepoint);
                            TE_INPUT_DEBUG(event.ToString());
                            data.EventCallback(event);
                        });

    glfwSetMouseButtonCallback(m_Window,
                               [](GLFWwindow *window, int button, int action, int mods)
                               {
                                   WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);

                                   if (action == GLFW_PRESS)
                                   {
                                       MouseButtonPressedEvent event((MouseCode)button);
                                       TE_INPUT_DEBUG(event.ToString());
                                       data.EventCallback(event);
                                   }
                                   else if (action == GLFW_RELEASE)
                                   {
                                       MouseButtonReleasedEvent event((MouseCode)button);
                                       TE_INPUT_DEBUG(event.ToString());
                                       data.EventCallback(event);
                                   }
                               });

    glfwSetScrollCallback(m_Window,
                          [](GLFWwindow *window, double xOffset, double yOffset)
                          {
                              WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                              Input::SetMouseScrollDelta((float)xOffset, (float)yOffset);
                              MouseScrolledEvent event((float)xOffset, (float)yOffset);
                              TE_INPUT_DEBUG(event.ToString());
                              data.EventCallback(event);
                          });

    glfwSetCursorPosCallback(m_Window,
                             [](GLFWwindow *window, double xpos, double ypos)
                             {
                                 WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
                                 MouseMovedEvent event((float)xpos, (float)ypos);
                                 TE_INPUT_DEBUG(event.ToString());
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

void IWindow::SwapBuffers(void *nativeWindow)
{
    if (nativeWindow)
        glfwSwapBuffers(static_cast<GLFWwindow *>(nativeWindow));
}

#include "PreRequisites.h"
#ifdef TE_PLATFORM_LINUX

#include <GLFW/glfw3.h>

#include "AssetManager.hpp"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Log.h"
#include "Settings/EngineSettings.hpp"
#include "Input.hpp"
#include "RenderCommand.hpp"
#include "RendererContext.hpp"
#include "Utils/TEFileSystem.hpp"
#include "LinuxWindow.hpp"

static bool s_GLFWInitialized = false;

LinuxWindow::LinuxWindow(const WindowProps &props)
{
    m_Window = nullptr;
    Init(props);
}

LinuxWindow::~LinuxWindow() { Shutdown(); }

void *LinuxWindow::GetGLLoaderFunction() const { return (void *)glfwGetProcAddress; }

void LinuxWindow::Init(const WindowProps &props)
{
    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;

    if (!s_GLFWInitialized)
    {
        glfwSetErrorCallback([](int error, const char *description)
                             { TE_CORE_ERROR("GLFW Error ({0}): {1}", error, description ? description : "Unknown"); });

        int success = glfwInit();
        if (!success)
        {
            TE_CORE_ERROR("Could not initialize GLFW for LinuxWindow!");
            return;
        }
        TE_CORE_INFO("LinuxWindow (X11/Wayland/GLFW) initialized successfully.");
        s_GLFWInitialized = true;
    }

    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::OpenGL:
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
        break;
    case GraphicsAPI::OpenGLES:
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        break;
    case GraphicsAPI::Vulkan:
        // Explicitly disable GLFW OpenGL context creation when using Vulkan on Linux
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        break;
    case GraphicsAPI::DirectX11:
    case GraphicsAPI::Metal:
    case GraphicsAPI::None:
    default:
        break;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);
    if (!m_Window && RendererContext::GetAPI() == GraphicsAPI::OpenGL)
    {
        TE_CORE_WARN("Failed to create OpenGL 4.1 context on Linux. Attempting fallback to OpenGL 3.3 Core Profile...");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);
    }

    if (!m_Window)
    {
        TE_CORE_ERROR("Failed to create GLFW window for LinuxWindow!");
        return;
    }

    // Set Window Icon
    {
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

    if (RendererContext::GetAPI() == GraphicsAPI::OpenGL || RendererContext::GetAPI() == GraphicsAPI::OpenGLES)
    {
        glfwMakeContextCurrent(m_Window);
    }

    glfwSetWindowUserPointer(m_Window, &m_Data);
    Input::Init(m_Window);
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

    TE_CORE_INFO("LinuxWindow successfully created: {0} ({1}x{2})", m_Data.Title, m_Data.Width, m_Data.Height);
}

void LinuxWindow::Shutdown()
{
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        TE_CORE_INFO("LinuxWindow destroyed.");
    }
}

void LinuxWindow::OnUpdate()
{
    if (!m_Window)
    {
        TE_CORE_ERROR("OnUpdate() called on null window!");
        return;
    }
    glfwPollEvents();
}

void LinuxWindow::SetVSync(bool enabled)
{
    if (m_Window)
        glfwSwapInterval(enabled ? 1 : 0);
    m_Data.VSync = enabled;
}

bool LinuxWindow::IsVSync() const { return m_Data.VSync; }

void LinuxWindow::ShowWindow()
{
    if (m_Window)
        glfwShowWindow(m_Window);
}

bool LinuxWindow::IsVisible() const
{
    if (m_Window)
        return glfwGetWindowAttrib(m_Window, GLFW_VISIBLE) == GLFW_TRUE;
    return false;
}

#endif // TE_PLATFORM_LINUX

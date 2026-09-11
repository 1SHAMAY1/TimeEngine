#include "Core/PreRequisites.h"
#ifdef TE_PLATFORM_MACOS

#include <GLFW/glfw3.h>

#include "Core/Asset/AssetManager.hpp"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Log.h"
#include "Core/Settings/EngineSettings.hpp"
#include "Input/Input.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/RendererContext.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Window/MacWindow.hpp"

static bool s_GLFWInitialized = false;

MacWindow::MacWindow(const WindowProps &props)
{
    m_Window = nullptr;
    Init(props);
}

MacWindow::~MacWindow() { Shutdown(); }

void *MacWindow::GetGLLoaderFunction() const { return nullptr; }

void MacWindow::Init(const WindowProps &props)
{
    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;

    if (!s_GLFWInitialized)
    {
        int success = glfwInit();
        if (!success)
        {
            TE_CORE_ERROR("Could not initialize GLFW for MacWindow!");
            return;
        }
        TE_CORE_INFO("MacWindow (Cocoa/GLFW) initialized successfully.");
        s_GLFWInitialized = true;
    }

    // MacWindow exclusively allows Apple Metal
    TE_CORE_ASSERT(RendererContext::GetAPI() == GraphicsAPI::Metal, "MacWindow only supports GraphicsAPI::Metal!");

    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::Metal:
        // Disable OpenGL client API on GLFW window for native Metal rendering
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        break;
    default:
        TE_CORE_ERROR("Unsupported GraphicsAPI on MacWindow! Only Metal is supported.");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        break;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, props.Title.c_str(), nullptr, nullptr);
    if (!m_Window)
    {
        TE_CORE_ERROR("Failed to create GLFW window for MacWindow!");
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

    glfwShowWindow(m_Window);

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

    TE_CORE_INFO("MacWindow successfully created: {0} ({1}x{2})", m_Data.Title, m_Data.Width, m_Data.Height);
}

void MacWindow::Shutdown()
{
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        TE_CORE_INFO("MacWindow destroyed.");
    }
}

void MacWindow::OnUpdate()
{
    if (!m_Window)
    {
        TE_CORE_ERROR("OnUpdate() called on null window!");
        return;
    }
    glfwPollEvents();
}

void MacWindow::SetVSync(bool enabled)
{
    if (m_Window)
        glfwSwapInterval(enabled ? 1 : 0);
    m_Data.VSync = enabled;
}

bool MacWindow::IsVSync() const { return m_Data.VSync; }

#endif // TE_PLATFORM_MACOS

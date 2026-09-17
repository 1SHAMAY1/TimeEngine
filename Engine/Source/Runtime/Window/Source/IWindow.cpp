#include "PreRequisites.h"
#include "IWindow.hpp"
#include "Log.h"
#include <GLFW/glfw3.h>

#ifdef TE_PLATFORM_WINDOWS
#include "WindowsWindow.hpp"
#elif defined(TE_PLATFORM_LINUX)
#include "LinuxWindow.hpp"
#elif defined(TE_PLATFORM_MACOS)
#include "MacWindow.hpp"
#endif

void IWindow::Terminate() { glfwTerminate(); }

void *IWindow::GetCurrentContext() { return glfwGetCurrentContext(); }

void IWindow::MakeContextCurrent(void *context) { glfwMakeContextCurrent(static_cast<GLFWwindow *>(context)); }

void IWindow::SwapBuffers(void *nativeWindow)
{
    if (nativeWindow)
        glfwSwapBuffers(static_cast<GLFWwindow *>(nativeWindow));
}

TEScope<IWindow> IWindow::Create(const WindowProps &props)
{
#ifdef TE_PLATFORM_WINDOWS
    return CreateScope<WindowsWindow>(props);
#elif defined(TE_PLATFORM_LINUX)
    return CreateScope<LinuxWindow>(props);
#elif defined(TE_PLATFORM_MACOS)
    return CreateScope<MacWindow>(props);
#else
    TE_CORE_ASSERT(false, "Unknown platform for IWindow::Create!");
    return nullptr;
#endif
}

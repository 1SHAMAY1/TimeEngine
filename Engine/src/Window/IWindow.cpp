#include "Core/PreRequisites.h"
#include "Window/WindowsWindow.hpp"

TEScope<IWindow> IWindow::Create(const WindowProps &props) { return CreateScope<WindowsWindow>(props); }

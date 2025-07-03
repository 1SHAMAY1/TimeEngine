#include "Window/WindowsWindow.hpp"

IWindow* IWindow::Create(const WindowProps& props) {
    return new WindowsWindow(props);
}

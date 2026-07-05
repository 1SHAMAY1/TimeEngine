#include "Input/Input.hpp"

#include "imgui.h"

#include "Utils/TimeGUI.hpp"
#include <GLFW/glfw3.h>

namespace TE
{
void *Input::s_Window = nullptr;
std::unordered_map<KeyCode, InputState> Input::s_KeyStates;
std::unordered_map<MouseCode, InputState> Input::s_MouseStates;
float Input::s_MouseScrollX = 0.0f;
float Input::s_MouseScrollY = 0.0f;
bool Input::s_MouseButtonDown[3] = {false, false, false};
bool Input::s_MouseButtonUp[3] = {false, false, false};

void Input::Init(void *nativeWindow) { s_Window = nativeWindow; }

bool Input::IsKeyPressed(KeyCode key)
{
    return glfwGetKey(static_cast<GLFWwindow *>(s_Window), static_cast<int>(key)) == GLFW_PRESS;
}

bool Input::IsMouseButtonPressed(MouseCode button)
{
    return glfwGetMouseButton(static_cast<GLFWwindow *>(s_Window), static_cast<int>(button)) == GLFW_PRESS;
}

float Input::GetMouseX()
{
    double x, y;
    glfwGetCursorPos(static_cast<GLFWwindow *>(s_Window), &x, &y);
    return (float)x;
}

float Input::GetMouseY()
{
    double x, y;
    glfwGetCursorPos(static_cast<GLFWwindow *>(s_Window), &x, &y);
    return (float)y;
}

std::pair<float, float> Input::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(static_cast<GLFWwindow *>(s_Window), &x, &y);
    return {(float)x, (float)y};
}

MouseDelta Input::GetMouseScrollDelta()
{
    MouseDelta delta(s_MouseScrollX, s_MouseScrollY);
    s_MouseScrollX = 0.0f;
    s_MouseScrollY = 0.0f;
    return delta;
}

void Input::SetMouseScrollDelta(float x, float y)
{
    s_MouseScrollX += x;
    s_MouseScrollY += y;
}

bool Input::GetMouseButtonDown(int button)
{
    if (button < 0 || button > 2)
        return false;
    return s_MouseButtonDown[button];
}

bool Input::GetMouseButtonUp(int button)
{
    if (button < 0 || button > 2)
        return false;
    return s_MouseButtonUp[button];
}

void Input::OnKeyPressed(KeyCode key, TE::Event *e, bool isRepeat)
{
    auto &state = s_KeyStates[key];
    state.IsPressed = true;
    state.LastEvent = e;
    if (isRepeat)
        state.RepeatCount++;
    else
    {
        state.RepeatCount = 0;
        state.PressedTime = std::chrono::steady_clock::now();
    }
}

void Input::OnKeyReleased(KeyCode key, TE::Event *e)
{
    auto &state = s_KeyStates[key];
    state.IsPressed = false;
    state.LastEvent = e;
    state.DurationHeld = std::chrono::duration<float>(std::chrono::steady_clock::now() - state.PressedTime).count();
}

void Input::OnMousePressed(MouseCode button, TE::Event *e)
{
    auto &state = s_MouseStates[button];
    state.IsPressed = true;
    state.LastEvent = e;
    state.RepeatCount++;
    state.PressedTime = std::chrono::steady_clock::now();
}

void Input::OnMouseReleased(MouseCode button, TE::Event *e)
{
    auto &state = s_MouseStates[button];
    state.IsPressed = false;
    state.LastEvent = e;
    state.DurationHeld = std::chrono::duration<float>(std::chrono::steady_clock::now() - state.PressedTime).count();
}

const InputState &Input::GetKeyState(KeyCode key) { return s_KeyStates[key]; }

const InputState &Input::GetMouseState(MouseCode button) { return s_MouseStates[button]; }

void Input::Update(float deltaTime)
{
    for (auto &[key, state] : s_KeyStates)
    {
        if (state.IsPressed)
            state.DurationHeld += deltaTime;
    }

    for (auto &[btn, state] : s_MouseStates)
    {
        if (state.IsPressed)
            state.DurationHeld += deltaTime;
    }
}

int Input::ToImGuiKey(KeyCode key)
{
    switch (key)
    {
    case Key::Tab:
        return ImGuiKey_Tab;
    case Key::Left:
        return ImGuiKey_LeftArrow;
    case Key::Right:
        return ImGuiKey_RightArrow;
    case Key::Up:
        return ImGuiKey_UpArrow;
    case Key::Down:
        return ImGuiKey_DownArrow;
    case Key::PageUp:
        return ImGuiKey_PageUp;
    case Key::PageDown:
        return ImGuiKey_PageDown;
    case Key::Home:
        return ImGuiKey_Home;
    case Key::End:
        return ImGuiKey_End;
    case Key::Insert:
        return ImGuiKey_Insert;
    case Key::Delete:
        return ImGuiKey_Delete;
    case Key::Backspace:
        return ImGuiKey_Backspace;
    case Key::Space:
        return ImGuiKey_Space;
    case Key::Enter:
        return ImGuiKey_Enter;
    case Key::Escape:
        return ImGuiKey_Escape;
    case Key::A:
        return ImGuiKey_A;
    case Key::B:
        return ImGuiKey_B;
    case Key::C:
        return ImGuiKey_C;
    case Key::D:
        return ImGuiKey_D;
    case Key::E:
        return ImGuiKey_E;
    case Key::F:
        return ImGuiKey_F;
    case Key::G:
        return ImGuiKey_G;
    case Key::H:
        return ImGuiKey_H;
    case Key::I:
        return ImGuiKey_I;
    case Key::J:
        return ImGuiKey_J;
    case Key::K:
        return ImGuiKey_K;
    case Key::L:
        return ImGuiKey_L;
    case Key::M:
        return ImGuiKey_M;
    case Key::N:
        return ImGuiKey_N;
    case Key::O:
        return ImGuiKey_O;
    case Key::P:
        return ImGuiKey_P;
    case Key::Q:
        return ImGuiKey_Q;
    case Key::R:
        return ImGuiKey_R;
    case Key::S:
        return ImGuiKey_S;
    case Key::T:
        return ImGuiKey_T;
    case Key::U:
        return ImGuiKey_U;
    case Key::V:
        return ImGuiKey_V;
    case Key::W:
        return ImGuiKey_W;
    case Key::X:
        return ImGuiKey_X;
    case Key::Y:
        return ImGuiKey_Y;
    case Key::Z:
        return ImGuiKey_Z;
    case Key::D0:
        return ImGuiKey_0;
    case Key::D1:
        return ImGuiKey_1;
    case Key::D2:
        return ImGuiKey_2;
    case Key::D3:
        return ImGuiKey_3;
    case Key::D4:
        return ImGuiKey_4;
    case Key::D5:
        return ImGuiKey_5;
    case Key::D6:
        return ImGuiKey_6;
    case Key::D7:
        return ImGuiKey_7;
    case Key::D8:
        return ImGuiKey_8;
    case Key::D9:
        return ImGuiKey_9;
    case Key::LeftShift:
        return ImGuiKey_LeftShift;
    case Key::RightShift:
        return ImGuiKey_RightShift;
    case Key::LeftControl:
        return ImGuiKey_LeftCtrl;
    case Key::RightControl:
        return ImGuiKey_RightCtrl;
    case Key::LeftAlt:
        return ImGuiKey_LeftAlt;
    case Key::RightAlt:
        return ImGuiKey_RightAlt;
    }
    return ImGuiKey_None;
}
} // namespace TE

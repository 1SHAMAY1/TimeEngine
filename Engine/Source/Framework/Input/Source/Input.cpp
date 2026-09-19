#include "PreRequisites.h"
#include "Input.hpp"

#include "TimeGUI.hpp"
#include <GLFW/glfw3.h>

void *Input::s_Window = nullptr;
TEMap<KeyCode, InputState> Input::s_KeyStates;
TEMap<MouseCode, InputState> Input::s_MouseStates;
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

void Input::OnKeyPressed(KeyCode key, Event *e, bool isRepeat)
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

void Input::OnKeyReleased(KeyCode key, Event *e)
{
    auto &state = s_KeyStates[key];
    state.IsPressed = false;
    state.LastEvent = e;
    state.DurationHeld = std::chrono::duration<float>(std::chrono::steady_clock::now() - state.PressedTime).count();
}

void Input::OnMousePressed(MouseCode button, Event *e)
{
    auto &state = s_MouseStates[button];
    state.IsPressed = true;
    state.LastEvent = e;
    state.RepeatCount++;
    state.PressedTime = std::chrono::steady_clock::now();
}

void Input::OnMouseReleased(MouseCode button, Event *e)
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
    for (auto &pair : s_KeyStates)
    {
        if (pair.second.IsPressed)
            pair.second.DurationHeld += deltaTime;
    }

    for (auto &pair : s_MouseStates)
    {
        if (pair.second.IsPressed)
            pair.second.DurationHeld += deltaTime;
    }
}

TimeGUIKey Input::ToTimeGUIKey(KeyCode key)
{
    switch (key)
    {
    case Key::Tab:
        return TimeGUIKey_Tab;
    case Key::Left:
        return TimeGUIKey_LeftArrow;
    case Key::Right:
        return TimeGUIKey_RightArrow;
    case Key::Up:
        return TimeGUIKey_UpArrow;
    case Key::Down:
        return TimeGUIKey_DownArrow;
    case Key::PageUp:
        return TimeGUIKey_PageUp;
    case Key::PageDown:
        return TimeGUIKey_PageDown;
    case Key::Home:
        return TimeGUIKey_Home;
    case Key::End:
        return TimeGUIKey_End;
    case Key::Insert:
        return TimeGUIKey_Insert;
    case Key::Delete:
        return TimeGUIKey_Delete;
    case Key::Backspace:
        return TimeGUIKey_Backspace;
    case Key::Space:
        return TimeGUIKey_Space;
    case Key::Enter:
        return TimeGUIKey_Enter;
    case Key::Escape:
        return TimeGUIKey_Escape;
    case Key::A:
        return TimeGUIKey_A;
    case Key::B:
        return TimeGUIKey_B;
    case Key::C:
        return TimeGUIKey_C;
    case Key::D:
        return TimeGUIKey_D;
    case Key::E:
        return TimeGUIKey_E;
    case Key::F:
        return TimeGUIKey_F;
    case Key::G:
        return TimeGUIKey_G;
    case Key::H:
        return TimeGUIKey_H;
    case Key::I:
        return TimeGUIKey_I;
    case Key::J:
        return TimeGUIKey_J;
    case Key::K:
        return TimeGUIKey_K;
    case Key::L:
        return TimeGUIKey_L;
    case Key::M:
        return TimeGUIKey_M;
    case Key::N:
        return TimeGUIKey_N;
    case Key::O:
        return TimeGUIKey_O;
    case Key::P:
        return TimeGUIKey_P;
    case Key::Q:
        return TimeGUIKey_Q;
    case Key::R:
        return TimeGUIKey_R;
    case Key::S:
        return TimeGUIKey_S;
    case Key::T:
        return TimeGUIKey_T;
    case Key::U:
        return TimeGUIKey_U;
    case Key::V:
        return TimeGUIKey_V;
    case Key::W:
        return TimeGUIKey_W;
    case Key::X:
        return TimeGUIKey_X;
    case Key::Y:
        return TimeGUIKey_Y;
    case Key::Z:
        return TimeGUIKey_Z;
    }
    return TimeGUIKey_None;
}

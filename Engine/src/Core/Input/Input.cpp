﻿#include "Input/Input.hpp"
#include <GLFW/glfw3.h>

namespace TE
{
    GLFWwindow* Input::s_Window = nullptr;
    std::unordered_map<KeyCode, InputState> Input::s_KeyStates;
    std::unordered_map<MouseCode, InputState> Input::s_MouseStates;
    float Input::s_MouseScrollX = 0.0f;
    float Input::s_MouseScrollY = 0.0f;
    bool Input::s_MouseButtonDown[3] = { false, false, false };
    bool Input::s_MouseButtonUp[3] = { false, false, false };

    void Input::Init(void* nativeWindow)
    {
        s_Window = static_cast<GLFWwindow*>(nativeWindow);
    }

    bool Input::IsKeyPressed(KeyCode key)
    {
        return glfwGetKey(s_Window, static_cast<int>(key)) == GLFW_PRESS;
    }

    bool Input::IsMouseButtonPressed(MouseCode button)
    {
        return glfwGetMouseButton(s_Window, static_cast<int>(button)) == GLFW_PRESS;
    }

    float Input::GetMouseX()
    {
        double x, y;
        glfwGetCursorPos(s_Window, &x, &y);
        return (float)x;
    }

    float Input::GetMouseY()
    {
        double x, y;
        glfwGetCursorPos(s_Window, &x, &y);
        return (float)y;
    }

    std::pair<float, float> Input::GetMousePosition()
    {
        double x, y;
        glfwGetCursorPos(s_Window, &x, &y);
        return { (float)x, (float)y };
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
        if (button < 0 || button > 2) return false;
        return s_MouseButtonDown[button];
    }

    bool Input::GetMouseButtonUp(int button)
    {
        if (button < 0 || button > 2) return false;
        return s_MouseButtonUp[button];
    }

    void Input::OnKeyPressed(KeyCode key, TE::Event* e, bool isRepeat)
    {
        auto& state = s_KeyStates[key];
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

    void Input::OnKeyReleased(KeyCode key, TE::Event* e)
    {
        auto& state = s_KeyStates[key];
        state.IsPressed = false;
        state.LastEvent = e;
        state.DurationHeld = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - state.PressedTime).count();
    }

    void Input::OnMousePressed(MouseCode button, TE::Event* e)
    {
        auto& state = s_MouseStates[button];
        state.IsPressed = true;
        state.LastEvent = e;
        state.RepeatCount++;
        state.PressedTime = std::chrono::steady_clock::now();
    }

    void Input::OnMouseReleased(MouseCode button, TE::Event* e)
    {
        auto& state = s_MouseStates[button];
        state.IsPressed = false;
        state.LastEvent = e;
        state.DurationHeld = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - state.PressedTime).count();
    }

    const InputState& Input::GetKeyState(KeyCode key)
    {
        return s_KeyStates[key];
    }

    const InputState& Input::GetMouseState(MouseCode button)
    {
        return s_MouseStates[button];
    }

    void Input::Update(float deltaTime)
    {
        for (auto& [key, state] : s_KeyStates)
        {
            if (state.IsPressed)
                state.DurationHeld += deltaTime;
        }

        for (auto& [btn, state] : s_MouseStates)
        {
            if (state.IsPressed)
                state.DurationHeld += deltaTime;
        }
    }
}

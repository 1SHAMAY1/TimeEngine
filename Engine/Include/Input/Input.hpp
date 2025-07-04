#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <chrono>

#include "Core/PreRequisites.h"
#include "Core/KeyCodes.hpp"
#include "Core/MouseCodes.hpp"
#include "Core/Events/Event.h"

namespace TE
{
    struct InputState
    {
        bool IsPressed = false;
        int RepeatCount = 0;
        float DurationHeld = 0.0f;
        TE::Event* LastEvent = nullptr;

        std::chrono::steady_clock::time_point PressedTime;
    };

    class TE_API Input
    {
    public:
        static void Init(void* nativeWindow);

        static bool IsKeyPressed(KeyCode key);
        static bool IsMouseButtonPressed(MouseCode button);
        static float GetMouseX();
        static float GetMouseY();
        static std::pair<float, float> GetMousePosition();

        static void OnKeyPressed(KeyCode key, TE::Event* e, bool isRepeat);
        static void OnKeyReleased(KeyCode key, TE::Event* e);
        static void OnMousePressed(MouseCode button, TE::Event* e);
        static void OnMouseReleased(MouseCode button, TE::Event* e);

        static const InputState& GetKeyState(KeyCode key);
        static const InputState& GetMouseState(MouseCode button);
        static void Update(float deltaTime);

    private:
        static GLFWwindow* s_Window;
        static std::unordered_map<KeyCode, InputState> s_KeyStates;
        static std::unordered_map<MouseCode, InputState> s_MouseStates;
    };
}

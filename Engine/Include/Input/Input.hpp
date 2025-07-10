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
    TE_API struct InputState
    {
        bool IsPressed = false;
        int RepeatCount = 0;
        float DurationHeld = 0.0f;
        TE::Event* LastEvent = nullptr;

        std::chrono::steady_clock::time_point PressedTime;
    };

    TE_API struct MouseDelta {
        float x = 0.0f;
        float y = 0.0f;
        MouseDelta() = default;
        MouseDelta(float x, float y) : x(x), y(y) {}
    };

    class Input
    {
    public:
        TE_API static void Init(void* nativeWindow);

        TE_API static bool IsKeyPressed(KeyCode key);
        TE_API static bool IsMouseButtonPressed(MouseCode button);
        TE_API static float GetMouseX();
        TE_API static float GetMouseY();
        TE_API static std::pair<float, float> GetMousePosition();

        TE_API static void OnKeyPressed(KeyCode key, TE::Event* e, bool isRepeat);
        TE_API static void OnKeyReleased(KeyCode key, TE::Event* e);
        TE_API static void OnMousePressed(MouseCode button, TE::Event* e);
        TE_API static void OnMouseReleased(MouseCode button, TE::Event* e);

        TE_API static const InputState& GetKeyState(KeyCode key);
        TE_API static const InputState& GetMouseState(MouseCode button);
        TE_API static void Update(float deltaTime);

        // --- Mouse Scroll ---
        TE_API static MouseDelta GetMouseScrollDelta(); // (x, y)
        TE_API static void SetMouseScrollDelta(float x, float y);

        // --- Mouse Buttons ---
        TE_API static bool GetMouseButtonDown(int button); // 0=left, 1=right, 2=middle
        TE_API static bool GetMouseButtonUp(int button);

    private:
        static GLFWwindow* s_Window;
        static std::unordered_map<KeyCode, InputState> s_KeyStates;
        static std::unordered_map<MouseCode, InputState> s_MouseStates;
        static float s_MouseScrollX;
        static float s_MouseScrollY;
        static bool s_MouseButtonDown[3];
        static bool s_MouseButtonUp[3];
    };
}

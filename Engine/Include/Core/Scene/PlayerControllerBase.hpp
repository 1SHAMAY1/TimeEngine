#pragma once
#include <unordered_map>
#include <string>
#include "Core/GameFrameWork/TComponent.hpp"

struct InputAction {
    std::string Name;
    int Key;
    bool IsPressed = false;
};

struct PlayerControllerBase : public TComponent {
    std::unordered_map<std::string, InputAction> InputMap;
    virtual ~PlayerControllerBase() = default;
    virtual void UpdateInput() = 0;
    void MapInput(const std::string& action, int key) { InputMap[action].Key = key; }
}; 
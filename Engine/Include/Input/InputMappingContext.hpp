#pragma once
#include "Core/KeyCodes.hpp"
#include "InputAction.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace TE
{

struct ActionKeyMapping
{
    std::shared_ptr<InputAction> Action;
    KeyCode Key;
    // Future: Modifiers and Triggers
};

class InputMappingContext
{
public:
    std::string Name;
    std::vector<ActionKeyMapping> Mappings;

    void AddMapping(std::shared_ptr<InputAction> action, KeyCode key) { Mappings.push_back({action, key}); }

    void RemoveMapping(std::shared_ptr<InputAction> action, KeyCode key)
    {
        auto it = std::remove_if(Mappings.begin(), Mappings.end(),
                                 [&](const ActionKeyMapping &m) { return m.Action == action && m.Key == key; });
        Mappings.erase(it, Mappings.end());
    }
};

} // namespace TE

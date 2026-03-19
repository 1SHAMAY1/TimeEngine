#pragma once
#include "InputMappingContext.hpp"
#include <string>
#include <unordered_map>

namespace TE
{

class InputRemapper
{
public:
    // Key: Context Name -> (Action Name -> New Key)
    using RemapOverrides = std::unordered_map<std::string, std::unordered_map<std::string, KeyCode>>;

    void SetRemap(const std::string &contextName, const std::string &actionName, KeyCode newKey)
    {
        m_Overrides[contextName][actionName] = newKey;
    }

    KeyCode GetKey(const std::string &contextName, const std::string &actionName, KeyCode defaultKey) const
    {
        if (m_Overrides.count(contextName))
        {
            auto &actions = m_Overrides.at(contextName);
            if (actions.count(actionName))
            {
                return actions.at(actionName);
            }
        }
        return defaultKey;
    }

    // Future: Save/Load to Project/User settings

private:
    RemapOverrides m_Overrides;
};

} // namespace TE

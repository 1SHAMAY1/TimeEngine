#pragma once
#include "InputMappingContext.hpp"

class InputRemapper
{
public:
    // Key: Context Name -> (Action Name -> New Key)
    using RemapOverrides = TEMap<TEString, TEMap<TEString, KeyCode>>;

    void SetRemap(const TEString &contextName, const TEString &actionName, KeyCode newKey)
    {
        m_Overrides[contextName][actionName] = newKey;
    }

    KeyCode GetKey(const TEString &contextName, const TEString &actionName, KeyCode defaultKey) const
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

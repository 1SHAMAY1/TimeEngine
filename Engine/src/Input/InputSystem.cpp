#include "Core/PreRequisites.h"
#include "Input/InputSystem.hpp"
#include "Input/Input.hpp"
#include <algorithm>

InputSystem &InputSystem::Get()
{
    static InputSystem instance;
    return instance;
}

void InputSystem::AddMappingContext(TERef<InputMappingContext> context, int priority)
{
    if (!context)
        return;

    // Check if already added
    for (auto &cp : m_Contexts)
    {
        if (cp.Context == context)
        {
            cp.Priority = priority;
            std::sort(m_Contexts.begin(), m_Contexts.end(),
                      [](const ContextPriority &a, const ContextPriority &b) { return a.Priority > b.Priority; });
            return;
        }
    }

    m_Contexts.Add({context, priority});
    std::sort(m_Contexts.begin(), m_Contexts.end(),
              [](const ContextPriority &a, const ContextPriority &b) { return a.Priority > b.Priority; });
}

void InputSystem::RemoveMappingContext(TERef<InputMappingContext> context)
{
    m_Contexts.RemoveBy([&](const ContextPriority &cp) { return cp.Context == context; });
}

void InputSystem::RegisterComponent(InputComponent *component)
{
    if (!component)
        return;

    auto it = std::find(m_Components.begin(), m_Components.end(), component);
    if (it == m_Components.end())
    {
        m_Components.Add(component);
    }
}

void InputSystem::UnregisterComponent(InputComponent *component) { m_Components.Remove(component); }

void InputSystem::Update(float deltaTime)
{
    for (const auto &cp : m_Contexts)
    {
        if (!cp.Context)
            continue;

        for (const auto &mapping : cp.Context->Mappings)
        {
            if (mapping.Action.Name.empty())
                continue;

            KeyCode key = static_cast<KeyCode>(mapping.Code);
            if (m_Remapper)
            {
                key = m_Remapper->GetKey(cp.Context->ContextName, mapping.Action.Name, key);
            }

            bool isPressed = Input::IsKeyPressed(key);
            InputActionValue val;

            switch (mapping.Action.ValueType)
            {
            case EInputActionValueType::Digital:
                val.Value = isPressed;
                break;
            case EInputActionValueType::Axis1D:
                val.Value = isPressed ? 1.0f : 0.0f;
                break;
            case EInputActionValueType::Axis2D:
                val.Value = isPressed ? TEVector2(1.0f, 0.0f) : TEVector2(0.0f, 0.0f);
                break;
            }

            if (isPressed)
            {
                for (auto *comp : m_Components)
                {
                    if (comp)
                    {
                        comp->ExecuteAction(mapping.Action, val);
                    }
                }
            }
        }
    }
}

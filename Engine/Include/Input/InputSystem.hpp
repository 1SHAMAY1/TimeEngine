#pragma once
#include "InputComponent.hpp"
#include "InputMappingContext.hpp"
#include "InputRemapper.hpp"
#include <memory>
#include <vector>


class TE_API InputSystem
{
public:
    static InputSystem &Get();

    void AddMappingContext(TERef<InputMappingContext> context, int priority = 0);
    void RemoveMappingContext(TERef<InputMappingContext> context);

    void Update(float deltaTime);

    void SetRemapper(TERef<InputRemapper> remapper) { m_Remapper = remapper; }

    // Internally used by components to register themselves
    void RegisterComponent(InputComponent *component);
    void UnregisterComponent(InputComponent *component);

private:
    InputSystem() = default;

    struct ContextPriority
    {
        TERef<InputMappingContext> Context;
        int Priority;
    };

    TEArray<ContextPriority> m_Contexts;
    TEArray<InputComponent *> m_Components;
    TERef<InputRemapper> m_Remapper;
};


#pragma once
#include "InputMappingContext.hpp"
#include "InputComponent.hpp"
#include "InputRemapper.hpp"
#include <vector>
#include <memory>

namespace TE {

    class InputSystem {
    public:
        static InputSystem& Get();

        void AddMappingContext(std::shared_ptr<InputMappingContext> context, int priority = 0);
        void RemoveMappingContext(std::shared_ptr<InputMappingContext> context);

        void Update(float deltaTime);

        void SetRemapper(std::shared_ptr<InputRemapper> remapper) { m_Remapper = remapper; }

        // Internally used by components to register themselves
        void RegisterComponent(InputComponent* component);
        void UnregisterComponent(InputComponent* component);

    private:
        InputSystem() = default;

        struct ContextPriority {
            std::shared_ptr<InputMappingContext> Context;
            int Priority;
        };

        std::vector<ContextPriority> m_Contexts;
        std::vector<InputComponent*> m_Components;
        std::shared_ptr<InputRemapper> m_Remapper;
    };

} // namespace TE

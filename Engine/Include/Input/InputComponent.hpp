#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "InputAction.hpp"
#include <functional>

TE_CLASS()
class TE_API InputComponent : public TComponent
{
public:
    GENERATED_BODY(InputComponent)

    InputComponent();
    virtual ~InputComponent() override;

    void OnAttach();
    void OnDetach();

    using ActionBinding = std::function<void(const InputActionValue &)>;

    void BindAction(const TEString &actionName, ActionBinding callback) { m_ActionBindings[actionName] = callback; }

    void ExecuteAction(const TEString &actionName, const InputActionValue &value)
    {
        if (m_ActionBindings.find(actionName) != m_ActionBindings.end())
        {
            m_ActionBindings[actionName](value);
        }
    }

    void ExecuteAction(const InputAction &action, const InputActionValue &value) { ExecuteAction(action.Name, value); }

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TEMap<TEString, ActionBinding> m_ActionBindings;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(InputComponent, "Input Component")
T_REGISTER_PRESET(InputComponent, "Input Component", "Input & Controls",
                  [](EntityID id, EntityManager *em) { em->AddComponent<InputComponent>(id); })
#endif

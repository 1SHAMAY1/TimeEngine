#pragma once
#include "Core/GameFrameWork/TComponent.hpp"
#include "InputAction.hpp"
#include <functional>
#include <map>
#include <memory>

namespace TE
{

class InputComponent : public TComponent
{
public:
    using ActionBinding = std::function<void(const InputActionValue &)>;

    void BindAction(std::shared_ptr<InputAction> action, ActionBinding callback) { m_Bindings[action] = callback; }

    void ExecuteAction(std::shared_ptr<InputAction> action, const InputActionValue &value)
    {
        if (m_Bindings.count(action))
        {
            m_Bindings[action](value);
        }
    }

    static constexpr const char *StaticClassName = "InputComponent";

private:
    std::map<std::shared_ptr<InputAction>, ActionBinding> m_Bindings;
};

} // namespace TE

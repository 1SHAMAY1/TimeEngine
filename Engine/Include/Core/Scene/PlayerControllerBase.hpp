#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Core/KeyCodes.hpp"
#include "Input/InputMappingContext.hpp"
#include "Input/Input.hpp"


struct LegacyInputAction {
    TEString Name;
    KeyCode Key = 0;
    bool IsPressed = false;
};

TE_CLASS()
class TE_API PlayerControllerBase : public TComponent {
public:
    GENERATED_BODY(PlayerControllerBase)

    PlayerControllerBase() = default;
    virtual ~PlayerControllerBase() = default;

    TEMap<TEString, LegacyInputAction> InputMap;
    TERef<InputMappingContext> MappingContext;

    TEPROPERTY()
    TEString MappingContextAssetPath;

    virtual void UpdateInput() = 0;
    void MapInput(const TEString& action, KeyCode key) { InputMap[action].Key = key; }

    void SetMappingContext(TERef<InputMappingContext> context) { MappingContext = context; }
    TERef<InputMappingContext> GetMappingContext() const { return MappingContext; }

    // Evaluates input mapped to an action in active MappingContext
    InputActionValue GetActionValue(const TEString &actionName) const
    {
        InputActionValue result;
        if (!MappingContext)
            return result;

        for (const auto &m : MappingContext->Mappings)
        {
            if (m.Action.Name != actionName)
                continue;

            float rawValue = 0.0f;
            if (m.BindingType == EInputBindingType::Keyboard)
            {
                rawValue = Input::IsKeyPressed((KeyCode)m.Code) ? 1.0f : 0.0f;
            }
            else if (m.BindingType == EInputBindingType::MouseButton)
            {
                rawValue = Input::GetMouseButtonDown(m.Code) ? 1.0f : 0.0f;
            }
            else if (m.BindingType == EInputBindingType::MouseAxis)
            {
                if (m.Code == 0) rawValue = Input::GetMouseX();
                else if (m.Code == 1) rawValue = Input::GetMouseY();
                else if (m.Code == 2) rawValue = Input::GetMouseScrollDelta().y;
            }

            // Apply deadzone & scale negate modifiers
            if (std::abs(rawValue) < m.Modifier.Deadzone)
                rawValue = 0.0f;
            if (m.Modifier.ScaleNegate)
                rawValue = -rawValue;

            rawValue *= m.Modifier.Multiplier;

            if (m.Action.ValueType == EInputActionValueType::Digital)
            {
                result.Value = (rawValue > 0.5f);
            }
            else if (m.Action.ValueType == EInputActionValueType::Axis1D)
            {
                result.Value = rawValue;
            }
            return result;
        }

        return result;
    }

    virtual void Tick(float deltaTime) override { UpdateInput(); }

    virtual TEString GetClassName() const override { return StaticClassName; }
};


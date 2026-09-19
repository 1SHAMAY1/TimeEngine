#include "PreRequisites.h"
#include "KeyCodes.hpp"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"
#include "InputAction.hpp"
#include "InputMappingContext.hpp"

TE_TEST_CASE(Input, InputActionAndMappingContext)
{
    InputAction jumpAction("Jump", EInputActionValueType::Digital);
    TE_CHECK_EQ(jumpAction.Name, "Jump");
    TE_CHECK(jumpAction.ValueType == EInputActionValueType::Digital);

    InputMappingContext imc("DefaultIMC");
    ActionKeyMapping mapping;
    mapping.Action = jumpAction;
    mapping.BindingType = EInputBindingType::Keyboard;
    mapping.Code = static_cast<uint32_t>(Key::Space);

    imc.AddMapping(mapping);
    TE_CHECK_EQ(imc.Mappings.Num(), 1);
    TE_CHECK_EQ(imc.Mappings[0].Action.Name, "Jump");
    TE_CHECK_EQ(imc.Mappings[0].Code, static_cast<uint32_t>(Key::Space));
}

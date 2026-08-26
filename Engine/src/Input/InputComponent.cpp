#include "Input/InputComponent.hpp"
#include "Core/PreRequisites.h"
#include "Input/InputSystem.hpp"

InputComponent::InputComponent() {}

InputComponent::~InputComponent() { OnDetach(); }

void InputComponent::OnAttach() { InputSystem::Get().RegisterComponent(this); }

void InputComponent::OnDetach() { InputSystem::Get().UnregisterComponent(this); }

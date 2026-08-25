#include "STNeuralActionTask.hpp"
#include "Core/Threading/Threading.hpp"

void STNeuralActionTask::EnterState(Entity agent)
{
    // TODO: Contributor implementation - Load or warm up neural weights
}

StateTaskStatus STNeuralActionTask::TickState(Entity agent, float dt)
{
    // TODO: Contributor implementation:
    // 1. Gather observation vector from agent ECS components
    // 2. Submit async forward pass via SUBMIT_AI or execute TinyNN
    // 3. Evaluate selected action output
    return StateTaskStatus::Running;
}

void STNeuralActionTask::ExitState(Entity agent)
{
    // TODO: Cleanup state
}

# NeuralAIPlugin Architecture

The `NeuralAIPlugin` provides an embedded, zero-dependency micro neural network inference engine and native **StateTree** tasks and conditions for TimeEngine.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph ECSWorld ["ECS Scene & Agent Entity"]
        Agent["Entity (Agent)"]
        Sensors["Sensor Components / Observations\n(Health, Ammo, Grounded, Velocity, Target Distance)"]
        StateTreeComp["StateTreeComponent\n(Active Task: STNeuralActionTask)"]
        Agent --> Sensors
        Agent --> StateTreeComp
    end

    subgraph NeuralAIPlugin ["NeuralAIPlugin (.dll / .teplugin)"]
        Task["STNeuralActionTask : public IStateTask"]
        Condition["STNeuralEvaluatorCondition : public IStateCondition"]
        TinyNN["TinyNNInference\n- SIMD Matrix Multiplications (W * x + b)\n- Activation: ReLU, Softmax, Sigmoid"]
        Weights["Weights Buffer (.tenet / JSON Asset)"]
        
        StateTreeComp --> Task
        Task --> TinyNN
        Condition --> TinyNN
        Weights --> TinyNN
    end

    subgraph Integrations ["Engine & Inter-Plugin Integrations"]
        ThreadPool["SUBMIT_AI(job)\n(ThreadingMacros.hpp)"]
        Tags["#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN)\nGameplayTag Filtering & Action Masking"]
        MCP["#if defined(TE_HAS_PLUGIN_MCPPLUGIN)\nMCP Tools: Inspect / Hot-Reload Weights"]
        
        TinyNN --> ThreadPool
        Task --> Tags
        Task --> MCP
    end
```

---

## 🔑 Key Components & Responsibilities

1. **`TinyNNInference`**:
   - Lightweight, header-only forward-pass inference engine (2-3 dense layers).
   - Designed with zero third-party ML runtime overhead (pure C++ with AVX2/SSE/NEON acceleration).
   - Operates on normalized observation vectors: $\mathbf{x} \in [-1.0, 1.0]^N$.

2. **`STNeuralActionTask` (`IStateTask`)**:
   - Collects ECS observations upon tick.
   - Evaluates action probability distribution: `[Patrol, Engage, Retreat, SeekCover]`.
   - Selects argmax/sample action and transitions agent state.

3. **`STNeuralEvaluatorCondition` (`IStateCondition`)**:
   - Asserts whether the neural model's confidence for a specific behavior exceeds a configured threshold.

4. **Cross-Plugin Preprocessor Integration**:
   - `#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN)`: Masks prohibited action indices based on active `GameplayTag` containers (e.g. Disarmed, Stunned).
   - `#if defined(TE_HAS_PLUGIN_MCPPLUGIN)`: Exposes neural network introspection and live weight hot-reloading tools over the Model Context Protocol.

---

## 🛠️ Contributor Implementation Guide

- Fill in matrix multiplication in `src/TinyNNInference.cpp`.
- Implement `STNeuralActionTask::TickState()` to pull components via `Entity` and trigger inference via `SUBMIT_AI`.

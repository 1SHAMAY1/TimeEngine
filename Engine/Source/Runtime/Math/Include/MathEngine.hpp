#pragma once
#include "PreRequisites.h"
#include "MathAPI.hpp"

/// Singleton subsystem managing the active Math computation backend.
class TE_API MathEngine
{
public:
    static MathEngine &Get();

    void Initialize(MathBackendType backend = MathBackendType::GLM);
    void Shutdown();

    void SetBackend(MathBackendType backend);
    MathBackendType GetBackendType() const;

    MathAPI *GetActiveAPI();

private:
    MathEngine() = default;
    ~MathEngine() = default;

    TEScope<MathAPI> m_ActiveBackend;
    MathBackendType m_CurrentBackendType = MathBackendType::GLM;
    bool m_Initialized = false;
};

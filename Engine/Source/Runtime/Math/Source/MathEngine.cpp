#include "PreRequisites.h"
#include "MathEngine.hpp"
#include "GLMMathAPI.hpp"
#include "CustomMathAPI.hpp"
#include "Log.h"

TEScope<MathAPI> MathAPI::Create(MathBackendType type)
{
    switch (type)
    {
    case MathBackendType::GLM:
        return CreateScope<GLMMathAPI>();
    case MathBackendType::Custom:
        return CreateScope<CustomMathAPI>();
    default:
        return CreateScope<CustomMathAPI>();
    }
}

MathEngine &MathEngine::Get()
{
    static MathEngine s_Instance;
    return s_Instance;
}

void MathEngine::Initialize(MathBackendType backend)
{
    if (m_Initialized)
        return;

    m_CurrentBackendType = backend;
    m_ActiveBackend = MathAPI::Create(backend);
    m_Initialized = true;
    TE_CORE_INFO("MathEngine initialized with backend: {0}", (int)backend);
}

void MathEngine::Shutdown()
{
    m_ActiveBackend.reset();
    m_Initialized = false;
    TE_CORE_INFO("MathEngine shut down.");
}

void MathEngine::SetBackend(MathBackendType backend)
{
    m_CurrentBackendType = backend;
    m_ActiveBackend = MathAPI::Create(backend);
    TE_CORE_INFO("MathEngine switched backend to: {0}", (int)backend);
}

MathBackendType MathEngine::GetBackendType() const { return m_CurrentBackendType; }

MathAPI *MathEngine::GetActiveAPI()
{
    if (!m_ActiveBackend)
    {
        Initialize(m_CurrentBackendType);
    }
    return m_ActiveBackend.get();
}

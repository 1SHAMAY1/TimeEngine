#include "Core/PreRequisites.h"
#include "Editor/Settings/ProjectMathSettings.hpp"
#include "Utils/Math/MathEngine.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(ProjectMathSettings);

void ProjectMathSettings::OnDrawSettingsUI(Ref<EditorLayer> /*editor*/)
{
    TimeGUI::Text("Math Engine Backend Configuration");
    TimeGUI::Separator();
    TimeGUI::Spacing();

    const char *backends[] = {"GLM (OpenGL Mathematics)", "Custom Math Library (SIMD/NEON/AVX2)"};

    int currentIdx = (m_SelectedBackend == MathBackendType::GLM) ? 0 : 1;
    if (TimeGUI::Combo("Active Math Backend", &currentIdx, backends, 2))
    {
        m_SelectedBackend = (currentIdx == 0) ? MathBackendType::GLM : MathBackendType::Custom;
        MathEngine::Get().SetBackend(m_SelectedBackend);
    }

    TimeGUI::Spacing();
    TimeGUI::TextDisabled("Note: Custom Math backend provides optimized SIMD/AVX kernels.");
}

#include "IKPlugin.hpp"
#include "Core/Log.h"
#include "MCP/IKMCPTools.hpp"

void IKPlugin::OnLoad()
{
    TE_CORE_INFO("[IKPlugin] Initializing 2D IKPlugin...");

    // Initialize MCP tools
    IK::IKMCPTools::RegisterTools();

    TE_CORE_INFO("[IKPlugin] Registered 2D Solvers: TwoBoneIK2D, FABRIK2D, CCD2D, AimConstraint2D, FootGrounder2D.");
    TE_CORE_INFO("[IKPlugin] 2D IKPlugin loaded successfully.");
}

void IKPlugin::OnUnload()
{
    TE_CORE_INFO("[IKPlugin] Unloading IKPlugin...");
    TE_CORE_INFO("[IKPlugin] IKPlugin unloaded.");
}

void IKPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    unsigned int bgCol = 0xFF261030;
    unsigned int borderCol = 0xFFA29BFE;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    TEVector2 root(min.x + 10.0f, min.y + 36.0f);
    TEVector2 joint(min.x + 26.0f, min.y + 14.0f);
    TEVector2 effector(min.x + 38.0f, min.y + 34.0f);

    // Bone link lines
    dl.AddLine(root, joint, 0xFFFD79A8, 3.0f);
    dl.AddLine(joint, effector, 0xFFE056FD, 3.0f);

    // Joints
    dl.AddCircleFilled(root, 4.0f, 0xFF6C5CE7);
    dl.AddCircle(root, 4.0f, 0xFFFFFFFF, 12, 1.2f);

    dl.AddCircleFilled(joint, 3.5f, 0xFFE056FD);
    dl.AddCircle(joint, 3.5f, 0xFFFFFFFF, 12, 1.2f);

    // Effector target crosshair
    dl.AddCircle(effector, 5.0f, 0xFF00CEC9, 12, 1.5f);
    dl.AddLine(TEVector2(effector.x - 7.0f, effector.y), TEVector2(effector.x + 7.0f, effector.y), 0xFF00CEC9, 1.2f);
    dl.AddLine(TEVector2(effector.x, effector.y - 7.0f), TEVector2(effector.x, effector.y + 7.0f), 0xFF00CEC9, 1.2f);
}

TE_REGISTER_PLUGIN(IKPlugin);

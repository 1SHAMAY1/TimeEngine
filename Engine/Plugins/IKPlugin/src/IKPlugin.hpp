#pragma once

#include "Core/Plugin/IPlugin.hpp"

class IKPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "IKPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override
    {
        return "2D Inverse Kinematics (IK) Plugin with Analytical 2-Bone, FABRIK, CCD, Aim/LookAt, and Foot Grounding Solvers";
    }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};

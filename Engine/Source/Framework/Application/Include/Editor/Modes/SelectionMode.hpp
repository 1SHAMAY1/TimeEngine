#pragma once
#include "PreRequisites.h"
#include "EditorMode.hpp"

class TE_API SelectionMode : public EditorMode
{
public:
    SelectionMode() = default;
    virtual ~SelectionMode() override = default;

    virtual TEString GetName() const override { return "Selection Mode"; }
    virtual TEString GetIcon() const override { return ""; }

    virtual bool ShouldHideStandardPanels() const override { return false; }
    virtual bool WantsFullscreenWorkspace() const override { return false; }

    virtual void OnEnter() override;
    virtual void OnUpdate(float dt) override;
    virtual void OnTimeGUIRender() override;
    virtual void OnExit() override;
};

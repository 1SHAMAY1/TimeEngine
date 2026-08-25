#pragma once

#include "Core/Plugin/IPlugin.hpp"
#include <memory>


class GameplayTagManagerLayer;

class GameplayTagPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "GameplayTagPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override { return "Hierarchical gameplay tags and container queries."; }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;

    GameplayTagManagerLayer *GetManagerLayer() const { return m_ManagerLayer.get(); }

private:
    TEScope<GameplayTagManagerLayer> m_ManagerLayer;
};


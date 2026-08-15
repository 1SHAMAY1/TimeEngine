#pragma once

#include "Core/Plugin/IPlugin.hpp"

namespace TE
{

class GameplayTagPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;
};

} // namespace TE

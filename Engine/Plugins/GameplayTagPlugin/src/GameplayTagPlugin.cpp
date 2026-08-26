#include "GameplayTagPlugin.hpp"
#include "Core/Log.h"
#include "GameplayTagManager.hpp"
#include "GameplayTagManagerLayer.hpp"

void GameplayTagPlugin::OnLoad()
{
    TE_CORE_INFO("[GameplayTagPlugin] Initializing GameplayTagPlugin...");
    m_ManagerLayer = CreateScope<GameplayTagManagerLayer>();
    TE_CORE_INFO("[GameplayTagPlugin] GameplayTagPlugin initialized successfully with GameplayTagManagerLayer.");
}

void GameplayTagPlugin::OnUnload()
{
    TE_CORE_INFO("[GameplayTagPlugin] Unloading GameplayTagPlugin...");
    if (m_ManagerLayer)
    {
        m_ManagerLayer.reset();
    }
    GameplayTagManager::Get().Clear();
    TE_CORE_INFO("[GameplayTagPlugin] GameplayTagPlugin unloaded.");
}

void GameplayTagPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    unsigned int bgCol = 0xFF2A200B;
    unsigned int borderCol = 0xFFFFA502;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Angled tag polygon
    TEVector2 tagPts[5] = {TEVector2(min.x + 10.0f, min.y + 18.0f), TEVector2(min.x + 20.0f, min.y + 8.0f),
                           TEVector2(min.x + 38.0f, min.y + 8.0f), TEVector2(min.x + 38.0f, min.y + 38.0f),
                           TEVector2(min.x + 10.0f, min.y + 38.0f)};
    dl.AddConvexPolyFilled(tagPts, 5, 0xFF4A3812);
    dl.AddPolyline(tagPts, 5, 0xFFFFBE76, 1, 1.5f);

    // Tag hole eyelet
    dl.AddCircleFilled(TEVector2(min.x + 18.0f, min.y + 15.0f), 2.5f, 0xFF1E1608);
    dl.AddCircle(TEVector2(min.x + 18.0f, min.y + 15.0f), 2.5f, 0xFFF6E58D, 10, 1.2f);

    // Subtag hierarchy notch lines
    dl.AddLine(TEVector2(min.x + 20.0f, min.y + 24.0f), TEVector2(min.x + 33.0f, min.y + 24.0f), 0xFF2ECC71, 2.0f);
    dl.AddLine(TEVector2(min.x + 24.0f, min.y + 30.0f), TEVector2(min.x + 33.0f, min.y + 30.0f), 0xFF00CEC9, 2.0f);
}

TE_REGISTER_PLUGIN(GameplayTagPlugin)

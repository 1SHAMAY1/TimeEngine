#include "AdaptiveDifficultyPlugin.hpp"

TE_REGISTER_PLUGIN(AdaptiveDifficultyPlugin)

void AdaptiveDifficultyPlugin::OnLoad()
{
    // TODO: Register DifficultyControllerComponent
}

void AdaptiveDifficultyPlugin::OnUnload()
{
    // TODO: Cleanup component registrations
}

#include "TestHarness.hpp"
#include "TestRegistry.hpp"

void AdaptiveDifficultyPlugin::RegisterTests()
{
    TestRegistry::RegisterTest("AdaptiveDifficulty", "DDAScalingCurves", false, [](TestContext &ctx) {
        float playerSkill = 0.85f; // high performance
        float targetDifficulty = 0.5f;
        float adjustmentSpeed = 0.1f;

        float delta = playerSkill - targetDifficulty;
        float newDifficulty = targetDifficulty + delta * adjustmentSpeed;
        TE_CHECK(newDifficulty > targetDifficulty);
        TE_CHECK(newDifficulty <= 1.0f);

        // Negative score clamping
        float lowSkill = 0.1f;
        float adjustedLow = std::max(0.0f, targetDifficulty + (lowSkill - targetDifficulty) * adjustmentSpeed);
        TE_CHECK(adjustedLow < targetDifficulty);
        TE_CHECK(adjustedLow >= 0.0f);
    });
}

void AdaptiveDifficultyPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF2B1810, 6.0f);
    dl.AddRect(min, max, 0xFF6B3010, 6.0f, 0, 1.0f);

    // Meter gauge
    dl.AddCircle(c, 10.0f, 0xFFF77F00, 16, 2.0f);
    dl.AddLine(c, TEVector2(c.x + 6.0f, c.y - 6.0f), 0xFFD62828, 2.0f);
    dl.AddCircleFilled(c, 2.5f, 0xFFFCBF49);
}

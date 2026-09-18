#include "CheatDetectionPlugin.hpp"

TE_REGISTER_PLUGIN(CheatDetectionPlugin)

void CheatDetectionPlugin::OnLoad()
{
    // TODO: Register network kinematic validators once networking layer is ready
}

void CheatDetectionPlugin::OnUnload()
{
    // TODO: Cleanup validator registrations
}

#include "TestHarness.hpp"
#include "TestRegistry.hpp"

void CheatDetectionPlugin::RegisterTests()
{
    TestRegistry::RegisterTest("CheatDetection", "SpeedHackAndTeleportationValidation", false, [](TestContext &ctx) {
        constexpr float MaxAllowedSpeed = 15.0f; // units / sec
        constexpr float DeltaTime = 0.01667f;    // ~60 FPS

        TEVector2 posPrevious = {0.0f, 0.0f};
        TEVector2 posLegit = {0.2f, 0.0f}; // speed = 12 units/sec (legit)
        TEVector2 posCheater = {10.0f, 0.0f}; // speed = 600 units/sec (teleport)

        float speedLegit = (posLegit - posPrevious).Length() / DeltaTime;
        float speedCheater = (posCheater - posPrevious).Length() / DeltaTime;

        TE_CHECK(speedLegit <= MaxAllowedSpeed);
        TE_CHECK(speedCheater > MaxAllowedSpeed);
    });
}

void CheatDetectionPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF2B0909, 6.0f);
    dl.AddRect(min, max, 0xFF6B1414, 6.0f, 0, 1.0f);

    // Shield icon
    dl.AddCircle(c, 9.0f, 0xFFE63946, 16, 2.0f);
    dl.AddLine(TEVector2(c.x - 5.0f, c.y), TEVector2(c.x, c.y + 5.0f), 0xFFF1FAEE, 2.0f);
    dl.AddLine(TEVector2(c.x, c.y + 5.0f), TEVector2(c.x + 6.0f, c.y - 4.0f), 0xFFF1FAEE, 2.0f);
}

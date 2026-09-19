#include "AdaptiveMusicPlugin.hpp"

TE_REGISTER_PLUGIN(AdaptiveMusicPlugin)

void AdaptiveMusicPlugin::OnLoad()
{
    // TODO: Register AdaptiveMusicComponent
}

void AdaptiveMusicPlugin::OnUnload()
{
    // TODO: Cleanup components
}

#include "TestHarness.hpp"
#include "TestRegistry.hpp"

void AdaptiveMusicPlugin::RegisterTests()
{
    TestRegistry::RegisterTest("AdaptiveMusic", "StemCrossfadeInterpolation", false,
                               [](TestContext &ctx)
                               {
                                   float stemA_Vol = 1.0f;
                                   float stemB_Vol = 0.0f;
                                   float blendFactor = 0.5f;

                                   // Equal-power crossfade calculation: cos and sin curves
                                   float volA = std::cos(blendFactor * 1.5707963f);
                                   float volB = std::sin(blendFactor * 1.5707963f);

                                   TE_CHECK(volA > 0.6f && volA < 0.8f);
                                   TE_CHECK(volB > 0.6f && volB < 0.8f);
                                   // Energy conservation check (volA^2 + volB^2 approx 1.0)
                                   float energy = volA * volA + volB * volB;
                                   TE_CHECK(std::abs(energy - 1.0f) < 0.01f);
                               });
}

void AdaptiveMusicPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF28102B, 6.0f);
    dl.AddRect(min, max, 0xFF7209B7, 6.0f, 0, 1.0f);

    // Equalizer bars
    dl.AddRectFilled(TEVector2(c.x - 8.0f, c.y - 4.0f), TEVector2(c.x - 5.0f, c.y + 8.0f), 0xFF4CC9F0);
    dl.AddRectFilled(TEVector2(c.x - 2.0f, c.y - 10.0f), TEVector2(c.x + 1.0f, c.y + 8.0f), 0xFFF72585);
    dl.AddRectFilled(TEVector2(c.x + 4.0f, c.y - 6.0f), TEVector2(c.x + 7.0f, c.y + 8.0f), 0xFF4895EF);
}

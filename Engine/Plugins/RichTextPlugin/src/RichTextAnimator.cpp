#include "RichTextAnimator.hpp"
#include <cmath>
#include <cstdlib>


static TEColor HSLToRGB(float h, float s, float l)
{
    float c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;

    float r = 0, g = 0, b = 0;
    if (h < 60.0f) { r = c; g = x; b = 0; }
    else if (h < 120.0f) { r = x; g = c; b = 0; }
    else if (h < 180.0f) { r = 0; g = c; b = x; }
    else if (h < 240.0f) { r = 0; g = x; b = c; }
    else if (h < 300.0f) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return TEColor(r + m, g + m, b + m, 1.0f);
}

void RichTextAnimator::AnimateLayout(RichTextLayoutResult &layout, float totalTime, float deltaTime,
                                    float typewriterProgress)
{
    size_t visibleCharLimit = static_cast<size_t>(layout.TotalCharacterCount * std::clamp(typewriterProgress, 0.0f, 1.0f));

    for (auto &glyph : layout.GlyphQuads)
    {
        const auto &style = glyph.Style;

        // Typewriter reveal clipping
        if (style.Effect == ERichTextEffect::Typewriter)
        {
            if (glyph.CharacterIndex > visibleCharLimit)
            {
                glyph.Color = TEColor(0.0f, 0.0f, 0.0f, 0.0f);
                continue;
            }
        }

        // Wave effect
        if (style.Effect == ERichTextEffect::Wave)
        {
            float amp = style.EffectParams.x > 0.0f ? style.EffectParams.x : 4.0f;
            float speed = style.EffectParams.y > 0.0f ? style.EffectParams.y : 3.0f;
            float waveOffset = std::sin(totalTime * speed + glyph.CharacterIndex * 0.4f) * amp;
            glyph.Position.y += waveOffset;
        }
        // Shake effect
        else if (style.Effect == ERichTextEffect::Shake)
        {
            float intensity = style.EffectParams.x > 0.0f ? style.EffectParams.x : 2.0f;
            float shakeX = (((std::rand() % 100) / 50.0f) - 1.0f) * intensity;
            float shakeY = (((std::rand() % 100) / 50.0f) - 1.0f) * intensity;
            glyph.Position.x += shakeX;
            glyph.Position.y += shakeY;
        }
        // Rainbow effect
        else if (style.Effect == ERichTextEffect::Rainbow)
        {
            float speed = style.EffectParams.x > 0.0f ? style.EffectParams.x : 2.0f;
            float hue = std::fmod((totalTime * speed * 60.0f + glyph.CharacterIndex * 20.0f), 360.0f);
            glyph.Color = HSLToRGB(hue, 0.9f, 0.6f);
        }
    }
}


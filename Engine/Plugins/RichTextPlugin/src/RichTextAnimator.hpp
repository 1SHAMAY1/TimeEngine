#pragma once

#include "RichTextLayout.hpp"
#include "RichTextTypes.hpp"


class RichTextAnimator
{
public:
    RichTextAnimator() = default;

    /// Applies dynamic real-time vertex & color modifications (Wave, Shake, Rainbow, Typewriter)
    static void AnimateLayout(RichTextLayoutResult &layout, float totalTime, float deltaTime,
                              float typewriterProgress = 1.0f);
};


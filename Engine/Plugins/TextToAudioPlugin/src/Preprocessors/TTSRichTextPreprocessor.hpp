#pragma once

#include "../TTSAudioTypes.hpp"
#include "Utils/TEString.hpp"

class TTSRichTextPreprocessor
{
public:
    static TTSProcessedSpeech ProcessText(const TEString &input);

    static TEString StripAllTags(const TEString &input);
    static TEString CompileToSSML(const TEString &input);
};

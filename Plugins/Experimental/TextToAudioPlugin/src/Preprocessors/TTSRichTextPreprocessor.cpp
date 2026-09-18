#include "TTSRichTextPreprocessor.hpp"
#include <regex>

#if defined(TE_HAS_PLUGIN_RICHTEXTPLUGIN) || defined(TE_PLUGIN_RICHTEXTPLUGIN)
#include "../../../RichTextPlugin/src/RichTextParser.hpp"
#endif

TEString TTSRichTextPreprocessor::StripAllTags(const TEString &input)
{
    TEString clean;
    bool inTag = false;
    for (size_t i = 0; i < input.Num(); ++i)
    {
        char c = input[i];
        if (c == '<')
        {
            inTag = true;
        }
        else if (c == '>')
        {
            inTag = false;
        }
        else if (!inTag)
        {
            clean.Append(c);
        }
    }
    return clean;
}

TEString TTSRichTextPreprocessor::CompileToSSML(const TEString &input)
{
    TEString ssml = "<speak version=\"1.0\" xmlns=\"http://www.w3.org/2001/10/synthesis\" xml:lang=\"en-US\">";

    size_t i = 0;
    while (i < input.Num())
    {
        if (input[i] == '<')
        {
            int closePos = input.Find(">", ESearchCase::CaseSensitive, ESearchDir::FromStart, static_cast<int>(i));
            if (closePos >= 0)
            {
                TEString tagContent = input.Mid(i + 1, closePos - i - 1);
                i = closePos + 1;

                // Process Acoustic Tags:
                // 1. Pause / Break / Stop
                if (tagContent.StartsWith("pause") || tagContent.StartsWith("break") || tagContent.StartsWith("stop"))
                {
                    std::regex timeRegex(R"((?:time|duration)=["']?([0-9]+)(ms|s)?["']?)");
                    std::cmatch match;
                    if (std::regex_search(tagContent.c_str(), match, timeRegex))
                    {
                        long ms = std::stol(match[1].str());
                        if (match[2].str() == "s")
                            ms *= 1000;

                        ssml += "<silence msec=\"" + TEString::FromInt64(ms) + "\"/>";
                    }
                    else
                    {
                        ssml += "<silence msec=\"500\"/>";
                    }
                }
                // 2. Pitch
                else if (tagContent.StartsWith("pitch") && !tagContent.StartsWith("/"))
                {
                    std::regex pitchRegex(R"(value=["']?([^"']+)["']?)");
                    std::cmatch match;
                    if (std::regex_search(tagContent.c_str(), match, pitchRegex))
                    {
                        ssml += "<prosody pitch=\"" + TEString(match[1].str().c_str()) + "\">";
                    }
                }
                else if (tagContent == "/pitch")
                {
                    ssml += "</prosody>";
                }
                // 3. Rate / Speed
                else if ((tagContent.StartsWith("rate") || tagContent.StartsWith("speed")) &&
                         !tagContent.StartsWith("/"))
                {
                    std::regex rateRegex(R"(value=["']?([^"']+)["']?)");
                    std::cmatch match;
                    if (std::regex_search(tagContent.c_str(), match, rateRegex))
                    {
                        ssml += "<prosody rate=\"" + TEString(match[1].str().c_str()) + "\">";
                    }
                }
                else if (tagContent == "/rate" || tagContent == "/speed")
                {
                    ssml += "</prosody>";
                }
                // 4. Volume
                else if (tagContent.StartsWith("volume") && !tagContent.StartsWith("/"))
                {
                    std::regex volRegex(R"(value=["']?([^"']+)["']?)");
                    std::cmatch match;
                    if (std::regex_search(tagContent.c_str(), match, volRegex))
                    {
                        ssml += "<prosody volume=\"" + TEString(match[1].str().c_str()) + "\">";
                    }
                }
                else if (tagContent == "/volume")
                {
                    ssml += "</prosody>";
                }
                // 5. Emphasis
                else if (tagContent.StartsWith("emphasis"))
                {
                    ssml += "<emph>";
                }
                else if (tagContent == "/emphasis")
                {
                    ssml += "</emph>";
                }
                // 6. Voice / Accent
                else if ((tagContent.StartsWith("voice") || tagContent.StartsWith("accent")) &&
                         !tagContent.StartsWith("/"))
                {
                    std::regex voiceRegex(R"((?:name|value)=["']?([^"']+)["']?)");
                    std::cmatch match;
                    if (std::regex_search(tagContent.c_str(), match, voiceRegex))
                    {
                        ssml += "<voice required=\"Name=" + TEString(match[1].str().c_str()) + "\">";
                    }
                }
                else if (tagContent == "/voice" || tagContent == "/accent")
                {
                    ssml += "</voice>";
                }
                // All other visual tags are stripped
                continue;
            }
        }

        ssml.Append(input[i]);
        i++;
    }

    ssml += "</speak>";
    return ssml;
}

TTSProcessedSpeech TTSRichTextPreprocessor::ProcessText(const TEString &input)
{
    TTSProcessedSpeech result;
    result.PlainText = StripAllTags(input);

    bool hasAcoustic = (input.Contains("<pause") || input.Contains("<break") || input.Contains("<stop") ||
                        input.Contains("<pitch") || input.Contains("<rate") || input.Contains("<speed") ||
                        input.Contains("<volume") || input.Contains("<accent") || input.Contains("<voice") ||
                        input.Contains("<emphasis") || input.Contains("<whisper"));

    result.HasAcousticMarkup = hasAcoustic;
    if (hasAcoustic)
    {
        result.SSMLText = CompileToSSML(input);
    }
    else
    {
        result.SSMLText = result.PlainText;
    }

    return result;
}

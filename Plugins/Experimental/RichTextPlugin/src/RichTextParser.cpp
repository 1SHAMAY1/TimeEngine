#include "RichTextParser.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

TEColor RichTextParser::ParseColor(const TEString &colorStr)
{
    TEString str = colorStr;
    if (!str.empty() && str[0] == '#')
    {
        str = str.substr(1);
        if (str.length() == 6)
        {
            uint32_t hexVal = std::stoul(str, nullptr, 16);
            float r = ((hexVal >> 16) & 0xFF) / 255.0f;
            float g = ((hexVal >> 8) & 0xFF) / 255.0f;
            float b = (hexVal & 0xFF) / 255.0f;
            return TEColor(r, g, b, 1.0f);
        }
        else if (str.length() == 8)
        {
            uint32_t hexVal = std::stoul(str, nullptr, 16);
            float r = ((hexVal >> 24) & 0xFF) / 255.0f;
            float g = ((hexVal >> 16) & 0xFF) / 255.0f;
            float b = ((hexVal >> 8) & 0xFF) / 255.0f;
            float a = (hexVal & 0xFF) / 255.0f;
            return TEColor(r, g, b, a);
        }
    }

    // Named colors
    TEString lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "red")
        return TEColor(1.0f, 0.0f, 0.0f, 1.0f);
    if (lower == "green")
        return TEColor(0.0f, 1.0f, 0.0f, 1.0f);
    if (lower == "blue")
        return TEColor(0.0f, 0.4f, 1.0f, 1.0f);
    if (lower == "yellow" || lower == "gold")
        return TEColor(1.0f, 0.84f, 0.0f, 1.0f);
    if (lower == "cyan")
        return TEColor(0.0f, 1.0f, 1.0f, 1.0f);
    if (lower == "magenta" || lower == "pink")
        return TEColor(1.0f, 0.2f, 0.8f, 1.0f);
    if (lower == "white")
        return TEColor(1.0f, 1.0f, 1.0f, 1.0f);
    if (lower == "black")
        return TEColor(0.0f, 0.0f, 0.0f, 1.0f);
    if (lower == "gray" || lower == "grey")
        return TEColor(0.5f, 0.5f, 0.5f, 1.0f);
    if (lower == "orange")
        return TEColor(1.0f, 0.5f, 0.0f, 1.0f);

    return TEColor(1.0f, 1.0f, 1.0f, 1.0f);
}

static void ParseAttributes(const TEString &attrStr, TEMap<TEString, TEString> &outAttrs)
{
    TEArray<TEString> tokens = attrStr.Split(' ');
    for (const auto &token : tokens)
    {
        size_t eqPos = token.find('=');
        if (eqPos != TEString::npos)
        {
            TEString k = token.substr(0, eqPos);
            TEString v = token.substr(eqPos + 1);
            if (v.Length() >= 2 && (v[0] == '"' || v[0] == '\'') && v[v.Length() - 1] == v[0])
                v = v.substr(1, v.Length() - 2);
            outAttrs[k] = v;
        }
    }
}

TEArray<RichTextSpan> RichTextParser::Parse(const TEString &markupText, const TERef<RichTextTable> &styleTable,
                                            const RichTextStyle &defaultStyle,
                                            const TEMap<TEString, TEString> &variables)
{
    // Step 1: Dynamic Localization Resolution
    TEString localized = markupText.ResolveLocalized();

    // Step 2: Dynamic Placeholder Formatting
    TEString resolvedText = localized;
    for (const auto &[k, v] : variables)
    {
        resolvedText = resolvedText.Replace("{" + k + "}", v);
    }

    TEArray<RichTextSpan> spans;
    std::stack<RichTextStyle> styleStack;
    styleStack.push(defaultStyle);

    std::stack<ERichTextAlign> alignStack;
    alignStack.push(ERichTextAlign::Left);

    TEString currentBuffer;
    const TEString &src = resolvedText.ToStdString();
    size_t i = 0;

    auto flushBuffer = [&]()
    {
        if (!currentBuffer.empty())
        {
            RichTextSpan span;
            span.Text = TEString(currentBuffer);
            span.Style = styleStack.top();
            span.Alignment = alignStack.top();
            span.IsIcon = false;
            spans.Add(span);
            currentBuffer.Clear();
        }
    };

    while (i < src.length())
    {
        // Handle escaped characters \< or \>
        if (src[i] == '\\' && i + 1 < src.length() && (src[i + 1] == '<' || src[i + 1] == '>'))
        {
            currentBuffer += src[i + 1];
            i += 2;
            continue;
        }

        // Tag open
        if (src[i] == '<')
        {
            size_t closePos = src.find('>', i + 1);
            if (closePos != TEString::npos)
            {
                flushBuffer();
                TEString tagContent = src.substr(i + 1, closePos - (i + 1));
                i = closePos + 1;

                if (tagContent.empty())
                    continue;

                // Closing tag
                if (tagContent[0] == '/')
                {
                    if (styleStack.size() > 1)
                        styleStack.pop();
                    if (alignStack.size() > 1)
                        alignStack.pop();
                    continue;
                }

                // Self-closing or opening tag
                RichTextStyle currentStyle = styleStack.top();
                ERichTextAlign currentAlign = alignStack.top();

                // Style Preset Tag: <style="Header"> or <Header>
                if (tagContent.find("style=") == 0)
                {
                    TEString styleName = tagContent.substr(6);
                    if (styleName.Length() >= 2 && (styleName[0] == '"' || styleName[0] == '\''))
                        styleName = styleName.substr(1, styleName.Length() - 2);

                    if (styleTable && styleTable->HasStyle(TEString(styleName)))
                    {
                        currentStyle = styleTable->GetStyle(TEString(styleName), currentStyle);
                    }
                    styleStack.push(currentStyle);
                }
                else if (tagContent == "b")
                {
                    currentStyle.Bold = true;
                    styleStack.push(currentStyle);
                }
                else if (tagContent == "i")
                {
                    currentStyle.Italic = true;
                    styleStack.push(currentStyle);
                }
                else if (tagContent == "u")
                {
                    currentStyle.Underline = true;
                    styleStack.push(currentStyle);
                }
                else if (tagContent == "s")
                {
                    currentStyle.Strikethrough = true;
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("color=") == 0)
                {
                    TEString colVal = tagContent.substr(6);
                    if (colVal.Length() >= 2 && (colVal[0] == '"' || colVal[0] == '\''))
                        colVal = colVal.substr(1, colVal.Length() - 2);
                    currentStyle.TextColor = ParseColor(colVal);
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("size=") == 0 || tagContent.find("scale=") == 0)
                {
                    size_t eqPos = tagContent.find('=');
                    float val = std::stof(tagContent.substr(eqPos + 1));
                    if (tagContent.find("size=") == 0)
                        currentStyle.FontSize = val;
                    else
                        currentStyle.Scale = val;
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("font=") == 0)
                {
                    TEString fontName = tagContent.substr(5);
                    if (fontName.Length() >= 2 && (fontName[0] == '"' || fontName[0] == '\''))
                        fontName = fontName.substr(1, fontName.Length() - 2);
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("align=") == 0)
                {
                    TEString alignVal = tagContent.substr(6);
                    if (alignVal == "center")
                        currentAlign = ERichTextAlign::Center;
                    else if (alignVal == "right")
                        currentAlign = ERichTextAlign::Right;
                    else if (alignVal == "justify")
                        currentAlign = ERichTextAlign::Justify;
                    else
                        currentAlign = ERichTextAlign::Left;
                    alignStack.push(currentAlign);
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("link=") == 0)
                {
                    TEString linkVal = tagContent.substr(5);
                    if (linkVal.Length() >= 2 && (linkVal[0] == '"' || linkVal[0] == '\''))
                        linkVal = linkVal.substr(1, linkVal.Length() - 2);
                    currentStyle.LinkID = TEString(linkVal);
                    currentStyle.Underline = true;
                    currentStyle.TextColor = TEColor(0.2f, 0.6f, 1.0f, 1.0f);
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("wave") == 0)
                {
                    currentStyle.Effect = ERichTextEffect::Wave;
                    TEMap<TEString, TEString> attrs;
                    ParseAttributes(tagContent, attrs);
                    if (attrs.count("amp"))
                        currentStyle.EffectParams.x = std::stof(attrs["amp"]);
                    if (attrs.count("speed"))
                        currentStyle.EffectParams.y = std::stof(attrs["speed"]);
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("shake") == 0)
                {
                    currentStyle.Effect = ERichTextEffect::Shake;
                    TEMap<TEString, TEString> attrs;
                    ParseAttributes(tagContent, attrs);
                    if (attrs.count("intensity"))
                        currentStyle.EffectParams.x = std::stof(attrs["intensity"]);
                    if (attrs.count("speed"))
                        currentStyle.EffectParams.y = std::stof(attrs["speed"]);
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("rainbow") == 0)
                {
                    currentStyle.Effect = ERichTextEffect::Rainbow;
                    TEMap<TEString, TEString> attrs;
                    ParseAttributes(tagContent, attrs);
                    if (attrs.count("speed"))
                        currentStyle.EffectParams.x = std::stof(attrs["speed"]);
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("typewriter") == 0)
                {
                    currentStyle.Effect = ERichTextEffect::Typewriter;
                    TEMap<TEString, TEString> attrs;
                    ParseAttributes(tagContent, attrs);
                    if (attrs.count("speed"))
                        currentStyle.EffectParams.x = std::stof(attrs["speed"]);
                    styleStack.push(currentStyle);
                }
                else if (tagContent.find("icon=") == 0 || tagContent.find("sprite=") == 0)
                {
                    size_t eqPos = tagContent.find('=');
                    TEString iconName = tagContent.substr(eqPos + 1);
                    if (iconName.Length() >= 2 && (iconName[0] == '"' || iconName[0] == '\''))
                        iconName = iconName.substr(1, iconName.Length() - 2);

                    RichTextSpan iconSpan;
                    iconSpan.Text = TEString(iconName);
                    iconSpan.Style = currentStyle;
                    iconSpan.Style.SpriteName = TEString(iconName);
                    iconSpan.Alignment = currentAlign;
                    iconSpan.IsIcon = true;
                    spans.Add(iconSpan);
                }
                else if (styleTable && styleTable->HasStyle(TEString(tagContent)))
                {
                    // Direct style alias tag: e.g. <Header>
                    currentStyle = styleTable->GetStyle(TEString(tagContent), currentStyle);
                    styleStack.push(currentStyle);
                }
                else
                {
                    // Unknown tag, keep style stack level
                    styleStack.push(currentStyle);
                }
                continue;
            }
        }

        currentBuffer += src[i];
        ++i;
    }

    flushBuffer();
    return spans;
}

TEString RichTextParser::StripTags(const TEString &markupText)
{
    TEString result;
    const TEString &src = markupText.ToStdString();
    bool insideTag = false;

    for (size_t i = 0; i < src.length(); ++i)
    {
        if (src[i] == '\\' && i + 1 < src.length() && (src[i + 1] == '<' || src[i + 1] == '>'))
        {
            result += src[i + 1];
            ++i;
            continue;
        }

        if (src[i] == '<')
        {
            insideTag = true;
            continue;
        }
        if (src[i] == '>')
        {
            insideTag = false;
            continue;
        }

        if (!insideTag)
        {
            result += src[i];
        }
    }

    return TEString(result);
}

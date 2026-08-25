#include "RichTextLayout.hpp"
#include <algorithm>


struct LayoutWord
{
    TEArray<RichTextGlyphQuad> Glyphs;
    float Width = 0.0f;
    float Height = 0.0f;
    bool IsWhitespace = false;
    bool IsNewline = false;
    ERichTextAlign Alignment = ERichTextAlign::Left;
};

struct LayoutLine
{
    TEArray<LayoutWord> Words;
    float Width = 0.0f;
    float Height = 0.0f;
    float MaxAscent = 0.0f;
    ERichTextAlign Alignment = ERichTextAlign::Left;
};

RichTextLayoutResult RichTextLayoutEngine::ComputeLayout(const TEArray<RichTextSpan> &spans,
                                                        const TERef<FontAsset> &defaultFont,
                                                        float wrapWidth,
                                                        float defaultLineSpacing)
{
    RichTextLayoutResult result;
    if (!defaultFont || spans.IsEmpty())
        return result;

    float baseLineHeight = defaultFont->GetLineHeight() * defaultLineSpacing;
    float baseAscent = defaultFont->GetAscent();

    TEArray<LayoutLine> lines;
    LayoutLine currentLine;
    currentLine.Height = baseLineHeight;
    currentLine.MaxAscent = baseAscent;

    LayoutWord currentWord;
    size_t globalCharIndex = 0;
    TEString prevChar;

    for (const auto &span : spans)
    {
        const auto &style = span.Style;
        float scale = style.Scale * (style.FontSize / defaultFont->GetPixelSize());
        currentLine.Alignment = span.Alignment;

        if (span.IsIcon)
        {
            // Inline Icon quad
            float iconDim = (style.FontSize > 0.0f ? style.FontSize : 24.0f) * style.Scale;
            RichTextGlyphQuad iconQuad;
            iconQuad.Codepoint = 0;
            iconQuad.Size = TEVector2(iconDim, iconDim);
            iconQuad.UV = TEVector4(0.0f, 0.0f, 1.0f, 1.0f);
            iconQuad.Color = style.TextColor;
            iconQuad.Style = style;
            iconQuad.CharacterIndex = globalCharIndex++;
            iconQuad.IsIcon = true;

            currentWord.Glyphs.Add(iconQuad);
            currentWord.Width += iconDim + 4.0f;
            if (iconDim > currentWord.Height)
                currentWord.Height = iconDim;
            currentWord.Alignment = span.Alignment;
            continue;
        }

        for (size_t i = 0; i < span.Text.Length(); ++i)
        {
            char ch = span.Text[i];
            if (ch == '\n')
            {
                if (!currentWord.Glyphs.IsEmpty())
                {
                    currentLine.Words.Add(currentWord);
                    currentLine.Width += currentWord.Width;
                    currentWord = LayoutWord{};
                }

                lines.Add(currentLine);
                currentLine = LayoutLine{};
                currentLine.Height = baseLineHeight;
                currentLine.MaxAscent = baseAscent;
                currentLine.Alignment = span.Alignment;
                prevChar.Clear();
                continue;
            }

            TEString cp;
            cp.Append(ch);
            FontGlyph glyph;
            if (!defaultFont->GetGlyph(cp, glyph))
                continue;

            float kern = (!prevChar.IsEmpty()) ? defaultFont->GetKerning(prevChar, cp) * scale : 0.0f;
            float advance = glyph.AdvanceX * scale + kern;

            RichTextGlyphQuad gQuad;
            gQuad.Codepoint = cp;
            gQuad.Size = TEVector2(glyph.Width * scale, glyph.Height * scale);
            gQuad.UV = glyph.UV;
            gQuad.Color = style.TextColor;
            gQuad.Style = style;
            gQuad.CharacterIndex = globalCharIndex++;
            gQuad.IsIcon = false;

            // Position offsets relative to baseline
            gQuad.Position.x = glyph.BearingX * scale;
            gQuad.Position.y = glyph.BearingY * scale;

            if (ch == ' ' || ch == '\t')
            {
                if (!currentWord.Glyphs.IsEmpty())
                {
                    currentLine.Words.Add(currentWord);
                    currentLine.Width += currentWord.Width;
                    currentWord = LayoutWord{};
                }

                LayoutWord spaceWord;
                spaceWord.IsWhitespace = true;
                spaceWord.Width = advance;
                spaceWord.Height = baseLineHeight;
                spaceWord.Alignment = span.Alignment;
                currentLine.Words.Add(spaceWord);
                currentLine.Width += advance;
                prevChar = cp;
                continue;
            }

            currentWord.Glyphs.Add(gQuad);
            currentWord.Width += advance;
            if (gQuad.Size.y > currentWord.Height)
                currentWord.Height = gQuad.Size.y;
            currentWord.Alignment = span.Alignment;

            // Word wrap check
            if (wrapWidth > 0.0f && (currentLine.Width + currentWord.Width) > wrapWidth)
            {
                if (!currentLine.Words.IsEmpty())
                {
                    lines.Add(currentLine);
                    currentLine = LayoutLine{};
                    currentLine.Height = baseLineHeight;
                    currentLine.MaxAscent = baseAscent;
                    currentLine.Alignment = span.Alignment;
                }
            }

            prevChar = cp;
        }

        if (!currentWord.Glyphs.IsEmpty())
        {
            currentLine.Words.Add(currentWord);
            currentLine.Width += currentWord.Width;
            currentWord = LayoutWord{};
        }
    }

    if (!currentLine.Words.IsEmpty() || lines.IsEmpty())
    {
        lines.Add(currentLine);
    }

    // Pass 2: Position quads & hitboxes across lines
    float currentY = 0.0f;
    float maxLineWidth = 0.0f;

    for (auto &line : lines)
    {
        float startX = 0.0f;
        if (wrapWidth > 0.0f && line.Width < wrapWidth)
        {
            if (line.Alignment == ERichTextAlign::Center)
                startX = (wrapWidth - line.Width) * 0.5f;
            else if (line.Alignment == ERichTextAlign::Right)
                startX = wrapWidth - line.Width;
        }

        if (line.Width > maxLineWidth)
            maxLineWidth = line.Width;

        float cursorX = startX;
        float baselineY = currentY + line.MaxAscent;

        for (auto &word : line.Words)
        {
            if (word.IsWhitespace)
            {
                cursorX += word.Width;
                continue;
            }

            TEString activeLinkID = "";
            TEVector2 linkMin{cursorX, currentY};

            for (auto &g : word.Glyphs)
            {
                if (g.IsIcon)
                {
                    g.Position = TEVector2(cursorX, baselineY - g.Size.y * 0.8f);
                    cursorX += g.Size.x + 4.0f;
                }
                else
                {
                    float glyphX = cursorX + g.Position.x;
                    float glyphY = baselineY - g.Position.y;
                    g.Position = TEVector2(glyphX, glyphY);

                    FontGlyph glyph;
                    bool hasGlyph = defaultFont->GetGlyph(g.Codepoint, glyph);
                    float scale = g.Style.Scale * (g.Style.FontSize / defaultFont->GetPixelSize());
                    cursorX += (hasGlyph ? glyph.AdvanceX * scale : g.Size.x);
                }

                if (!g.Style.LinkID.IsEmpty())
                {
                    activeLinkID = g.Style.LinkID;
                }

                result.GlyphQuads.Add(g);
            }

            if (!activeLinkID.IsEmpty())
            {
                RichTextLinkHitbox hitbox;
                hitbox.LinkID = activeLinkID;
                hitbox.Min = linkMin;
                hitbox.Max = TEVector2(cursorX, currentY + line.Height);
                result.LinkHitboxes.Add(hitbox);
            }
        }

        currentY += line.Height;
    }

    result.Bounds = TEVector2(wrapWidth > 0.0f ? wrapWidth : maxLineWidth, currentY);
    result.TotalCharacterCount = globalCharIndex;
    return result;
}

TEString RichTextLayoutEngine::HitTestLink(const RichTextLayoutResult &layout, const TEVector2 &localPos)
{
    for (const auto &hitbox : layout.LinkHitboxes)
    {
        if (localPos.x >= hitbox.Min.x && localPos.x <= hitbox.Max.x &&
            localPos.y >= hitbox.Min.y && localPos.y <= hitbox.Max.y)
        {
            return hitbox.LinkID;
        }
    }
    return TEString::None;
}


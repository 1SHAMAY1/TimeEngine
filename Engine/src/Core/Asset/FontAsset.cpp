#include "Core/Asset/FontAsset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Renderer/Texture.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <sstream>

FontAsset::FontAsset() {}

FontAsset::FontAsset(const TEString &name, float pixelSize) : m_Name(name), m_PixelSize(pixelSize) {}

TERef<Asset> FontAsset::Clone() const
{
    auto copy = CreateRef<FontAsset>(m_Name);
    copy->m_SourcePath = m_SourcePath;
    copy->m_AtlasTexturePath = m_AtlasTexturePath;
    copy->m_PixelSize = m_PixelSize;
    copy->m_AtlasWidth = m_AtlasWidth;
    copy->m_AtlasHeight = m_AtlasHeight;
    copy->m_LineHeight = m_LineHeight;
    copy->m_Ascent = m_Ascent;
    copy->m_Descent = m_Descent;
    copy->m_Glyphs = m_Glyphs;
    copy->m_Kerning = m_Kerning;
    copy->m_AtlasTexture = m_AtlasTexture;
    return copy;
}

bool FontAsset::BakeFromTTF(const TEString &ttfPath, float pixelSize, uint32_t atlasWidth, uint32_t atlasHeight)
{
    std::ifstream file(ttfPath.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        TE_CORE_ERROR("FontAsset::BakeFromTTF - Failed to open TTF file: {0}", ttfPath);
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    TEArray<uint8_t> ttfBuffer;
    ttfBuffer.Resize(size);
    if (!file.read(reinterpret_cast<char *>(ttfBuffer.GetData()), size))
    {
        TE_CORE_ERROR("FontAsset::BakeFromTTF - Failed to read TTF file: {0}", ttfPath);
        return false;
    }

    m_SourcePath = ttfPath;
    m_PixelSize = pixelSize;
    m_AtlasWidth = atlasWidth;
    m_AtlasHeight = atlasHeight;

    TEArray<uint8_t> rgbaBitmap;
    if (!AssetManager::BakeFontAtlas(ttfBuffer, pixelSize, atlasWidth, atlasHeight, m_Ascent, m_Descent, m_LineHeight,
                                     m_Glyphs, rgbaBitmap))
    {
        TE_CORE_ERROR("FontAsset::BakeFromTTF - BakeFontAtlas failed for: {0}", ttfPath);
        return false;
    }

    TEString atlasPath = ttfPath.GetParentPath() / (ttfPath.GetStem() + "_Atlas.png");
    m_AtlasTexturePath = atlasPath;

    AssetManager::ExportImagePNG(atlasPath, atlasWidth, atlasHeight, 4, rgbaBitmap.data());
    TE_CORE_INFO("FontAsset::BakeFromTTF - Generated atlas at {0}", m_AtlasTexturePath);

    // Create / reload atlas texture
    m_AtlasTexture = CreateRef<Texture>(m_AtlasTexturePath);
    return true;
}

TERef<Texture> FontAsset::GetAtlasTexture()
{
    if (!m_AtlasTexture && !m_AtlasTexturePath.IsEmpty())
    {
        m_AtlasTexture = CreateRef<Texture>(m_AtlasTexturePath);
    }
    return m_AtlasTexture;
}

bool FontAsset::GetGlyph(const TEString &character, FontGlyph &outGlyph) const
{
    if (character.IsEmpty())
        return false;

    auto it = m_Glyphs.find(character);
    if (it != m_Glyphs.end())
    {
        outGlyph = it->second;
        return true;
    }

    auto spaceIt = m_Glyphs.find(" ");
    if (spaceIt != m_Glyphs.end())
    {
        outGlyph = spaceIt->second;
        return true;
    }
    return false;
}

FontGlyph FontAsset::GetGlyph(const TEString &character) const
{
    FontGlyph g;
    GetGlyph(character, g);
    return g;
}

bool FontAsset::HasGlyph(const TEString &character) const { return m_Glyphs.find(character) != m_Glyphs.end(); }

float FontAsset::GetKerning(const TEString &first, const TEString &second) const
{
    if (first.IsEmpty() || second.IsEmpty())
        return 0.0f;

    TEString key = first + "_" + second;
    auto it = m_Kerning.find(key);
    return (it != m_Kerning.end()) ? it->second : 0.0f;
}

void FontAsset::SetKerning(const TEString &first, const TEString &second, float kern)
{
    if (!first.IsEmpty() && !second.IsEmpty())
    {
        TEString key = first + "_" + second;
        m_Kerning[key] = kern;
    }
}

TEVector2 FontAsset::MeasureString(const TEString &text, float scale) const
{
    float x = 0.0f;
    float maxWidth = 0.0f;
    float totalHeight = m_LineHeight * scale;

    for (size_t i = 0; i < text.Len(); ++i)
    {
        char c = text.c_str()[i];
        if (c == '\n')
        {
            maxWidth = (std::max)(maxWidth, x);
            x = 0.0f;
            totalHeight += m_LineHeight * scale;
            continue;
        }

        TEString charStr(&c, 1);
        FontGlyph g;
        if (GetGlyph(charStr, g))
        {
            x += g.AdvanceX * scale;
        }
    }
    maxWidth = (std::max)(maxWidth, x);
    return TEVector2(maxWidth, totalHeight);
}

bool FontAsset::LoadFromFile(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
    {
        TE_CORE_ERROR("FontAsset: Failed to open file for reading: {0}", path);
        return false;
    }

    m_Handle = AssetRegistry::RegisterPath(path);
    m_Glyphs.Clear();
    m_Kerning.clear();

    bool success =
        TEFileSystem::ForEachLine(path,
                                  [this](const TEString &line)
                                  {
                                      if (line.StartsWith("FontAsset: "))
                                      {
                                          m_Name = line.Mid(11).Trim();
                                      }
                                      else if (line.StartsWith("SourcePath: "))
                                      {
                                          m_SourcePath = line.Mid(12).Trim();
                                      }
                                      else if (line.StartsWith("AtlasTexturePath: "))
                                      {
                                          m_AtlasTexturePath = line.Mid(18).Trim();
                                      }
                                      else if (line.StartsWith("PixelSize: "))
                                      {
                                          m_PixelSize = std::stof(line.Mid(11).c_str());
                                      }
                                      else if (line.StartsWith("AtlasWidth: "))
                                      {
                                          m_AtlasWidth = static_cast<uint32_t>(std::stoul(line.Mid(12).c_str()));
                                      }
                                      else if (line.StartsWith("AtlasHeight: "))
                                      {
                                          m_AtlasHeight = static_cast<uint32_t>(std::stoul(line.Mid(13).c_str()));
                                      }
                                      else if (line.StartsWith("LineHeight: "))
                                      {
                                          m_LineHeight = std::stof(line.Mid(12).c_str());
                                      }
                                      else if (line.StartsWith("Ascent: "))
                                      {
                                          m_Ascent = std::stof(line.Mid(8).c_str());
                                      }
                                      else if (line.StartsWith("Descent: "))
                                      {
                                          m_Descent = std::stof(line.Mid(9).c_str());
                                      }
                                      else if (line.StartsWith("Glyph: "))
                                      {
                                          auto parts = line.Mid(7).Split("|");
                                          if (parts.size() >= 7)
                                          {
                                              FontGlyph g;
                                              g.Character = parts[0];
                                              g.AdvanceX = parts[1].ToFloat();
                                              g.BearingX = parts[2].ToFloat();
                                              g.BearingY = parts[3].ToFloat();
                                              g.Width = parts[4].ToFloat();
                                              g.Height = parts[5].ToFloat();

                                              auto uvParts = parts[6].Split(" ");
                                              if (uvParts.size() >= 4)
                                              {
                                                  g.UV.x = uvParts[0].ToFloat();
                                                  g.UV.y = uvParts[1].ToFloat();
                                                  g.UV.z = uvParts[2].ToFloat();
                                                  g.UV.w = uvParts[3].ToFloat();
                                              }

                                              m_Glyphs[g.Character] = g;
                                          }
                                      }
                                      else if (line.StartsWith("Kerning: "))
                                      {
                                          auto parts = line.Mid(9).Split("|");
                                          if (parts.size() >= 3)
                                          {
                                              TEString key = parts[0] + "_" + parts[1];
                                              m_Kerning[key] = parts[2].ToFloat();
                                          }
                                      }
                                      return true;
                                  });

    // Load texture atlas
    if (!m_AtlasTexturePath.IsEmpty())
    {
        m_AtlasTexture = CreateRef<Texture>(m_AtlasTexturePath);
    }

    return success;
}

bool FontAsset::SaveToFile(const TEString &path)
{
    std::ofstream hout(path.c_str());
    if (!hout.is_open())
    {
        TE_CORE_ERROR("FontAsset: Failed to open file for writing: {0}", path);
        return false;
    }

    hout << "FontAsset: " << m_Name.c_str() << "\n";
    hout << "SourcePath: " << m_SourcePath.c_str() << "\n";
    hout << "AtlasTexturePath: " << m_AtlasTexturePath.c_str() << "\n";
    hout << "PixelSize: " << m_PixelSize << "\n";
    hout << "AtlasWidth: " << m_AtlasWidth << "\n";
    hout << "AtlasHeight: " << m_AtlasHeight << "\n";
    hout << "LineHeight: " << m_LineHeight << "\n";
    hout << "Ascent: " << m_Ascent << "\n";
    hout << "Descent: " << m_Descent << "\n";
    hout << "GlyphCount: " << m_Glyphs.size() << "\n";

    for (const auto &[ch, g] : m_Glyphs)
    {
        hout << "Glyph: " << g.Character.c_str() << "|" << g.AdvanceX << "|" << g.BearingX << "|" << g.BearingY << "|"
             << g.Width << "|" << g.Height << "|" << g.UV.x << " " << g.UV.y << " " << g.UV.z << " " << g.UV.w << "\n";
    }

    hout << "KerningCount: " << m_Kerning.size() << "\n";
    for (const auto &[key, kern] : m_Kerning)
    {
        auto keyParts = key.Split("_");
        if (keyParts.size() >= 2)
        {
            hout << "Kerning: " << keyParts[0].c_str() << "|" << keyParts[1].c_str() << "|" << kern << "\n";
        }
    }

    hout.close();
    return true;
}

void FontAsset::OnContentBrowserCreate(const TEString &path)
{
    m_Name = path.GetStem();
    SaveToFile(path);
}

float FontAsset::MeasureStringWidth(const TEString &text, float scale) const { return MeasureString(text, scale).x; }

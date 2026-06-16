#include "Renderer/Texture.hpp"
#include "Renderer/TextureSerializer.hpp"
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "Core/Log.h"
#include <stb_image.h>
#include <stb_image_write.h>

#include "Core/Asset/AssetRegistry.hpp"
#include <filesystem>

namespace TE
{

Texture::Texture(const std::string &path) : m_FilePath(path), m_RendererID(0)
{
    m_Handle = AssetRegistry::RegisterPath(path);
    m_Name = std::filesystem::path(path).filename().string();
    int width, height, channels;
    stbi_set_flip_vertically_on_load(0);
    stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (data)
    {
        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if (channels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        else if (channels == 2)
        {
            internalFormat = GL_RG8;
            dataFormat = GL_RG;
        }
        else if (channels == 1)
        {
            internalFormat = GL_R8;
            dataFormat = GL_RED;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, internalFormat, width, height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTextureSubImage2D(m_RendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }
    else
    {
        TE_CORE_ERROR("Failed to load texture: {0}", path);
    }
}

Texture::~Texture() { glDeleteTextures(1, &m_RendererID); }

void Texture::Bind(uint32_t slot) const { glBindTextureUnit(slot, m_RendererID); }

void Texture::Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

void Texture::OnContentBrowserCreate(const std::filesystem::path &path)
{
    std::filesystem::create_directories(path);
    std::string baseName = "NewTexture";
    std::filesystem::path finalPath = path / (baseName + ".tetexture");
    int counter = 1;
    while (std::filesystem::exists(finalPath))
    {
        finalPath = path / (baseName + "_" + std::to_string(counter++) + ".tetexture");
    }

    std::string texName = finalPath.stem().string();
    std::filesystem::path pngPath = path / (texName + ".png");

    // Write a 1x1 white PNG
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    if (stbi_write_png(pngPath.string().c_str(), 1, 1, 4, whitePixel, 4) == 0)
    {
        TE_CORE_ERROR("Failed to write blank PNG for Texture at {0}", pngPath.string());
    }

    // Write the metadata file
    auto newTexture = std::make_shared<Texture>(pngPath.string());
    newTexture->m_Name = texName;
    TextureSerializer serializer(newTexture);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New Texture at {0}", finalPath.string());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create Texture metadata at {0}", finalPath.string());
    }
}

} // namespace TE

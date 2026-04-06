#include "Utility/ImageUtils.hpp"
#include <stb_image_write.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Core/Log.h"
#include <filesystem>
#include <stb_image_write.h>

namespace TE
{

bool ImageUtils::SavePNG(const std::string &path, int width, int height, int channels, const void *data)
{
    // Create directory if not exists
    std::filesystem::path p = path;
    if (p.has_parent_path())
    {
        std::filesystem::create_directories(p.parent_path());
    }

    int result = stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
    if (result == 0)
    {
        TE_CORE_ERROR("Failed to save PNG: {0}", path);
        return false;
    }
    TE_CORE_INFO("Successfully saved PNG to {0}", path);
    return true;
}

} // namespace TE

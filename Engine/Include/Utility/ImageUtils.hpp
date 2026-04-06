#pragma once
#include <string>
#include <vector>

namespace TE
{

class ImageUtils
{
public:
    static bool SavePNG(const std::string &path, int width, int height, int channels, const void *data);
};

} // namespace TE

#pragma once

#include <string>
#include <optional>

namespace TE {

    class PlatformUtils
    {
    public:
        // Returns empty string if cancelled
        static std::string OpenFolder(const char* initialPath = "");
        static std::string OpenFile(const char* filter);
        static std::string SaveFile(const char* filter);
    };

}

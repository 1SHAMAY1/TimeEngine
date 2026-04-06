#pragma once
#include "Core/KeyCodes.hpp"
#include <string>
#include <memory>

namespace TE {

    using AssetHandle = uint64_t;

    class Asset {
    public:
        virtual ~Asset() = default;

        virtual AssetHandle GetHandle() const = 0;
        virtual const std::string& GetType() const = 0;
        virtual const std::string& GetName() const = 0;
        virtual const std::string& GetHoverDescription() const = 0;
        
        virtual std::shared_ptr<class Texture> GetIcon() const = 0;
        virtual std::shared_ptr<class Texture> GetThumbnail() const = 0;
    };

} // namespace TE

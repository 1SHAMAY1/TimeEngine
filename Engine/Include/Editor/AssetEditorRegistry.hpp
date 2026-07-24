#pragma once
#include "Editor/AssetEditor.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace TE
{

class TE_API AssetEditorRegistry
{
public:
    static void Register(std::shared_ptr<AssetEditor> editor)
    {
        if (editor)
        {
            GetEditorsMap()[editor->GetAssetType()] = editor;
        }
    }

    static std::shared_ptr<AssetEditor> GetEditor(const std::string &assetType)
    {
        auto &map = GetEditorsMap();
        auto it = map.find(assetType);
        if (it != map.end())
            return it->second;
        return nullptr;
    }

private:
    static std::unordered_map<std::string, std::shared_ptr<AssetEditor>> &GetEditorsMap()
    {
        static std::unordered_map<std::string, std::shared_ptr<AssetEditor>> s_Editors;
        return s_Editors;
    }
};

template <typename T> struct AssetEditorAutoRegister
{
    AssetEditorAutoRegister() { AssetEditorRegistry::Register(std::make_shared<T>()); }
};

#define TE_REGISTER_ASSET_EDITOR(EditorClass)                                                                          \
    static ::TE::AssetEditorAutoRegister<EditorClass> s_AutoRegister_##EditorClass;

} // namespace TE

#pragma once
#include "Editor/AssetEditor.hpp"
#include "GameFrameWork/GameplayUtils.hpp"

class TE_API AssetEditorRegistry
{
public:
    static void Init();
    static void Register(TERef<AssetEditor> editor);
    static TERef<AssetEditor> GetEditor(const TEString &assetType);
    static TERef<AssetEditor> GetEditorForPath(const TEString &path);
    static TEArray<TERef<AssetEditor>> GetRegisteredEditors();
    static bool DrawAssetIcon(const TEString &assetType, const TEVector2 &min, const TEVector2 &max);
    static void OpenAsset(const TEString &assetPath);
    static void CloseAsset(const TEString &assetPath);
    static void MarkAssetDirty(const TEString &assetPath, bool dirty = true);
    static bool IsAssetDirty(const TEString &assetPath);
    static void OpenAssetPicker(const TEString &targetDirectory);
    static void OnTimeGUIRender();
    static void Clear();

private:
    static TEMap<TEString, TERef<AssetEditor>> &GetEditorsMap();
    static TEArray<EditorTab> &GetOpenTabs();
};

template <typename T> struct AssetEditorAutoRegister
{
    AssetEditorAutoRegister() { AssetEditorRegistry::Register(CreateRef<T>()); }
};

#define TE_REGISTER_ASSET_EDITOR(EditorClass) static AssetEditorAutoRegister<EditorClass> s_AutoRegister_##EditorClass;

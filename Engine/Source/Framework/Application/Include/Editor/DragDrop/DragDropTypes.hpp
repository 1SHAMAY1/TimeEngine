#pragma once

#include "PreRequisites.h"
#include "EngineTypes/TEString.hpp"
#include "EngineTypes/TEArray.hpp"
#include "MathUtils.hpp"
#include "GameplayUtils.hpp"

// Standard Core Payload Type Identifiers
#define TE_DND_ASSET_PATH "TE_DND_ASSET_PATH"
#define TE_DND_ASSET_HANDLE "TE_DND_ASSET_HANDLE"
#define TE_DND_ENTITY_ID "TE_DND_ENTITY_ID"
#define TE_DND_CONTENT_BROWSER_ITEMS "TE_DND_CONTENT_BROWSER_ITEMS"
#define TE_DND_ENTITY_LIST "TE_DND_ENTITY_LIST"
#define TE_DND_OS_FILES "TE_DND_OS_FILES"

// Standard Context Type Identifiers
namespace DragDropTargetContext
{
inline const TEString Viewport = "Viewport";
inline const TEString Inspector = "Inspector";
inline const TEString SceneHierarchy = "SceneHierarchy";
inline const TEString ContentBrowser = "ContentBrowser";
inline const TEString ContentBrowserFolder = "ContentBrowserFolder";
inline const TEString AssetEditor = "AssetEditor";
inline const TEString Universal = "*";
} // namespace DragDropTargetContext

class EditorLayer;
class Scene;

struct TE_API DragDropContext
{
    TEString TargetContextId;
    TEVector2 ScreenPosition = {0.0f, 0.0f};
    TEVector2 LocalTargetPosition = {0.0f, 0.0f};
    void *TargetUserData = nullptr;
    Ref<EditorLayer> Editor = nullptr;
    Ref<Scene> ActiveScene = nullptr;
};

struct TE_API DragDropPayload
{
    TEString PayloadType;
    const void *Data = nullptr;
    size_t DataSize = 0;
    bool IsPreview = false;
    bool IsDelivery = false;

    bool IsType(const TEString &type) const { return PayloadType == type; }

    TEString AsString() const
    {
        if (!Data || DataSize == 0)
            return "";
        const char *str = (const char *)Data;
        size_t len = DataSize;
        if (len > 0 && str[len - 1] == '\0')
            len--;
        return TEString(str, len);
    }

    TEArray<TEString> AsPathArray() const;
    uint64_t AsEntityID() const;
    TEArray<uint64_t> AsEntityIDArray() const;
};

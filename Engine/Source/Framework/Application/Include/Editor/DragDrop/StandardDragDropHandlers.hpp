#pragma once

#include "PreRequisites.h"
#include "Editor/DragDrop/IDragDropHandler.hpp"

// -----------------------------------------------------------------------------
// Scene Viewport Drop Handler: Handles .tescene drops onto Viewport
// -----------------------------------------------------------------------------
class TE_API SceneViewportDropHandler : public IDragDropHandler
{
public:
    TEString GetSupportedPayloadType() const override { return TE_DND_ASSET_PATH; }
    TEString GetSupportedTargetContext() const override { return DragDropTargetContext::Viewport; }
    TEString GetSupportedExtensions() const override { return ".tescene"; }
    int GetPriority() const override { return 10; }

    bool CanHandle(const DragDropContext &context, const DragDropPayload &payload) const override;
    TEString GetTooltipText(const DragDropContext &context, const DragDropPayload &payload) const override;
    bool OnDrop(const DragDropContext &context, const DragDropPayload &payload) override;
};

// -----------------------------------------------------------------------------
// Sprite 2D Viewport Drop Handler: Handles image files (.png, .jpg, .tga, etc.)
// -----------------------------------------------------------------------------
class TE_API Sprite2DViewportDropHandler : public IDragDropHandler
{
public:
    TEString GetSupportedPayloadType() const override { return TE_DND_ASSET_PATH; }
    TEString GetSupportedTargetContext() const override { return DragDropTargetContext::Viewport; }
    TEString GetSupportedExtensions() const override { return ".png,.jpg,.jpeg,.tga,.bmp"; }
    int GetPriority() const override { return 10; }

    bool CanHandle(const DragDropContext &context, const DragDropPayload &payload) const override;
    TEString GetTooltipText(const DragDropContext &context, const DragDropPayload &payload) const override;
    bool OnDrop(const DragDropContext &context, const DragDropPayload &payload) override;
};

// -----------------------------------------------------------------------------
// Hierarchy Reparent Drop Handler: Handles entity reparenting in Scene Hierarchy
// -----------------------------------------------------------------------------
class TE_API HierarchyReparentDropHandler : public IDragDropHandler
{
public:
    TEString GetSupportedPayloadType() const override { return TE_DND_ENTITY_ID; }
    TEString GetSupportedTargetContext() const override { return DragDropTargetContext::SceneHierarchy; }
    TEString GetSupportedExtensions() const override { return "*"; }
    int GetPriority() const override { return 10; }

    bool CanHandle(const DragDropContext &context, const DragDropPayload &payload) const override;
    TEString GetTooltipText(const DragDropContext &context, const DragDropPayload &payload) const override;
    bool OnDrop(const DragDropContext &context, const DragDropPayload &payload) override;
};

// -----------------------------------------------------------------------------
// OS File Import Drop Handler: Handles dropping external files into Content Browser
// -----------------------------------------------------------------------------
class TE_API OSFileImportDropHandler : public IDragDropHandler
{
public:
    TEString GetSupportedPayloadType() const override { return TE_DND_OS_FILES; }
    TEString GetSupportedTargetContext() const override { return DragDropTargetContext::ContentBrowser; }
    TEString GetSupportedExtensions() const override { return "*"; }
    int GetPriority() const override { return 10; }

    bool CanHandle(const DragDropContext &context, const DragDropPayload &payload) const override;
    TEString GetTooltipText(const DragDropContext &context, const DragDropPayload &payload) const override;
    bool OnDrop(const DragDropContext &context, const DragDropPayload &payload) override;
};

// -----------------------------------------------------------------------------
// Folder Asset Move Drop Handler: Handles moving dragged assets/items into folders
// -----------------------------------------------------------------------------
class TE_API FolderAssetMoveDropHandler : public IDragDropHandler
{
public:
    TEString GetSupportedPayloadType() const override { return TE_DND_ASSET_PATH; }
    TEString GetSupportedTargetContext() const override { return DragDropTargetContext::ContentBrowserFolder; }
    TEString GetSupportedExtensions() const override { return "*"; }
    int GetPriority() const override { return 10; }

    bool CanHandle(const DragDropContext &context, const DragDropPayload &payload) const override;
    TEString GetTooltipText(const DragDropContext &context, const DragDropPayload &payload) const override;
    bool OnDrop(const DragDropContext &context, const DragDropPayload &payload) override;
};

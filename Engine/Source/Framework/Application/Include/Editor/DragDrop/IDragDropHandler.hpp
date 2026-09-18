#pragma once

#include "PreRequisites.h"
#include "Editor/DragDrop/DragDropTypes.hpp"

class TE_API IDragDropHandler
{
public:
    virtual ~IDragDropHandler() = default;

    /// Primary payload type handled (e.g. TE_DND_ASSET_PATH, TE_DND_ENTITY_ID, or custom plugin type)
    virtual TEString GetSupportedPayloadType() const = 0;

    /// Target context identifier (e.g. "Viewport", "Inspector", "SceneHierarchy", "ContentBrowser", or "*")
    virtual TEString GetSupportedTargetContext() const = 0;

    /// Comma-delimited list of accepted extensions (e.g. ".png,.jpg", ".tescene") or "*" for all
    virtual TEString GetSupportedExtensions() const { return "*"; }

    /// Priority order: higher numbers execute first, allowing plugins to override core handlers cleanly
    virtual int GetPriority() const { return 0; }

    /// Checks if this handler can accept the given payload in this context
    virtual bool CanHandle(const DragDropContext &context, const DragDropPayload &payload) const = 0;

    /// Optional hover feedback (e.g. rendering a ghost outline or custom highlight)
    virtual void OnHover(const DragDropContext &context, const DragDropPayload &payload) {}

    /// Custom tooltip text displayed during drag over target
    virtual TEString GetTooltipText(const DragDropContext &context, const DragDropPayload &payload) const { return ""; }

    /// Executes the drop action. Returns true if handled successfully.
    virtual bool OnDrop(const DragDropContext &context, const DragDropPayload &payload) = 0;
};

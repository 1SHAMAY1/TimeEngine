#pragma once

#include "PreRequisites.h"
#include "Editor/DragDrop/DragDropTypes.hpp"
#include "Editor/DragDrop/DragDropRegistry.hpp"
#include "TimeGUI.hpp"

class Scene;
class EditorLayer;

class TE_API DragDropManager
{
public:
    // -------------------------------------------------------------------------
    // Drag Sources (Call immediately after rendering a draggable item)
    // -------------------------------------------------------------------------
    static bool BeginSource(const TEString &type, const void *data, size_t size, const TEString &tooltip = "",
                            int flags = 0);
    static bool BeginSourceString(const TEString &type, const TEString &str, const TEString &tooltip = "",
                                  int flags = 0);
    static bool BeginSourcePathArray(const TEArray<TEString> &paths, const TEString &tooltip = "", int flags = 0);
    static bool BeginSourceEntity(uint64_t entityId, const TEString &entityName = "", int flags = 0);
    static bool BeginSourceEntityArray(const TEArray<uint64_t> &entityIds, const TEString &tooltip = "",
                                       int flags = 0);

    // -------------------------------------------------------------------------
    // Target Execution (Call when hovering over a drop target zone/window)
    // -------------------------------------------------------------------------
    static bool ExecuteTarget(const TEString &targetContextId, void *userData = nullptr,
                              Ref<EditorLayer> editor = nullptr, const TEString &allowedPayloadType = "");

    // -------------------------------------------------------------------------
    // UI Feedback Helpers
    // -------------------------------------------------------------------------
    static void DrawDropTargetHighlight(const TEVector2 &min, const TEVector2 &max, bool isValid,
                                        float rounding = 4.0f);

    // -------------------------------------------------------------------------
    // Utility & Codecs
    // -------------------------------------------------------------------------
    static TEArray<uint8_t> EncodeStringArray(const TEArray<TEString> &strings);
    static TEArray<TEString> DecodeStringArray(const void *data, size_t size);
    static TEArray<uint8_t> EncodeEntityArray(const TEArray<uint64_t> &entities);
    static TEArray<uint64_t> DecodeEntityArray(const void *data, size_t size);

    static bool ValidateExtension(const TEString &path, const TEString &expectedExtensions);
    static bool IsDescendantOf(Ref<Scene> scene, uint64_t candidateChildId, uint64_t candidateParentId);
};

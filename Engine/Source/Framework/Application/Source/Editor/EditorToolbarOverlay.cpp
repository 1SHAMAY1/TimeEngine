#include "PreRequisites.h"
#include "EditorToolbarOverlay.hpp"

#include "Editor/ToolbarOverlays/SaveAllToolbarOverlay.hpp"
#include "Editor/ToolbarOverlays/EditorModeSelectorOverlay.hpp"
#include "Editor/ToolbarOverlays/SimulationToolbarOverlay.hpp"
#include "Editor/ToolbarOverlays/StandaloneToolbarOverlay.hpp"
#include "Editor/ToolbarOverlays/PackageGameToolbarOverlay.hpp"
#include "Editor/ToolbarOverlays/RestartEditorToolbarOverlay.hpp"

// ── Decentralized Toolbar Overlay Registry ─────────────────────────────────────
// All built-in toolbar button classes are registered here, ensuring cross-platform
// symbol linkage across MSVC, GCC, and Clang. Dynamic/plugin overlays can also
// register themselves via TE_REGISTER_TOOLBAR_OVERLAY(Type) or RegisterOverlay().

static TEArray<TERef<IEditorToolbarOverlay>> s_ToolbarOverlays;
static bool s_DefaultOverlaysInitialized = false;

static void EnsureDefaultOverlaysRegistered()
{
    if (s_DefaultOverlaysInitialized)
        return;

    s_DefaultOverlaysInitialized = true;

    EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<SaveAllToolbarOverlay>());
    EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<EditorModeSelectorOverlay>());
    EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<PlayToolbarOverlay>());
    EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<PauseToolbarOverlay>());
    EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<StopToolbarOverlay>());
    EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<StandaloneToolbarOverlay>());
    EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<PackageGameToolbarOverlay>());
    EditorToolbarOverlayRegistry::RegisterOverlay(CreateRef<RestartEditorToolbarOverlay>());
}

void EditorToolbarOverlayRegistry::RegisterOverlay(TERef<IEditorToolbarOverlay> overlay)
{
    if (overlay)
        s_ToolbarOverlays.Add(overlay);
}

TEArray<TERef<IEditorToolbarOverlay>> EditorToolbarOverlayRegistry::GetOverlays()
{
    EnsureDefaultOverlaysRegistered();
    return s_ToolbarOverlays;
}

void EditorToolbarOverlayRegistry::Clear()
{
    s_ToolbarOverlays.Clear();
    s_DefaultOverlaysInitialized = false;
}

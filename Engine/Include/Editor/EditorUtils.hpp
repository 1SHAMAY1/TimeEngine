#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TimeGUI.hpp"

class EditorLayer;
class IEditorPanel;
class Scene;

// ==========================================
// EditorUtils - Editor-only Function Library
// ==========================================
// Stateless utility class (TFunctionLibrary child) providing
// helpers used across panels, overlays, and EditorLayer.
// ==========================================

class TE_API EditorUtils : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "EditorUtils";

    // -- Panel helpers ------------------------------------------------

    /// Finds a panel by its ID string. Returns nullptr if not found.
    static TERef<IEditorPanel> FindPanel(Ref<EditorLayer> editor, const TEString &id);

    /// Toggles panel visibility by ID.
    static void TogglePanel(Ref<EditorLayer> editor, const TEString &id);

    /// Returns true if the panel with the given ID is currently visible.
    static bool IsPanelVisible(Ref<EditorLayer> editor, const TEString &id);

    // -- UI helpers ---------------------------------------------------

    /// Draws a bold separator label (category header style).
    static void DrawSectionHeader(const TEString &label);

    /// Draws a centered label inside the current window.
    static void DrawCenteredText(const TEString &label);

    /// Draws a tooltip icon "(?)" that reveals text on hover.
    static void DrawHelpMarker(const TEString &text);

    /// Draws a color-coded badge label (small, rounded, filled).
    static void DrawBadge(const TEString &label, float r, float g, float b, float a = 1.0f);

    /// Begins a collapsible card-style group with a header. Returns true if expanded.
    static bool BeginCard(const char *id, const char *header);

    /// Ends a card group opened with BeginCard.
    static void EndCard();

    /// Draws a procedural engine-style save/floppy icon into a draw list at the specified rectangle.
    static void DrawSaveIcon(const TEVector2 &min, const TEVector2 &max, bool isDirty = false,
                             unsigned int tintColor = 0xFFFFFFFF);

    // -- Path / Asset helpers -----------------------------------------

    /// Returns a human-readable size string (e.g. "1.2 MB").
    static TEString FormatFileSize(uintmax_t bytes);

    /// Returns the asset type label for a path extension (e.g. ".png" -> "Texture").
    static TEString GetAssetTypeLabel(const TEString &path);

    /// Returns true if the path points to a recognized engine asset type.
    static bool IsKnownAssetExtension(const TEString &path);

    /// Strips the project directory prefix to return a project-relative display path.
    static TEString MakeProjectRelative(const TEString &absPath, const TEString &projectDir);

    // -- Scene / State helpers ----------------------------------------

    /// Returns "Edit", "Play", or "Pause" string for the current scene state.
    static const char *SceneStateLabel(Ref<EditorLayer> editor);

    /// Saves the active scene to disk. Returns false if no path is set (needs Save-As).
    static bool QuickSaveScene(Ref<EditorLayer> editor);

    /// Returns true if the editor is currently in play or pause mode.
    static bool IsInPlayMode(Ref<EditorLayer> editor);

    /// Applies the professional AAA dark glass editor theme matching the showcase.
    static void SetEditorThemeColors();

    // -- Viewport Grid & Vector UI Drawing Helpers ---------------------

    /// Draws an infinite procedural 2D grid in world-space onto the Viewport's drawlist
    static void DrawInfinite2DGrid(TimeGUIDrawList &dl, const TEVector2 &viewportMin, const TEVector2 &viewportSize,
                                   const TEVector2 &cameraPos, float zoom, float primaryGridStep = 100.0f,
                                   float subGridDivisions = 10.0f);

    /// Draws world-space transform vector handles (Translate arrows, Rotate ring, Scale box)
    static void DrawTransformGizmo(TimeGUIDrawList &dl, const TEVector2 &screenPos, int gizmoType,
                                   bool isHovered = false, bool isDragging = false);

    /// Vector Icon Drawing Helpers for UI Toolbars and Overlays
    static void DrawHamburgerIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawPlayIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawPauseIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawStopIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawRestartIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawSelectIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawTranslateIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawRotateIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawScaleIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawAddIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawCrossIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawChevronLeftIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static void DrawChevronRightIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color);
    static bool DrawNavIconButton(const char *strID, bool isForward, bool enabled,
                                  const TEVector2 &btnSize = TEVector2(24.0f, 24.0f));

    struct FileBrowserConfig
    {
        TEString Title = "Browse Files";
        TEString ActionButtonText = "Select";
        TEString InitialDirectory = "";
        TEString DefaultFilename = "";
        TEString FilterExtension = "";
        bool AllowFilenameInput = true;
    };

    // -- In-Engine Visual File Browser Dialog ---------------------------
    static void OpenFileBrowser(const FileBrowserConfig &config,
                                std::function<void(const TEString &selectedPath)> onConfirm);
    static void OpenFileBrowser(const TEString &title, const TEString &actionButtonText,
                                const TEString &defaultFilename, const TEString &filterExtension,
                                bool allowFilenameInput, std::function<void(const TEString &selectedPath)> onConfirm);
    static void DrawFileBrowserModal();
    static bool IsFileBrowserOpen();

    // -- Managed Allocation Helpers for Editor Resources -----------------

    template <typename T, typename... Args> static Ref<T> CreateEditorResource(Args &&...args)
    {
        return CreateRef<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args> static Scope<T> CreateEditorScope(Args &&...args)
    {
        return CreateScope<T>(std::forward<Args>(args)...);
    }

    template <typename T> static void DestroyEditorResource(Ref<T> &ptr) { ptr.Reset(); }
};

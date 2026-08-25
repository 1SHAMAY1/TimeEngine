#pragma once

#include "Dialogs/SpriteExportLayer.hpp"
#include "Editor/EditorMode.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "SpriteEditorTypes.hpp"
#include "SpriteModeLibrary.hpp"
#include "SpriteScriptRuntime.hpp"
#include "Submodes/ISubmode.hpp"


class SpriteEditorLayer;

class SpriteMode : public EditorMode
{
public:
    SpriteMode();
    virtual ~SpriteMode();

    virtual bool ShouldHideStandardPanels() const override { return true; }
    virtual bool WantsFullscreenWorkspace() const override { return true; }

    TEScope<SpriteExportLayer> m_ExportLayer;

    virtual TEString GetName() const override { return "Sprite Mode"; }
    virtual TEString GetWorkspaceWindowName() const override { return "Sprite Studio"; }
    virtual TEString GetIcon() const override { return "S"; }

    virtual void OnEnter() override;
    virtual void OnUpdate(float dt) override;
    virtual void OnTimeGUIRender() override; // Renders decentralized submode UI
    virtual void OnExit() override;
    virtual bool OnShortcut(const TEString &shortcutId) override;

    // Undo / Redo System
    void SaveUndoState();
    void Undo();
    void Redo();

    // Export & Rendering
    void RefreshPreview();
    void PerformExport();
    void ExecuteProceduralCode(TimeGUI::TimeGUIDrawList dl, TEVector2 origin, TEVector2 cellSize, float dt);
    void RenderVectorShapes(TimeGUI::TimeGUIDrawList dl, TEVector2 origin, TEVector2 cellSize, float zoom = 1.0f,
                            TEVector2 pan = TEVector2(0, 0), int hoveredIdx = -1, int selectedIdx = -1);
    void RenderPixelGrid(TimeGUI::TimeGUIDrawList dl, TEVector2 origin, TEVector2 cellSize, float zoom = 1.0f,
                         TEVector2 pan = TEVector2(0, 0), int frameIndex = -1);

    void AddColorToHistory(TEVector4 color);

public:
    // Shared Mode State
    SpriteCreationMode m_CreationMode = SpriteCreationMode::PixelPaint;
    TEArray<TEScope<ISubmode>> m_Submodes;
    int m_ActiveSubmodeIndex = 0; // Default to PixelPaint

    // Vector Editor State
    TEArray<VectorElement> m_VectorElements;
    VectorShapeType m_ActiveVectorTool = VectorShapeType::Pen;
    TEVector4 m_ActiveFillColor = TEVector4(1.0f, 1.0f, 1.0f, 0.0f);
    TEVector4 m_ActiveStrokeColor = TEVector4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_ActiveStrokeThickness = 2.0f;
    float m_DefaultStrokeRounding = 0.0f;
    float m_DefaultFillRounding = 0.0f;
    bool m_DefaultSubtract = false;
    int m_SelectedElementIdx = -1;
    VectorElement m_CurrentDrawingElement;
    bool m_IsDrawing = false;
    int m_ActiveAnchorIdx = -1;
    bool m_IsDraggingAnchor = false;
    bool m_IsMovingShape = false;
    TEVector2 m_DragStartMousePos = TEVector2(0.0f, 0.0f);
    bool m_IsMarqueeSelecting = false;
    TEVector2 m_MarqueeStart = TEVector2(0, 0);
    TEVector2 m_MarqueeEnd = TEVector2(0, 0);

    // Procedural Code Editor State
    TEString m_ProcBuffer;
    TEScope<SpriteScriptRuntime> m_ScriptRuntime;
    bool m_ProcAnimPlaying = true;
    float m_ProcAnimTime = 0.0f;
    int m_ProcAnimFrame = 0;
    int m_ProcTotalFrames = 8;
    float m_ProcFPS = 12.0f;
    bool m_ProcAnimLoop = true;
    TEArray<CustomKeyword> m_Keywords;

    // Pixel Paint / Animation State (Piskel Studio)
    TEArray<PixelFrame> m_PixelFrames;
    int m_ActiveFrameIndex = 0;
    int m_ActiveLayerIndex = 0;
    int m_PixelGridWidth = 32;
    int m_PixelGridHeight = 32;
    TEVector4 m_PixelPaintColor = TEVector4(1.0f, 1.0f, 1.0f, 1.0f);
    TEArray<TEVector4> m_ColorHistory;

    // Canvas / Framebuffers
    TEVector2 m_CanvasPan = TEVector2(0.0f, 0.0f);
    float m_CanvasZoom = 1.0f;
    TERef<Framebuffer> m_PreviewFB = nullptr;
    TERef<Framebuffer> m_VectorCanvasFB = nullptr;
    TEVector2 m_LastSimSize = TEVector2(0, 0);

    // Export Options
    bool m_ShowExportPopup = false;
    bool m_ExportIsSheet = false;
    bool m_ExportTransparent = true;
    bool m_ExportRequested = false;
    bool m_ExportMatchIDE = true;
    bool m_PreviewDirty = true;
    TEString m_ExportPath = "Sandbox/SavedSprites/NewSprite.png";
    TEVector2 m_ExportSize = TEVector2(128, 128);
    TEVector2 m_ExportOffset = TEVector2(0.0f, 0.0f);
    int m_ExportFrames = 1, m_ExportCols = 1, m_ExportRows = 1;
    int m_LastVtxCount = 0, m_LastCmdCount = 0;

    // Undo / Redo Stacks
    TEArray<SpriteModeState> m_UndoStack;
    TEArray<SpriteModeState> m_RedoStack;
    bool m_IsUndoingRedoing = false;
};

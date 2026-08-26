#pragma once

#include "../Assets/SkeletalDataAsset.hpp"
#include "../Core/SkeletalPoseEvaluator.hpp"
#include "Core/PreRequisites.h"
#include "Editor/EditorMode.hpp"
#include "SkeletalEditorTypes.hpp"
#include "Utils/TimeGUI.hpp"

namespace Skeletal2D
{

class SkeletalAnimationEditorMode : public EditorMode
{
public:
    SkeletalAnimationEditorMode();
    virtual ~SkeletalAnimationEditorMode() override = default;

    virtual void OnEnter() override;
    virtual void OnUpdate(float dt) override;
    virtual void OnTimeGUIRender() override;
    virtual void OnExit() override;
    virtual bool OnShortcut(const TEString &shortcutId) override;

    virtual bool ShouldHideStandardPanels() const override { return true; }
    virtual bool WantsFullscreenWorkspace() const override { return true; }

    virtual TEString GetName() const override { return "Skeletal Rig"; }
    virtual TEString GetWorkspaceWindowName() const override { return "Skeletal Rig Studio"; }
    virtual TEString GetIcon() const override { return "Resources/Editor/SkeletalIcon.png"; }

    // Asset Management
    void SetActiveAsset(TERef<SkeletalDataAsset> asset);
    TERef<SkeletalDataAsset> GetActiveAsset() const { return m_ActiveAsset; }
    void NewRig();
    void SaveAsset();
    void RefreshTextureAssets();

    // Undo / Redo
    void SaveUndoState();
    void Undo();
    void Redo();

    // Bone / Attachment Operations
    void CreateBone(const TEString &name, int parentIndex, const TEVector2 &startPos, const TEVector2 &endPos);
    void DeleteBone(int boneIndex);
    void AttachTextureToSelectedBone(const TEString &texturePath);

private:
    // Drawing UI Panels
    void DrawTopHeader();
    void DrawToolSidebar();
    void DrawInteractiveCanvas();
    void DrawOutlinerAndInspector();
    void DrawTimelineDock();

    // Coordinate Conversion & Picking
    TEVector2 ScreenToCanvas(const TEVector2 &screenPos, const TEVector2 &canvasOrigin) const;
    TEVector2 CanvasToScreen(const TEVector2 &canvasPos, const TEVector2 &canvasOrigin) const;
    int PickBoneAtPosition(const TEVector2 &canvasPos, float tolerance = 14.0f) const;

private:
    TERef<SkeletalDataAsset> m_ActiveAsset = nullptr;
    SkeletalPoseEvaluator m_Evaluator;

    // Active Tool & Selection
    SkeletalEditorTool m_ActiveTool = SkeletalEditorTool::Select;
    int m_SelectedBoneIndex = -1;
    TEString m_SelectedSlotName;
    TEString m_SelectedAnimation = "idle";

    // Playback
    float m_CurrentTime = 0.0f;
    bool m_IsPlaying = false;
    float m_PlaybackSpeed = 1.0f;
    bool m_Loop = true;

    // Viewport Navigation
    TEVector2 m_CanvasPan = TEVector2(0.0f, 0.0f);
    float m_CanvasZoom = 1.0f;

    // Bone Creation Dragging State
    BoneCreationDrag m_BoneDrag;

    // Texture Asset Discovery (.tetexture)
    TEArray<TEString> m_CachedTexturePaths;
    int m_SelectedTextureComboIndex = 0;

    // Undo / Redo
    TEArray<SkeletalModeState> m_UndoStack;
    TEArray<SkeletalModeState> m_RedoStack;
    bool m_IsUndoingRedoing = false;
};

} // namespace Skeletal2D

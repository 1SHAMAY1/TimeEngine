#include "SkeletalAnimationEditorMode.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Input/Input.hpp"
#include "Input/ShortcutManager.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include <algorithm>
#include <cmath>

namespace Skeletal2D {

SkeletalAnimationEditorMode::SkeletalAnimationEditorMode()
{
}

void SkeletalAnimationEditorMode::OnEnter()
{
    TE_CORE_INFO("[SkeletalAnimationEditorMode] Entering Unified 2D Skeletal Studio Mode...");
    RefreshTextureAssets();

    if (!m_ActiveAsset)
    {
        NewRig();
    }
    else
    {
        SetActiveAsset(m_ActiveAsset);
    }
}

void SkeletalAnimationEditorMode::OnExit()
{
    TE_CORE_INFO("[SkeletalAnimationEditorMode] Exiting Skeletal Studio Mode...");
}

void SkeletalAnimationEditorMode::NewRig()
{
    m_ActiveAsset = CreateRef<SkeletalDataAsset>();
    m_ActiveAsset->SetName("NewSkeleton");

    // Seed default root & torso bones
    BoneTransform rootPose;
    rootPose.Position = {0.0f, -50.0f};
    rootPose.Rotation = 0.0f;
    rootPose.Scale = {1.0f, 1.0f};
    int rootIdx = m_ActiveAsset->GetHierarchy().AddBone("root", -1, rootPose, 50.0f);

    BoneTransform torsoPose;
    torsoPose.Position = {0.0f, 50.0f};
    torsoPose.Rotation = 0.0f;
    torsoPose.Scale = {1.0f, 1.0f};
    m_ActiveAsset->GetHierarchy().AddBone("torso", rootIdx, torsoPose, 60.0f);

    m_ActiveAsset->GetHierarchy().CalculateBindPoseMatrices();

    // Create default idle animation
    auto idleClip = CreateRef<AnimationClip>();
    idleClip->Name = "idle";
    idleClip->Duration = 1.0f;
    idleClip->IsLooping = true;
    m_ActiveAsset->AddAnimation("idle", idleClip);

    SetActiveAsset(m_ActiveAsset);
    SaveUndoState();
}

void SkeletalAnimationEditorMode::SetActiveAsset(TERef<SkeletalDataAsset> asset)
{
    m_ActiveAsset = asset;
    if (m_ActiveAsset)
    {
        m_Evaluator.SetHierarchy(m_ActiveAsset->GetHierarchy());
        m_Evaluator.SetSkinData(m_ActiveAsset->GetDefaultSkin());

        auto clip = m_ActiveAsset->GetAnimation(m_SelectedAnimation);
        if (clip)
        {
            m_Evaluator.SetAnimation(0, clip, m_Loop, 0.0f);
        }
    }
}

void SkeletalAnimationEditorMode::SaveAsset()
{
    if (m_ActiveAsset)
    {
        TEString savePath = Project::GetAssetDirectory() / "Animations" / (m_ActiveAsset->GetName() + ".teskeleton");
        m_ActiveAsset->SaveToFile(savePath);
        TE_CORE_INFO("[SkeletalAnimationEditorMode] Saved skeleton asset to: {0}", savePath);
    }
}

void SkeletalAnimationEditorMode::RefreshTextureAssets()
{
    m_CachedTexturePaths.Clear();
    TEString assetDir = Project::GetAssetDirectory();
    if (!assetDir.IsEmpty() && TEFileSystem::Exists(assetDir))
    {
        TEArray<TEString> files = TEFileSystem::GetFiles(assetDir, ".tetexture", true);
        for (const auto &file : files)
        {
            if (file.GetExtension() == ".tetexture")
            {
                m_CachedTexturePaths.Add(file);
            }
        }
    }
}

void SkeletalAnimationEditorMode::SaveUndoState()
{
    if (m_IsUndoingRedoing || !m_ActiveAsset)
        return;

    SkeletalModeState state;
    state.Hierarchy = m_ActiveAsset->GetHierarchy();
    state.ActiveSkin = m_ActiveAsset->GetDefaultSkin();
    state.Animations = m_ActiveAsset->GetAnimations();
    state.SelectedAnimation = m_SelectedAnimation;
    state.SelectedBoneIndex = m_SelectedBoneIndex;
    state.CurrentTime = m_CurrentTime;

    m_UndoStack.Add(state);
    if (m_UndoStack.Size() > 50)
    {
        m_UndoStack.erase(m_UndoStack.begin());
    }
    m_RedoStack.Clear();
}

void SkeletalAnimationEditorMode::Undo()
{
    if (m_UndoStack.Size() <= 1 || !m_ActiveAsset)
        return;

    m_IsUndoingRedoing = true;

    // Push current to redo
    SkeletalModeState currentState;
    currentState.Hierarchy = m_ActiveAsset->GetHierarchy();
    currentState.ActiveSkin = m_ActiveAsset->GetDefaultSkin();
    currentState.Animations = m_ActiveAsset->GetAnimations();
    currentState.SelectedAnimation = m_SelectedAnimation;
    currentState.SelectedBoneIndex = m_SelectedBoneIndex;
    currentState.CurrentTime = m_CurrentTime;
    m_RedoStack.Add(currentState);

    // Pop and apply top of undo
    m_UndoStack.pop_back();
    const SkeletalModeState &state = m_UndoStack.Last();
    m_ActiveAsset->SetHierarchy(state.Hierarchy);
    m_ActiveAsset->SetDefaultSkin(state.ActiveSkin);
    m_SelectedAnimation = state.SelectedAnimation;
    m_SelectedBoneIndex = state.SelectedBoneIndex;
    m_CurrentTime = state.CurrentTime;

    m_Evaluator.SetHierarchy(m_ActiveAsset->GetHierarchy());
    m_Evaluator.SetSkinData(m_ActiveAsset->GetDefaultSkin());

    m_IsUndoingRedoing = false;
}

void SkeletalAnimationEditorMode::Redo()
{
    if (m_RedoStack.IsEmpty() || !m_ActiveAsset)
        return;

    m_IsUndoingRedoing = true;

    SkeletalModeState nextState = m_RedoStack.Last();
    m_RedoStack.pop_back();

    m_UndoStack.Add(nextState);

    m_ActiveAsset->SetHierarchy(nextState.Hierarchy);
    m_ActiveAsset->SetDefaultSkin(nextState.ActiveSkin);
    m_SelectedAnimation = nextState.SelectedAnimation;
    m_SelectedBoneIndex = nextState.SelectedBoneIndex;
    m_CurrentTime = nextState.CurrentTime;

    m_Evaluator.SetHierarchy(m_ActiveAsset->GetHierarchy());
    m_Evaluator.SetSkinData(m_ActiveAsset->GetDefaultSkin());

    m_IsUndoingRedoing = false;
}

void SkeletalAnimationEditorMode::CreateBone(const TEString &name, int parentIndex, const TEVector2 &startPos,
                                            const TEVector2 &endPos)
{
    if (!m_ActiveAsset)
        return;

    float dx = endPos.x - startPos.x;
    float dy = endPos.y - startPos.y;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 10.0f)
        length = 40.0f;

    float rot = std::atan2(dy, dx);

    BoneTransform restPose;
    restPose.Position = {startPos.x, startPos.y};
    restPose.Rotation = rot;
    restPose.Scale = {1.0f, 1.0f};

    int newIdx = m_ActiveAsset->GetHierarchy().AddBone(name, parentIndex, restPose, length);
    m_ActiveAsset->GetHierarchy().CalculateBindPoseMatrices();
    m_Evaluator.SetHierarchy(m_ActiveAsset->GetHierarchy());

    m_SelectedBoneIndex = newIdx;
    SaveUndoState();
    TE_CORE_INFO("[SkeletalAnimationEditorMode] Created bone '{0}' (Index: {1}, Parent: {2})", name, newIdx, parentIndex);
}

void SkeletalAnimationEditorMode::DeleteBone(int boneIndex)
{
    if (!m_ActiveAsset || boneIndex < 0 || boneIndex >= (int)m_ActiveAsset->GetHierarchy().GetBoneCount())
        return;

    auto &bones = m_ActiveAsset->GetHierarchy().GetBones();
    bones.erase(bones.begin() + boneIndex);

    // Re-index parent references
    for (size_t i = 0; i < bones.size(); ++i)
    {
        bones[i].Index = (int)i;
        if (bones[i].ParentIndex == boneIndex)
        {
            bones[i].ParentIndex = -1;
        }
        else if (bones[i].ParentIndex > boneIndex)
        {
            bones[i].ParentIndex -= 1;
        }
    }

    m_ActiveAsset->GetHierarchy().CalculateBindPoseMatrices();
    m_Evaluator.SetHierarchy(m_ActiveAsset->GetHierarchy());
    m_SelectedBoneIndex = -1;
    SaveUndoState();
}

void SkeletalAnimationEditorMode::AttachTextureToSelectedBone(const TEString &texturePath)
{
    if (!m_ActiveAsset || m_SelectedBoneIndex < 0)
        return;

    BoneNode *bone = m_ActiveAsset->GetHierarchy().GetBone(m_SelectedBoneIndex);
    if (!bone)
        return;

    SkinData &skin = m_ActiveAsset->GetDefaultSkin();
    TEString slotName = bone->Name + "_slot";

    SkinSlot slot;
    slot.Index = m_SelectedBoneIndex;
    slot.Name = slotName;
    slot.BoneIndex = m_SelectedBoneIndex;
    slot.ActiveAttachmentName = bone->Name + "_tex";

    SkinAttachment attach;
    attach.Name = slot.ActiveAttachmentName;
    attach.Type = AttachmentType::Region;
    attach.Path = texturePath;
    attach.Size = {bone->Length, bone->Length};
    attach.Offset = {bone->Length * 0.5f, 0.0f};

    slot.Attachments[attach.Name.c_str()] = attach;
    skin.Slots[slotName.c_str()] = slot;

    m_Evaluator.SetSkinData(skin);
    SaveUndoState();
    TE_CORE_INFO("[SkeletalAnimationEditorMode] Attached texture '{0}' to bone '{1}'", texturePath, bone->Name);
}

bool SkeletalAnimationEditorMode::OnShortcut(const TEString &shortcutId)
{
    if (shortcutId == "Undo")
    {
        Undo();
        return true;
    }
    if (shortcutId == "Redo")
    {
        Redo();
        return true;
    }
    if (shortcutId == "Delete" && m_SelectedBoneIndex >= 0)
    {
        DeleteBone(m_SelectedBoneIndex);
        return true;
    }
    return false;
}

void SkeletalAnimationEditorMode::OnUpdate(float dt)
{
    if (m_IsPlaying && m_ActiveAsset)
    {
        m_CurrentTime += dt * m_PlaybackSpeed;
        auto clip = m_ActiveAsset->GetAnimation(m_SelectedAnimation);
        if (clip && clip->Duration > 0.0f)
        {
            if (m_Loop)
            {
                m_CurrentTime = std::fmod(m_CurrentTime, clip->Duration);
            }
            else if (m_CurrentTime > clip->Duration)
            {
                m_CurrentTime = clip->Duration;
                m_IsPlaying = false;
            }
        }
    }

    m_Evaluator.SetTimeScale(m_PlaybackSpeed);
    m_Evaluator.Update(dt, glm::mat4(1.0f));
}

void SkeletalAnimationEditorMode::OnTimeGUIRender()
{
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(8, 8));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 6.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(8, 8));

    TimeGUI::SetNextWindowDockID(TimeGUI::GetID("MyDockSpace"), TimeGUICond_FirstUseEver);
    TimeGUI::Begin(GetWorkspaceWindowName().c_str(), nullptr, TimeGUIWindowFlags_NoCollapse | TimeGUIWindowFlags_NoMove);

    DrawTopHeader();
    TimeGUI::Separator();

    float totalAvailHeight = TimeGUI::GetContentRegionAvail().y;
    float timelineHeight = 120.0f;
    float mainAreaHeight = std::max(200.0f, totalAvailHeight - timelineHeight - 12.0f);

    // 3-Column Studio Workspace: Left (Tools) | Center (Canvas) | Right (Inspector/Outliner)
    if (TimeGUI::BeginTable("##StudioLayout", 3, TimeGUITableFlags_Resizable | TimeGUITableFlags_BordersInnerV, TEVector2(0, mainAreaHeight)))
    {
        TimeGUI::TableSetupColumn("Tools", TimeGUITableColumnFlags_WidthFixed, 150.0f);
        TimeGUI::TableSetupColumn("Canvas", TimeGUITableColumnFlags_WidthStretch, 0.6f);
        TimeGUI::TableSetupColumn("Outliner & Inspector", TimeGUITableColumnFlags_WidthFixed, 280.0f);

        // 1. Tool Sidebar (Left)
        TimeGUI::TableNextColumn();
        DrawToolSidebar();

        // 2. Interactive Canvas Viewport (Center)
        TimeGUI::TableNextColumn();
        DrawInteractiveCanvas();

        // 3. Outliner & Properties Inspector (Right)
        TimeGUI::TableNextColumn();
        DrawOutlinerAndInspector();

        TimeGUI::EndTable();
    }

    TimeGUI::Separator();

    // Bottom Animation Timeline Dock
    DrawTimelineDock();

    TimeGUI::End();
    TimeGUI::PopStyleVar(3);
}

void SkeletalAnimationEditorMode::DrawTopHeader()
{
    TimeGUI::BeginChild("##TopHeader", TEVector2(0, 36), false, TimeGUIWindowFlags_NoScrollbar);

    TimeGUI::TextColored(TEVector4(0.0f, 0.85f, 0.85f, 1.0f), "SKELETAL 2D STUDIO");
    TimeGUI::SameLine();
    TimeGUI::Spacing();
    TimeGUI::SameLine();

    if (TimeGUI::Button("New Rig##TopNew", TEVector2(80, 26)))
    {
        NewRig();
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Rig##TopSave", TEVector2(80, 26)))
    {
        SaveAsset();
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Refresh Textures##TopTex", TEVector2(130, 26)))
    {
        RefreshTextureAssets();
    }

    TimeGUI::SameLine();
    TimeGUI::Spacing();
    TimeGUI::SameLine();

    if (TimeGUI::Button("Undo##TopUndo", TEVector2(70, 26)))
    {
        Undo();
    }
    if (TimeGUI::IsItemHovered())
    {
        TEString sc = ShortcutManager::GetShortcutFormatString("Editor_Undo");
        TimeGUI::SetTooltip(!sc.empty() ? ("Undo (" + sc + ")").c_str() : "Undo");
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Redo##TopRedo", TEVector2(70, 26)))
    {
        Redo();
    }
    if (TimeGUI::IsItemHovered())
    {
        TEString sc = ShortcutManager::GetShortcutFormatString("Editor_Redo");
        TimeGUI::SetTooltip(!sc.empty() ? ("Redo (" + sc + ")").c_str() : "Redo");
    }

    TimeGUI::EndChild();
}

void SkeletalAnimationEditorMode::DrawToolSidebar()
{
    TimeGUI::BeginChild("##ToolSidebar", TEVector2(0, 0), true);
    TimeGUI::TextColored(TEVector4(0.2f, 0.8f, 0.9f, 1.0f), "STUDIO TOOLS");
    TimeGUI::Separator();

    auto ToolButton = [&](const TEString &label, const TEString &tooltip, SkeletalEditorTool tool)
    {
        bool isActive = (m_ActiveTool == tool);
        if (isActive)
        {
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.0f, 0.65f, 0.65f, 1.0f));
        }
        if (TimeGUI::Button(label.c_str(), TEVector2(-1, 30)))
        {
            m_ActiveTool = tool;
        }
        if (isActive)
        {
            TimeGUI::PopStyleColor();
        }
        if (TimeGUI::IsItemHovered() && !tooltip.empty())
        {
            TimeGUI::SetTooltip(tooltip.c_str());
        }
    };

    ToolButton("Select", "Select bones / objects in viewport", SkeletalEditorTool::Select);
    ToolButton("Create Bone", "Click and drag on viewport to create parent-child bones", SkeletalEditorTool::CreateBone);
    ToolButton("Translate", "Translate active bone transform", SkeletalEditorTool::Translate);
    ToolButton("Rotate", "Rotate active bone", SkeletalEditorTool::Rotate);
    ToolButton("Scale", "Scale active bone length and transform", SkeletalEditorTool::Scale);
    ToolButton("Attach Texture", "Bind a .tetexture asset to the selected bone", SkeletalEditorTool::AttachTexture);
    ToolButton("Add Socket", "Attach an attachment socket to the selected bone", SkeletalEditorTool::AddSocket);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::TextColored(TEVector4(0.8f, 0.8f, 0.8f, 1.0f), "Quick Stats");
    if (m_ActiveAsset)
    {
        TimeGUI::Text("Bones: %d", (int)m_ActiveAsset->GetHierarchy().GetBoneCount());
        TimeGUI::Text("Animations: %d", (int)m_ActiveAsset->GetAnimations().size());
        TimeGUI::Text("Selected: %s", (m_SelectedBoneIndex >= 0 && m_ActiveAsset->GetHierarchy().GetBone(m_SelectedBoneIndex))
                                          ? m_ActiveAsset->GetHierarchy().GetBone(m_SelectedBoneIndex)->Name.c_str()
                                          : "None");
    }

    TimeGUI::EndChild();
}

TEVector2 SkeletalAnimationEditorMode::ScreenToCanvas(const TEVector2 &screenPos, const TEVector2 &canvasOrigin) const
{
    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();
    TEVector2 center = canvasOrigin + canvasSize * 0.5f;
    return (screenPos - center) / m_CanvasZoom - m_CanvasPan;
}

TEVector2 SkeletalAnimationEditorMode::CanvasToScreen(const TEVector2 &canvasPos, const TEVector2 &canvasOrigin) const
{
    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();
    TEVector2 center = canvasOrigin + canvasSize * 0.5f;
    return center + (canvasPos + m_CanvasPan) * m_CanvasZoom;
}

int SkeletalAnimationEditorMode::PickBoneAtPosition(const TEVector2 &canvasPos, float tolerance) const
{
    if (!m_ActiveAsset)
        return -1;

    const auto &bones = m_ActiveAsset->GetHierarchy().GetBones();
    for (size_t i = 0; i < bones.size(); ++i)
    {
        const auto &bone = bones[i];
        TEVector2 bPos(bone.WorldMatrix[3][0], bone.WorldMatrix[3][1]);
        float dist = std::sqrt((canvasPos.x - bPos.x) * (canvasPos.x - bPos.x) + (canvasPos.y - bPos.y) * (canvasPos.y - bPos.y));
        if (dist <= tolerance)
        {
            return (int)i;
        }
    }
    return -1;
}

void SkeletalAnimationEditorMode::DrawInteractiveCanvas()
{
    TimeGUI::BeginChild("##InteractiveCanvasViewport", TEVector2(0, 0), true,
                        TimeGUIWindowFlags_NoScrollbar | TimeGUIWindowFlags_NoScrollWithMouse);

    TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();
    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();

    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    // Dark Studio Background
    dl.AddRectFilled(canvasPos, canvasPos + canvasSize, 0xFF14181B);

    // Subtle Grid
    float gridSize = 32.0f * m_CanvasZoom;
    if (gridSize > 8.0f)
    {
        TEVector2 offset = CanvasToScreen(TEVector2(0, 0), canvasPos);
        float startX = std::fmod(offset.x, gridSize);
        float startY = std::fmod(offset.y, gridSize);

        for (float x = startX; x < canvasSize.x; x += gridSize)
        {
            dl.AddLine(TEVector2(canvasPos.x + x, canvasPos.y), TEVector2(canvasPos.x + x, canvasPos.y + canvasSize.y),
                       0x22FFFFFF, 1.0f);
        }
        for (float y = startY; y < canvasSize.y; y += gridSize)
        {
            dl.AddLine(TEVector2(canvasPos.x, canvasPos.y + y), TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + y),
                       0x22FFFFFF, 1.0f);
        }
    }

    // Coordinate Origin Crosshair
    TEVector2 originScreen = CanvasToScreen(TEVector2(0, 0), canvasPos);
    dl.AddLine(TEVector2(originScreen.x - 20.0f, originScreen.y), TEVector2(originScreen.x + 20.0f, originScreen.y), 0x8800CEC9, 1.5f);
    dl.AddLine(TEVector2(originScreen.x, originScreen.y - 20.0f), TEVector2(originScreen.x, originScreen.y + 20.0f), 0x8800CEC9, 1.5f);

    // Handle Pan & Zoom
    bool hovered = TimeGUI::IsWindowHovered();
    if (hovered)
    {
        float wheel = TimeGUI::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            m_CanvasZoom = std::clamp(m_CanvasZoom + wheel * 0.1f, 0.1f, 10.0f);
        }
        if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Right) || TimeGUI::IsMouseDragging(TimeGUIMouseButton_Middle))
        {
            m_CanvasPan.x += TimeGUI::GetIO().MouseDelta.x / m_CanvasZoom;
            m_CanvasPan.y += TimeGUI::GetIO().MouseDelta.y / m_CanvasZoom;
        }
    }

    // Render Bones
    if (m_ActiveAsset)
    {
        const auto &bones = m_ActiveAsset->GetHierarchy().GetBones();
        for (size_t i = 0; i < bones.size(); ++i)
        {
            const auto &bone = bones[i];
            bool isSelected = ((int)i == m_SelectedBoneIndex);

            // World start position
            TEVector2 bStart(bone.WorldMatrix[3][0], bone.WorldMatrix[3][1]);
            TEVector2 screenStart = CanvasToScreen(bStart, canvasPos);

            // Calculate tip vector from matrix rotation and length
            float rad = std::atan2(bone.WorldMatrix[0][1], bone.WorldMatrix[0][0]);
            float cosR = std::cos(rad);
            float sinR = std::sin(rad);

            TEVector2 bTip = bStart + TEVector2(cosR * bone.Length, sinR * bone.Length);
            TEVector2 screenTip = CanvasToScreen(bTip, canvasPos);

            // Octahedral bone cone side wings
            float wingOffset = bone.Length * 0.25f;
            float wingWidth = 8.0f;
            TEVector2 bMid = bStart + TEVector2(cosR * wingOffset, sinR * wingOffset);
            TEVector2 normal(-sinR * wingWidth, cosR * wingWidth);

            TEVector2 sWing1 = CanvasToScreen(bMid + normal, canvasPos);
            TEVector2 sWing2 = CanvasToScreen(bMid - normal, canvasPos);

            TEVector2 boneQuad[4] = {screenStart, sWing1, screenTip, sWing2};

            unsigned int fillCol = isSelected ? 0x9900CEC9 : 0x66008985;
            unsigned int outlineCol = isSelected ? 0xFFF1C40F : 0xFF81ECEC;

            dl.AddConvexPolyFilled(boneQuad, 4, fillCol);
            dl.AddPolyline(boneQuad, 4, outlineCol, 1, isSelected ? 2.0f : 1.2f);

            // Joint Hub Circles
            dl.AddCircleFilled(screenStart, isSelected ? 5.5f : 4.0f, isSelected ? 0xFFF1C40F : 0xFF00CEC9);
            dl.AddCircle(screenStart, isSelected ? 5.5f : 4.0f, 0xFFFFFFFF, 12, 1.2f);

            dl.AddCircleFilled(screenTip, 3.5f, 0xFF55EFC4);
            dl.AddCircle(screenTip, 3.5f, 0xFFFFFFFF, 12, 1.0f);
        }
    }

    // Bone Creation Interaction
    TEVector2 mousePos = TimeGUI::GetMousePos();
    TEVector2 mouseCanvasPos = ScreenToCanvas(mousePos, canvasPos);

    if (hovered && m_ActiveTool == SkeletalEditorTool::CreateBone)
    {
        if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
        {
            m_BoneDrag.IsDragging = true;
            m_BoneDrag.StartPos = mouseCanvasPos;
            m_BoneDrag.CurrentPos = mouseCanvasPos;
            m_BoneDrag.ParentCandidateIndex = PickBoneAtPosition(mouseCanvasPos, 20.0f);
        }
        else if (m_BoneDrag.IsDragging && TimeGUI::IsMouseDragging(TimeGUIMouseButton_Left))
        {
            m_BoneDrag.CurrentPos = mouseCanvasPos;
        }
        else if (m_BoneDrag.IsDragging && TimeGUI::IsMouseReleased(TimeGUIMouseButton_Left))
        {
            m_BoneDrag.IsDragging = false;
            int count = (int)(m_ActiveAsset ? m_ActiveAsset->GetHierarchy().GetBoneCount() : 0);
            TEString newBoneName = "bone_" + TEString::FromInt(count);
            CreateBone(newBoneName, m_BoneDrag.ParentCandidateIndex, m_BoneDrag.StartPos, m_BoneDrag.CurrentPos);
        }
    }
    else if (hovered && m_ActiveTool == SkeletalEditorTool::Select && TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
    {
        m_SelectedBoneIndex = PickBoneAtPosition(mouseCanvasPos, 20.0f);
    }

    // Draw Bone Creation Drag Preview
    if (m_BoneDrag.IsDragging)
    {
        TEVector2 screenDragStart = CanvasToScreen(m_BoneDrag.StartPos, canvasPos);
        TEVector2 screenDragEnd = CanvasToScreen(m_BoneDrag.CurrentPos, canvasPos);

        dl.AddLine(screenDragStart, screenDragEnd, 0xFFF1C40F, 2.0f);
        dl.AddCircleFilled(screenDragStart, 5.0f, 0xFF00CEC9);
        dl.AddCircleFilled(screenDragEnd, 4.0f, 0xFFE74C3C);
    }

    TimeGUI::EndChild();
}

void SkeletalAnimationEditorMode::DrawOutlinerAndInspector()
{
    TimeGUI::BeginChild("##OutlinerAndInspector", TEVector2(0, 0), true);

    // 1. Hierarchy Outliner
    TimeGUI::TextColored(TEVector4(0.2f, 0.8f, 0.9f, 1.0f), "BONE HIERARCHY");
    TimeGUI::Separator();

    if (m_ActiveAsset)
    {
        auto &bones = m_ActiveAsset->GetHierarchy().GetBones();
        for (size_t i = 0; i < bones.size(); ++i)
        {
            auto &bone = bones[i];
            bool isSelected = ((int)i == m_SelectedBoneIndex);

            TEString treeLabel = bone.Name + " (Parent: " + (bone.ParentIndex >= 0 ? TEString::FromInt(bone.ParentIndex) : "None") + ")";
            if (TimeGUI::Selectable(treeLabel.c_str(), isSelected))
            {
                m_SelectedBoneIndex = (int)i;
            }
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // 2. Bone Property Inspector
    TimeGUI::TextColored(TEVector4(0.2f, 0.8f, 0.9f, 1.0f), "BONE PROPERTIES");
    TimeGUI::Separator();

    if (m_ActiveAsset && m_SelectedBoneIndex >= 0 && m_SelectedBoneIndex < (int)m_ActiveAsset->GetHierarchy().GetBoneCount())
    {
        BoneNode *bone = m_ActiveAsset->GetHierarchy().GetBone(m_SelectedBoneIndex);
        if (bone)
        {
            TimeGUI::Text("Index: %d", bone->Index);

            TEString nameInput = bone->Name;
            if (TimeGUI::InputText("Name##BoneName", nameInput))
            {
                bone->Name = nameInput;
                SaveUndoState();
            }

            if (TimeGUI::DragFloat("Length##BoneLen", &bone->Length, 1.0f, 5.0f, 500.0f))
            {
                m_ActiveAsset->GetHierarchy().CalculateBindPoseMatrices();
                m_Evaluator.SetHierarchy(m_ActiveAsset->GetHierarchy());
                SaveUndoState();
            }

            float pos[2] = {bone->RestPose.Position.x, bone->RestPose.Position.y};
            if (TimeGUI::DragFloat2("Rest Pos##BonePos", pos, 1.0f))
            {
                bone->RestPose.Position = {pos[0], pos[1]};
                bone->LocalPose.Position = bone->RestPose.Position;
                m_ActiveAsset->GetHierarchy().CalculateBindPoseMatrices();
                m_Evaluator.SetHierarchy(m_ActiveAsset->GetHierarchy());
                SaveUndoState();
            }

            float rotDeg = glm::degrees(bone->RestPose.Rotation);
            if (TimeGUI::DragFloat("Rest Rot##BoneRot", &rotDeg, 1.0f, -360.0f, 360.0f, "%.1f deg"))
            {
                bone->RestPose.Rotation = glm::radians(rotDeg);
                bone->LocalPose.Rotation = bone->RestPose.Rotation;
                m_ActiveAsset->GetHierarchy().CalculateBindPoseMatrices();
                m_Evaluator.SetHierarchy(m_ActiveAsset->GetHierarchy());
                SaveUndoState();
            }

            TimeGUI::Spacing();
            TimeGUI::Separator();

            // 3. Scrollable .tetexture Attachment Selector
            TimeGUI::TextColored(TEVector4(0.2f, 0.8f, 0.9f, 1.0f), ".TETEXTURE ATTACHMENT");

            if (m_CachedTexturePaths.IsEmpty())
            {
                TimeGUI::TextDisabled("No .tetexture assets found in Assets/");
                if (TimeGUI::Button("Scan Project Textures##ScanTex"))
                {
                    RefreshTextureAssets();
                }
            }
            else
            {
                TEString previewText = (m_SelectedTextureComboIndex >= 0 && m_SelectedTextureComboIndex < (int)m_CachedTexturePaths.Size())
                                           ? m_CachedTexturePaths[m_SelectedTextureComboIndex].GetFilename()
                                           : "Select .tetexture...";

                if (TimeGUI::BeginCombo(".tetexture Asset##TexPicker", previewText.c_str()))
                {
                    for (int i = 0; i < (int)m_CachedTexturePaths.Size(); ++i)
                    {
                        bool isSelected = (m_SelectedTextureComboIndex == i);
                        TEString itemText = m_CachedTexturePaths[i].GetFilename();
                        if (TimeGUI::Selectable(itemText.c_str(), isSelected))
                        {
                            m_SelectedTextureComboIndex = i;
                        }
                    }
                    TimeGUI::EndCombo();
                }

                if (TimeGUI::Button("Attach to Selected Bone##AttachBtn", TEVector2(-1, 26)))
                {
                    if (m_SelectedTextureComboIndex >= 0 && m_SelectedTextureComboIndex < (int)m_CachedTexturePaths.Size())
                    {
                        AttachTextureToSelectedBone(m_CachedTexturePaths[m_SelectedTextureComboIndex]);
                    }
                }
            }

            TimeGUI::Spacing();
            if (TimeGUI::Button("Delete Bone##DelBone", TEVector2(-1, 26)))
            {
                DeleteBone(m_SelectedBoneIndex);
            }
        }
    }
    else
    {
        TimeGUI::TextDisabled("Select a bone to edit properties.");
    }

    TimeGUI::EndChild();
}

void SkeletalAnimationEditorMode::DrawTimelineDock()
{
    TimeGUI::BeginChild("##TimelineDock", TEVector2(0, 110), true);

    TimeGUI::TextColored(TEVector4(0.2f, 0.8f, 0.9f, 1.0f), "ANIMATION TIMELINE");
    TimeGUI::SameLine();

    // Playback Controls
    if (TimeGUI::Button(m_IsPlaying ? "Pause##PlayPause" : "Play##PlayPause", TEVector2(60, 24)))
    {
        m_IsPlaying = !m_IsPlaying;
        if (m_IsPlaying && m_ActiveAsset)
        {
            auto clip = m_ActiveAsset->GetAnimation(m_SelectedAnimation);
            if (clip)
            {
                m_Evaluator.SetAnimation(0, clip, m_Loop, 0.2f);
            }
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Stop##TimeStop", TEVector2(60, 24)))
    {
        m_IsPlaying = false;
        m_CurrentTime = 0.0f;
        m_Evaluator.ClearTracks();
    }

    TimeGUI::SameLine();
    TimeGUI::Checkbox("Loop##TimeLoop", &m_Loop);

    TimeGUI::SameLine();
    TimeGUI::SetNextItemWidth(100);
    TimeGUI::SliderFloat("Speed##TimeSpeed", &m_PlaybackSpeed, 0.1f, 3.0f, "%.1fx");

    TimeGUI::SameLine();
    TimeGUI::SetNextItemWidth(140);
    if (m_ActiveAsset)
    {
        const auto &anims = m_ActiveAsset->GetAnimations();
        if (TimeGUI::BeginCombo("Clip##TimeClip", m_SelectedAnimation.c_str()))
        {
            for (const auto &pair : anims)
            {
                bool isSelected = (m_SelectedAnimation == pair.first.c_str());
                if (TimeGUI::Selectable(pair.first.c_str(), isSelected))
                {
                    m_SelectedAnimation = pair.first.c_str();
                    if (m_IsPlaying)
                    {
                        m_Evaluator.SetAnimation(0, pair.second, m_Loop, 0.2f);
                    }
                }
            }
            TimeGUI::EndCombo();
        }
    }

    // Time Scrubber
    float maxDuration = 1.0f;
    if (m_ActiveAsset)
    {
        auto clip = m_ActiveAsset->GetAnimation(m_SelectedAnimation);
        if (clip)
            maxDuration = clip->Duration;
    }

    TimeGUI::SetNextItemWidth(-1);
    if (TimeGUI::SliderFloat("##TimeScrubber", &m_CurrentTime, 0.0f, maxDuration, "Time: %.2fs"))
    {
        if (m_ActiveAsset)
        {
            auto clip = m_ActiveAsset->GetAnimation(m_SelectedAnimation);
            if (clip)
            {
                m_Evaluator.SetAnimation(0, clip, false, 0.0f);
            }
        }
    }

    TimeGUI::EndChild();
}

} // namespace Skeletal2D

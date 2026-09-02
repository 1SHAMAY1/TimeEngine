#include "Core/PreRequisites.h"
#include "Editor/Panels/ViewportPanel.hpp"
#include "Core/Log.h"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorUtils.hpp"
#include "Editor/ViewportContextMenu.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Layers/EditorLayer.hpp"
#include "Layers/ProfilingLayer.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Utils/TimeGUI.hpp"

void ViewportPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    if (!editor || !m_Visible)
        return;

    TEString sceneTitle = "Viewport";
    auto activeScene = editor->GetActiveScene();
    if (activeScene && !activeScene->GetName().empty())
    {
        sceneTitle = activeScene->GetName();
        if (sceneTitle.EndsWith(".tescene"))
            sceneTitle = sceneTitle.Left(sceneTitle.Length() - 8);
    }
    TEString windowTitle = sceneTitle + "###Viewport";

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(0.0f, 0.0f));
    TimeGUI::Begin(windowTitle.c_str(), nullptr,
                   TimeGUIWindowFlags_NoScrollbar | TimeGUIWindowFlags_NoScrollWithMouse | TimeGUIWindowFlags_NoMove);
    TimeGUI::PopStyleVar();

    editor->SetViewportFocused(TimeGUI::IsWindowFocused());
    editor->SetViewportHovered(TimeGUI::IsWindowHovered());

    TEVector2 viewportPanelSize = TimeGUI::GetContentRegionAvail();
    editor->SetViewportPos(TimeGUI::GetCursorScreenPos());

    auto fb = editor->GetFramebuffer();
    if (fb)
    {
        const FramebufferSpecification &spec = fb->GetSpecification();
        if (viewportPanelSize.x > 0.0f && viewportPanelSize.y > 0.0f &&
            (spec.Width != (uint32_t)viewportPanelSize.x || spec.Height != (uint32_t)viewportPanelSize.y))
        {
            fb->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
            auto lightFb = editor->GetLightMapFramebuffer();
            if (lightFb)
                lightFb->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
        }

        uintptr_t textureID = fb->GetColorAttachmentRendererID();
        TimeGUI::Image((TimeGUI::TimeGUITextureID)textureID, TEVector2(viewportPanelSize.x, viewportPanelSize.y),
                       TEVector2(0, 1), TEVector2(1, 0));
    }

    // Viewport Screen Bounds & Infinite 2D Grid
    TEVector2 vpMin = TimeGUI::GetItemRectMin();
    TEVector2 vpSize = TimeGUI::GetItemRectSize();
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    editor->SetViewportPos(vpMin);

    TEVector camPos = editor->GetCameraPosition();
    float camZoom = editor->GetCameraZoom();
    EditorUtils::DrawInfinite2DGrid(dl, vpMin, vpSize, TEVector2(camPos.x, camPos.y), camZoom);

    bool isGizmoHovered = false;

    // ── Interactive Transform Gizmo for Selected Entity ───────────────────────
    if (activeScene && editor->GetGizmoType() != EditorLayer::GizmoType::None)
    {
        const auto &selected = editor->GetSelectedEntities();
        if (!selected.empty())
        {
            Entity primaryEntity = *selected.begin();
            auto &em = activeScene->GetEntityManager();
            auto *tc = em.GetComponent<TransformComponent>(primaryEntity);
            if (tc)
            {
                TEMatrix4 worldMat = GameplayUtils::ResolveWorldTransform(em, primaryEntity, nullptr);
                TEVector2 worldPos(worldMat[3][0], worldMat[3][1]);
                TEVector2 screenPos = vpMin + GameplayUtils::WorldToViewportPixel(
                                                  worldPos, vpSize, TEVector2(camPos.x, camPos.y), camZoom);

                int gizmoMode = 0;
                if (editor->GetGizmoType() == EditorLayer::GizmoType::Translate)
                    gizmoMode = 1;
                else if (editor->GetGizmoType() == EditorLayer::GizmoType::Rotate)
                    gizmoMode = 2;
                else if (editor->GetGizmoType() == EditorLayer::GizmoType::Scale)
                    gizmoMode = 3;

                if (gizmoMode > 0)
                {
                    static int s_ActiveGizmoAxis = -1; // 0 = Center, 1 = X, 2 = Y
                    static bool s_IsGizmoDragging = false;
                    static TEVector2 s_DragStartMouse = {0, 0};
                    static TEVector s_DragStartPos = {0, 0, 0};
                    static float s_DragStartRot = 0.0f;
                    static TEVector s_DragStartScale = {1, 1, 1};

                    TEVector2 mousePos = TimeGUI::GetMousePos();
                    float armLen = 48.0f;
                    float hitRadius = 14.0f;

                    bool hoveredCenter = (mousePos - screenPos).Length() <= 10.0f;
                    bool hoveredX = (mousePos.x >= screenPos.x && mousePos.x <= screenPos.x + armLen + 10.0f &&
                                     std::abs(mousePos.y - screenPos.y) <= hitRadius);
                    bool hoveredY = (mousePos.y <= screenPos.y && mousePos.y >= screenPos.y - armLen - 10.0f &&
                                     std::abs(mousePos.x - screenPos.x) <= hitRadius);
                    isGizmoHovered = hoveredCenter || hoveredX || hoveredY;

                    editor->SetGizmoDragging(s_IsGizmoDragging || isGizmoHovered);

                    if (editor->IsViewportHovered() && TimeGUI::IsMouseClicked(0) && isGizmoHovered &&
                        !TimeGUI::IsAnyItemHovered())
                    {
                        s_IsGizmoDragging = true;
                        s_DragStartMouse = mousePos;
                        s_DragStartPos = tc->Transform.Position;
                        s_DragStartRot = tc->Transform.Rotation.Yaw;
                        s_DragStartScale = tc->Transform.Scale.Scale;
                        if (hoveredCenter)
                            s_ActiveGizmoAxis = 0;
                        else if (hoveredX)
                            s_ActiveGizmoAxis = 1;
                        else if (hoveredY)
                            s_ActiveGizmoAxis = 2;
                    }

                    if (s_IsGizmoDragging)
                    {
                        if (TimeGUI::IsMouseDown(0))
                        {
                            float aspect = (vpSize.y > 0) ? vpSize.x / vpSize.y : 1.0f;
                            float worldPerPixelX = (aspect * camZoom * 2.0f) / (vpSize.x > 0 ? vpSize.x : 1.0f);
                            float worldPerPixelY = (camZoom * 2.0f) / (vpSize.y > 0 ? vpSize.y : 1.0f);

                            TEVector2 mouseDelta = mousePos - s_DragStartMouse;
                            float worldDeltaX = mouseDelta.x * worldPerPixelX;
                            float worldDeltaY = -mouseDelta.y * worldPerPixelY;

                            float localDeltaX = worldDeltaX;
                            float localDeltaY = worldDeltaY;

                            if (tc->Parent != 0 && em.IsValid(tc->Parent))
                            {
                                TEMatrix4 pMat =
                                    GameplayUtils::ResolveWorldTransform(em, Entity(tc->Parent, &em), nullptr);
                                float pRot = std::atan2(pMat[0][1], pMat[0][0]);
                                float pScaleX = std::sqrt(pMat[0][0] * pMat[0][0] + pMat[0][1] * pMat[0][1]);
                                float pScaleY = std::sqrt(pMat[1][0] * pMat[1][0] + pMat[1][1] * pMat[1][1]);
                                if (pScaleX < 0.0001f)
                                    pScaleX = 1.0f;
                                if (pScaleY < 0.0001f)
                                    pScaleY = 1.0f;

                                float cosA = std::cos(-pRot);
                                float sinA = std::sin(-pRot);
                                localDeltaX = (worldDeltaX * cosA - worldDeltaY * sinA) / pScaleX;
                                localDeltaY = (worldDeltaX * sinA + worldDeltaY * cosA) / pScaleY;
                            }

                            if (gizmoMode == 1) // Translate
                            {
                                if (s_ActiveGizmoAxis == 0 || s_ActiveGizmoAxis == 1)
                                    tc->Transform.Position.x = s_DragStartPos.x + localDeltaX;
                                if (s_ActiveGizmoAxis == 0 || s_ActiveGizmoAxis == 2)
                                    tc->Transform.Position.y = s_DragStartPos.y + localDeltaY;
                            }
                            else if (gizmoMode == 2) // Rotate
                            {
                                float angleDelta = (mouseDelta.x - mouseDelta.y) * 0.5f;
                                tc->Transform.Rotation.Yaw = s_DragStartRot + angleDelta;
                            }
                            else if (gizmoMode == 3) // Scale
                            {
                                float scaleFactor = 0.02f;
                                if (s_ActiveGizmoAxis == 0 || s_ActiveGizmoAxis == 1)
                                    tc->Transform.Scale.Scale.x =
                                        Max(0.01f, s_DragStartScale.x + mouseDelta.x * scaleFactor);
                                if (s_ActiveGizmoAxis == 0 || s_ActiveGizmoAxis == 2)
                                    tc->Transform.Scale.Scale.y =
                                        Max(0.01f, s_DragStartScale.y - mouseDelta.y * scaleFactor);
                            }
                            if (editor && editor->GetActiveScene())
                            {
                                editor->GetActiveScene()->MarkDirty(true);
                            }
                        }
                        else
                        {
                            s_IsGizmoDragging = false;
                            s_ActiveGizmoAxis = -1;
                            editor->SetGizmoDragging(false);
                        }
                    }

                    EditorUtils::DrawTransformGizmo(dl, screenPos, gizmoMode, isGizmoHovered, s_IsGizmoDragging);
                }
            }
        }
    }

    TEVector2 mousePos = TimeGUI::GetMousePos();
    TEVector2 overlayPosLeft = TEVector2(vpMin.x + 12.0f, vpMin.y + 12.0f);
    float pillLeftWidth = 36.0f;
    float pillHeight = 32.0f;
    bool isOverLeftPill = (mousePos.x >= overlayPosLeft.x && mousePos.x <= overlayPosLeft.x + pillLeftWidth &&
                           mousePos.y >= overlayPosLeft.y && mousePos.y <= overlayPosLeft.y + pillHeight);

    float pillRightWidth = 260.0f;
    TEVector2 overlayPosRight = TEVector2(vpMin.x + vpSize.x - pillRightWidth - 12.0f, vpMin.y + 12.0f);
    bool isOverRightPill = (mousePos.x >= overlayPosRight.x && mousePos.x <= overlayPosRight.x + pillRightWidth &&
                            mousePos.y >= overlayPosRight.y && mousePos.y <= overlayPosRight.y + pillHeight);

    bool isOverOverlayUI = isOverLeftPill || isOverRightPill;

    // ── Left-Click Entity Selection Raycast ───────────────────────────────────
    if (editor->IsViewportHovered() && !editor->IsGizmoDragging() && !isGizmoHovered && !isOverOverlayUI &&
        TimeGUI::IsMouseClicked(0))
    {
        if (mousePos.x >= vpMin.x && mousePos.x <= vpMin.x + vpSize.x && mousePos.y >= vpMin.y &&
            mousePos.y <= vpMin.y + vpSize.y)
        {
            TEVector2 pixelPos = mousePos - vpMin;
            TEVector2 worldMouse =
                GameplayUtils::ViewportPixelToWorld(pixelPos, vpSize, TEVector2(camPos.x, camPos.y), camZoom);

            Entity hitEntity = GameplayUtils::PickEntity(*activeScene, worldMouse);
            TE_CORE_INFO("[Viewport Click] Pixel=({0}, {1}), World=({2}, {3}), Hit={4}", pixelPos.x, pixelPos.y,
                         worldMouse.x, worldMouse.y, hitEntity.IsValid() ? (uint64_t)hitEntity : 0);

            if (hitEntity.IsValid())
            {
                bool control = TimeGUI::GetIO().KeyCtrl;
                editor->SelectEntity(hitEntity, false, control);
            }
            else
            {
                editor->ClearSelection();
            }
        }
    }

    // ── Right-Click to Spawn ViewportContextMenu Layer ────────────────────────
    if (editor->IsViewportHovered() && !isOverOverlayUI && TimeGUI::IsMouseClicked(1))
    {
        if (mousePos.x >= vpMin.x && mousePos.x <= vpMin.x + vpSize.x && mousePos.y >= vpMin.y &&
            mousePos.y <= vpMin.y + vpSize.y)
        {
            ViewportContextMenu::OpenAt(mousePos, editor->GetHoveredEntity());
        }
    }

    // ── Group 1: Top-Left Hamburger Menu Pill ─────────────────────────────────
    dl.AddRectFilled(overlayPosLeft, TEVector2(overlayPosLeft.x + pillLeftWidth, overlayPosLeft.y + pillHeight),
                     0xD013161C, 6.0f);
    dl.AddRect(overlayPosLeft, TEVector2(overlayPosLeft.x + pillLeftWidth, overlayPosLeft.y + pillHeight), 0x90283040,
               6.0f, 0, 1.0f);

    TimeGUI::SetCursorScreenPos(TEVector2(overlayPosLeft.x + 4.0f, overlayPosLeft.y + 3.0f));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 4.0f);
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.16f, 0.19f, 0.25f, 0.90f));
    TimeGUI::PushStyleColor(TimeGUICol_Border, TEVector4(0.25f, 0.30f, 0.40f, 0.8f));

    // Hamburger 3-lines Button
    if (TimeGUI::Button("##VpHamburgerMenu", TEVector2(28.0f, 26.0f)))
    {
        TimeGUI::OpenPopup("ViewportHamburgerPopup");
    }
    TEVector2 hMin = TimeGUI::GetItemRectMin();
    TEVector2 hCenter = TEVector2(hMin.x + 14.0f, hMin.y + 13.0f);
    EditorUtils::DrawHamburgerIcon(dl, hCenter, 16.0f, 0xFFFFFFFF);
    if (TimeGUI::IsItemHovered())
        TimeGUI::SetTooltip("Viewport Display & Tools Menu");

    if (TimeGUI::BeginPopup("ViewportHamburgerPopup"))
    {
        static int shadingMode = 0; // 0 = Lit, 1 = Wireframe, 2 = Unlit
        TimeGUI::TextDisabled("Shading Mode");
        if (TimeGUI::RadioButton("Lit", shadingMode == 0))
        {
            shadingMode = 0;
        }
        if (TimeGUI::RadioButton("Wireframe", shadingMode == 1))
        {
            shadingMode = 1;
        }
        if (TimeGUI::RadioButton("Unlit", shadingMode == 2))
        {
            shadingMode = 2;
        }

        TimeGUI::Separator();

        TimeGUI::TextDisabled("Diagnostics");
        auto profilerPanel = editor ? editor->GetPanelByID("ProfilerPanel") : nullptr;
        bool isProfilingVisible = profilerPanel ? profilerPanel->IsVisible() : false;
        if (TimeGUI::Checkbox("Profiling Tool", &isProfilingVisible))
        {
            if (profilerPanel)
                profilerPanel->SetVisible(isProfilingVisible);
        }

        TimeGUI::EndPopup();
    }

    TimeGUI::PopStyleColor(2);
    TimeGUI::PopStyleVar(1);

    // ── Group 2: Right-Side Transform Vector Icons & Snapping Pill (Top-Right)
    if (overlayPosRight.x > overlayPosLeft.x + pillLeftWidth + 10.0f)
    {
        dl.AddRectFilled(overlayPosRight, TEVector2(overlayPosRight.x + pillRightWidth, overlayPosRight.y + pillHeight),
                         0xD013161C, 6.0f);
        dl.AddRect(overlayPosRight, TEVector2(overlayPosRight.x + pillRightWidth, overlayPosRight.y + pillHeight),
                   0x90283040, 6.0f, 0, 1.0f);

        TimeGUI::SetCursorScreenPos(TEVector2(overlayPosRight.x + 4.0f, overlayPosRight.y + 3.0f));

        TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 4.0f);
        TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(3.0f, 0.0f));
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.16f, 0.19f, 0.25f, 0.90f));
        TimeGUI::PushStyleColor(TimeGUICol_Border, TEVector4(0.25f, 0.30f, 0.40f, 0.8f));

        float btnW = 28.0f;
        float btnH = 26.0f;

        // 1. Select Tool (Arrow Icon)
        bool isSelect = (editor->GetGizmoType() == EditorLayer::GizmoType::None);
        if (isSelect)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.24f, 0.48f, 0.85f, 0.95f));
        if (TimeGUI::Button("##VpToolSelect", TEVector2(btnW, btnH)))
            editor->SetGizmoType(EditorLayer::GizmoType::None);
        TEVector2 bMin1 = TimeGUI::GetItemRectMin();
        TEVector2 bCenter1 = TEVector2(bMin1.x + btnW * 0.5f, bMin1.y + btnH * 0.5f);
        EditorUtils::DrawSelectIcon(dl, bCenter1, 16.0f, 0xFFFFFFFF);
        if (TimeGUI::IsItemHovered())
            TimeGUI::SetTooltip("Select Tool (Q)");
        if (isSelect)
            TimeGUI::PopStyleColor();

        TimeGUI::SameLine();

        // 2. Translate Tool (4-way Arrow Icon)
        bool isTranslate = (editor->GetGizmoType() == EditorLayer::GizmoType::Translate);
        if (isTranslate)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.24f, 0.48f, 0.85f, 0.95f));
        if (TimeGUI::Button("##VpToolTranslate", TEVector2(btnW, btnH)))
            editor->SetGizmoType(EditorLayer::GizmoType::Translate);
        TEVector2 bMin2 = TimeGUI::GetItemRectMin();
        TEVector2 bCenter2 = TEVector2(bMin2.x + btnW * 0.5f, bMin2.y + btnH * 0.5f);
        EditorUtils::DrawTranslateIcon(dl, bCenter2, 16.0f, 0xFFFFFFFF);
        if (TimeGUI::IsItemHovered())
            TimeGUI::SetTooltip("Move / Translate (W)");
        if (isTranslate)
            TimeGUI::PopStyleColor();

        TimeGUI::SameLine();

        // 3. Rotate Tool (Arc Icon)
        bool isRotate = (editor->GetGizmoType() == EditorLayer::GizmoType::Rotate);
        if (isRotate)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.24f, 0.48f, 0.85f, 0.95f));
        if (TimeGUI::Button("##VpToolRotate", TEVector2(btnW, btnH)))
            editor->SetGizmoType(EditorLayer::GizmoType::Rotate);
        TEVector2 bMin3 = TimeGUI::GetItemRectMin();
        TEVector2 bCenter3 = TEVector2(bMin3.x + btnW * 0.5f, bMin3.y + btnH * 0.5f);
        EditorUtils::DrawRotateIcon(dl, bCenter3, 16.0f, 0xFFFFFFFF);
        if (TimeGUI::IsItemHovered())
            TimeGUI::SetTooltip("Rotate (E)");
        if (isRotate)
            TimeGUI::PopStyleColor();

        TimeGUI::SameLine();

        // 4. Scale Tool (Diagonal Box Icon)
        bool isScale = (editor->GetGizmoType() == EditorLayer::GizmoType::Scale);
        if (isScale)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.24f, 0.48f, 0.85f, 0.95f));
        if (TimeGUI::Button("##VpToolScale", TEVector2(btnW, btnH)))
            editor->SetGizmoType(EditorLayer::GizmoType::Scale);
        TEVector2 bMin4 = TimeGUI::GetItemRectMin();
        TEVector2 bCenter4 = TEVector2(bMin4.x + btnW * 0.5f, bMin4.y + btnH * 0.5f);
        EditorUtils::DrawScaleIcon(dl, bCenter4, 16.0f, 0xFFFFFFFF);
        if (TimeGUI::IsItemHovered())
            TimeGUI::SetTooltip("Scale (R)");
        if (isScale)
            TimeGUI::PopStyleColor();

        TimeGUI::SameLine(0, 6);

        // 5. Snapping Pill Button with Right-Click / Left-Click 3x4 Grid Selection
        static float s_CurrentSnap = 1.0f;
        TEString snapLabel = (s_CurrentSnap <= 0.0f) ? "No Snap" : (TEString::FromFloat(s_CurrentSnap, 1));
        if (s_CurrentSnap == 1.0f || s_CurrentSnap == 2.0f || s_CurrentSnap == 5.0f || s_CurrentSnap == 10.0f ||
            s_CurrentSnap == 20.0f || s_CurrentSnap == 25.0f || s_CurrentSnap == 50.0f || s_CurrentSnap == 100.0f)
        {
            snapLabel = TEString::FromInt((int)s_CurrentSnap);
        }

        TEString btnSnapText = " " + snapLabel + "px ##VpSnapBtn";
        if (TimeGUI::Button(btnSnapText, TEVector2(72.0f, btnH)))
        {
            TimeGUI::OpenPopup("SnappingGridPopup");
        }
        if (TimeGUI::IsItemHovered())
            TimeGUI::SetTooltip("Grid Snapping (Click / Right-Click to Change)");

        if (TimeGUI::BeginPopupContextItem("SnappingGridPopup"))
        {
            TimeGUI::Spacing();
            TimeGUI::TextColored(TEVector4(0.35f, 0.70f, 1.0f, 1.0f), "Grid Snap Increments");
            TimeGUI::Separator();
            TimeGUI::Spacing();

            const float snapValues[12] = {0.0f, 0.1f, 0.2f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f, 25.0f, 50.0f, 100.0f};
            const char *snapLabels[12] = {"No Snap", "0.1", "0.2", "0.5", "1.0", "2.0",
                                          "5.0",     "10",  "20",  "25",  "50",  "100"};

            for (int r = 0; r < 4; ++r)
            {
                for (int c = 0; c < 3; ++c)
                {
                    int idx = r * 3 + c;
                    bool active = (s_CurrentSnap == snapValues[idx]);
                    if (active)
                        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.24f, 0.48f, 0.85f, 0.95f));

                    if (TimeGUI::Button(snapLabels[idx], TEVector2(74.0f, 32.0f)))
                    {
                        s_CurrentSnap = snapValues[idx];
                        TimeGUI::CloseCurrentPopup();
                    }

                    if (active)
                        TimeGUI::PopStyleColor();

                    if (c < 2)
                        TimeGUI::SameLine(0, 6);
                }
                TimeGUI::Spacing();
            }

            TimeGUI::EndPopup();
        }

        TimeGUI::PopStyleColor(2);
        TimeGUI::PopStyleVar(2);
    }

    // Draw active ViewportContextMenu Layer
    static ViewportContextMenu s_ContextMenu;
    s_ContextMenu.OnTimeGUIRender(editor.get());

    TimeGUI::End();
}

bool ViewportPanel::OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor)
{
    if (!editor || !editor->IsViewportFocused())
        return false;

    if (shortcutId == "Editor_DeleteSelected")
    {
        if (!editor->GetSelectedEntities().IsEmpty())
        {
            editor->TriggerDeleteSelectedEntities();
            return true;
        }
        return false;
    }
    if (shortcutId == "Editor_Copy")
    {
        editor->CopySelectedEntities();
        return true;
    }
    if (shortcutId == "Editor_Paste")
    {
        editor->PasteSelectedEntities();
        return true;
    }
    if (shortcutId == "Editor_Duplicate")
    {
        editor->DuplicateSelectedEntities();
        return true;
    }
    if (shortcutId == "Editor_ClearSelection")
    {
        editor->ClearSelection();
        return true;
    }

    return false;
}

TE_REGISTER_EDITOR_PANEL(ViewportPanel);

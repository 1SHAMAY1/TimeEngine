#include "Editor/EditorUtils.hpp"
#include "Core/PreRequisites.h"
#include "Core/Project/Project.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/EditorPanel.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"

// ── Panel helpers ────────────────────────────────────────────────────────────

TERef<IEditorPanel> EditorUtils::FindPanel(Ref<EditorLayer> editor, const TEString &id)
{
    if (!editor)
        return nullptr;
    return editor->GetPanelByID(id);
}

void EditorUtils::TogglePanel(Ref<EditorLayer> editor, const TEString &id)
{
    auto panel = FindPanel(editor, id);
    if (panel)
        panel->SetVisible(!panel->IsVisible());
}

bool EditorUtils::IsPanelVisible(Ref<EditorLayer> editor, const TEString &id)
{
    auto panel = FindPanel(editor, id);
    return panel && panel->IsVisible();
}

// ── UI helpers ────────────────────────────────────────────────────────────────

void EditorUtils::DrawSectionHeader(const TEString &label)
{
    TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4(0.85f, 0.85f, 0.85f, 1.0f));
    TimeGUI::Separator();
    TimeGUI::Text("%s", label);
    TimeGUI::Separator();
    TimeGUI::PopStyleColor();
}

void EditorUtils::DrawCenteredText(const TEString &label)
{
    float winW = TimeGUI::GetContentRegionAvail().x;
    float textW = TimeGUI::CalcTextSize(label).x;
    if (textW < winW)
        TimeGUI::SetCursorPosX(TimeGUI::GetCursorPosX() + (winW - textW) * 0.5f);
    TimeGUI::TextUnformatted(label);
}

void EditorUtils::DrawHelpMarker(const TEString &text)
{
    TimeGUI::TextDisabled("(?)");
    if (TimeGUI::IsItemHovered())
    {
        TimeGUI::SetItemTooltip(text);
    }
}

void EditorUtils::DrawBadge(const TEString &label, float r, float g, float b, float a)
{
    TEVector2 pos = TimeGUI::GetCursorScreenPos();
    TEVector2 textSz = TimeGUI::CalcTextSize(label);
    TEVector2 pad = {6.0f, 2.0f};
    TEVector2 rectMin = pos;
    TEVector2 rectMax = {pos.x + textSz.x + pad.x * 2.0f, pos.y + textSz.y + pad.y * 2.0f};

    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    TEVector4 col(r, g, b, a);
    dl.AddRectFilled(rectMin, rectMax, TimeGUI::ColorConvertFloat4ToU32(col), 4.0f);
    dl.AddText({rectMin.x + pad.x, rectMin.y + pad.y}, 0xFFFFFFFF, label);
    TimeGUI::Dummy({rectMax.x - rectMin.x, rectMax.y - rectMin.y});
}

bool EditorUtils::BeginCard(const char *id, const char *header)
{
    TimeGUI::PushStyleVar(TimeGUIStyleVar_ChildRounding, 6.0f);
    TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.12f, 0.12f, 0.12f, 1.0f));
    bool open = TimeGUI::BeginChild(id, {0, 0}, true);
    if (open)
    {
        TimeGUI::TextUnformatted(header);
        TimeGUI::Separator();
    }
    return open;
}

void EditorUtils::EndCard()
{
    TimeGUI::EndChild();
    TimeGUI::PopStyleColor();
    TimeGUI::PopStyleVar();
}

void EditorUtils::DrawSaveIcon(const TEVector2 &min, const TEVector2 &max, bool isDirty, unsigned int tintColor)
{
    auto dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    if (w <= 2.0f || h <= 2.0f)
        return;

    // Outer floppy body
    unsigned int bodyColor = (tintColor == 0xFFFFFFFF) ? IM_COL32(40, 72, 120, 255) : tintColor;
    unsigned int borderColor = IM_COL32(75, 125, 185, 255);
    unsigned int shutterColor = IM_COL32(195, 200, 210, 255);
    unsigned int labelColor = IM_COL32(235, 238, 245, 255);
    unsigned int lineCol = IM_COL32(100, 130, 170, 220);

    // Body rectangle
    dl.AddRectFilled(min, max, bodyColor, 2.0f);
    dl.AddRect(min, max, borderColor, 2.0f, 0, 1.2f);

    // Top metal shutter
    float shutterW = w * 0.55f;
    float shutterH = h * 0.38f;
    TEVector2 shutterMin(min.x + (w - shutterW) * 0.5f, min.y);
    TEVector2 shutterMax(shutterMin.x + shutterW, min.y + shutterH);
    dl.AddRectFilled(shutterMin, shutterMax, shutterColor, 1.0f);

    // Shutter slot
    float slotW = shutterW * 0.28f;
    float slotH = shutterH * 0.60f;
    TEVector2 slotMin(shutterMin.x + shutterW * 0.18f, min.y + shutterH * 0.20f);
    TEVector2 slotMax(slotMin.x + slotW, slotMin.y + slotH);
    dl.AddRectFilled(slotMin, slotMax, IM_COL32(35, 40, 50, 255));

    // Bottom paper label
    float labelW = w * 0.68f;
    float labelH = h * 0.42f;
    TEVector2 labelMin(min.x + (w - labelW) * 0.5f, max.y - labelH - 1.0f);
    TEVector2 labelMax(labelMin.x + labelW, max.y - 1.0f);
    dl.AddRectFilled(labelMin, labelMax, labelColor, 1.0f);

    // Label detail lines
    float lineY1 = labelMin.y + labelH * 0.35f;
    float lineY2 = labelMin.y + labelH * 0.70f;
    dl.AddLine(TEVector2(labelMin.x + 2.0f, lineY1), TEVector2(labelMax.x - 2.0f, lineY1), lineCol, 1.0f);
    dl.AddLine(TEVector2(labelMin.x + 2.0f, lineY2), TEVector2(labelMax.x - 2.0f, lineY2), lineCol, 1.0f);

    // If dirty, draw bright amber asterisk / indicator badge at top-right corner
    if (isDirty)
    {
        float dotRadius = 3.5f;
        TEVector2 dotCenter(max.x - 1.0f, min.y + 1.0f);
        dl.AddCircleFilled(dotCenter, dotRadius + 1.0f, IM_COL32(0, 0, 0, 200));
        dl.AddCircleFilled(dotCenter, dotRadius, IM_COL32(255, 175, 25, 255));
    }
}

// ── Path / Asset helpers ──────────────────────────────────────────────────────

TEString EditorUtils::FormatFileSize(uintmax_t bytes)
{
    std::ostringstream oss;
    if (bytes >= 1024 * 1024 * 1024)
        oss << (bytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
    else if (bytes >= 1024 * 1024)
        oss << (bytes / (1024.0 * 1024.0)) << " MB";
    else if (bytes >= 1024)
        oss << (bytes / 1024.0) << " KB";
    else
        oss << bytes << " B";
    return oss.str();
}

TEString EditorUtils::GetAssetTypeLabel(const TEString &path)
{
    TEString ext = path.GetExtension();
    if (ext.Equals(".tescene", ESearchCase::IgnoreCase))
        return "Scene";

    auto editor = AssetEditorRegistry::GetEditorForPath(path);
    if (editor)
        return editor->GetAssetType();

    return "File";
}

bool EditorUtils::IsKnownAssetExtension(const TEString &path)
{
    TEString ext = path.GetExtension();
    if (ext.Equals(".tescene", ESearchCase::IgnoreCase))
        return true;

    return AssetEditorRegistry::GetEditorForPath(path) != nullptr;
}

TEString EditorUtils::MakeProjectRelative(const TEString &absPath, const TEString &projectDir)
{
    TEString normAbs = absPath.Replace("\\", "/");
    TEString normProj = projectDir.Replace("\\", "/");
    if (!normProj.IsEmpty() && normProj.EndsWith("/"))
        normProj = normProj.Mid(0, normProj.Length() - 1);
    if (normAbs.StartsWith(normProj))
    {
        TEString rel = normAbs.Mid(normProj.Length());
        if (rel.StartsWith("/"))
            rel = rel.Mid(1);
        return rel;
    }
    return absPath;
}

// ── Scene / State helpers ─────────────────────────────────────────────────────

const char *EditorUtils::SceneStateLabel(Ref<EditorLayer> editor)
{
    if (!editor)
        return "Edit";
    switch (editor->GetSceneState())
    {
    case EditorLayer::SceneState::Play:
        return "Play";
    case EditorLayer::SceneState::Pause:
        return "Pause";
    default:
        return "Edit";
    }
}

bool EditorUtils::IsInPlayMode(Ref<EditorLayer> editor)
{
    if (!editor)
        return false;
    auto state = editor->GetSceneState();
    return state == EditorLayer::SceneState::Play || state == EditorLayer::SceneState::Pause;
}

bool EditorUtils::QuickSaveScene(Ref<EditorLayer> editor)
{
    if (!editor || !editor->GetActiveScene())
        return false;

    TEString scenePath = Project::GetProjectDirectory() / "Scenes" / "Untitled.tescene";

    SceneSerializer serializer(editor->GetActiveScene());
    serializer.Serialize(scenePath);
    TE_CORE_INFO("EditorUtils::QuickSaveScene -> {0}", scenePath);
    return true;
}

void EditorUtils::SetEditorThemeColors()
{
    auto &style = TimeGUI::GetStyle();
    auto &colors = style.Colors;

    // --- Style Tweaks ---
    style.WindowRounding = 6.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.TabRounding = 5.0f;
    style.GrabRounding = 3.0f;

    style.WindowPadding = TEVector2(8.0f, 8.0f);
    style.FramePadding = TEVector2(6.0f, 5.0f);
    style.ItemSpacing = TEVector2(8.0f, 6.0f);
    style.IndentSpacing = 18.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;

    // --- Colors (Minimal Dark Glass Theme) ---
    // Backgrounds
    colors[TimeGUICol_WindowBg] = TEVector4(0.06f, 0.07f, 0.09f, 1.0f);
    colors[TimeGUICol_ChildBg] = TEVector4(0.09f, 0.10f, 0.13f, 0.75f);
    colors[TimeGUICol_PopupBg] = TEVector4(0.08f, 0.09f, 0.11f, 0.98f);
    colors[TimeGUICol_MenuBarBg] = TEVector4(0.09f, 0.10f, 0.12f, 1.0f);

    // Text
    colors[TimeGUICol_Text] = TEVector4(0.90f, 0.92f, 0.95f, 1.0f);
    colors[TimeGUICol_TextDisabled] = TEVector4(0.48f, 0.52f, 0.58f, 1.0f);
    colors[TimeGUICol_InputTextCursor] = TEVector4(1.0f, 1.0f, 1.0f, 1.0f);

    // Borders
    colors[TimeGUICol_Border] = TEVector4(0.18f, 0.20f, 0.24f, 0.9f);
    colors[TimeGUICol_BorderShadow] = TEVector4(0.0f, 0.0f, 0.0f, 0.0f);

    // Frame BG (Inputs, text boxes)
    colors[TimeGUICol_FrameBg] = TEVector4(0.13f, 0.15f, 0.18f, 0.85f);
    colors[TimeGUICol_FrameBgHovered] = TEVector4(0.20f, 0.23f, 0.28f, 1.0f);
    colors[TimeGUICol_FrameBgActive] = TEVector4(0.16f, 0.18f, 0.22f, 1.0f);

    // Tabs
    colors[TimeGUICol_Tab] = TEVector4(0.11f, 0.13f, 0.16f, 1.0f);
    colors[TimeGUICol_TabHovered] = TEVector4(0.22f, 0.26f, 0.34f, 1.0f);
    colors[TimeGUICol_TabSelected] = TEVector4(0.16f, 0.18f, 0.23f, 1.0f);
    colors[TimeGUICol_TabSelectedOverline] = TEVector4(0.28f, 0.56f, 0.92f, 1.0f);
    colors[TimeGUICol_TabDimmed] = TEVector4(0.09f, 0.10f, 0.12f, 1.0f);
    colors[TimeGUICol_TabDimmedSelected] = TEVector4(0.13f, 0.15f, 0.18f, 1.0f);

    // Titles
    colors[TimeGUICol_TitleBg] = TEVector4(0.08f, 0.09f, 0.11f, 1.0f);
    colors[TimeGUICol_TitleBgActive] = TEVector4(0.11f, 0.13f, 0.16f, 1.0f);
    colors[TimeGUICol_TitleBgCollapsed] = TEVector4(0.06f, 0.07f, 0.09f, 0.75f);

    // Headers (Tree nodes, collapsing headers, selectables)
    colors[TimeGUICol_Header] = TEVector4(0.16f, 0.19f, 0.24f, 0.7f);
    colors[TimeGUICol_HeaderHovered] = TEVector4(0.24f, 0.28f, 0.36f, 0.85f);
    colors[TimeGUICol_HeaderActive] = TEVector4(0.20f, 0.24f, 0.32f, 1.0f);

    // Buttons
    colors[TimeGUICol_Button] = TEVector4(0.14f, 0.16f, 0.20f, 0.85f);
    colors[TimeGUICol_ButtonHovered] = TEVector4(0.24f, 0.28f, 0.36f, 1.0f);
    colors[TimeGUICol_ButtonActive] = TEVector4(0.18f, 0.21f, 0.27f, 1.0f);

    // Scrollbar
    colors[TimeGUICol_ScrollbarBg] = TEVector4(0.10f, 0.11f, 0.13f, 0.6f);
    colors[TimeGUICol_ScrollbarGrab] = TEVector4(0.24f, 0.27f, 0.33f, 1.0f);
    colors[TimeGUICol_ScrollbarGrabHovered] = TEVector4(0.32f, 0.36f, 0.44f, 1.0f);
    colors[TimeGUICol_ScrollbarGrabActive] = TEVector4(0.40f, 0.46f, 0.56f, 1.0f);

    // Checkmark & Sliders
    colors[TimeGUICol_CheckMark] = TEVector4(0.26f, 0.59f, 0.98f, 1.0f);
    colors[TimeGUICol_SliderGrab] = TEVector4(0.26f, 0.59f, 0.98f, 0.9f);
    colors[TimeGUICol_SliderGrabActive] = TEVector4(0.36f, 0.69f, 1.0f, 1.0f);

    // Separators
    colors[TimeGUICol_Separator] = TEVector4(0.22f, 0.24f, 0.28f, 0.8f);
    colors[TimeGUICol_SeparatorHovered] = TEVector4(0.30f, 0.35f, 0.45f, 1.0f);
    colors[TimeGUICol_SeparatorActive] = TEVector4(0.26f, 0.59f, 0.98f, 1.0f);

    // Docking
    colors[TimeGUICol_DockingPreview] = TEVector4(0.26f, 0.59f, 0.98f, 0.4f);
    colors[TimeGUICol_DockingEmptyBg] = TEVector4(0.08f, 0.09f, 0.10f, 1.0f);
}

void EditorUtils::DrawInfinite2DGrid(TimeGUIDrawList &dl, const TEVector2 &viewportMin, const TEVector2 &viewportSize,
                                     const TEVector2 &cameraPos, float zoom, float primaryGridStep,
                                     float subGridDivisions)
{
    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f || zoom <= 0.0001f)
        return;

    TEVector2 viewportMax = TEVector2(viewportMin.x + viewportSize.x, viewportMin.y + viewportSize.y);
    dl.PushClipRect(viewportMin, viewportMax, true);

    TEVector2 center = TEVector2(viewportMin.x + viewportSize.x * 0.5f, viewportMin.y + viewportSize.y * 0.5f);

    float pixelsPerUnit = (viewportSize.y * 0.5f) / zoom;
    if (pixelsPerUnit <= 0.001f)
    {
        dl.PopClipRect();
        return;
    }

    float minPixelSpacing = 40.0f;
    float step = primaryGridStep;
    while (step * pixelsPerUnit < minPixelSpacing)
        step *= subGridDivisions;
    while (step * pixelsPerUnit > minPixelSpacing * subGridDivisions)
        step /= subGridDivisions;

    float subStep = step / subGridDivisions;

    float worldHalfWidth = (viewportSize.x * 0.5f) / pixelsPerUnit;
    float worldHalfHeight = (viewportSize.y * 0.5f) / pixelsPerUnit;

    float worldMinX = cameraPos.x - worldHalfWidth;
    float worldMaxX = cameraPos.x + worldHalfWidth;
    float worldMinY = cameraPos.y - worldHalfHeight;
    float worldMaxY = cameraPos.y + worldHalfHeight;

    unsigned int subGridColor = 0x1AFFFFFF;     // Faint gray 10%
    unsigned int primaryGridColor = 0x33FFFFFF; // Crisp gray 20%
    unsigned int xAxisColor = 0x884040E0;       // Soft Red (X-Axis)
    unsigned int yAxisColor = 0x8840E040;       // Soft Green (Y-Axis)

    auto WorldToScreen = [&](const TEVector2 &world) -> TEVector2
    {
        return TEVector2(center.x + (world.x - cameraPos.x) * pixelsPerUnit,
                         center.y - (world.y - cameraPos.y) * pixelsPerUnit);
    };

    if (subStep * pixelsPerUnit > 8.0f)
    {
        float startX = std::floor(worldMinX / subStep) * subStep;
        for (float x = startX; x <= worldMaxX; x += subStep)
        {
            TEVector2 p1 = WorldToScreen(TEVector2(x, worldMinY));
            dl.AddLine(TEVector2(p1.x, viewportMin.y), TEVector2(p1.x, viewportMax.y), subGridColor, 1.0f);
        }

        float startY = std::floor(worldMinY / subStep) * subStep;
        for (float y = startY; y <= worldMaxY; y += subStep)
        {
            TEVector2 p1 = WorldToScreen(TEVector2(worldMinX, y));
            dl.AddLine(TEVector2(viewportMin.x, p1.y), TEVector2(viewportMax.x, p1.y), subGridColor, 1.0f);
        }
    }

    float majorStartX = std::floor(worldMinX / step) * step;
    for (float x = majorStartX; x <= worldMaxX; x += step)
    {
        TEVector2 p = WorldToScreen(TEVector2(x, 0.0f));
        dl.AddLine(TEVector2(p.x, viewportMin.y), TEVector2(p.x, viewportMax.y), primaryGridColor, 1.0f);
    }

    float majorStartY = std::floor(worldMinY / step) * step;
    for (float y = majorStartY; y <= worldMaxY; y += step)
    {
        TEVector2 p = WorldToScreen(TEVector2(0.0f, y));
        dl.AddLine(TEVector2(viewportMin.x, p.y), TEVector2(viewportMax.x, p.y), primaryGridColor, 1.0f);
    }

    TEVector2 origin = WorldToScreen(TEVector2(0.0f, 0.0f));

    if (origin.y >= viewportMin.y && origin.y <= viewportMax.y)
    {
        dl.AddLine(TEVector2(viewportMin.x, origin.y), TEVector2(viewportMax.x, origin.y), xAxisColor, 1.5f);
    }

    if (origin.x >= viewportMin.x && origin.x <= viewportMax.x)
    {
        dl.AddLine(TEVector2(origin.x, viewportMin.y), TEVector2(origin.x, viewportMax.y), yAxisColor, 1.5f);
    }

    dl.PopClipRect();
}

void EditorUtils::DrawTransformGizmo(TimeGUIDrawList &dl, const TEVector2 &screenPos, int gizmoType, bool isHovered,
                                     bool isDragging)
{
    float armLength = 48.0f;
    float arrowHeadSize = 8.0f;

    unsigned int redCol = isDragging ? 0xFFFF6666 : (isHovered ? 0xFFFF4444 : 0xFFEE3333);
    unsigned int greenCol = isDragging ? 0xFF66FF66 : (isHovered ? 0xFF44FF44 : 0xFF33EE33);
    unsigned int centerCol = 0xFFFFFFFF;

    dl.AddCircleFilled(screenPos, 4.0f, centerCol);

    if (gizmoType == 1) // Translate Mode (W)
    {
        TEVector2 xEnd = TEVector2(screenPos.x + armLength, screenPos.y);
        dl.AddLine(screenPos, xEnd, redCol, 2.5f);
        dl.AddTriangleFilled(TEVector2(xEnd.x + arrowHeadSize, xEnd.y),
                             TEVector2(xEnd.x, xEnd.y - arrowHeadSize * 0.5f),
                             TEVector2(xEnd.x, xEnd.y + arrowHeadSize * 0.5f), redCol);

        TEVector2 yEnd = TEVector2(screenPos.x, screenPos.y - armLength);
        dl.AddLine(screenPos, yEnd, greenCol, 2.5f);
        dl.AddTriangleFilled(TEVector2(yEnd.x, yEnd.y - arrowHeadSize),
                             TEVector2(yEnd.x - arrowHeadSize * 0.5f, yEnd.y),
                             TEVector2(yEnd.x + arrowHeadSize * 0.5f, yEnd.y), greenCol);
    }
    else if (gizmoType == 2) // Rotate Mode (E)
    {
        dl.AddCircle(screenPos, armLength * 0.8f, 0xFF44AAFF, 32, 2.0f);
    }
    else if (gizmoType == 3) // Scale Mode (R)
    {
        TEVector2 xEnd = TEVector2(screenPos.x + armLength, screenPos.y);
        dl.AddLine(screenPos, xEnd, redCol, 2.5f);
        dl.AddRectFilled(TEVector2(xEnd.x - 4.0f, xEnd.y - 4.0f), TEVector2(xEnd.x + 4.0f, xEnd.y + 4.0f), redCol);

        TEVector2 yEnd = TEVector2(screenPos.x, screenPos.y - armLength);
        dl.AddLine(screenPos, yEnd, greenCol, 2.5f);
        dl.AddRectFilled(TEVector2(screenPos.x - 4.0f, screenPos.y - 4.0f),
                         TEVector2(screenPos.x + 4.0f, screenPos.y + 4.0f), 0xFF4A88E8, 1.0f);
    }
}

void EditorUtils::DrawHamburgerIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float w = size * 0.28f;
    float h = size * 0.38f;
    float thickness = 2.0f;

    // Line 1 (Left)
    dl.AddLine(TEVector2(center.x - w, center.y - h), TEVector2(center.x - w, center.y + h), color, thickness);
    // Line 2 (Middle)
    dl.AddLine(TEVector2(center.x, center.y - h), TEVector2(center.x, center.y + h), color, thickness);
    // Line 3 (Right)
    dl.AddLine(TEVector2(center.x + w, center.y - h), TEVector2(center.x + w, center.y + h), color, thickness);
}

void EditorUtils::DrawPlayIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float half = size * 0.4f;
    dl.AddTriangleFilled(TEVector2(center.x - half * 0.6f, center.y - half),
                         TEVector2(center.x - half * 0.6f, center.y + half),
                         TEVector2(center.x + half * 0.9f, center.y), color);
}

void EditorUtils::DrawPauseIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float halfW = size * 0.35f;
    float halfH = size * 0.4f;
    float barW = size * 0.2f;

    // Bar 1 (Left)
    dl.AddRectFilled(TEVector2(center.x - halfW, center.y - halfH),
                     TEVector2(center.x - halfW + barW, center.y + halfH), color, 1.0f);
    // Bar 2 (Right)
    dl.AddRectFilled(TEVector2(center.x + halfW - barW, center.y - halfH),
                     TEVector2(center.x + halfW, center.y + halfH), color, 1.0f);
}

void EditorUtils::DrawStopIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float half = size * 0.35f;
    dl.AddRectFilled(TEVector2(center.x - half, center.y - half), TEVector2(center.x + half, center.y + half), color,
                     2.0f);
}

void EditorUtils::DrawRestartIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float r = size * 0.38f;
    const int numSegments = 16;
    float startAngle = -2.35619f; // -135 deg
    float endAngle = 1.8326f;     // 105 deg
    float angleStep = (endAngle - startAngle) / (float)numSegments;

    for (int i = 0; i < numSegments; ++i)
    {
        float a1 = startAngle + (float)i * angleStep;
        float a2 = startAngle + (float)(i + 1) * angleStep;
        TEVector2 p1 = TEVector2(center.x + std::cos(a1) * r, center.y + std::sin(a1) * r);
        TEVector2 p2 = TEVector2(center.x + std::cos(a2) * r, center.y + std::sin(a2) * r);
        dl.AddLine(p1, p2, color, 2.0f);
    }

    // Arrowhead at end of the arc
    float tipAngle = endAngle;
    TEVector2 tip = TEVector2(center.x + std::cos(tipAngle) * r, center.y + std::sin(tipAngle) * r);
    float arr = size * 0.22f;
    TEVector2 a1 = TEVector2(tip.x + arr * 0.9f, tip.y - arr * 0.3f);
    TEVector2 a2 = TEVector2(tip.x - arr * 0.2f, tip.y - arr * 0.9f);
    dl.AddTriangleFilled(tip, a1, a2, color);
}

void EditorUtils::DrawSelectIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float half = size * 0.5f;
    TEVector2 p1 = TEVector2(center.x - half * 0.6f, center.y - half * 0.8f);
    TEVector2 p2 = TEVector2(center.x - half * 0.6f, center.y + half * 0.7f);
    TEVector2 p3 = TEVector2(center.x - half * 0.1f, center.y + half * 0.2f);
    TEVector2 p4 = TEVector2(center.x + half * 0.4f, center.y + half * 0.8f);
    TEVector2 p5 = TEVector2(center.x + half * 0.7f, center.y + half * 0.5f);
    TEVector2 p6 = TEVector2(center.x + half * 0.2f, center.y - half * 0.1f);
    TEVector2 p7 = TEVector2(center.x + half * 0.8f, center.y - half * 0.1f);

    dl.AddTriangleFilled(p1, p2, p3, color);
    dl.AddTriangleFilled(p1, p3, p7, color);
    dl.AddLine(p3, p4, color, 2.0f);
}

void EditorUtils::DrawTranslateIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float h = size * 0.45f;
    float arr = size * 0.22f;

    dl.AddLine(TEVector2(center.x - h, center.y), TEVector2(center.x + h, center.y), color, 2.0f);
    dl.AddLine(TEVector2(center.x, center.y - h), TEVector2(center.x, center.y + h), color, 2.0f);

    dl.AddTriangleFilled(TEVector2(center.x - h - 1.0f, center.y), TEVector2(center.x - h + arr, center.y - arr * 0.6f),
                         TEVector2(center.x - h + arr, center.y + arr * 0.6f), color);
    dl.AddTriangleFilled(TEVector2(center.x + h + 1.0f, center.y), TEVector2(center.x + h - arr, center.y - arr * 0.6f),
                         TEVector2(center.x + h - arr, center.y + arr * 0.6f), color);
    dl.AddTriangleFilled(TEVector2(center.x, center.y - h - 1.0f), TEVector2(center.x - arr * 0.6f, center.y - h + arr),
                         TEVector2(center.x + arr * 0.6f, center.y - h + arr), color);
    dl.AddTriangleFilled(TEVector2(center.x, center.y + h + 1.0f), TEVector2(center.x - arr * 0.6f, center.y + h - arr),
                         TEVector2(center.x + arr * 0.6f, center.y + h - arr), color);
}

void EditorUtils::DrawRotateIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float r = size * 0.38f;
    dl.AddCircle(center, r, color, 16, 2.0f);

    float arr = size * 0.22f;
    dl.AddTriangleFilled(TEVector2(center.x + r + 2.0f, center.y + 1.0f),
                         TEVector2(center.x + r - arr * 0.8f, center.y - arr * 0.6f),
                         TEVector2(center.x + r + arr * 0.4f, center.y - arr * 0.8f), color);
}

void EditorUtils::DrawScaleIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float h = size * 0.38f;
    float box = size * 0.22f;

    dl.AddLine(TEVector2(center.x - h, center.y + h), TEVector2(center.x + h, center.y - h), color, 2.0f);

    dl.AddRectFilled(TEVector2(center.x - h - box * 0.5f, center.y + h - box * 0.5f),
                     TEVector2(center.x - h + box * 0.5f, center.y + h + box * 0.5f), color, 1.0f);

    dl.AddRectFilled(TEVector2(center.x + h - box * 0.5f, center.y - h - box * 0.5f),
                     TEVector2(center.x + h + box * 0.5f, center.y - h + box * 0.5f), color, 1.0f);
}

void EditorUtils::DrawAddIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float arm = size * 0.38f;
    dl.AddLine(TEVector2(center.x - arm, center.y), TEVector2(center.x + arm, center.y), color, 1.8f);
    dl.AddLine(TEVector2(center.x, center.y - arm), TEVector2(center.x, center.y + arm), color, 1.8f);
}

void EditorUtils::DrawCrossIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float arm = size * 0.32f;
    dl.AddLine(TEVector2(center.x - arm, center.y - arm), TEVector2(center.x + arm, center.y + arm), color, 1.8f);
    dl.AddLine(TEVector2(center.x + arm, center.y - arm), TEVector2(center.x - arm, center.y + arm), color, 1.8f);
}

void EditorUtils::DrawChevronLeftIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float halfH = size * 0.30f;
    float halfW = size * 0.18f;
    TEVector2 pts[3] = {TEVector2(center.x + halfW, center.y - halfH), TEVector2(center.x - halfW, center.y),
                        TEVector2(center.x + halfW, center.y + halfH)};
    dl.AddPolyline(pts, 3, color, 0, 2.0f);
}

void EditorUtils::DrawChevronRightIcon(TimeGUIDrawList &dl, const TEVector2 &center, float size, unsigned int color)
{
    float halfH = size * 0.30f;
    float halfW = size * 0.18f;
    TEVector2 pts[3] = {TEVector2(center.x - halfW, center.y - halfH), TEVector2(center.x + halfW, center.y),
                        TEVector2(center.x - halfW, center.y + halfH)};
    dl.AddPolyline(pts, 3, color, 0, 2.0f);
}

bool EditorUtils::DrawNavIconButton(const char *strID, bool isForward, bool enabled, const TEVector2 &btnSize)
{
    TimeGUI::PushID(strID);
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.0f, 0.0f, 0.0f, 0.0f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered,
                            enabled ? TEVector4(1.0f, 1.0f, 1.0f, 0.12f) : TEVector4(0.0f, 0.0f, 0.0f, 0.0f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive,
                            enabled ? TEVector4(1.0f, 1.0f, 1.0f, 0.22f) : TEVector4(0.0f, 0.0f, 0.0f, 0.0f));

    bool clicked = TimeGUI::Button("##nav", btnSize) && enabled;

    TimeGUI::PopStyleColor(3);

    TEVector2 bMin = TimeGUI::GetItemRectMin();
    TEVector2 bMax = TimeGUI::GetItemRectMax();
    TEVector2 center = TEVector2((bMin.x + bMax.x) * 0.5f, (bMin.y + bMax.y) * 0.5f);
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    unsigned int iconColor = enabled ? (TimeGUI::IsItemHovered() ? 0xFFFFFFFF : 0xDDFFFFFF)
                                     : 0x38FFFFFF; // Disabled: subtle transparent white

    if (!isForward)
        DrawChevronLeftIcon(dl, center, btnSize.y, iconColor);
    else
        DrawChevronRightIcon(dl, center, btnSize.y, iconColor);

    TimeGUI::PopID();
    return clicked;
}

// ── In-Engine Visual File Browser Dialog ───────────────────────────────────────

static bool s_FileBrowserOpen = false;
static bool s_FileBrowserOpenTrigger = false;
static EditorUtils::FileBrowserConfig s_CurrentBrowserConfig;
static std::function<void(const TEString &)> s_BrowserCallback = nullptr;
static TEString s_BrowserCurrentDir = "";
static TEString s_BrowserFilenameInput = "";
static TEString s_BrowserSelectedFile = "";

void EditorUtils::OpenFileBrowser(const FileBrowserConfig &config,
                                  std::function<void(const TEString &selectedPath)> onConfirm)
{
    s_CurrentBrowserConfig = config;
    s_BrowserCallback = onConfirm;

    if (!s_CurrentBrowserConfig.FilterExtension.empty() && !s_CurrentBrowserConfig.FilterExtension.StartsWith("."))
    {
        s_CurrentBrowserConfig.FilterExtension = "." + s_CurrentBrowserConfig.FilterExtension;
    }

    s_BrowserFilenameInput = s_CurrentBrowserConfig.DefaultFilename;

    TEString startDir = s_CurrentBrowserConfig.InitialDirectory;
    if (startDir.empty() || !TEFileSystem::Exists(startDir))
        startDir = Project::GetAssetDirectory();
    if (startDir.empty() || !TEFileSystem::Exists(startDir))
        startDir = Project::GetProjectDirectory();
    if (startDir.empty() || !TEFileSystem::Exists(startDir))
        startDir = TEFileSystem::GetCurrentWorkingDirectory();

    TEString scenesDir = startDir / "Scenes";
    if (s_CurrentBrowserConfig.FilterExtension.Equals(".tescene", ESearchCase::IgnoreCase) &&
        TEFileSystem::Exists(scenesDir))
        s_BrowserCurrentDir = scenesDir;
    else
        s_BrowserCurrentDir = startDir;

    s_BrowserSelectedFile = "";
    s_FileBrowserOpen = true;
    s_FileBrowserOpenTrigger = true;
}

void EditorUtils::OpenFileBrowser(const TEString &title, const TEString &actionButtonText,
                                  const TEString &defaultFilename, const TEString &filterExtension,
                                  bool allowFilenameInput, std::function<void(const TEString &selectedPath)> onConfirm)
{
    FileBrowserConfig config;
    config.Title = title;
    config.ActionButtonText = actionButtonText;
    config.DefaultFilename = defaultFilename;
    config.FilterExtension = filterExtension;
    config.AllowFilenameInput = allowFilenameInput;
    OpenFileBrowser(config, onConfirm);
}

bool EditorUtils::IsFileBrowserOpen() { return s_FileBrowserOpen; }

void EditorUtils::DrawFileBrowserModal()
{
    if (!s_FileBrowserOpen && !s_FileBrowserOpenTrigger)
        return;

    if (s_FileBrowserOpenTrigger)
    {
        TimeGUI::OpenPopup(s_CurrentBrowserConfig.Title.c_str());
        s_FileBrowserOpenTrigger = false;
    }

    TEVector2 displaySize = TimeGUI::GetIO().DisplaySize;
    TEVector2 centerPos = TEVector2(displaySize.x * 0.5f, displaySize.y * 0.5f);
    TimeGUI::SetNextWindowPos(centerPos, TimeGUICond_Appearing, TEVector2(0.5f, 0.5f));
    TimeGUI::SetNextWindowSize(TEVector2(850.0f, 560.0f), TimeGUICond_Appearing);
    if (TimeGUI::BeginPopupModal(s_CurrentBrowserConfig.Title.c_str(), &s_FileBrowserOpen,
                                 TimeGUIWindowFlags_NoCollapse))
    {
        // 1. Top Navigation Bar
        TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(6.0f, 4.0f));

        bool canGoUp = !s_BrowserCurrentDir.empty() && s_BrowserCurrentDir != Project::GetProjectDirectory();
        if (!canGoUp)
            TimeGUI::BeginDisabled();

        if (TimeGUI::Button(" Up ##BrowserUp", TEVector2(50.0f, 26.0f)))
        {
            TEString parentDir = s_BrowserCurrentDir.GetParentPath();
            if (!parentDir.empty() && TEFileSystem::Exists(parentDir))
                s_BrowserCurrentDir = parentDir;
        }

        if (!canGoUp)
            TimeGUI::EndDisabled();

        TimeGUI::SameLine();

        TEString assetsDir = Project::GetAssetDirectory();
        if (TimeGUI::Button(" Assets ##NavAssets", TEVector2(65.0f, 26.0f)))
        {
            if (TEFileSystem::Exists(assetsDir))
                s_BrowserCurrentDir = assetsDir;
        }
        TimeGUI::SameLine();

        TEString scenesDir = assetsDir / "Scenes";
        if (TimeGUI::Button(" Scenes ##NavScenes", TEVector2(65.0f, 26.0f)))
        {
            if (TEFileSystem::Exists(scenesDir))
                s_BrowserCurrentDir = scenesDir;
        }
        TimeGUI::SameLine();

        TimeGUI::SetNextItemWidth(-1);
        TEString displayDir = EditorUtils::MakeProjectRelative(s_BrowserCurrentDir, Project::GetProjectDirectory());
        TimeGUI::InputText("##BrowserPathDisplay", displayDir, TimeGUIInputTextFlags_ReadOnly);

        TimeGUI::PopStyleVar();
        TimeGUI::Separator();

        // 2. Main Content Split: Left Navigation / Right File List
        TimeGUI::Columns(2, "##FileBrowserColumns", true);
        TimeGUI::SetColumnWidth(0, 200.0f);

        // Left Navigation
        TimeGUI::BeginChild("##BrowserQuickNav", TEVector2(0, 320.0f), true);
        TimeGUI::TextColored(TEColor(0.5f, 0.7f, 1.0f, 1.0f), "Project Folders");
        TimeGUI::Separator();

        TEString projDir = Project::GetProjectDirectory();
        if (!projDir.empty())
        {
            if (TimeGUI::Selectable(" Project Root", s_BrowserCurrentDir == projDir))
                s_BrowserCurrentDir = projDir;
            if (TimeGUI::Selectable(" Assets", s_BrowserCurrentDir == assetsDir))
                s_BrowserCurrentDir = assetsDir;
            if (TimeGUI::Selectable(" Scenes", s_BrowserCurrentDir == scenesDir))
                s_BrowserCurrentDir = scenesDir;
            TEString scriptsDir = assetsDir / "Scripts";
            if (TimeGUI::Selectable(" Scripts", s_BrowserCurrentDir == scriptsDir))
                s_BrowserCurrentDir = scriptsDir;
            TEString texturesDir = assetsDir / "Textures";
            if (TimeGUI::Selectable(" Textures", s_BrowserCurrentDir == texturesDir))
                s_BrowserCurrentDir = texturesDir;
        }
        TimeGUI::EndChild();

        TimeGUI::NextColumn();

        // Right File List
        TimeGUI::BeginChild("##BrowserItemsList", TEVector2(0, 320.0f), true);

        if (!s_BrowserCurrentDir.empty() && TEFileSystem::Exists(s_BrowserCurrentDir))
        {
            auto subdirs = TEFileSystem::GetDirectories(s_BrowserCurrentDir, false);
            for (const auto &dir : subdirs)
            {
                TEString folderName = dir.GetFilename();
                if (folderName.empty())
                    folderName = dir;

                TEString label = "[Folder]  " + folderName;
                if (TimeGUI::Selectable(label.c_str(), false, TimeGUISelectableFlags_AllowDoubleClick))
                {
                    if (TimeGUI::IsMouseDoubleClicked(0))
                    {
                        s_BrowserCurrentDir = dir;
                    }
                }
            }

            auto files = TEFileSystem::GetFiles(s_BrowserCurrentDir, "", false);
            for (const auto &file : files)
            {
                TEString ext = file.GetExtension();
                if (!s_CurrentBrowserConfig.FilterExtension.empty() &&
                    !ext.Equals(s_CurrentBrowserConfig.FilterExtension, ESearchCase::IgnoreCase))
                    continue;

                TEString fileName = file.GetFilename();
                bool isSelected = (s_BrowserSelectedFile == file);

                TEString label = "[File]    " + fileName;
                if (TimeGUI::Selectable(label.c_str(), isSelected, TimeGUISelectableFlags_AllowDoubleClick))
                {
                    s_BrowserSelectedFile = file;
                    if (s_CurrentBrowserConfig.AllowFilenameInput)
                    {
                        s_BrowserFilenameInput = file.GetStem();
                    }

                    if (TimeGUI::IsMouseDoubleClicked(0))
                    {
                        if (s_BrowserCallback)
                            s_BrowserCallback(file);
                        s_FileBrowserOpen = false;
                        TimeGUI::CloseCurrentPopup();
                    }
                }
            }
        }
        else
        {
            TimeGUI::TextDisabled("Directory not found.");
        }

        TimeGUI::EndChild();
        TimeGUI::Columns(1);
        TimeGUI::Separator();

        // 3. Bottom Controls
        TimeGUI::Spacing();
        if (s_CurrentBrowserConfig.AllowFilenameInput)
        {
            TimeGUI::Text("File Name:");
            TimeGUI::SameLine(90.0f);
            TimeGUI::SetNextItemWidth(420.0f);
            TimeGUI::InputText("##BrowserFileNameInput", s_BrowserFilenameInput);
            if (!s_CurrentBrowserConfig.FilterExtension.empty())
            {
                TimeGUI::SameLine();
                TimeGUI::TextDisabled(s_CurrentBrowserConfig.FilterExtension.c_str());
            }
        }
        else
        {
            TimeGUI::Text("Selected:");
            TimeGUI::SameLine(90.0f);
            TimeGUI::TextColored(TEColor(0.8f, 0.9f, 1.0f, 1.0f), "%s", s_BrowserSelectedFile.GetFilename().c_str());
        }

        // Action Buttons
        TimeGUI::Spacing();
        TimeGUI::SetCursorPosX(TimeGUI::GetWindowWidth() - 220.0f);

        TEString actionText =
            s_CurrentBrowserConfig.ActionButtonText.empty() ? "Select" : s_CurrentBrowserConfig.ActionButtonText;
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.50f, 0.85f, 0.95f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.24f, 0.60f, 0.95f, 1.0f));

        if (TimeGUI::Button(actionText.c_str(), TEVector2(95.0f, 28.0f)))
        {
            TEString chosenPath;
            if (s_CurrentBrowserConfig.AllowFilenameInput)
            {
                if (!s_BrowserFilenameInput.empty())
                {
                    TEString finalName = s_BrowserFilenameInput;
                    if (!s_CurrentBrowserConfig.FilterExtension.empty() &&
                        !finalName.EndsWith(s_CurrentBrowserConfig.FilterExtension, ESearchCase::IgnoreCase))
                    {
                        finalName += s_CurrentBrowserConfig.FilterExtension;
                    }
                    chosenPath = s_BrowserCurrentDir / finalName;
                }
            }
            else
            {
                chosenPath = s_BrowserSelectedFile;
            }

            if (!chosenPath.empty())
            {
                if (s_BrowserCallback)
                    s_BrowserCallback(chosenPath);
                s_FileBrowserOpen = false;
                TimeGUI::CloseCurrentPopup();
            }
        }

        TimeGUI::PopStyleColor(2);

        TimeGUI::SameLine();
        if (TimeGUI::Button("Cancel", TEVector2(95.0f, 28.0f)))
        {
            s_FileBrowserOpen = false;
            TimeGUI::CloseCurrentPopup();
        }

        TimeGUI::EndPopup();
    }
}

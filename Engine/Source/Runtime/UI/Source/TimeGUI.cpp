#include "PreRequisites.h"
#include "TimeGUI.hpp"
#include "UIEngine.hpp"
#include "Log.h"
#include <cmath>

namespace TimeGUI
{

static TimeGUIStyle s_StyleInstance;
static TimeGUIIO s_IOInstance;

// =========================================================================
// Lifecycle Management
// =========================================================================

bool Init(void *nativeWindow) { return UIEngine::Get().Initialize(nativeWindow); }

bool InitOpenGLBackend() { return UIEngine::Get().InitializeRenderBackend(); }

void ShutdownOpenGLBackend() { UIEngine::Get().ShutdownRenderBackend(); }

void BindWidgetThreadContext() { UIEngine::Get().BindThreadContext(); }

void Shutdown() { UIEngine::Get().Shutdown(); }

void PrepareGLFWFrame() { UIEngine::Get().PrepareFrame(); }

void BeginFrame(uint32_t width, uint32_t height) { UIEngine::Get().BeginFrame(width, height); }

void *EndFrame(uint32_t width, uint32_t height) { return UIEngine::Get().EndFrame(width, height); }

void RenderDrawData(void *drawData) { UIEngine::Get().RenderDrawData(drawData); }

// =========================================================================
// Styles & IO
// =========================================================================

void StyleColorsDark()
{
    s_StyleInstance.WindowRounding = 4.0f;
    s_StyleInstance.ChildRounding = 4.0f;
    s_StyleInstance.FrameRounding = 3.0f;
    s_StyleInstance.PopupRounding = 4.0f;
    s_StyleInstance.ScrollbarRounding = 9.0f;
    s_StyleInstance.GrabRounding = 3.0f;
    s_StyleInstance.TabRounding = 4.0f;
    s_StyleInstance.WindowBorderSize = 1.0f;
    s_StyleInstance.FrameBorderSize = 0.0f;
    s_StyleInstance.PopupBorderSize = 1.0f;
    s_StyleInstance.WindowPadding = TEVector2(8.0f, 8.0f);
    s_StyleInstance.FramePadding = TEVector2(4.0f, 3.0f);
    s_StyleInstance.ItemSpacing = TEVector2(8.0f, 4.0f);

    s_StyleInstance.Colors[TimeGUICol_Text] = TEColor(0.90f, 0.90f, 0.90f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_TextDisabled] = TEColor(0.50f, 0.50f, 0.50f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_WindowBg] = TEColor(0.13f, 0.14f, 0.15f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_ChildBg] = TEColor(0.13f, 0.14f, 0.15f, 0.00f);
    s_StyleInstance.Colors[TimeGUICol_PopupBg] = TEColor(0.19f, 0.19f, 0.19f, 0.92f);
    s_StyleInstance.Colors[TimeGUICol_Border] = TEColor(0.25f, 0.25f, 0.25f, 0.50f);
    s_StyleInstance.Colors[TimeGUICol_BorderShadow] = TEColor(0.00f, 0.00f, 0.00f, 0.00f);
    s_StyleInstance.Colors[TimeGUICol_FrameBg] = TEColor(0.20f, 0.21f, 0.22f, 0.54f);
    s_StyleInstance.Colors[TimeGUICol_FrameBgHovered] = TEColor(0.40f, 0.40f, 0.40f, 0.40f);
    s_StyleInstance.Colors[TimeGUICol_FrameBgActive] = TEColor(0.18f, 0.18f, 0.18f, 0.67f);
    s_StyleInstance.Colors[TimeGUICol_TitleBg] = TEColor(0.09f, 0.10f, 0.10f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_TitleBgActive] = TEColor(0.14f, 0.15f, 0.16f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_TitleBgCollapsed] = TEColor(0.00f, 0.00f, 0.00f, 0.51f);
    s_StyleInstance.Colors[TimeGUICol_MenuBarBg] = TEColor(0.14f, 0.14f, 0.14f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_ScrollbarBg] = TEColor(0.02f, 0.02f, 0.02f, 0.53f);
    s_StyleInstance.Colors[TimeGUICol_ScrollbarGrab] = TEColor(0.31f, 0.31f, 0.31f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_ScrollbarGrabHovered] = TEColor(0.41f, 0.41f, 0.41f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_ScrollbarGrabActive] = TEColor(0.51f, 0.51f, 0.51f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_CheckMark] = TEColor(0.94f, 0.94f, 0.94f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_CheckboxSelectedBg] = TEColor(0.24f, 0.28f, 0.35f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_SliderGrab] = TEColor(0.51f, 0.51f, 0.51f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_SliderGrabActive] = TEColor(0.86f, 0.86f, 0.86f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_Button] = TEColor(0.44f, 0.44f, 0.44f, 0.40f);
    s_StyleInstance.Colors[TimeGUICol_ButtonHovered] = TEColor(0.46f, 0.47f, 0.48f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_ButtonActive] = TEColor(0.42f, 0.42f, 0.42f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_Header] = TEColor(0.70f, 0.70f, 0.70f, 0.31f);
    s_StyleInstance.Colors[TimeGUICol_HeaderHovered] = TEColor(0.70f, 0.70f, 0.70f, 0.80f);
    s_StyleInstance.Colors[TimeGUICol_HeaderActive] = TEColor(0.48f, 0.50f, 0.52f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_Separator] = TEColor(0.43f, 0.43f, 0.50f, 0.50f);
    s_StyleInstance.Colors[TimeGUICol_SeparatorHovered] = TEColor(0.72f, 0.72f, 0.72f, 0.78f);
    s_StyleInstance.Colors[TimeGUICol_SeparatorActive] = TEColor(0.51f, 0.51f, 0.51f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_ResizeGrip] = TEColor(0.91f, 0.91f, 0.91f, 0.25f);
    s_StyleInstance.Colors[TimeGUICol_ResizeGripHovered] = TEColor(0.81f, 0.81f, 0.81f, 0.67f);
    s_StyleInstance.Colors[TimeGUICol_ResizeGripActive] = TEColor(0.46f, 0.46f, 0.46f, 0.95f);
    s_StyleInstance.Colors[TimeGUICol_InputTextCursor] = TEColor(1.00f, 1.00f, 1.00f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_Tab] = TEColor(0.18f, 0.20f, 0.22f, 0.86f);
    s_StyleInstance.Colors[TimeGUICol_TabHovered] = TEColor(0.38f, 0.40f, 0.42f, 0.80f);
    s_StyleInstance.Colors[TimeGUICol_TabSelected] = TEColor(0.28f, 0.30f, 0.32f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_TabSelectedOverline] = TEColor(0.00f, 0.00f, 0.00f, 0.00f);
    s_StyleInstance.Colors[TimeGUICol_TabDimmed] = TEColor(0.12f, 0.13f, 0.14f, 0.97f);
    s_StyleInstance.Colors[TimeGUICol_TabDimmedSelected] = TEColor(0.20f, 0.22f, 0.24f, 1.00f);
    s_StyleInstance.Colors[TimeGUICol_DockingPreview] = TEColor(0.25f, 0.28f, 0.34f, 0.40f);
    s_StyleInstance.Colors[TimeGUICol_DockingEmptyBg] = TEColor(0.20f, 0.20f, 0.20f, 1.00f);
}

void StyleColorsClassic() { StyleColorsDark(); }
void StyleColorsLight() { StyleColorsDark(); }

TimeGUIStyle &GetStyle()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetStyle();
    return s_StyleInstance;
}

TimeGUIIO &GetIO()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetIO();
    return s_IOInstance;
}

// =========================================================================
// Windows & Containers
// =========================================================================

bool Begin(const TEString &name, bool *open, TimeGUIWindowFlags flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->Begin(name, open, flags);
    return false;
}

void End()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->End();
}

bool BeginChild(const TEString &strId, const TEVector2 &size, bool border, TimeGUIWindowFlags flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginChild(strId, size, border, flags);
    return false;
}

void EndChild()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndChild();
}

bool BeginMenuBar()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginMenuBar();
    return false;
}

void EndMenuBar()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndMenuBar();
}

bool BeginMenu(const TEString &label, bool enabled)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginMenu(label, enabled);
    return false;
}

void EndMenu()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndMenu();
}

bool MenuItem(const TEString &label, const TEString &shortcut, bool selected, bool enabled)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->MenuItem(label, shortcut, selected, enabled);
    return false;
}

bool MenuItem(const TEString &label, const TEString &shortcut, bool *p_selected, bool enabled)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->MenuItem(label, shortcut, p_selected, enabled);
    return false;
}

// =========================================================================
// Basic Widgets
// =========================================================================

bool Button(const TEString &label, float width, float height) { return Button(label, TEVector2(width, height)); }

bool Button(const TEString &label, const TEVector2 &size)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->Button(label, size);
    return false;
}

bool InvisibleButton(const TEString &strId, const TEVector2 &size, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InvisibleButton(strId, size, flags);
    return false;
}

void TextUnformatted(const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->TextUnformatted(text);
}

void AlignTextToFramePadding()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->AlignTextToFramePadding();
}

void Text(const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->Text(text);
}

void TextDisabled(const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->TextDisabled(text);
}

void TextColored(const TEColor &color, const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->TextColored(color, text);
}

void TextColored(const TEVector4 &color, const TEString &text)
{
    TextColored(TEColor(color.x, color.y, color.z, color.w), text);
}

void TextWrapped(const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->TextWrapped(text);
}

bool Checkbox(const TEString &label, bool *checked)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->Checkbox(label, checked);
    return false;
}

bool RadioButton(const TEString &label, bool active)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->RadioButton(label, active);
    return false;
}

bool RadioButton(const TEString &label, int *v, int v_button)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->RadioButton(label, v, v_button);
    return false;
}

// =========================================================================
// Sliders & Drags
// =========================================================================

bool DragFloat(const TEString &label, float *value, float speed, float min, float max, const TEString &format,
               int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DragFloat(label, value, speed, min, max, format, flags);
    return false;
}

bool DragFloat2(const TEString &label, float *v, float speed, float min, float max, const TEString &format, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DragFloat2(label, v, speed, min, max, format, flags);
    return false;
}

bool DragFloat3(const TEString &label, float *v, float speed, float min, float max, const TEString &format, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DragFloat3(label, v, speed, min, max, format, flags);
    return false;
}

bool DragFloat4(const TEString &label, float *v, float speed, float min, float max, const TEString &format, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DragFloat4(label, v, speed, min, max, format, flags);
    return false;
}

bool DragInt(const TEString &label, int *v, float speed, int min, int max)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DragInt(label, v, speed, min, max);
    return false;
}

bool InputInt(const TEString &label, int *v, int step, int step_fast, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InputInt(label, v, step, step_fast, flags);
    return false;
}

bool SliderFloat(const TEString &label, float *v, float v_min, float v_max, const TEString &format, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->SliderFloat(label, v, v_min, v_max, format, flags);
    return false;
}

bool SliderInt(const TEString &label, int *v, int v_min, int v_max, const TEString &format, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->SliderInt(label, v, v_min, v_max, format, flags);
    return false;
}

bool ColorEdit3(const TEString &label, float *col)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->ColorEdit3(label, col);
    return false;
}

bool ColorEdit4(const TEString &label, float *col, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->ColorEdit4(label, col, flags);
    return false;
}

bool ColorPicker4(const TEString &label, float *col, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->ColorPicker4(label, col, flags);
    return false;
}

bool ColorButton(const TEString &desc_id, const TEVector4 &col, int flags, const TEVector2 &size)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->ColorButton(desc_id, col, flags, size);
    return false;
}

bool SmallButton(const TEString &label)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->SmallButton(label);
    return false;
}

// =========================================================================
// Combos & Selectables
// =========================================================================

bool Combo(const TEString &label, int *currentItem, const char *const items[], int itemsCount,
           int popupMaxHeightInItems)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->Combo(label, currentItem, items, itemsCount, popupMaxHeightInItems);
    return false;
}

bool BeginCombo(const TEString &label, const TEString &previewValue, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginCombo(label, previewValue, flags);
    return false;
}

void EndCombo()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndCombo();
}

bool Selectable(const TEString &label, bool selected, int flags, const TEVector2 &size)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->Selectable(label, selected, flags, size);
    return false;
}

bool Selectable(const TEString &label, bool *selected, int flags, const TEVector2 &size)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->Selectable(label, selected, flags, size);
    return false;
}

// =========================================================================
// Input Fields
// =========================================================================

bool InputText(const TEString &label, TEString &value, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InputText(label, value, flags);
    return false;
}

bool InputText(const TEString &label, char *buf, size_t bufSize)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InputText(label, buf, bufSize, 0);
    return false;
}

bool InputText(const TEString &label, char *buf, size_t bufSize, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InputText(label, buf, bufSize, flags);
    return false;
}

bool InputTextWithHint(const TEString &label, const TEString &hint, TEString &value, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InputTextWithHint(label, hint, value, flags);
    return false;
}

bool InputTextWithHint(const TEString &label, const char *hint, char *buf, size_t bufSize, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InputTextWithHint(label, hint, buf, bufSize, flags);
    return false;
}

bool InputTextMultiline(const TEString &label, TEString &value, const TEVector2 &size, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InputTextMultiline(label, value, size, flags);
    return false;
}

bool InputTextMultiline(const TEString &label, char *buf, size_t bufSize, const TEVector2 &size, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->InputTextMultiline(label, buf, bufSize, size, flags);
    return false;
}

// =========================================================================
// Layout & Spacing
// =========================================================================

void Separator()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->Separator();
}

void SeparatorEx(int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SeparatorEx(flags);
}

void Spacing()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->Spacing();
}

void NewLine()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->NewLine();
}

void SameLine(float offset_from_start_x, float spacing)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SameLine(offset_from_start_x, spacing);
}

void Dummy(const TEVector2 &size)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->Dummy(size);
}

void BeginGroup()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->BeginGroup();
}

void EndGroup()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndGroup();
}

void Indent(float indentW)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->Indent(indentW);
}

void Unindent(float indentW)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->Unindent(indentW);
}

void Columns(int count, const char *id, bool border)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->Columns(count, id, border);
}

void NextColumn()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->NextColumn();
}

void SetColumnWidth(int columnIndex, float width)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetColumnWidth(columnIndex, width);
}

// =========================================================================
// ID & Item Width
// =========================================================================

void PushID(const TEString &strId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PushID(strId);
}

void PushID(int intId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PushID(intId);
}

void PushID(void *ptrId) { PushID(TEString::Format("%p", ptrId)); }

void PopID()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PopID();
}

unsigned int GetID(const TEString &strId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetID(strId);
    return 0;
}

void PushItemWidth(float itemWidth)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PushItemWidth(itemWidth);
}

void PopItemWidth()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PopItemWidth();
}

void PushMultiItemsWidths(int components, float width_full)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PushMultiItemsWidths(components, width_full);
}

float CalcItemWidth()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->CalcItemWidth();
    return 0.0f;
}

void BeginDisabled(bool disabled)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->BeginDisabled(disabled);
}

void EndDisabled()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndDisabled();
}

// =========================================================================
// Styling & Fonts
// =========================================================================

void PushStyleColor(TimeGUICol idx, const TEColor &color)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PushStyleColor(idx, color);
}

void PopStyleColor(int count)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PopStyleColor(count);
}

void PushStyleVar(TimeGUIStyleVar idx, float val)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PushStyleVar(idx, val);
}

void PushStyleVar(TimeGUIStyleVar idx, const TEVector2 &val)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PushStyleVar(idx, val);
}

void PopStyleVar(int count)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PopStyleVar(count);
}

void PushFont(const TimeGUIFont &font)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PushFont(font.nativeFont);
}

void PopFont()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->PopFont();
}

TimeGUIFont GetDefaultFont()
{
    TimeGUIFont f;
    if (auto *api = UIEngine::Get().GetUIAPI())
        f.nativeFont = api->GetDefaultFont();
    return f;
}

TimeGUITextureID GetFontAtlasTextureID()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetFontAtlasTextureID();
    return nullptr;
}

// =========================================================================
// Tables
// =========================================================================

bool BeginTable(const TEString &strId, int column, int flags, const TEVector2 &outerSize, float innerWidth)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginTable(strId, column, flags, outerSize, innerWidth);
    return false;
}

void EndTable()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndTable();
}

void TableSetupColumn(const TEString &label, int flags, float initWidthOrWeight, unsigned int userId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->TableSetupColumn(label, flags, initWidthOrWeight, userId);
}

void TableHeadersRow()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->TableHeadersRow();
}

bool TableNextColumn()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->TableNextColumn();
    return false;
}

void TableNextRow(float rowMinHeight, int rowFlags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->TableNextRow(rowMinHeight, rowFlags);
}

// =========================================================================
// Trees & Collapsing Headers
// =========================================================================

bool TreeNodeEx(const TEString &label, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->TreeNodeEx(label, flags);
    return false;
}

bool TreeNodeEx(void *ptrId, int flags, const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->TreeNodeEx(ptrId, flags, text);
    return false;
}

void TreePop()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->TreePop();
}

bool CollapsingHeader(const TEString &label, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->CollapsingHeader(label, flags);
    return false;
}

void SetNextItemOpen(bool isOpen)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetNextItemOpen(isOpen);
}

// =========================================================================
// Popups, Tooltips, Focus & Cursor
// =========================================================================

void OpenPopup(const TEString &strId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->OpenPopup(strId);
}

bool BeginPopup(const TEString &strId, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginPopup(strId, flags);
    return false;
}

bool BeginPopupContextWindow(const TEString &strId, int mouseButton, bool alsoOverItems)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginPopupContextWindow(strId, mouseButton, alsoOverItems);
    return false;
}

bool BeginPopupContextItem(const TEString &strId, int mouseButton)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginPopupContextItem(strId, mouseButton);
    return false;
}

bool BeginPopupModal(const TEString &name, bool *open, TimeGUIWindowFlags flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginPopupModal(name, open, flags);
    return false;
}

void EndPopup()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndPopup();
}

void CloseCurrentPopup()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->CloseCurrentPopup();
}

bool IsPopupOpen(const TEString &strId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsPopupOpen(strId);
    return false;
}

void BeginTooltip()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->BeginTooltip();
}

void EndTooltip()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndTooltip();
}

void SetTooltip(const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetTooltip(text);
}

void SetItemTooltip(const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetItemTooltip(text);
}

void SetItemDefaultFocus()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetItemDefaultFocus();
}

void SetMouseCursor(int cursorType)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetMouseCursor(cursorType);
}

// =========================================================================
// Drag & Drop
// =========================================================================

bool BeginDragDropSource(int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginDragDropSource(flags);
    return false;
}

bool SetDragDropPayload(const TEString &type, const void *data, size_t size, int cond)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->SetDragDropPayload(type, data, size, cond);
    return false;
}

void EndDragDropSource()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndDragDropSource();
}

bool BeginDragDropTarget()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginDragDropTarget();
    return false;
}

const TimeGUIPayload *AcceptDragDropPayload(const TEString &type, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->AcceptDragDropPayload(type, flags);
    return nullptr;
}

void EndDragDropTarget()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndDragDropTarget();
}

const TimeGUIPayload *GetDragDropPayload()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetDragDropPayload();
    return nullptr;
}

// =========================================================================
// Docking
// =========================================================================

unsigned int DockSpace(unsigned int id, const TEVector2 &size, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DockSpace(id, size, flags);
    return 0;
}

void DockBuilderRemoveNode(unsigned int nodeId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DockBuilderRemoveNode(nodeId);
}

void DockBuilderAddNode(unsigned int nodeId, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DockBuilderAddNode(nodeId, flags);
}

void DockBuilderSetNodeSize(unsigned int nodeId, const TEVector2 &size)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DockBuilderSetNodeSize(nodeId, size);
}

unsigned int DockBuilderSplitNode(unsigned int nodeId, int splitDir, float sizeRatio, unsigned int *outIdDir1,
                                  unsigned int *outIdDir2)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DockBuilderSplitNode(nodeId, splitDir, sizeRatio, outIdDir1, outIdDir2);
    return 0;
}

void DockBuilderDockWindow(const TEString &windowName, unsigned int nodeId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DockBuilderDockWindow(windowName, nodeId);
}

void DockBuilderFinish(unsigned int nodeId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DockBuilderFinish(nodeId);
}

// =========================================================================
// Tabs & Cards
// =========================================================================

bool BeginTabItem(const TEString &label, bool *open, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginTabItem(label, open, flags);
    return false;
}

void EndTabItem()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndTabItem();
}

bool BeginTabBar(const TEString &strId, int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginTabBar(strId, flags);
    return false;
}

void EndTabBar()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndTabBar();
}

bool BeginProjectCard(const TEString &id, const TEVector2 &size, bool &hovered)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->BeginProjectCard(id, size, hovered);
    return false;
}

void EndProjectCard()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->EndProjectCard();
}

// =========================================================================
// Images
// =========================================================================

void Image(TimeGUITextureID userTextureId, const TEVector2 &size, const TEVector2 &uv0, const TEVector2 &uv1,
           const TEVector4 &tintCol)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->Image(userTextureId, size, uv0, uv1, tintCol);
}

bool ImageButton(const TEString &strId, TimeGUITextureID userTextureId, const TEVector2 &size, const TEVector2 &uv0,
                 const TEVector2 &uv1)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->ImageButton(strId, userTextureId, size, uv0, uv1);
    return false;
}

// =========================================================================
// Next Window & Item Controls
// =========================================================================

void SetNextWindowPos(const TEVector2 &pos, TimeGUICond cond, const TEVector2 &pivot)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetNextWindowPos(pos, cond, pivot);
}

void SetNextWindowSize(const TEVector2 &size, TimeGUICond cond)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetNextWindowSize(size, cond);
}

void SetNextWindowContentSize(const TEVector2 &size)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetNextWindowContentSize(size);
}

void SetNextWindowDockID(unsigned int dockId, TimeGUICond cond)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetNextWindowDockID(dockId, cond);
}

void SetNextWindowViewport(unsigned int viewportId)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetNextWindowViewport(viewportId);
}

void SetNextItemWidth(float itemWidth)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetNextItemWidth(itemWidth);
}

void SetNextItemAllowOverlap()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetNextItemAllowOverlap();
}

// =========================================================================
// State, Queries & Input
// =========================================================================

TEVector2 GetCursorPos()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetCursorPos();
    return {0, 0};
}

void SetCursorPos(const TEVector2 &pos)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetCursorPos(pos);
}

float GetCursorPosX()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetCursorPosX();
    return 0.0f;
}

float GetCursorPosY()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetCursorPosY();
    return 0.0f;
}

void SetCursorPosX(float x)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetCursorPosX(x);
}

void SetCursorPosY(float y)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetCursorPosY(y);
}

TEVector2 GetCursorScreenPos()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetCursorScreenPos();
    return {0, 0};
}

void SetCursorScreenPos(const TEVector2 &pos)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetCursorScreenPos(pos);
}

TEVector2 GetWindowSize()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetWindowSize();
    return {0, 0};
}

TEVector2 GetWindowPos()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetWindowPos();
    return {0, 0};
}

float GetWindowWidth()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetWindowWidth();
    return 0.0f;
}

float GetWindowHeight()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetWindowHeight();
    return 0.0f;
}

TEVector2 GetContentRegionAvail()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetContentRegionAvail();
    return {0, 0};
}

float GetFrameHeight()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetFrameHeight();
    return 20.0f;
}

TEVector2 GetWindowContentRegionMin()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetWindowContentRegionMin();
    return {0, 0};
}

TEVector2 GetWindowContentRegionMax()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetWindowContentRegionMax();
    return {0, 0};
}

void SetScrollHereY(float centerYRatio)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetScrollHereY(centerYRatio);
}

float GetScrollY()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetScrollY();
    return 0.0f;
}

float GetScrollMaxY()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetScrollMaxY();
    return 0.0f;
}

void SetScrollY(float scrollY)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetScrollY(scrollY);
}

TEArray<unsigned int> GetInputQueueCharacters()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetInputQueueCharacters();
    return {};
}

void ClearInputQueueCharacters()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->ClearInputQueueCharacters();
}

TEVector2 GetMousePos()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetMousePos();
    return {0, 0};
}

bool IsMouseDown(int button)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsMouseDown(button);
    return false;
}

bool IsMouseReleased(int button)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsMouseReleased(button);
    return false;
}

bool IsMouseClicked(int button, bool repeat)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsMouseClicked(button, repeat);
    return false;
}

bool IsMouseDoubleClicked(int button)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsMouseDoubleClicked(button);
    return false;
}

bool IsMouseDragging(int button, float lock_threshold)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsMouseDragging(button, lock_threshold);
    return false;
}

TEVector2 GetMouseDragDelta(int button, float lock_threshold)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetMouseDragDelta(button, lock_threshold);
    return {0, 0};
}

void ResetMouseDragDelta(int button)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->ResetMouseDragDelta(button);
}

bool IsAnyItemHovered()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsAnyItemHovered();
    return false;
}

bool IsKeyPressed(int key)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsKeyPressed(key);
    return false;
}

double GetTime()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetTime();
    return 0.0;
}

TEVector2 CalcTextSize(const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->CalcTextSize(text);
    return {static_cast<float>(text.Length() * 7), 14.0f};
}

bool IsItemDeactivatedAfterEdit()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsItemDeactivatedAfterEdit();
    return false;
}

bool IsItemHovered(int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsItemHovered(flags);
    return false;
}

bool IsItemActive()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsItemActive();
    return false;
}

bool IsItemFocused()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsItemFocused();
    return false;
}

bool IsItemClicked(int mouseButton)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsItemClicked(mouseButton);
    return false;
}

bool IsWindowHovered(int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsWindowHovered(flags);
    return false;
}

bool IsWindowFocused(int flags)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsWindowFocused(flags);
    return false;
}

bool IsWindowAppearing()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->IsWindowAppearing();
    return false;
}

TEVector2 GetItemRectMin()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetItemRectMin();
    return {0, 0};
}

TEVector2 GetItemRectSize()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetItemRectSize();
    return {0, 0};
}

TEVector2 GetItemRectMax()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetItemRectMax();
    return {0, 0};
}

void SetKeyboardFocusHere(int offset)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetKeyboardFocusHere(offset);
}

void SetClipboardText(const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetClipboardText(text);
}

TEString GetClipboardText()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetClipboardText();
    return "";
}

void SetCaretColor(const TEVector4 &color)
{
    s_StyleInstance.Colors[TimeGUICol_InputTextCursor] = TEColor(color.x, color.y, color.z, color.w);
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SetCaretColor(color);
}

TEVector4 GetCaretColor()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->GetCaretColor();
    const TEColor &c = s_StyleInstance.Colors[TimeGUICol_InputTextCursor];
    return TEVector4(c.r, c.g, c.b, c.a);
}

TimeGUIViewport GetMainViewport()
{
    TimeGUIViewport tvp;
    if (auto *api = UIEngine::Get().GetUIAPI())
    {
        tvp.Pos = api->GetMainViewportPos();
        tvp.Size = api->GetMainViewportSize();
        tvp.ID = api->GetMainViewportID();
    }
    else
    {
        tvp.Pos = TEVector2(0.0f, 0.0f);
        tvp.Size = s_IOInstance.DisplaySize;
        tvp.ID = 0;
    }
    return tvp;
}

void PushSuspendedInput(const TEVector2 &stubMousePos) {}
void PopSuspendedInput() {}

void SaveIniSettingsToDisk(const char *ini_filename)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->SaveIniSettingsToDisk(ini_filename);
}

void LoadIniSettingsFromDisk(const char *ini_filename)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->LoadIniSettingsFromDisk(ini_filename);
}

// =========================================================================
// Color Conversions (Pure Math)
// =========================================================================

unsigned int GetColorU32(const TEColor &color)
{
    unsigned int r = (unsigned int)(Clamp01(color.r) * 255.0f + 0.5f);
    unsigned int g = (unsigned int)(Clamp01(color.g) * 255.0f + 0.5f);
    unsigned int b = (unsigned int)(Clamp01(color.b) * 255.0f + 0.5f);
    unsigned int a = (unsigned int)(Clamp01(color.a) * 255.0f + 0.5f);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

unsigned int GetColorU32(TimeGUICol idx, float alpha_mul)
{
    if (idx >= 0 && idx < TimeGUICol_COUNT)
    {
        TEColor c = s_StyleInstance.Colors[idx];
        c.a *= alpha_mul;
        return GetColorU32(c);
    }
    return 0xFFFFFFFF;
}

TEVector4 ColorConvertU32ToFloat4(unsigned int in)
{
    float s = 1.0f / 255.0f;
    return TEVector4((float)(in & 0xFF) * s, (float)((in >> 8) & 0xFF) * s, (float)((in >> 16) & 0xFF) * s,
                     (float)((in >> 24) & 0xFF) * s);
}

unsigned int ColorConvertFloat4ToU32(const TEVector4 &in)
{
    unsigned int r = (unsigned int)(Clamp01(in.x) * 255.0f + 0.5f);
    unsigned int g = (unsigned int)(Clamp01(in.y) * 255.0f + 0.5f);
    unsigned int b = (unsigned int)(Clamp01(in.z) * 255.0f + 0.5f);
    unsigned int a = (unsigned int)(Clamp01(in.w) * 255.0f + 0.5f);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b)
{
    if (s == 0.0f)
    {
        out_r = out_g = out_b = v;
        return;
    }

    h = std::fmod(h, 1.0f) / (60.0f / 360.0f);
    int i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i)
    {
    case 0:
        out_r = v;
        out_g = t;
        out_b = p;
        break;
    case 1:
        out_r = q;
        out_g = v;
        out_b = p;
        break;
    case 2:
        out_r = p;
        out_g = v;
        out_b = t;
        break;
    case 3:
        out_r = p;
        out_g = q;
        out_b = v;
        break;
    case 4:
        out_r = t;
        out_g = p;
        out_b = v;
        break;
    case 5:
    default:
        out_r = v;
        out_g = p;
        out_b = q;
        break;
    }
}

void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v)
{
    float K = 0.f;
    if (g < b)
    {
        std::swap(g, b);
        K = -1.f;
    }
    if (r < g)
    {
        std::swap(r, g);
        K = -2.f / 6.f - K;
    }

    float chroma = r - (g < b ? g : b);
    out_h = std::fabs(K + (g - b) / (6.f * chroma + 1e-20f));
    out_s = chroma / (r + 1e-20f);
    out_v = r;
}

// =========================================================================
// DrawList Commands
// =========================================================================

TimeGUIDrawList CreateDrawList()
{
    TimeGUIDrawList dl;
    if (auto *api = UIEngine::Get().GetUIAPI())
        dl.nativeDrawList = api->DrawListCreate();
    return dl;
}

void DestroyDrawList(TimeGUIDrawList &dl)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListDestroy(dl.nativeDrawList);
    dl.nativeDrawList = nullptr;
}

void RenderDrawList(const TimeGUIDrawList &dl, const TEVector2 &displaySize)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListRender(dl.nativeDrawList, displaySize);
}

TimeGUIDrawList GetBackgroundDrawList()
{
    TimeGUIDrawList dl;
    if (auto *api = UIEngine::Get().GetUIAPI())
        dl.nativeDrawList = api->GetBackgroundDrawList();
    return dl;
}

TimeGUIDrawList GetForegroundDrawList()
{
    TimeGUIDrawList dl;
    if (auto *api = UIEngine::Get().GetUIAPI())
        dl.nativeDrawList = api->GetForegroundDrawList();
    return dl;
}

TimeGUIDrawList GetWindowDrawList()
{
    TimeGUIDrawList dl;
    if (auto *api = UIEngine::Get().GetUIAPI())
        dl.nativeDrawList = api->GetWindowDrawList();
    return dl;
}

void TimeGUIDrawList::AddLine(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float thickness)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddLine(nativeDrawList, p1, p2, color, thickness);
}

void TimeGUIDrawList::AddRectFilled(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddRectFilled(nativeDrawList, p1, p2, color, rounding);
}

void TimeGUIDrawList::AddText(const TEVector2 &pos, unsigned int color, const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddText(nativeDrawList, pos, color, text);
}

void TimeGUIDrawList::AddText(const TimeGUIFont &font, float fontSize, const TEVector2 &pos, unsigned int color,
                              const TEString &text)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddText(nativeDrawList, font.nativeFont, fontSize, pos, color, text);
}

void TimeGUIDrawList::AddRect(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding, int flags,
                              float thickness)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddRect(nativeDrawList, p1, p2, color, rounding, flags, thickness);
}

void TimeGUIDrawList::AddPolyline(const TEVector2 *points, int num_points, unsigned int color, int flags,
                                  float thickness)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddPolyline(nativeDrawList, points, num_points, color, flags, thickness);
}

void TimeGUIDrawList::AddConvexPolyFilled(const TEVector2 *points, int num_points, unsigned int color)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddConvexPolyFilled(nativeDrawList, points, num_points, color);
}

void TimeGUIDrawList::AddTriangleFilled(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                        unsigned int color)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddTriangleFilled(nativeDrawList, p1, p2, p3, color);
}

void TimeGUIDrawList::AddBezierCubic(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3, const TEVector2 &p4,
                                     unsigned int color, float thickness, int num_segments)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddBezierCubic(nativeDrawList, p1, p2, p3, p4, color, thickness, num_segments);
}

void TimeGUIDrawList::AddCallback(void (*callback)(TimeGUIDrawList parent_list, const void *cmd), void *callback_data)
{
}

void TimeGUIDrawList::AddDrawCmd() {}

void TimeGUIDrawList::PushClipRect(const TEVector2 &clip_rect_min, const TEVector2 &clip_rect_max,
                                   bool intersect_with_current_clip_rect)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListPushClipRect(nativeDrawList, clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
}

void TimeGUIDrawList::PopClipRect()
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListPopClipRect(nativeDrawList);
}

void TimeGUIDrawList::PushTextureID(TimeGUITextureID texture_id)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListPushTextureID(nativeDrawList, texture_id);
}

void TimeGUIDrawList::ResetForNewFrame() {}

void TimeGUIDrawList::AddCircle(const TEVector2 &center, float radius, unsigned int color, int num_segments,
                                float thickness)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddCircle(nativeDrawList, center, radius, color, num_segments, thickness);
}

void TimeGUIDrawList::AddCircleFilled(const TEVector2 &center, float radius, unsigned int color, int num_segments)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddCircleFilled(nativeDrawList, center, radius, color, num_segments);
}

void TimeGUIDrawList::AddQuadFilled(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3, const TEVector2 &p4,
                                    unsigned int color)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddQuadFilled(nativeDrawList, p1, p2, p3, p4, color);
}

void TimeGUIDrawList::AddImage(TimeGUITextureID user_texture_id, const TEVector2 &p_min, const TEVector2 &p_max,
                               const TEVector2 &uv_min, const TEVector2 &uv_max, unsigned int col)
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        api->DrawListAddImage(nativeDrawList, user_texture_id, p_min, p_max, uv_min, uv_max, col);
}

int TimeGUIDrawList::GetVertexCount() const
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DrawListGetVertexCount(nativeDrawList);
    return 0;
}

int TimeGUIDrawList::GetCommandCount() const
{
    if (auto *api = UIEngine::Get().GetUIAPI())
        return api->DrawListGetCommandCount(nativeDrawList);
    return 0;
}

// =========================================================================
// Composite UI Controls (Clean Engine Facade Helpers)
// =========================================================================

TEString CleanLabel(const TEString &label)
{
    size_t hashPos = label.Find("##");
    if (hashPos != TEString::npos)
        return label.Substr(0, hashPos);
    return label;
}

bool DrawVec3Control(const TEString &label, TEVector &values, float resetValue, float columnWidth)
{
    bool changed = false;
    PushID(label);

    Columns(2);
    SetColumnWidth(0, columnWidth);
    Text(CleanLabel(label));
    NextColumn();

    PushMultiItemsWidths(3, CalcItemWidth());

    PushStyleColor(TimeGUICol_Button, TEColor(0.8f, 0.1f, 0.15f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.9f, 0.2f, 0.2f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.8f, 0.1f, 0.15f, 1.0f));
    if (Button("X", TEVector2(0, 0)))
    {
        values.x = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();
    SameLine();

    PushStyleColor(TimeGUICol_Button, TEColor(0.2f, 0.7f, 0.2f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.3f, 0.8f, 0.3f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.2f, 0.7f, 0.2f, 1.0f));
    if (Button("Y", TEVector2(0, 0)))
    {
        values.y = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();
    SameLine();

    PushStyleColor(TimeGUICol_Button, TEColor(0.1f, 0.25f, 0.8f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.2f, 0.35f, 0.9f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.1f, 0.25f, 0.8f, 1.0f));
    if (Button("Z", TEVector2(0, 0)))
    {
        values.z = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();

    Columns(1);
    PopID();
    return changed;
}

bool DrawVec2Control(const TEString &label, TEVector2 &values, float resetValue, float columnWidth)
{
    bool changed = false;
    PushID(label);

    Columns(2);
    SetColumnWidth(0, columnWidth);
    Text(CleanLabel(label));
    NextColumn();

    PushMultiItemsWidths(2, CalcItemWidth());

    PushStyleColor(TimeGUICol_Button, TEColor(0.8f, 0.1f, 0.15f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.9f, 0.2f, 0.2f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.8f, 0.1f, 0.15f, 1.0f));
    if (Button("X", TEVector2(0, 0)))
    {
        values.x = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();
    SameLine();

    PushStyleColor(TimeGUICol_Button, TEColor(0.2f, 0.7f, 0.2f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.3f, 0.8f, 0.3f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.2f, 0.7f, 0.2f, 1.0f));
    if (Button("Y", TEVector2(0, 0)))
    {
        values.y = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();

    Columns(1);
    PopID();
    return changed;
}

bool DrawVec4Control(const TEString &label, TEVector4 &values, float resetValue, float columnWidth)
{
    bool changed = false;
    PushID(label);

    Columns(2);
    SetColumnWidth(0, columnWidth);
    Text(CleanLabel(label));
    NextColumn();

    PushMultiItemsWidths(4, CalcItemWidth());

    PushStyleColor(TimeGUICol_Button, TEColor(0.8f, 0.1f, 0.15f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.9f, 0.2f, 0.2f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.8f, 0.1f, 0.15f, 1.0f));
    if (Button("X", TEVector2(0, 0)))
    {
        values.x = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();
    SameLine();

    PushStyleColor(TimeGUICol_Button, TEColor(0.2f, 0.7f, 0.2f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.3f, 0.8f, 0.3f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.2f, 0.7f, 0.2f, 1.0f));
    if (Button("Y", TEVector2(0, 0)))
    {
        values.y = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();
    SameLine();

    PushStyleColor(TimeGUICol_Button, TEColor(0.1f, 0.25f, 0.8f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.2f, 0.35f, 0.9f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.1f, 0.25f, 0.8f, 1.0f));
    if (Button("Z", TEVector2(0, 0)))
    {
        values.z = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();
    SameLine();

    PushStyleColor(TimeGUICol_Button, TEColor(0.5f, 0.5f, 0.5f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.6f, 0.6f, 0.6f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.5f, 0.5f, 0.5f, 1.0f));
    if (Button("W", TEVector2(0, 0)))
    {
        values.w = resetValue;
        changed = true;
    }
    PopStyleColor(3);

    SameLine();
    if (DragFloat("##W", &values.w, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    PopItemWidth();

    Columns(1);
    PopID();
    return changed;
}

bool DrawColorControl(const TEString &label, TEColor &values, float columnWidth)
{
    bool changed = false;
    PushID(label);

    Columns(2);
    SetColumnWidth(0, columnWidth);
    Text(CleanLabel(label));
    NextColumn();

    float col[4] = {values.r, values.g, values.b, values.a};
    if (ColorEdit4("##Color", col))
    {
        values = TEColor(col[0], col[1], col[2], col[3]);
        changed = true;
    }

    Columns(1);
    PopID();
    return changed;
}

bool DrawDeleteButton(const TEString &id, float size, float fontScale)
{
    PushID(id);
    PushStyleColor(TimeGUICol_Button, TEColor(0.8f, 0.1f, 0.15f, 0.7f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.9f, 0.2f, 0.2f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.7f, 0.05f, 0.1f, 1.0f));

    bool clicked = Button("-", TEVector2(size, size));

    PopStyleColor(3);
    PopID();
    return clicked;
}

bool DrawPlusButton(const TEString &id, float size, float fontScale)
{
    PushID(id);
    PushStyleColor(TimeGUICol_Button, TEColor(0.2f, 0.7f, 0.2f, 0.7f));
    PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.3f, 0.8f, 0.3f, 1.0f));
    PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.15f, 0.6f, 0.15f, 1.0f));

    bool clicked = Button("+", TEVector2(size, size));

    PopStyleColor(3);
    PopID();
    return clicked;
}

} // namespace TimeGUI

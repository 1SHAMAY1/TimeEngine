#pragma once
#include "PreRequisites.h"
#include "UIBackendType.hpp"
#include "EngineTypes/TEString.hpp"
#include "TEColor.hpp"
#include "MathUtils.hpp"
#include "UITypes.hpp"

/// Abstract interface for all immediate-mode UI operations.
/// Concrete backends (ImGuiUIAPI, ForgeUIAPI) inherit and implement these.
/// Client code and TimeGUI facade dispatch directly to this interface.
class TE_API UIAPI
{
public:
    virtual ~UIAPI() = default;

    // -------------------------------------------------------------------------
    // Identity & Backend Type
    // -------------------------------------------------------------------------
    virtual UIBackendType GetType() const = 0;

    // -------------------------------------------------------------------------
    // Lifecycle Management
    // -------------------------------------------------------------------------
    virtual bool Init(void *nativeWindow) = 0;
    virtual void Shutdown() = 0;
    virtual bool InitRenderBackend() = 0;
    virtual void ShutdownRenderBackend() = 0;

    // -------------------------------------------------------------------------
    // Threading & Context Management
    // -------------------------------------------------------------------------
    virtual void BindThreadContext() = 0;

    // -------------------------------------------------------------------------
    // Frame Lifecycle
    // -------------------------------------------------------------------------
    virtual void PrepareFrame() = 0;
    virtual void BeginFrame(uint32_t width, uint32_t height) = 0;
    virtual void *EndFrame(uint32_t width, uint32_t height) = 0;
    virtual void RenderDrawData(void *drawData) = 0;

    // -------------------------------------------------------------------------
    // Capabilities
    // -------------------------------------------------------------------------
    virtual bool SupportsDocking() const { return false; }
    virtual bool SupportsViewports() const { return false; }

    // -------------------------------------------------------------------------
    // IO & Style State
    // -------------------------------------------------------------------------
    virtual UIIO &GetIO() = 0;
    virtual UIStyle &GetStyle() = 0;
    virtual void SetStyle(const UIStyle &style) = 0;

    // -------------------------------------------------------------------------
    // Viewport & Root Canvas Queries
    // -------------------------------------------------------------------------
    virtual TEVector2 GetMainViewportPos() = 0;
    virtual TEVector2 GetMainViewportSize() = 0;
    virtual uint32_t GetMainViewportID() = 0;

    virtual void *GetBackgroundDrawList() = 0;
    virtual void *GetForegroundDrawList() = 0;
    virtual void *GetWindowDrawList() = 0;

    // -------------------------------------------------------------------------
    // Windows & Containers
    // -------------------------------------------------------------------------
    virtual bool Begin(const TEString &name, bool *open = nullptr, int flags = 0) = 0;
    virtual void End() = 0;
    virtual bool BeginChild(const TEString &strId, const TEVector2 &size = {0, 0}, bool border = false,
                            int flags = 0) = 0;
    virtual void EndChild() = 0;
    virtual bool BeginMenuBar() = 0;
    virtual void EndMenuBar() = 0;
    virtual bool BeginMenu(const TEString &label, bool enabled = true) = 0;
    virtual void EndMenu() = 0;
    virtual bool MenuItem(const TEString &label, const TEString &shortcut = "", bool selected = false,
                          bool enabled = true) = 0;
    virtual bool MenuItem(const TEString &label, const TEString &shortcut, bool *p_selected, bool enabled = true) = 0;

    // -------------------------------------------------------------------------
    // Basic Widgets
    // -------------------------------------------------------------------------
    virtual bool Button(const TEString &label, const TEVector2 &size = {0, 0}) = 0;
    virtual bool SmallButton(const TEString &label) = 0;
    virtual bool InvisibleButton(const TEString &strId, const TEVector2 &size, int flags = 0) = 0;
    virtual void Text(const TEString &text) = 0;
    virtual void TextUnformatted(const TEString &text) = 0;
    virtual void TextColored(const TEColor &color, const TEString &text) = 0;
    virtual void TextDisabled(const TEString &text) = 0;
    virtual void TextWrapped(const TEString &text) = 0;
    virtual bool Checkbox(const TEString &label, bool *checked) = 0;
    virtual bool RadioButton(const TEString &label, bool active) = 0;
    virtual bool RadioButton(const TEString &label, int *v, int v_button) = 0;

    // -------------------------------------------------------------------------
    // Sliders & Drags
    // -------------------------------------------------------------------------
    virtual bool SliderFloat(const TEString &label, float *v, float v_min, float v_max, const TEString &format = "%.3f",
                             int flags = 0) = 0;
    virtual bool SliderInt(const TEString &label, int *v, int v_min, int v_max, const TEString &format = "%d",
                           int flags = 0) = 0;
    virtual bool DragFloat(const TEString &label, float *value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                           const TEString &format = "%.3f", int flags = 0) = 0;
    virtual bool DragFloat2(const TEString &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                            const TEString &format = "%.3f", int flags = 0) = 0;
    virtual bool DragFloat3(const TEString &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                            const TEString &format = "%.3f", int flags = 0) = 0;
    virtual bool DragFloat4(const TEString &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                            const TEString &format = "%.3f", int flags = 0) = 0;
    virtual bool DragInt(const TEString &label, int *v, float speed = 1.0f, int min = 0, int max = 0) = 0;

    // -------------------------------------------------------------------------
    // Input Fields
    // -------------------------------------------------------------------------
    virtual bool InputText(const TEString &label, TEString &value, int flags = 0) = 0;
    virtual bool InputText(const TEString &label, char *buf, size_t bufSize, int flags = 0) = 0;
    virtual bool InputTextWithHint(const TEString &label, const TEString &hint, TEString &value, int flags = 0) = 0;
    virtual bool InputTextWithHint(const TEString &label, const char *hint, char *buf, size_t bufSize,
                                   int flags = 0) = 0;
    virtual bool InputTextMultiline(const TEString &label, TEString &value, const TEVector2 &size = {0, 0},
                                    int flags = 0) = 0;
    virtual bool InputTextMultiline(const TEString &label, char *buf, size_t bufSize, const TEVector2 &size = {0, 0},
                                    int flags = 0) = 0;
    virtual bool InputInt(const TEString &label, int *v, int step = 1, int step_fast = 100, int flags = 0) = 0;

    // -------------------------------------------------------------------------
    // Color Pickers & Edits
    // -------------------------------------------------------------------------
    virtual bool ColorEdit3(const TEString &label, float *col) = 0;
    virtual bool ColorEdit4(const TEString &label, float *col, int flags = 0) = 0;
    virtual bool ColorPicker4(const TEString &label, float *col, int flags = 0) = 0;
    virtual bool ColorButton(const TEString &desc_id, const TEVector4 &col, int flags = 0,
                             const TEVector2 &size = {0, 0}) = 0;

    // -------------------------------------------------------------------------
    // Combos & Selectables
    // -------------------------------------------------------------------------
    virtual bool Combo(const TEString &label, int *currentItem, const char *const items[], int itemsCount,
                       int popupMaxHeightInItems = -1) = 0;
    virtual bool BeginCombo(const TEString &label, const TEString &previewValue, int flags = 0) = 0;
    virtual void EndCombo() = 0;
    virtual bool Selectable(const TEString &label, bool selected = false, int flags = 0,
                            const TEVector2 &size = {0, 0}) = 0;
    virtual bool Selectable(const TEString &label, bool *selected, int flags = 0, const TEVector2 &size = {0, 0}) = 0;

    // -------------------------------------------------------------------------
    // Images
    // -------------------------------------------------------------------------
    virtual void Image(void *userTextureId, const TEVector2 &size, const TEVector2 &uv0 = {0, 0},
                       const TEVector2 &uv1 = {1, 1}, const TEVector4 &tintCol = {1, 1, 1, 1}) = 0;
    virtual bool ImageButton(const TEString &strId, void *userTextureId, const TEVector2 &size,
                             const TEVector2 &uv0 = {0, 0}, const TEVector2 &uv1 = {1, 1}) = 0;

    // -------------------------------------------------------------------------
    // Layout, Groups, Spacing
    // -------------------------------------------------------------------------
    virtual void Separator() = 0;
    virtual void SeparatorEx(int flags) = 0;
    virtual void Spacing() = 0;
    virtual void NewLine() = 0;
    virtual void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f) = 0;
    virtual void Dummy(const TEVector2 &size) = 0;
    virtual void BeginGroup() = 0;
    virtual void EndGroup() = 0;
    virtual void Indent(float indentW = 0.0f) = 0;
    virtual void Unindent(float indentW = 0.0f) = 0;
    virtual void Columns(int count = 1, const char *id = nullptr, bool border = true) = 0;
    virtual void NextColumn() = 0;
    virtual void SetColumnWidth(int columnIndex, float width) = 0;
    virtual void AlignTextToFramePadding() = 0;

    // -------------------------------------------------------------------------
    // ID & Item Stack
    // -------------------------------------------------------------------------
    virtual void PushID(const TEString &strId) = 0;
    virtual void PushID(int intId) = 0;
    virtual void PopID() = 0;
    virtual unsigned int GetID(const TEString &strId) = 0;
    virtual void PushItemWidth(float itemWidth) = 0;
    virtual void PopItemWidth() = 0;
    virtual void PushMultiItemsWidths(int components, float width_full) = 0;
    virtual float CalcItemWidth() = 0;
    virtual void BeginDisabled(bool disabled = true) = 0;
    virtual void EndDisabled() = 0;

    // -------------------------------------------------------------------------
    // Styling & Fonts
    // -------------------------------------------------------------------------
    virtual void PushStyleColor(int idx, const TEColor &color) = 0;
    virtual void PopStyleColor(int count = 1) = 0;
    virtual void PushStyleVar(int idx, float val) = 0;
    virtual void PushStyleVar(int idx, const TEVector2 &val) = 0;
    virtual void PopStyleVar(int count = 1) = 0;
    virtual void PushFont(void *font) = 0;
    virtual void PopFont() = 0;
    virtual void *GetDefaultFont() = 0;
    virtual void *GetFontAtlasTextureID() = 0;

    // -------------------------------------------------------------------------
    // Tables
    // -------------------------------------------------------------------------
    virtual bool BeginTable(const TEString &strId, int column, int flags = 0, const TEVector2 &outerSize = {0, 0},
                            float innerWidth = 0.0f) = 0;
    virtual void EndTable() = 0;
    virtual void TableSetupColumn(const TEString &label, int flags = 0, float initWidthOrWeight = 0.0f,
                                  unsigned int userId = 0) = 0;
    virtual void TableHeadersRow() = 0;
    virtual bool TableNextColumn() = 0;
    virtual void TableNextRow(float rowMinHeight = 0.0f, int rowFlags = 0) = 0;

    // -------------------------------------------------------------------------
    // Trees & Collapsing Headers
    // -------------------------------------------------------------------------
    virtual bool TreeNodeEx(const TEString &label, int flags = 0) = 0;
    virtual bool TreeNodeEx(void *ptrId, int flags, const TEString &text) = 0;
    virtual void TreePop() = 0;
    virtual bool CollapsingHeader(const TEString &label, int flags = 0) = 0;
    virtual void SetNextItemOpen(bool isOpen) = 0;

    // -------------------------------------------------------------------------
    // Popups, Tooltips, Focus & Cursor
    // -------------------------------------------------------------------------
    virtual void OpenPopup(const TEString &strId) = 0;
    virtual bool BeginPopup(const TEString &strId, int flags = 0) = 0;
    virtual bool BeginPopupContextWindow(const TEString &strId = "", int mouseButton = 1,
                                         bool alsoOverItems = true) = 0;
    virtual bool BeginPopupContextItem(const TEString &strId = "", int mouseButton = 1) = 0;
    virtual bool BeginPopupModal(const TEString &name, bool *open = nullptr, int flags = 0) = 0;
    virtual void EndPopup() = 0;
    virtual void CloseCurrentPopup() = 0;
    virtual bool IsPopupOpen(const TEString &strId) = 0;
    virtual void BeginTooltip() = 0;
    virtual void EndTooltip() = 0;
    virtual void SetTooltip(const TEString &text) = 0;
    virtual void SetItemTooltip(const TEString &text) = 0;
    virtual void SetItemDefaultFocus() = 0;
    virtual void SetMouseCursor(int cursorType) = 0;

    // -------------------------------------------------------------------------
    // Drag & Drop
    // -------------------------------------------------------------------------
    virtual bool BeginDragDropSource(int flags = 0) = 0;
    virtual bool SetDragDropPayload(const TEString &type, const void *data, size_t size, int cond = 0) = 0;
    virtual void EndDragDropSource() = 0;
    virtual bool BeginDragDropTarget() = 0;
    virtual const TimeGUIPayload *AcceptDragDropPayload(const TEString &type, int flags = 0) = 0;
    virtual void EndDragDropTarget() = 0;
    virtual const TimeGUIPayload *GetDragDropPayload() = 0;


    // -------------------------------------------------------------------------
    // Docking
    // -------------------------------------------------------------------------
    virtual unsigned int DockSpace(unsigned int id, const TEVector2 &size = {0, 0}, int flags = 0) = 0;
    virtual void DockBuilderRemoveNode(unsigned int nodeId) = 0;
    virtual void DockBuilderAddNode(unsigned int nodeId, int flags = 0) = 0;
    virtual void DockBuilderSetNodeSize(unsigned int nodeId, const TEVector2 &size) = 0;
    virtual unsigned int DockBuilderSplitNode(unsigned int nodeId, int splitDir, float sizeRatio,
                                              unsigned int *outIdDir1, unsigned int *outIdDir2) = 0;
    virtual void DockBuilderDockWindow(const TEString &windowName, unsigned int nodeId) = 0;
    virtual void DockBuilderFinish(unsigned int nodeId) = 0;

    // -------------------------------------------------------------------------
    // Tabs & Cards
    // -------------------------------------------------------------------------
    virtual bool BeginTabItem(const TEString &label, bool *open = nullptr, int flags = 0) = 0;
    virtual void EndTabItem() = 0;
    virtual bool BeginTabBar(const TEString &strId, int flags = 0) = 0;
    virtual void EndTabBar() = 0;
    virtual bool BeginProjectCard(const TEString &id, const TEVector2 &size, bool &hovered) = 0;
    virtual void EndProjectCard() = 0;

    // -------------------------------------------------------------------------
    // Next Window & Item Controls
    // -------------------------------------------------------------------------
    virtual void SetNextWindowPos(const TEVector2 &pos, int cond = 0, const TEVector2 &pivot = {0, 0}) = 0;
    virtual void SetNextWindowSize(const TEVector2 &size, int cond = 0) = 0;
    virtual void SetNextWindowContentSize(const TEVector2 &size) = 0;
    virtual void SetNextWindowDockID(unsigned int dockId, int cond = 0) = 0;
    virtual void SetNextWindowViewport(unsigned int viewportId) = 0;
    virtual void SetNextItemWidth(float itemWidth) = 0;
    virtual void SetNextItemAllowOverlap() = 0;

    // -------------------------------------------------------------------------
    // State, Queries & Input
    // -------------------------------------------------------------------------
    virtual TEVector2 GetCursorPos() = 0;
    virtual void SetCursorPos(const TEVector2 &pos) = 0;
    virtual float GetCursorPosX() = 0;
    virtual float GetCursorPosY() = 0;
    virtual void SetCursorPosX(float x) = 0;
    virtual void SetCursorPosY(float y) = 0;
    virtual TEVector2 GetCursorScreenPos() = 0;
    virtual void SetCursorScreenPos(const TEVector2 &pos) = 0;
    virtual TEVector2 GetWindowSize() = 0;
    virtual TEVector2 GetWindowPos() = 0;
    virtual float GetWindowWidth() = 0;
    virtual float GetWindowHeight() = 0;
    virtual TEVector2 GetContentRegionAvail() = 0;
    virtual float GetFrameHeight() = 0;
    virtual TEVector2 GetWindowContentRegionMin() = 0;
    virtual TEVector2 GetWindowContentRegionMax() = 0;
    virtual void SetScrollHereY(float centerYRatio = 0.5f) = 0;
    virtual float GetScrollY() = 0;
    virtual float GetScrollMaxY() = 0;
    virtual void SetScrollY(float scrollY) = 0;
    virtual TEArray<unsigned int> GetInputQueueCharacters() = 0;
    virtual void ClearInputQueueCharacters() = 0;
    virtual TEVector2 GetMousePos() = 0;
    virtual bool IsMouseDown(int button) = 0;
    virtual bool IsMouseReleased(int button) = 0;
    virtual bool IsMouseClicked(int button, bool repeat = false) = 0;
    virtual bool IsMouseDoubleClicked(int button) = 0;
    virtual bool IsMouseDragging(int button, float lock_threshold = -1.0f) = 0;
    virtual TEVector2 GetMouseDragDelta(int button = 0, float lock_threshold = -1.0f) = 0;
    virtual void ResetMouseDragDelta(int button = 0) = 0;
    virtual bool IsAnyItemHovered() = 0;
    virtual bool IsKeyPressed(int key) = 0;
    virtual double GetTime() = 0;
    virtual TEVector2 CalcTextSize(const TEString &text) = 0;
    virtual bool IsItemDeactivatedAfterEdit() = 0;
    virtual bool IsItemHovered(int flags = 0) = 0;
    virtual bool IsItemActive() = 0;
    virtual bool IsItemFocused() = 0;
    virtual bool IsItemClicked(int mouse_button = 0) = 0;
    virtual bool IsWindowHovered(int flags = 0) = 0;
    virtual bool IsWindowFocused(int flags = 0) = 0;
    virtual bool IsWindowAppearing() = 0;
    virtual TEVector2 GetItemRectMin() = 0;
    virtual TEVector2 GetItemRectSize() = 0;
    virtual TEVector2 GetItemRectMax() = 0;
    virtual void SetKeyboardFocusHere(int offset = 0) = 0;
    virtual void SetClipboardText(const TEString &text) = 0;
    virtual TEString GetClipboardText() = 0;
    virtual void SetCaretColor(const TEVector4 &color) = 0;
    virtual TEVector4 GetCaretColor() = 0;
    virtual void SaveIniSettingsToDisk(const char *ini_filename) = 0;
    virtual void LoadIniSettingsFromDisk(const char *ini_filename) = 0;

    // -------------------------------------------------------------------------
    // DrawList Commands
    // -------------------------------------------------------------------------
    virtual void DrawListAddLine(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                 float thickness) = 0;
    virtual void DrawListAddRect(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                 float rounding, int flags, float thickness) = 0;
    virtual void DrawListAddRectFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                       float rounding) = 0;
    virtual void DrawListAddCircle(void *drawList, const TEVector2 &center, float radius, unsigned int color,
                                   int num_segments, float thickness) = 0;
    virtual void DrawListAddCircleFilled(void *drawList, const TEVector2 &center, float radius, unsigned int color,
                                         int num_segments) = 0;
    virtual void DrawListAddText(void *drawList, const TEVector2 &pos, unsigned int color, const TEString &text) = 0;
    virtual void DrawListAddText(void *drawList, void *font, float fontSize, const TEVector2 &pos, unsigned int color,
                                 const TEString &text) = 0;
    virtual void DrawListAddImage(void *drawList, void *texture_id, const TEVector2 &p_min, const TEVector2 &p_max,
                                  const TEVector2 &uv_min, const TEVector2 &uv_max, unsigned int col) = 0;
    virtual void DrawListAddBezierCubic(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                        const TEVector2 &p4, unsigned int color, float thickness, int num_segments) = 0;
    virtual void DrawListPushClipRect(void *drawList, const TEVector2 &clip_rect_min, const TEVector2 &clip_rect_max,
                                      bool intersect_with_current_clip_rect) = 0;
    virtual void DrawListPopClipRect(void *drawList) = 0;
    virtual void DrawListAddPolyline(void *drawList, const TEVector2 *points, int num_points, unsigned int color,
                                     int flags, float thickness) = 0;
    virtual void DrawListAddConvexPolyFilled(void *drawList, const TEVector2 *points, int num_points,
                                             unsigned int color) = 0;
    virtual void DrawListAddTriangleFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2,
                                           const TEVector2 &p3, unsigned int color) = 0;
    virtual void DrawListAddQuadFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                       const TEVector2 &p4, unsigned int color) = 0;
    virtual void DrawListPushTextureID(void *drawList, void *texture_id) = 0;
    virtual void *DrawListCreate() = 0;
    virtual void DrawListDestroy(void *drawList) = 0;
    virtual void DrawListRender(void *drawList, const TEVector2 &displaySize) = 0;
    virtual int DrawListGetVertexCount(void *drawList) = 0;
    virtual int DrawListGetCommandCount(void *drawList) = 0;

    // -------------------------------------------------------------------------
    // Factory Method (Mirrors RendererAPI::Create)
    // -------------------------------------------------------------------------
    static TEScope<UIAPI> Create(UIBackendType type);
};

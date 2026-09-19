#pragma once
#include "PreRequisites.h"
#include "GameplayUtils.hpp"
#include "UIAPI.hpp"

/// ForgeUI concrete UI backend implementation.
/// Sole location where ForgeUI vendor headers and symbols are used.
class ForgeUIAPI final : public UIAPI
{
public:
    ForgeUIAPI() = default;
    ~ForgeUIAPI() override = default;

    UIBackendType GetType() const override { return UIBackendType::ForgeUI; }

    bool Init(void *nativeWindow) override;
    void Shutdown() override;
    bool InitRenderBackend() override;
    void ShutdownRenderBackend() override;

    void BindThreadContext() override;

    void PrepareFrame() override;
    void BeginFrame(uint32_t width, uint32_t height) override;
    void *EndFrame(uint32_t width, uint32_t height) override;
    void RenderDrawData(void *drawData) override;

    bool SupportsDocking() const override { return false; }
    bool SupportsViewports() const override { return false; }

    UIIO &GetIO() override { return m_IO; }
    UIStyle &GetStyle() override { return m_Style; }
    void SetStyle(const UIStyle &style) override { m_Style = style; }

    TEVector2 GetMainViewportPos() override;
    TEVector2 GetMainViewportSize() override;
    uint32_t GetMainViewportID() override;

    void *GetBackgroundDrawList() override;
    void *GetForegroundDrawList() override;
    void *GetWindowDrawList() override;

    // Windows & Containers
    bool Begin(const TEString &name, bool *open = nullptr, int flags = 0) override;
    void End() override;
    bool BeginChild(const TEString &strId, const TEVector2 &size = {0, 0}, bool border = false, int flags = 0) override;
    void EndChild() override;
    bool BeginMenuBar() override;
    void EndMenuBar() override;
    bool BeginMenu(const TEString &label, bool enabled = true) override;
    void EndMenu() override;
    bool MenuItem(const TEString &label, const TEString &shortcut = "", bool selected = false,
                  bool enabled = true) override;
    bool MenuItem(const TEString &label, const TEString &shortcut, bool *p_selected, bool enabled = true) override;

    // Basic Widgets
    bool Button(const TEString &label, const TEVector2 &size = {0, 0}) override;
    bool SmallButton(const TEString &label) override;
    bool InvisibleButton(const TEString &strId, const TEVector2 &size, int flags = 0) override;
    void Text(const TEString &text) override;
    void TextUnformatted(const TEString &text) override;
    void TextColored(const TEColor &color, const TEString &text) override;
    void TextDisabled(const TEString &text) override;
    void TextWrapped(const TEString &text) override;
    bool Checkbox(const TEString &label, bool *checked) override;
    bool RadioButton(const TEString &label, bool active) override;
    bool RadioButton(const TEString &label, int *v, int v_button) override;

    // Sliders & Drags
    bool SliderFloat(const TEString &label, float *v, float v_min, float v_max, const TEString &format = "%.3f",
                     int flags = 0) override;
    bool SliderInt(const TEString &label, int *v, int v_min, int v_max, const TEString &format = "%d",
                   int flags = 0) override;
    bool DragFloat(const TEString &label, float *value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                   const TEString &format = "%.3f", int flags = 0) override;
    bool DragFloat2(const TEString &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                    const TEString &format = "%.3f", int flags = 0) override;
    bool DragFloat3(const TEString &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                    const TEString &format = "%.3f", int flags = 0) override;
    bool DragFloat4(const TEString &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                    const TEString &format = "%.3f", int flags = 0) override;
    bool DragInt(const TEString &label, int *v, float speed = 1.0f, int min = 0, int max = 0) override;

    // Input Fields
    bool InputText(const TEString &label, TEString &value, int flags = 0) override;
    bool InputText(const TEString &label, char *buf, size_t bufSize, int flags = 0) override;
    bool InputTextWithHint(const TEString &label, const TEString &hint, TEString &value, int flags = 0) override;
    bool InputTextWithHint(const TEString &label, const char *hint, char *buf, size_t bufSize, int flags = 0) override;
    bool InputTextMultiline(const TEString &label, TEString &value, const TEVector2 &size = {0, 0},
                            int flags = 0) override;
    bool InputTextMultiline(const TEString &label, char *buf, size_t bufSize, const TEVector2 &size = {0, 0},
                            int flags = 0) override;
    bool InputInt(const TEString &label, int *v, int step = 1, int step_fast = 100, int flags = 0) override;

    // Color Pickers & Edits
    bool ColorEdit3(const TEString &label, float *col) override;
    bool ColorEdit4(const TEString &label, float *col, int flags = 0) override;
    bool ColorPicker4(const TEString &label, float *col, int flags = 0) override;
    bool ColorButton(const TEString &desc_id, const TEVector4 &col, int flags = 0,
                     const TEVector2 &size = {0, 0}) override;

    // Combos & Selectables
    bool Combo(const TEString &label, int *currentItem, const char *const items[], int itemsCount,
               int popupMaxHeightInItems = -1) override;
    bool BeginCombo(const TEString &label, const TEString &previewValue, int flags = 0) override;
    void EndCombo() override;
    bool Selectable(const TEString &label, bool selected = false, int flags = 0,
                    const TEVector2 &size = {0, 0}) override;
    bool Selectable(const TEString &label, bool *selected, int flags = 0, const TEVector2 &size = {0, 0}) override;

    // Images
    void Image(void *userTextureId, const TEVector2 &size, const TEVector2 &uv0 = {0, 0}, const TEVector2 &uv1 = {1, 1},
               const TEVector4 &tintCol = {1, 1, 1, 1}) override;
    bool ImageButton(const TEString &strId, void *userTextureId, const TEVector2 &size, const TEVector2 &uv0 = {0, 0},
                     const TEVector2 &uv1 = {1, 1}) override;

    // Layout, Groups, Spacing
    void Separator() override;
    void SeparatorEx(int flags) override;
    void Spacing() override;
    void NewLine() override;
    void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f) override;
    void Dummy(const TEVector2 &size) override;
    void BeginGroup() override;
    void EndGroup() override;
    void Indent(float indentW = 0.0f) override;
    void Unindent(float indentW = 0.0f) override;
    void Columns(int count = 1, const char *id = nullptr, bool border = true) override;
    void NextColumn() override;
    void SetColumnWidth(int columnIndex, float width) override;
    void AlignTextToFramePadding() override;

    // ID & Item Stack
    void PushID(const TEString &strId) override;
    void PushID(int intId) override;
    void PopID() override;
    unsigned int GetID(const TEString &strId) override;
    void PushItemWidth(float itemWidth) override;
    void PopItemWidth() override;
    void PushMultiItemsWidths(int components, float width_full) override;
    float CalcItemWidth() override;
    void BeginDisabled(bool disabled = true) override;
    void EndDisabled() override;

    // Styling & Fonts
    void PushStyleColor(int idx, const TEColor &color) override;
    void PopStyleColor(int count = 1) override;
    void PushStyleVar(int idx, float val) override;
    void PushStyleVar(int idx, const TEVector2 &val) override;
    void PopStyleVar(int count = 1) override;
    void PushFont(void *font) override;
    void PopFont() override;
    void *GetDefaultFont() override;
    void *GetFontAtlasTextureID() override;

    // Tables
    bool BeginTable(const TEString &strId, int column, int flags = 0, const TEVector2 &outerSize = {0, 0},
                    float innerWidth = 0.0f) override;
    void EndTable() override;
    void TableSetupColumn(const TEString &label, int flags = 0, float initWidthOrWeight = 0.0f,
                          unsigned int userId = 0) override;
    void TableHeadersRow() override;
    bool TableNextColumn() override;
    void TableNextRow(float rowMinHeight = 0.0f, int rowFlags = 0) override;

    // Trees & Collapsing Headers
    bool TreeNodeEx(const TEString &label, int flags = 0) override;
    bool TreeNodeEx(void *ptrId, int flags, const TEString &text) override;
    void TreePop() override;
    bool CollapsingHeader(const TEString &label, int flags = 0) override;
    void SetNextItemOpen(bool isOpen) override;

    // Popups, Tooltips, Focus & Cursor
    void OpenPopup(const TEString &strId) override;
    bool BeginPopup(const TEString &strId, int flags = 0) override;
    bool BeginPopupContextWindow(const TEString &strId = "", int mouseButton = 1, bool alsoOverItems = true) override;
    bool BeginPopupContextItem(const TEString &strId = "", int mouseButton = 1) override;
    bool BeginPopupModal(const TEString &name, bool *open = nullptr, int flags = 0) override;
    void EndPopup() override;
    void CloseCurrentPopup() override;
    bool IsPopupOpen(const TEString &strId) override;
    void BeginTooltip() override;
    void EndTooltip() override;
    void SetTooltip(const TEString &text) override;
    void SetItemTooltip(const TEString &text) override;
    void SetItemDefaultFocus() override;
    void SetMouseCursor(int cursorType) override;

    // Drag & Drop
    bool BeginDragDropSource(int flags = 0) override;
    bool SetDragDropPayload(const TEString &type, const void *data, size_t size, int cond = 0) override;
    void EndDragDropSource() override;
    bool BeginDragDropTarget() override;
    const TimeGUIPayload *AcceptDragDropPayload(const TEString &type, int flags = 0) override;
    void EndDragDropTarget() override;
    const TimeGUIPayload *GetDragDropPayload() override;

    // Docking
    unsigned int DockSpace(unsigned int id, const TEVector2 &size = {0, 0}, int flags = 0) override;
    void DockBuilderRemoveNode(unsigned int nodeId) override;
    void DockBuilderAddNode(unsigned int nodeId, int flags = 0) override;
    void DockBuilderSetNodeSize(unsigned int nodeId, const TEVector2 &size) override;
    unsigned int DockBuilderSplitNode(unsigned int nodeId, int splitDir, float sizeRatio, unsigned int *outIdDir1,
                                      unsigned int *outIdDir2) override;
    void DockBuilderDockWindow(const TEString &windowName, unsigned int nodeId) override;
    void DockBuilderFinish(unsigned int nodeId) override;

    // Tabs & Cards
    bool BeginTabItem(const TEString &label, bool *open = nullptr, int flags = 0) override;
    void EndTabItem() override;
    bool BeginTabBar(const TEString &strId, int flags = 0) override;
    void EndTabBar() override;
    bool BeginProjectCard(const TEString &id, const TEVector2 &size, bool &hovered) override;
    void EndProjectCard() override;

    // Next Window & Item Controls
    void SetNextWindowPos(const TEVector2 &pos, int cond = 0, const TEVector2 &pivot = {0, 0}) override;
    void SetNextWindowSize(const TEVector2 &size, int cond = 0) override;
    void SetNextWindowContentSize(const TEVector2 &size) override;
    void SetNextWindowDockID(unsigned int dockId, int cond = 0) override;
    void SetNextWindowViewport(unsigned int viewportId) override;
    void SetNextItemWidth(float itemWidth) override;
    void SetNextItemAllowOverlap() override;

    // State, Queries & Input
    TEVector2 GetCursorPos() override;
    void SetCursorPos(const TEVector2 &pos) override;
    float GetCursorPosX() override;
    float GetCursorPosY() override;
    void SetCursorPosX(float x) override;
    void SetCursorPosY(float y) override;
    TEVector2 GetCursorScreenPos() override;
    void SetCursorScreenPos(const TEVector2 &pos) override;
    TEVector2 GetWindowSize() override;
    TEVector2 GetWindowPos() override;
    float GetWindowWidth() override;
    float GetWindowHeight() override;
    TEVector2 GetContentRegionAvail() override;
    float GetFrameHeight() override;
    TEVector2 GetWindowContentRegionMin() override;
    TEVector2 GetWindowContentRegionMax() override;
    void SetScrollHereY(float centerYRatio = 0.5f) override;
    float GetScrollY() override;
    float GetScrollMaxY() override;
    void SetScrollY(float scrollY) override;
    TEArray<unsigned int> GetInputQueueCharacters() override;
    void ClearInputQueueCharacters() override;
    TEVector2 GetMousePos() override;
    bool IsMouseDown(int button) override;
    bool IsMouseReleased(int button) override;
    bool IsMouseClicked(int button, bool repeat = false) override;
    bool IsMouseDoubleClicked(int button) override;
    bool IsMouseDragging(int button, float lock_threshold = -1.0f) override;
    TEVector2 GetMouseDragDelta(int button = 0, float lock_threshold = -1.0f) override;
    void ResetMouseDragDelta(int button = 0) override;
    bool IsAnyItemHovered() override;
    bool IsKeyPressed(int key) override;
    double GetTime() override;
    TEVector2 CalcTextSize(const TEString &text) override;
    bool IsItemDeactivatedAfterEdit() override;
    bool IsItemHovered(int flags = 0) override;
    bool IsItemActive() override;
    bool IsItemFocused() override;
    bool IsItemClicked(int mouse_button = 0) override;
    bool IsWindowHovered(int flags = 0) override;
    bool IsWindowFocused(int flags = 0) override;
    bool IsWindowAppearing() override;
    TEVector2 GetItemRectMin() override;
    TEVector2 GetItemRectSize() override;
    TEVector2 GetItemRectMax() override;
    void SetKeyboardFocusHere(int offset = 0) override;
    void SetClipboardText(const TEString &text) override;
    TEString GetClipboardText() override;
    void SetCaretColor(const TEVector4 &color) override;
    TEVector4 GetCaretColor() override;
    void SaveIniSettingsToDisk(const char *ini_filename) override;
    void LoadIniSettingsFromDisk(const char *ini_filename) override;

    // DrawList Commands
    void DrawListAddLine(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                         float thickness) override;
    void DrawListAddRect(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding,
                         int flags, float thickness) override;
    void DrawListAddRectFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                               float rounding) override;
    void DrawListAddCircle(void *drawList, const TEVector2 &center, float radius, unsigned int color, int num_segments,
                           float thickness) override;
    void DrawListAddCircleFilled(void *drawList, const TEVector2 &center, float radius, unsigned int color,
                                 int num_segments) override;
    void DrawListAddText(void *drawList, const TEVector2 &pos, unsigned int color, const TEString &text) override;
    void DrawListAddText(void *drawList, void *font, float fontSize, const TEVector2 &pos, unsigned int color,
                         const TEString &text) override;
    void DrawListAddImage(void *drawList, void *texture_id, const TEVector2 &p_min, const TEVector2 &p_max,
                          const TEVector2 &uv_min, const TEVector2 &uv_max, unsigned int col) override;
    void DrawListAddBezierCubic(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                const TEVector2 &p4, unsigned int color, float thickness, int num_segments) override;
    void DrawListPushClipRect(void *drawList, const TEVector2 &clip_rect_min, const TEVector2 &clip_rect_max,
                              bool intersect_with_current_clip_rect) override;
    void DrawListPopClipRect(void *drawList) override;
    void DrawListAddPolyline(void *drawList, const TEVector2 *points, int num_points, unsigned int color, int flags,
                             float thickness) override;
    void DrawListAddConvexPolyFilled(void *drawList, const TEVector2 *points, int num_points,
                                     unsigned int color) override;
    void DrawListAddTriangleFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                   unsigned int color) override;
    void DrawListAddQuadFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                               const TEVector2 &p4, unsigned int color) override;
    void DrawListPushTextureID(void *drawList, void *texture_id) override;
    void *DrawListCreate() override;
    void DrawListDestroy(void *drawList) override;
    void DrawListRender(void *drawList, const TEVector2 &displaySize) override;
    int DrawListGetVertexCount(void *drawList) override;
    int DrawListGetCommandCount(void *drawList) override;

private:
    void *m_Context = nullptr; // forge::ForgeContext*
    void *m_Backend = nullptr; // forge::IForgeBackend*
    bool m_Initialized = false;
    bool m_RenderInitialized = false;
    void *m_NativeWindow = nullptr;
    uint32_t m_Width = 1280;
    uint32_t m_Height = 720;
    TEVector2 m_NextWindowPos = {0.0f, 0.0f};
    TEVector2 m_NextWindowSize = {400.0f, 300.0f};
    bool m_HasNextWindowPos = false;
    bool m_HasNextWindowSize = false;

    struct ColumnState
    {
        int count = 1;
        int current = 0;
        float widths[16] = {};
        float startX[16] = {};
        float columnStartY = 0.0f;
        float maxCursorY = 0.0f;
        bool active = false;
    };

    struct WindowLayoutState
    {
        TEVector2 pos = {0.0f, 0.0f};
        TEVector2 size = {400.0f, 300.0f};
        TEVector2 cursor = {12.0f, 36.0f};
        float startX = 12.0f;
        float startY = 36.0f;
        float lineHeight = 0.0f;
        bool sameLine = false;
        float sameLineSpacing = 8.0f;
        bool isChild = false;
        ColumnState columns;
    };

    TEArray<WindowLayoutState> m_WindowStack;
    TEVector2 m_LastItemMin = {0.0f, 0.0f};
    TEVector2 m_LastItemMax = {0.0f, 0.0f};
    TEVector2 m_LastItemSize = {0.0f, 0.0f};

    // Style stacks
    TEArray<TEVector4> m_ChildBgColors;
    TEArray<TEVector4> m_ButtonColors;
    TEArray<float> m_FrameRoundings;

    bool m_PrevMouseDown[3] = {false, false, false};

    WindowLayoutState *CurrentWindowLayout();
    TEVector2 AllocateItem(float width, float height);
    void DrawString(float x, float y, unsigned int color, const TEString &text, float fontSize = 14.0f);

    UIIO m_IO;
    UIStyle m_Style;
};

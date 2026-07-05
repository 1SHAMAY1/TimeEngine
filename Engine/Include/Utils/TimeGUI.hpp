#pragma once
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"
#include <string>

// Forward declaration of raw global ImDrawList and ImFont to keep headers clean and allow operator-> mapping
struct ImDrawList;
struct ImFont;

#ifndef IM_COL32
#define IM_COL32(R, G, B, A)                                                                                           \
    (((unsigned int)(A) << 24) | ((unsigned int)(B) << 16) | ((unsigned int)(G) << 8) | ((unsigned int)(R)))
#endif

#ifndef IM_COL32_WHITE
#define IM_COL32_WHITE IM_COL32(255, 255, 255, 255)
#endif
#ifndef IM_COL32_BLACK
#define IM_COL32_BLACK IM_COL32(0, 0, 0, 255)
#endif

// Convenience alias – matches ImU32 so LogoLayer / draw code can use unsigned int directly
typedef unsigned int TimeGUIColor32;

namespace TE
{

namespace TimeGUI
{
// Clean, non-vendor TextureID type
typedef void *TimeGUITextureID;

// Clean, non-vendor Window Flags wrapper
enum TimeGUIWindowFlags_
{
    TimeGUIWindowFlags_None = 0,
    TimeGUIWindowFlags_NoTitleBar = 1 << 0,
    TimeGUIWindowFlags_NoResize = 1 << 1,
    TimeGUIWindowFlags_NoMove = 1 << 2,
    TimeGUIWindowFlags_NoScrollbar = 1 << 3,
    TimeGUIWindowFlags_NoScrollWithMouse = 1 << 4,
    TimeGUIWindowFlags_NoCollapse = 1 << 5,
    TimeGUIWindowFlags_AlwaysAutoResize = 1 << 6,
    TimeGUIWindowFlags_NoBackground = 1 << 7,
    TimeGUIWindowFlags_NoSavedSettings = 1 << 8,
    TimeGUIWindowFlags_NoMouseInputs = 1 << 9,
    TimeGUIWindowFlags_MenuBar = 1 << 10,
    TimeGUIWindowFlags_HorizontalScrollbar = 1 << 11,
    TimeGUIWindowFlags_NoFocusOnAppearing = 1 << 12,
    TimeGUIWindowFlags_NoBringToFrontOnFocus = 1 << 13,
    TimeGUIWindowFlags_AlwaysVerticalScrollbar = 1 << 14,
    TimeGUIWindowFlags_AlwaysHorizontalScrollbar = 1 << 15,
    TimeGUIWindowFlags_NoNavInputs = 1 << 16,
    TimeGUIWindowFlags_NoNavFocus = 1 << 17,
    TimeGUIWindowFlags_UnsavedDocument = 1 << 18,
    TimeGUIWindowFlags_NoDocking = 1 << 19,
    TimeGUIWindowFlags_NoNav = TimeGUIWindowFlags_NoNavInputs | TimeGUIWindowFlags_NoNavFocus,
    TimeGUIWindowFlags_NoDecoration = TimeGUIWindowFlags_NoTitleBar | TimeGUIWindowFlags_NoResize |
                                      TimeGUIWindowFlags_NoScrollbar | TimeGUIWindowFlags_NoCollapse,
    TimeGUIWindowFlags_NoInputs =
        TimeGUIWindowFlags_NoMouseInputs | TimeGUIWindowFlags_NoNavInputs | TimeGUIWindowFlags_NoNavFocus,
};
typedef int TimeGUIWindowFlags;

// DrawList poly-line close flag wrapper
enum TimeGUIDrawFlags_
{
    TimeGUIDrawFlags_None = 0,
    TimeGUIDrawFlags_Closed = 1 << 0, // Matches ImDrawFlags_Closed
};
typedef int TimeGUIDrawFlags;

// Color edit flags wrapper
enum TimeGUIColorEditFlags_
{
    TimeGUIColorEditFlags_None = 0,
    TimeGUIColorEditFlags_NoAlpha = 1 << 1,
    TimeGUIColorEditFlags_NoPicker = 1 << 2,
    TimeGUIColorEditFlags_NoOptions = 1 << 3,
    TimeGUIColorEditFlags_NoSmallPreview = 1 << 4,
    TimeGUIColorEditFlags_NoInputs = 1 << 5,
    TimeGUIColorEditFlags_NoTooltip = 1 << 6,
    TimeGUIColorEditFlags_NoLabel = 1 << 7,
    TimeGUIColorEditFlags_NoSidePreview = 1 << 8,
    TimeGUIColorEditFlags_NoDragDrop = 1 << 9,
    TimeGUIColorEditFlags_NoBorder = 1 << 10,
    TimeGUIColorEditFlags_AlphaBar = 1 << 16,
    TimeGUIColorEditFlags_AlphaPreview = 1 << 17,
    TimeGUIColorEditFlags_AlphaPreviewHalf = 1 << 18,
    TimeGUIColorEditFlags_DisplayRGB = 1 << 20,
    TimeGUIColorEditFlags_InputRGB = 1 << 24,
};
typedef int TimeGUIColorEditFlags;

// Mouse button constants
enum TimeGUIMouseButton_
{
    TimeGUIMouseButton_Left = 0,
    TimeGUIMouseButton_Right = 1,
    TimeGUIMouseButton_Middle = 2,
};

// Keyboard keys constants matching ImGuiKey values
enum TimeGUIKey_
{
    TimeGUIKey_None = 0,
    TimeGUIKey_Tab = 512,
    TimeGUIKey_LeftArrow = 513,
    TimeGUIKey_RightArrow = 514,
    TimeGUIKey_UpArrow = 515,
    TimeGUIKey_DownArrow = 516,
    TimeGUIKey_PageUp = 517,
    TimeGUIKey_PageDown = 518,
    TimeGUIKey_Home = 519,
    TimeGUIKey_End = 520,
    TimeGUIKey_Insert = 521,
    TimeGUIKey_Delete = 522,
    TimeGUIKey_Backspace = 523,
    TimeGUIKey_Space = 524,
    TimeGUIKey_Enter = 525,
    TimeGUIKey_Escape = 526,
    TimeGUIKey_A = 542,
    TimeGUIKey_B = 543,
    TimeGUIKey_C = 544,
    TimeGUIKey_D = 545,
    TimeGUIKey_E = 546,
    TimeGUIKey_F = 547,
    TimeGUIKey_G = 548,
    TimeGUIKey_H = 549,
    TimeGUIKey_I = 550,
    TimeGUIKey_J = 551,
    TimeGUIKey_K = 552,
    TimeGUIKey_L = 553,
    TimeGUIKey_M = 554,
    TimeGUIKey_N = 555,
    TimeGUIKey_O = 556,
    TimeGUIKey_P = 557,
    TimeGUIKey_Q = 558,
    TimeGUIKey_R = 559,
    TimeGUIKey_S = 560,
    TimeGUIKey_T = 561,
    TimeGUIKey_U = 562,
    TimeGUIKey_V = 563,
    TimeGUIKey_W = 564,
    TimeGUIKey_X = 565,
    TimeGUIKey_Y = 566,
    TimeGUIKey_Z = 567,
    TimeGUIKey_F2 = 583,
};
typedef int TimeGUIKey;

// Clean, non-vendor Color style wrapper
enum TimeGUICol_
{
    TimeGUICol_Text,
    TimeGUICol_TextDisabled,
    TimeGUICol_WindowBg,
    TimeGUICol_ChildBg,
    TimeGUICol_PopupBg,
    TimeGUICol_Border,
    TimeGUICol_BorderShadow,
    TimeGUICol_FrameBg,
    TimeGUICol_FrameBgHovered,
    TimeGUICol_FrameBgActive,
    TimeGUICol_TitleBg,
    TimeGUICol_TitleBgActive,
    TimeGUICol_TitleBgCollapsed,
    TimeGUICol_MenuBarBg,
    TimeGUICol_ScrollbarBg,
    TimeGUICol_ScrollbarGrab,
    TimeGUICol_ScrollbarGrabHovered,
    TimeGUICol_ScrollbarGrabActive,
    TimeGUICol_CheckMark,
    TimeGUICol_SliderGrab,
    TimeGUICol_SliderGrabActive,
    TimeGUICol_Button,
    TimeGUICol_ButtonHovered,
    TimeGUICol_ButtonActive,
    TimeGUICol_Header,
    TimeGUICol_HeaderHovered,
    TimeGUICol_HeaderActive,
    TimeGUICol_Separator,
    TimeGUICol_SeparatorHovered,
    TimeGUICol_SeparatorActive,
    TimeGUICol_ResizeGrip,
    TimeGUICol_ResizeGripHovered,
    TimeGUICol_ResizeGripActive,
    TimeGUICol_InputTextCursor,
    TimeGUICol_TabHovered,
    TimeGUICol_Tab,
    TimeGUICol_TabSelected,
    TimeGUICol_TabSelectedOverline,
    TimeGUICol_TabDimmed,
    TimeGUICol_TabDimmedSelected,
    TimeGUICol_TabDimmedSelectedOverline,
    TimeGUICol_DockingPreview,
    TimeGUICol_DockingEmptyBg,
    TimeGUICol_PlotLines,
    TimeGUICol_PlotLinesHovered,
    TimeGUICol_PlotHistogram,
    TimeGUICol_PlotHistogramHovered,
    TimeGUICol_TableHeaderBg,
    TimeGUICol_TableBorderStrong,
    TimeGUICol_TableBorderLight,
    TimeGUICol_TableRowBg,
    TimeGUICol_TableRowBgAlt,
    TimeGUICol_TextLink,
    TimeGUICol_TextSelectedBg,
    TimeGUICol_TreeLines,
    TimeGUICol_DragDropTarget,
    TimeGUICol_DragDropTargetBg,
    TimeGUICol_UnsavedMarker,
    TimeGUICol_NavCursor,
    TimeGUICol_NavWindowingHighlight,
    TimeGUICol_NavWindowingDimBg,
    TimeGUICol_ModalWindowDimBg,
    TimeGUICol_COUNT,

    TimeGUICol_TabActive = TimeGUICol_TabSelected,
    TimeGUICol_TabUnfocused = TimeGUICol_TabDimmed,
    TimeGUICol_TabUnfocusedActive = TimeGUICol_TabDimmedSelected,
    TimeGUICol_NavHighlight = TimeGUICol_NavCursor,
};
typedef int TimeGUICol;

// Clean, non-vendor Style variable wrapper
enum TimeGUIStyleVar_
{
    TimeGUIStyleVar_Alpha,
    TimeGUIStyleVar_DisabledAlpha,
    TimeGUIStyleVar_WindowPadding,
    TimeGUIStyleVar_WindowRounding,
    TimeGUIStyleVar_WindowBorderSize,
    TimeGUIStyleVar_WindowMinSize,
    TimeGUIStyleVar_WindowTitleAlign,
    TimeGUIStyleVar_ChildRounding,
    TimeGUIStyleVar_ChildBorderSize,
    TimeGUIStyleVar_PopupRounding,
    TimeGUIStyleVar_PopupBorderSize,
    TimeGUIStyleVar_FramePadding,
    TimeGUIStyleVar_FrameRounding,
    TimeGUIStyleVar_FrameBorderSize,
    TimeGUIStyleVar_ItemSpacing,
    TimeGUIStyleVar_ItemInnerSpacing,
    TimeGUIStyleVar_IndentSpacing,
    TimeGUIStyleVar_CellPadding,
    TimeGUIStyleVar_ScrollbarSize,
    TimeGUIStyleVar_ScrollbarRounding,
    TimeGUIStyleVar_ScrollbarPadding,
    TimeGUIStyleVar_GrabMinSize,
    TimeGUIStyleVar_GrabRounding,
    TimeGUIStyleVar_ImageRounding,
    TimeGUIStyleVar_ImageBorderSize,
    TimeGUIStyleVar_TabRounding,
    TimeGUIStyleVar_TabBorderSize,
    TimeGUIStyleVar_TabMinWidthBase,
    TimeGUIStyleVar_TabMinWidthShrink,
    TimeGUIStyleVar_TabBarBorderSize,
    TimeGUIStyleVar_TabBarOverlineSize,
    TimeGUIStyleVar_COUNT
};
typedef int TimeGUIStyleVar;

// Clean, non-vendor Condition wrapper
enum TimeGUICond_
{
    TimeGUICond_None = 0,
    TimeGUICond_Always = 1 << 0,
    TimeGUICond_Once = 1 << 1,
    TimeGUICond_FirstUseEver = 1 << 2,
    TimeGUICond_Appearing = 1 << 3
};
typedef int TimeGUICond;

// Hovered flags wrapper
enum TimeGUIHoveredFlags_
{
    TimeGUIHoveredFlags_None = 0,
    TimeGUIHoveredFlags_ChildWindows = 1 << 0,
    TimeGUIHoveredFlags_RootWindow = 1 << 1,
    TimeGUIHoveredFlags_AnyWindow = 1 << 2,
    TimeGUIHoveredFlags_NoPopupHierarchy = 1 << 3,
    TimeGUIHoveredFlags_DockHierarchy = 1 << 4,
    TimeGUIHoveredFlags_AllowWhenBlockedByPopup = 1 << 5,
    TimeGUIHoveredFlags_AllowWhenBlockedByActiveItem = 1 << 7,
    TimeGUIHoveredFlags_AllowWhenOverlapped = 1 << 8,
    TimeGUIHoveredFlags_AllowWhenDisabled = 1 << 9,
    TimeGUIHoveredFlags_NoNavOverride = 1 << 10,
    TimeGUIHoveredFlags_RectOnly = TimeGUIHoveredFlags_AllowWhenBlockedByPopup |
                                   TimeGUIHoveredFlags_AllowWhenBlockedByActiveItem |
                                   TimeGUIHoveredFlags_AllowWhenOverlapped,
    TimeGUIHoveredFlags_RootAndChildWindows = TimeGUIHoveredFlags_RootWindow | TimeGUIHoveredFlags_ChildWindows,
};
typedef int TimeGUIHoveredFlags;

// InputText flags wrapper
enum TimeGUIInputTextFlags_
{
    TimeGUIInputTextFlags_None = 0,
    TimeGUIInputTextFlags_CharsDecimal = 1 << 0,
    TimeGUIInputTextFlags_CharsHexadecimal = 1 << 1,
    TimeGUIInputTextFlags_CharsUppercase = 1 << 2,
    TimeGUIInputTextFlags_CharsNoBlank = 1 << 3,
    TimeGUIInputTextFlags_AutoSelectAll = 1 << 4,
    TimeGUIInputTextFlags_EnterReturnsTrue = 1 << 5,
    TimeGUIInputTextFlags_CallbackCompletion = 1 << 6,
    TimeGUIInputTextFlags_CallbackHistory = 1 << 7,
    TimeGUIInputTextFlags_CallbackAlways = 1 << 8,
    TimeGUIInputTextFlags_CallbackCharFilter = 1 << 9,
    TimeGUIInputTextFlags_AllowTabInput = 1 << 10,
    TimeGUIInputTextFlags_CtrlEnterForNewLine = 1 << 11,
    TimeGUIInputTextFlags_NoHorizontalScroll = 1 << 12,
    TimeGUIInputTextFlags_AlwaysOverwrite = 1 << 13,
    TimeGUIInputTextFlags_ReadOnly = 1 << 14,
    TimeGUIInputTextFlags_Password = 1 << 15,
    TimeGUIInputTextFlags_NoUndoRedo = 1 << 16,
    TimeGUIInputTextFlags_CharsScientific = 1 << 17,
    TimeGUIInputTextFlags_CallbackResize = 1 << 18,
    TimeGUIInputTextFlags_CallbackEdit = 1 << 19,
    TimeGUIInputTextFlags_EscapeClearsAll = 1 << 20,
};
typedef int TimeGUIInputTextFlags;

// Popup flags wrapper
enum TimeGUIPopupFlags_
{
    TimeGUIPopupFlags_None = 0,
    TimeGUIPopupFlags_MouseButtonLeft = 0,
    TimeGUIPopupFlags_MouseButtonRight = 1,
    TimeGUIPopupFlags_MouseButtonMiddle = 2,
    TimeGUIPopupFlags_MouseButtonMask_ = 0x1F,
    TimeGUIPopupFlags_MouseButtonDefault_ = 1,
    TimeGUIPopupFlags_NoOpenOverExistingPopup = 1 << 5,
    TimeGUIPopupFlags_NoOpenOverItems = 1 << 6,
    TimeGUIPopupFlags_AnyPopupId = 1 << 7,
    TimeGUIPopupFlags_AnyPopupLevel = 1 << 8,
    TimeGUIPopupFlags_AnyPopup = TimeGUIPopupFlags_AnyPopupId | TimeGUIPopupFlags_AnyPopupLevel,
};
typedef int TimeGUIPopupFlags;

// Mouse cursor types wrapper
enum TimeGUIMouseCursor_
{
    TimeGUIMouseCursor_None = -1,
    TimeGUIMouseCursor_Arrow = 0,
    TimeGUIMouseCursor_TextInput,
    TimeGUIMouseCursor_ResizeAll,
    TimeGUIMouseCursor_ResizeNS,
    TimeGUIMouseCursor_ResizeEW,
    TimeGUIMouseCursor_ResizeNESW,
    TimeGUIMouseCursor_ResizeNWSE,
    TimeGUIMouseCursor_Hand,
    TimeGUIMouseCursor_NotAllowed,
    TimeGUIMouseCursor_COUNT
};
typedef int TimeGUIMouseCursor;

// Dock node flags wrapper (public + internal combined)
enum TimeGUIDockNodeFlags_
{
    TimeGUIDockNodeFlags_None = 0,
    TimeGUIDockNodeFlags_KeepAliveOnly = 1 << 0,
    TimeGUIDockNodeFlags_NoDockingOverCentralNode = 1 << 2,
    TimeGUIDockNodeFlags_PassthruCentralNode = 1 << 3,
    TimeGUIDockNodeFlags_NoDockingSplit = 1 << 4,
    TimeGUIDockNodeFlags_NoResize = 1 << 5,
    TimeGUIDockNodeFlags_AutoHideTabBar = 1 << 6,
    TimeGUIDockNodeFlags_NoUndocking = 1 << 7,
    // Internal flags (used with DockBuilderAddNode)
    TimeGUIDockNodeFlags_DockSpace = 1 << 10,
    TimeGUIDockNodeFlags_NoWindowMenuButton = 1 << 14,
};
typedef int TimeGUIDockNodeFlags;

// Config flags subset
enum TimeGUIConfigFlags_
{
    TimeGUIConfigFlags_None = 0,
    TimeGUIConfigFlags_NavEnableKeyboard = 1 << 0,
    TimeGUIConfigFlags_DockingEnable = 1 << 7,
    TimeGUIConfigFlags_ViewportsEnable = 1 << 10,
};
typedef int TimeGUIConfigFlags;

// Combo flags wrapper
enum TimeGUIComboFlags_
{
    TimeGUIComboFlags_None = 0,
    TimeGUIComboFlags_PopupAlignLeft = 1 << 0,
    TimeGUIComboFlags_HeightSmall = 1 << 1,
    TimeGUIComboFlags_HeightRegular = 1 << 2,
    TimeGUIComboFlags_HeightLarge = 1 << 3,
    TimeGUIComboFlags_HeightLargest = 1 << 4,
    TimeGUIComboFlags_NoArrowButton = 1 << 5,
    TimeGUIComboFlags_NoPreview = 1 << 6,
    TimeGUIComboFlags_WidthFitPreview = 1 << 7,
    TimeGUIComboFlags_HeightMask_ = TimeGUIComboFlags_HeightSmall | TimeGUIComboFlags_HeightRegular |
                                    TimeGUIComboFlags_HeightLarge | TimeGUIComboFlags_HeightLargest,
};
typedef int TimeGUIComboFlags;

// Selectable flags wrapper
enum TimeGUISelectableFlags_
{
    TimeGUISelectableFlags_None = 0,
    TimeGUISelectableFlags_DontClosePopups = 1 << 0,
    TimeGUISelectableFlags_SpanAllColumns = 1 << 1,
    TimeGUISelectableFlags_AllowDoubleClick = 1 << 2,
    TimeGUISelectableFlags_Disabled = 1 << 3,
    TimeGUISelectableFlags_AllowOverlap = 1 << 4,
};
typedef int TimeGUISelectableFlags;

// Table flags wrapper
enum TimeGUITableFlags_
{
    TimeGUITableFlags_None = 0,
    TimeGUITableFlags_Resizable = 1 << 0,
    TimeGUITableFlags_Reorderable = 1 << 2,
    TimeGUITableFlags_Hideable = 1 << 3,
    TimeGUITableFlags_Sortable = 1 << 4,
    TimeGUITableFlags_NoSavedSettings = 1 << 5,
    TimeGUITableFlags_ContextMenuInHeader = 1 << 6,
    TimeGUITableFlags_RowBg = 1 << 7,
    TimeGUITableFlags_BordersInnerH = 1 << 8,
    TimeGUITableFlags_BordersOuterH = 1 << 9,
    TimeGUITableFlags_BordersInnerV = 1 << 10,
    TimeGUITableFlags_BordersOuterV = 1 << 11,
    TimeGUITableFlags_BordersH = TimeGUITableFlags_BordersInnerH | TimeGUITableFlags_BordersOuterH,
    TimeGUITableFlags_BordersV = TimeGUITableFlags_BordersInnerV | TimeGUITableFlags_BordersOuterV,
    TimeGUITableFlags_BordersInner = TimeGUITableFlags_BordersInnerH | TimeGUITableFlags_BordersInnerV,
    TimeGUITableFlags_BordersOuter = TimeGUITableFlags_BordersOuterH | TimeGUITableFlags_BordersOuterV,
    TimeGUITableFlags_Borders = TimeGUITableFlags_BordersInner | TimeGUITableFlags_BordersOuter,
    TimeGUITableFlags_NoBordersInBody = 1 << 12,
    TimeGUITableFlags_NoBordersInBodyUntilHeader = 1 << 13,
    TimeGUITableFlags_SizingFixedFit = 1 << 14,
    TimeGUITableFlags_SizingFixedSame = 2 << 14,
    TimeGUITableFlags_SizingStretchProp = 3 << 14,
    TimeGUITableFlags_SizingStretchSame = 4 << 14,
    TimeGUITableFlags_NoHostExtendX = 1 << 16,
    TimeGUITableFlags_NoHostExtendY = 1 << 17,
    TimeGUITableFlags_KeepColumnsVisible = 1 << 18,
    TimeGUITableFlags_PreciseWidths = 1 << 19,
    TimeGUITableFlags_NoClip = 1 << 20,
    TimeGUITableFlags_PadOuterX = 1 << 21,
    TimeGUITableFlags_NoPadOuterX = 1 << 22,
    TimeGUITableFlags_NoPadInnerX = 1 << 23,
    TimeGUITableFlags_ScrollX = 1 << 24,
    TimeGUITableFlags_ScrollY = 1 << 25,
    TimeGUITableFlags_SortMulti = 1 << 26,
    TimeGUITableFlags_SortTristate = 1 << 27,
};
typedef int TimeGUITableFlags;

// Table Column flags wrapper
enum TimeGUITableColumnFlags_
{
    TimeGUITableColumnFlags_None = 0,
    TimeGUITableColumnFlags_Disabled = 1 << 0,
    TimeGUITableColumnFlags_DefaultHide = 1 << 1,
    TimeGUITableColumnFlags_DefaultSort = 1 << 2,
    TimeGUITableColumnFlags_WidthStretch = 1 << 3,
    TimeGUITableColumnFlags_WidthFixed = 1 << 4,
    TimeGUITableColumnFlags_NoResize = 1 << 5,
    TimeGUITableColumnFlags_NoReorder = 1 << 6,
    TimeGUITableColumnFlags_NoHide = 1 << 7,
    TimeGUITableColumnFlags_NoClip = 1 << 8,
    TimeGUITableColumnFlags_NoSort = 1 << 9,
    TimeGUITableColumnFlags_NoSortAscending = 1 << 10,
    TimeGUITableColumnFlags_NoSortDescending = 1 << 11,
    TimeGUITableColumnFlags_NoHeaderLabel = 1 << 12,
    TimeGUITableColumnFlags_NoHeaderWidth = 1 << 13,
    TimeGUITableColumnFlags_PreferSortAscending = 1 << 14,
    TimeGUITableColumnFlags_PreferSortDescending = 1 << 15,
    TimeGUITableColumnFlags_IndentEnable = 1 << 16,
    TimeGUITableColumnFlags_IndentDisable = 1 << 17,
    TimeGUITableColumnFlags_IsEnabled = 1 << 24,
    TimeGUITableColumnFlags_IsHovered = 1 << 25,
    TimeGUITableColumnFlags_IsVisible = 1 << 26,
    TimeGUITableColumnFlags_IsSorted = 1 << 27,
    TimeGUITableColumnFlags_Stream = 1 << 28,
};
typedef int TimeGUITableColumnFlags;

// Tree Node flags wrapper
enum TimeGUITreeNodeFlags_
{
    TimeGUITreeNodeFlags_None = 0,
    TimeGUITreeNodeFlags_Selected = 1 << 0,
    TimeGUITreeNodeFlags_Framed = 1 << 1,
    TimeGUITreeNodeFlags_AllowOverlap = 1 << 2,
    TimeGUITreeNodeFlags_NoTreePushOnOpen = 1 << 3,
    TimeGUITreeNodeFlags_NoAutoOpenOnLog = 1 << 4,
    TimeGUITreeNodeFlags_DefaultOpen = 1 << 5,
    TimeGUITreeNodeFlags_OpenOnDoubleCompact = 1 << 6,
    TimeGUITreeNodeFlags_OpenOnArrow = 1 << 7,
    TimeGUITreeNodeFlags_Leaf = 1 << 8,
    TimeGUITreeNodeFlags_Bullet = 1 << 9,
    TimeGUITreeNodeFlags_FramePadding = 1 << 10,
    TimeGUITreeNodeFlags_SpanAvailWidth = 1 << 11,
    TimeGUITreeNodeFlags_SpanFullWidth = 1 << 12,
    TimeGUITreeNodeFlags_SpanAllColumns = 1 << 13,
    TimeGUITreeNodeFlags_NavLeftJumpsBackHere = 1 << 14,
    TimeGUITreeNodeFlags_CollapsingHeader =
        TimeGUITreeNodeFlags_Framed | TimeGUITreeNodeFlags_NoTreePushOnOpen | TimeGUITreeNodeFlags_NoAutoOpenOnLog,
};
typedef int TimeGUITreeNodeFlags;

// Separator flags wrapper
enum TimeGUISeparatorFlags_
{
    TimeGUISeparatorFlags_None = 0,
    TimeGUISeparatorFlags_Horizontal = 1 << 0,
    TimeGUISeparatorFlags_Vertical = 1 << 1,
    TimeGUISeparatorFlags_SpanAllColumns = 1 << 2,
};
typedef int TimeGUISeparatorFlags;

// Clean font wrapper
struct TE_API TimeGUIFont
{
    void *nativeFont = nullptr;
    TEVector2 CalcTextSizeA(float size, float max_width, float wrap_width, const char *text_begin) const;
    ::ImFont *operator->() const;
};

// Synced clean IO wrapper structure
struct TE_API TimeGUIIO
{
    float DeltaTime = 0.0f;
    float MouseWheel = 0.0f;
    TEVector2 MouseDelta;
    TEVector2 DisplaySize;
    bool KeyShift = false;
    bool KeyCtrl = false;
    int ConfigFlags = 0;
    TimeGUIFont DefaultFont;
};

// Synced clean Style wrapper structure
struct TE_API TimeGUIStyle
{
    float WindowRounding = 0.0f;
    float ChildRounding = 0.0f;
    float FrameRounding = 0.0f;
    float PopupRounding = 0.0f;
    float TabRounding = 0.0f;
    float GrabRounding = 0.0f;
    float ScrollbarRounding = 0.0f;
    float WindowBorderSize = 0.0f;
    float FrameBorderSize = 0.0f;
    float PopupBorderSize = 0.0f;
    float IndentSpacing = 0.0f;
    TEVector2 ItemSpacing;
    TEVector2 FramePadding;
    TEVector2 WindowPadding;
    bool AntiAliasedLines = true;
    bool AntiAliasedFill = true;

    TEColor Colors[TimeGUICol_COUNT];
};

// Clean DrawList wrapper
struct TE_API TimeGUIDrawList
{
    void *nativeDrawList = nullptr;

    void AddLine(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float thickness = 1.0f);
    void AddRectFilled(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding = 0.0f);
    void AddText(const TEVector2 &pos, unsigned int color, const std::string &text);
    void AddText(const TimeGUIFont &font, float fontSize, const TEVector2 &pos, unsigned int color,
                 const std::string &text);
    void AddRect(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding = 0.0f, int flags = 0,
                 float thickness = 1.0f);
    void AddPolyline(const TEVector2 *points, int num_points, unsigned int color, int flags, float thickness);
    void AddConvexPolyFilled(const TEVector2 *points, int num_points, unsigned int color);
    void AddTriangleFilled(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3, unsigned int color);
    void AddBezierCubic(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3, const TEVector2 &p4,
                        unsigned int color, float thickness, int num_segments = 0);
    void AddCallback(void (*callback)(TimeGUIDrawList parent_list, const void *cmd), void *callback_data);
    void AddDrawCmd();
    void PushClipRect(const TEVector2 &clip_rect_min, const TEVector2 &clip_rect_max,
                      bool intersect_with_current_clip_rect = false);
    void PopClipRect();
    void PushTextureID(TimeGUITextureID texture_id);
    void ResetForNewFrame();

    void AddCircle(const TEVector2 &center, float radius, unsigned int color, int num_segments = 0,
                   float thickness = 1.0f);
    void AddCircleFilled(const TEVector2 &center, float radius, unsigned int color, int num_segments = 0);
    void AddQuadFilled(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3, const TEVector2 &p4,
                       unsigned int color);
    void AddImage(TimeGUITextureID user_texture_id, const TEVector2 &p_min, const TEVector2 &p_max,
                  const TEVector2 &uv_min = {0, 0}, const TEVector2 &uv_max = {1, 1}, unsigned int col = 0xFFFFFFFF);
    int GetVertexCount() const;
    int GetCommandCount() const;

    // Cast helper to obtain low-level ImDrawList if needed inside implementations
    template <typename T> T *As() const { return static_cast<T *>(nativeDrawList); }

    // Member access operator to allow direct use of dl->...
    ::ImDrawList *operator->() const { return static_cast<::ImDrawList *>(nativeDrawList); }
};

// Getters for Synced clean wrappers
TE_API TimeGUIIO &GetIO();
TE_API TimeGUIStyle &GetStyle();
TE_API TimeGUIDrawList GetWindowDrawList();
TE_API void *GetDrawListSharedData(); // For IM_NEW(ImDrawList) uses in grid drawing

// Basic window and widget controls
TE_API bool Begin(const std::string &name, bool *open = nullptr, TimeGUIWindowFlags flags = 0);
TE_API void End();
TE_API bool BeginChild(const std::string &strId, const TEVector2 &size = {0, 0}, bool border = false,
                       TimeGUIWindowFlags flags = 0);
TE_API void EndChild();

TE_API bool BeginMenuBar();
TE_API void EndMenuBar();
TE_API bool BeginMenu(const std::string &label, bool enabled = true);
TE_API void EndMenu();
TE_API bool MenuItem(const std::string &label, const std::string &shortcut = "", bool selected = false,
                     bool enabled = true);
TE_API bool MenuItem(const std::string &label, const std::string &shortcut, bool *p_selected, bool enabled = true);

TE_API TEVector2 GetCursorPos();
TE_API TEVector2 GetWindowContentRegionMin();
TE_API bool IsItemFocused();
TE_API TEVector2 GetItemRectMin();
TE_API TEVector2 GetItemRectSize();

TE_API bool Button(const std::string &label, float width = 0.0f, float height = 0.0f);
TE_API bool Button(const std::string &label, const TEVector2 &size);

TE_API void Text(const std::string &text);
TE_API void Text(const char *fmt, ...);

TE_API void TextUnformatted(const std::string &text);
TE_API void TextUnformatted(const char *text);
TE_API void AlignTextToFramePadding();

TE_API void TextDisabled(const std::string &text);
TE_API void TextDisabled(const char *fmt, ...);

TE_API void TextColored(const TEColor &color, const std::string &text);
TE_API void TextColored(const TEColor &color, const char *fmt, ...);
TE_API void TextColored(const TEVector4 &color, const std::string &text);
TE_API void TextColored(const TEVector4 &color, const char *fmt, ...);

TE_API void TextWrapped(const std::string &text);
TE_API void TextWrapped(const char *fmt, ...);

TE_API bool Checkbox(const std::string &label, bool *checked);
TE_API bool DragFloat(const std::string &label, float *value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                      const std::string &format = "%.3f", int flags = 0);
TE_API bool DragFloat2(const std::string &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                       const std::string &format = "%.3f", int flags = 0);
TE_API bool DragFloat3(const std::string &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                       const std::string &format = "%.3f", int flags = 0);
TE_API bool DragFloat4(const std::string &label, float *v, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
                       const std::string &format = "%.3f", int flags = 0);
TE_API bool DragInt(const std::string &label, int *v, float speed = 1.0f, int min = 0, int max = 0);
TE_API bool InputInt(const std::string &label, int *v, int step = 1, int step_fast = 100, int flags = 0);

TE_API bool SliderFloat(const std::string &label, float *v, float v_min, float v_max,
                        const std::string &format = "%.3f", int flags = 0);
TE_API bool SliderInt(const std::string &label, int *v, int v_min, int v_max, const std::string &format = "%d",
                      int flags = 0);

TE_API bool ColorEdit3(const std::string &label, float *col);
TE_API bool ColorEdit4(const std::string &label, float *col, int flags = 0);
TE_API bool ColorPicker4(const std::string &label, float *col, int flags = 0);
TE_API bool Combo(const std::string &label, int *currentItem, const char *const items[], int itemsCount,
                  int popupMaxHeightInItems = -1);
TE_API bool BeginCombo(const std::string &label, const std::string &previewValue, int flags = 0);
TE_API void EndCombo();
TE_API bool Selectable(const std::string &label, bool selected = false, int flags = 0, const TEVector2 &size = {0, 0});
TE_API bool Selectable(const std::string &label, bool *selected, int flags = 0, const TEVector2 &size = {0, 0});
TE_API bool RadioButton(const std::string &label, bool active);
TE_API bool RadioButton(const std::string &label, int *v, int v_button);
TE_API bool SmallButton(const std::string &label);

TE_API bool InputText(const std::string &label, std::string &value);
TE_API bool InputText(const std::string &label, char *buf, size_t bufSize);
TE_API bool InputText(const std::string &label, char *buf, size_t bufSize, TimeGUIInputTextFlags flags);
TE_API bool InputTextMultiline(const std::string &label, char *buf, size_t bufSize, const TEVector2 &size = {0, 0},
                               int flags = 0);

TE_API void Separator();
TE_API void SeparatorEx(int flags);
TE_API void Spacing();
TE_API void NewLine();
TE_API void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f);
TE_API void Dummy(const TEVector2 &size);
TE_API void BeginGroup();
TE_API void EndGroup();
TE_API void Indent(float indentW = 0.0f);
TE_API void Unindent(float indentW = 0.0f);

TE_API void Columns(int count = 1, const char *id = nullptr, bool border = true);
TE_API void NextColumn();
TE_API void SetColumnWidth(int columnIndex, float width);

TE_API void PushID(const std::string &strId);
TE_API void PushID(int intId);
TE_API void PopID();

TE_API void PushItemWidth(float itemWidth);
TE_API void PopItemWidth();
TE_API void PushMultiItemsWidths(int components, float width_full);
TE_API float CalcItemWidth();

TE_API void BeginDisabled(bool disabled = true);
TE_API void EndDisabled();

TE_API void PushSuspendedInput(const TEVector2 &stubMousePos = {-9999.0f, -9999.0f});
TE_API void PopSuspendedInput();

// Context / Utility functions
TE_API void SetNextWindowPos(const TEVector2 &pos, TimeGUICond cond = 0, const TEVector2 &pivot = {0, 0});
TE_API void SetNextWindowSize(const TEVector2 &size, TimeGUICond cond = 0);
TE_API void SetNextItemWidth(float itemWidth);
TE_API void PopItemWidth();
TE_API void SetNextItemAllowOverlap();

TE_API void Image(TimeGUITextureID userTextureId, const TEVector2 &size, const TEVector2 &uv0 = {0, 0},
                  const TEVector2 &uv1 = {1, 1});
TE_API bool ImageButton(const std::string &strId, TimeGUITextureID userTextureId, const TEVector2 &size,
                        const TEVector2 &uv0 = {0, 0}, const TEVector2 &uv1 = {1, 1});

TE_API float GetWindowWidth();
TE_API float GetWindowHeight();
TE_API TEVector2 GetWindowSize();
TE_API TEVector2 GetWindowPos();

TE_API TEVector2 GetMousePos();
TE_API void SetCursorScreenPos(const TEVector2 &pos);
TE_API void SetCursorPos(const TEVector2 &localPos);
TE_API void SetCursorPosX(float x);
TE_API void SetCursorPosY(float y);
TE_API float GetCursorPosX();
TE_API float GetCursorPosY();
TE_API TEVector2 GetCursorScreenPos();
TE_API float GetFrameHeight();
TE_API TEVector2 GetContentRegionAvail();

TE_API bool IsItemHovered(int flags = 0);
TE_API bool IsItemActive();
TE_API bool IsItemClicked(int mouseButton = 0);
TE_API bool IsWindowHovered(int flags = 0);
TE_API bool IsWindowFocused(int flags = 0);
TE_API bool IsMouseDown(int button);
TE_API bool IsMouseReleased(int button);
TE_API bool IsMouseClicked(int button, bool repeat = false);
TE_API bool IsAnyItemHovered();
TE_API bool IsMouseDoubleClicked(int button);
TE_API bool IsMouseDragging(int button, float lock_threshold = -1.0f);
TE_API TEVector2 GetMouseDragDelta(int button = 0, float lock_threshold = -1.0f);
TE_API bool IsKeyPressed(int key);
TE_API double GetTime();

struct TimeGUIViewport
{
    TEVector2 Pos;
    TEVector2 Size;
    unsigned int ID;
};
TE_API TimeGUIViewport GetMainViewport();
TE_API void SetNextWindowViewport(unsigned int viewportId);

// Table operations
TE_API bool BeginTable(const std::string &strId, int column, int flags = 0, const TEVector2 &outerSize = {0, 0},
                       float innerWidth = 0.0f);
TE_API void EndTable();
TE_API void TableSetupColumn(const std::string &label, int flags = 0, float initWidthOrWeight = 0.0f,
                             unsigned int userId = 0);
TE_API void TableHeadersRow();
TE_API bool TableNextColumn();
TE_API void TableNextRow(float rowMinHeight = 0.0f, int rowFlags = 0);

TE_API bool TreeNodeEx(const std::string &label, int flags = 0);
TE_API bool TreeNodeEx(void *ptrId, int flags, const std::string &text);
TE_API void TreePop();
TE_API bool CollapsingHeader(const std::string &label, int flags = 0);

TE_API void OpenPopup(const std::string &strId);
TE_API bool BeginPopup(const std::string &strId, int flags = 0);
TE_API bool BeginPopupContextWindow(const std::string &strId = "", int mouseButton = 1, bool alsoOverItems = true);
TE_API bool BeginPopupContextItem(const std::string &strId = "", int mouseButton = 1);
TE_API bool BeginPopupModal(const std::string &name, bool *open = nullptr, TimeGUIWindowFlags flags = 0);
TE_API void EndPopup();
TE_API void CloseCurrentPopup();

TE_API void PushStyleColor(TimeGUICol idx, const TEColor &color);
TE_API void PopStyleColor(int count = 1);
TE_API void PushStyleVar(TimeGUIStyleVar idx, float val);
TE_API void PushStyleVar(TimeGUIStyleVar idx, const TEVector2 &val);
TE_API void PopStyleVar(int count = 1);

TE_API void PushFont(const TimeGUIFont &font);
TE_API void PopFont();

TE_API unsigned int GetID(const std::string &strId);
TE_API void SetItemTooltip(const std::string &text);
TE_API void SetItemTooltip(const char *fmt, ...);
TE_API void SetTooltip(const std::string &text);
TE_API void SetItemDefaultFocus();
TE_API void SetMouseCursor(int cursorType);
TE_API unsigned int GetColorU32(const TEColor &color);
TE_API unsigned int GetColorU32(TimeGUICol idx, float alpha_mul = 1.0f);
TE_API TEVector2 CalcTextSize(const std::string &text);

TE_API TEVector4 ColorConvertU32ToFloat4(unsigned int in);
TE_API unsigned int ColorConvertFloat4ToU32(const TEVector4 &in);

TE_API bool BeginDragDropSource(int flags = 0);
TE_API bool SetDragDropPayload(const std::string &type, const void *data, size_t size, int cond = 0);
TE_API void EndDragDropSource();

TE_API unsigned int DockSpace(unsigned int id, const TEVector2 &size = {0, 0}, int flags = 0);
TE_API void DockBuilderRemoveNode(unsigned int nodeId);
TE_API void DockBuilderAddNode(unsigned int nodeId, int flags = 0);
TE_API void DockBuilderSetNodeSize(unsigned int nodeId, const TEVector2 &size);
TE_API unsigned int DockBuilderSplitNode(unsigned int nodeId, int splitDir, float sizeRatio, unsigned int *outIdDir1,
                                         unsigned int *outIdDir2);
TE_API void DockBuilderDockWindow(const std::string &windowName, unsigned int nodeId);
TE_API void DockBuilderFinish(unsigned int nodeId);

TE_API bool BeginTabItem(const std::string &label, bool *open = nullptr, int flags = 0);
TE_API void EndTabItem();
TE_API bool BeginTabBar(const std::string &strId, int flags = 0);
TE_API void EndTabBar();

TE_API bool BeginProjectCard(const std::string &id, const TEVector2 &size, bool &hovered);
TE_API void EndProjectCard();

TE_API bool IsItemDeactivatedAfterEdit();
TE_API TimeGUIDrawList CreateDrawList();
TE_API void DestroyDrawList(TimeGUIDrawList &dl);
TE_API void RenderDrawList(const TimeGUIDrawList &dl, const TEVector2 &displaySize);
TE_API void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b);
TE_API void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v);

TE_API TimeGUIDrawList GetBackgroundDrawList();
TE_API TimeGUIFont GetDefaultFont();
TE_API TimeGUITextureID GetFontAtlasTextureID();

// Common properties drawing controls (consolidated from UIUtils.hpp)
TE_API std::string CleanLabel(const std::string &label);
TE_API bool DrawVec3Control(const std::string &label, TEVector &values, float resetValue = 0.0f,
                            float columnWidth = 100.0f);
TE_API bool DrawVec2Control(const std::string &label, TEVector2 &values, float resetValue = 0.0f,
                            float columnWidth = 100.0f);
TE_API bool DrawVec4Control(const std::string &label, TEVector4 &values, float resetValue = 0.0f,
                            float columnWidth = 100.0f);
TE_API bool DrawColorControl(const std::string &label, TEColor &values, float columnWidth = 100.0f);
TE_API bool DrawDeleteButton(const std::string &id, float size = 0.0f, float fontScale = 1.3f);
TE_API bool DrawPlusButton(const std::string &id, float size = 0.0f, float fontScale = 1.3f);
} // namespace TimeGUI

// Bring enums and functions into namespace TE so they can be referenced globally within TE layers
using namespace TimeGUI;

namespace UIUtils = TimeGUI;

} // namespace TE

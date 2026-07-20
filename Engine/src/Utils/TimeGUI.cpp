#include "Utils/TimeGUI.hpp"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace TE
{

namespace TimeGUI
{
static TimeGUIStyle s_StyleInstance;
static TimeGUIIO s_IOInstance;

static ImGuiWindowFlags TranslateWindowFlags(TimeGUIWindowFlags f)
{
    ImGuiWindowFlags out = 0;
    if (f & TimeGUIWindowFlags_NoTitleBar)
        out |= ImGuiWindowFlags_NoTitleBar;
    if (f & TimeGUIWindowFlags_NoResize)
        out |= ImGuiWindowFlags_NoResize;
    if (f & TimeGUIWindowFlags_NoMove)
        out |= ImGuiWindowFlags_NoMove;
    if (f & TimeGUIWindowFlags_NoScrollbar)
        out |= ImGuiWindowFlags_NoScrollbar;
    if (f & TimeGUIWindowFlags_NoScrollWithMouse)
        out |= ImGuiWindowFlags_NoScrollWithMouse;
    if (f & TimeGUIWindowFlags_NoCollapse)
        out |= ImGuiWindowFlags_NoCollapse;
    if (f & TimeGUIWindowFlags_AlwaysAutoResize)
        out |= ImGuiWindowFlags_AlwaysAutoResize;
    if (f & TimeGUIWindowFlags_NoBackground)
        out |= ImGuiWindowFlags_NoBackground;
    if (f & TimeGUIWindowFlags_NoSavedSettings)
        out |= ImGuiWindowFlags_NoSavedSettings;
    if (f & TimeGUIWindowFlags_NoMouseInputs)
        out |= ImGuiWindowFlags_NoMouseInputs;
    if (f & TimeGUIWindowFlags_MenuBar)
        out |= ImGuiWindowFlags_MenuBar;
    if (f & TimeGUIWindowFlags_HorizontalScrollbar)
        out |= ImGuiWindowFlags_HorizontalScrollbar;
    if (f & TimeGUIWindowFlags_NoFocusOnAppearing)
        out |= ImGuiWindowFlags_NoFocusOnAppearing;
    if (f & TimeGUIWindowFlags_NoBringToFrontOnFocus)
        out |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (f & TimeGUIWindowFlags_AlwaysVerticalScrollbar)
        out |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
    if (f & TimeGUIWindowFlags_AlwaysHorizontalScrollbar)
        out |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
    if (f & TimeGUIWindowFlags_NoNavInputs)
        out |= ImGuiWindowFlags_NoNavInputs;
    if (f & TimeGUIWindowFlags_NoNavFocus)
        out |= ImGuiWindowFlags_NoNavFocus;
    if (f & TimeGUIWindowFlags_UnsavedDocument)
        out |= ImGuiWindowFlags_UnsavedDocument;
    if (f & TimeGUIWindowFlags_NoDocking)
        out |= ImGuiWindowFlags_NoDocking;
    return out;
}

static ImGuiColorEditFlags TranslateColorEditFlags(TimeGUIColorEditFlags f)
{
    ImGuiColorEditFlags out = 0;
    if (f & TimeGUIColorEditFlags_NoAlpha)
        out |= ImGuiColorEditFlags_NoAlpha;
    if (f & TimeGUIColorEditFlags_NoPicker)
        out |= ImGuiColorEditFlags_NoPicker;
    if (f & TimeGUIColorEditFlags_NoOptions)
        out |= ImGuiColorEditFlags_NoOptions;
    if (f & TimeGUIColorEditFlags_NoSmallPreview)
        out |= ImGuiColorEditFlags_NoSmallPreview;
    if (f & TimeGUIColorEditFlags_NoInputs)
        out |= ImGuiColorEditFlags_NoInputs;
    if (f & TimeGUIColorEditFlags_NoTooltip)
        out |= ImGuiColorEditFlags_NoTooltip;
    if (f & TimeGUIColorEditFlags_NoLabel)
        out |= ImGuiColorEditFlags_NoLabel;
    if (f & TimeGUIColorEditFlags_NoSidePreview)
        out |= ImGuiColorEditFlags_NoSidePreview;
    if (f & TimeGUIColorEditFlags_NoDragDrop)
        out |= ImGuiColorEditFlags_NoDragDrop;
    if (f & TimeGUIColorEditFlags_NoBorder)
        out |= ImGuiColorEditFlags_NoBorder;
    if (f & TimeGUIColorEditFlags_AlphaBar)
        out |= ImGuiColorEditFlags_AlphaBar;
    if (f & TimeGUIColorEditFlags_AlphaPreview)
        out |= ImGuiColorEditFlags_AlphaPreview;
    if (f & TimeGUIColorEditFlags_AlphaPreviewHalf)
        out |= ImGuiColorEditFlags_AlphaPreviewHalf;
    if (f & TimeGUIColorEditFlags_DisplayRGB)
        out |= ImGuiColorEditFlags_DisplayRGB;
    if (f & TimeGUIColorEditFlags_InputRGB)
        out |= ImGuiColorEditFlags_InputRGB;
    return out;
}

static ImGuiHoveredFlags TranslateHoveredFlags(TimeGUIHoveredFlags f)
{
    ImGuiHoveredFlags out = 0;
    if (f & TimeGUIHoveredFlags_ChildWindows)
        out |= ImGuiHoveredFlags_ChildWindows;
    if (f & TimeGUIHoveredFlags_RootWindow)
        out |= ImGuiHoveredFlags_RootWindow;
    if (f & TimeGUIHoveredFlags_AnyWindow)
        out |= ImGuiHoveredFlags_AnyWindow;
    if (f & TimeGUIHoveredFlags_NoPopupHierarchy)
        out |= ImGuiHoveredFlags_NoPopupHierarchy;
    if (f & TimeGUIHoveredFlags_DockHierarchy)
        out |= ImGuiHoveredFlags_DockHierarchy;
    if (f & TimeGUIHoveredFlags_AllowWhenBlockedByPopup)
        out |= ImGuiHoveredFlags_AllowWhenBlockedByPopup;
    if (f & TimeGUIHoveredFlags_AllowWhenBlockedByActiveItem)
        out |= ImGuiHoveredFlags_AllowWhenBlockedByActiveItem;
    if (f & TimeGUIHoveredFlags_AllowWhenOverlapped)
        out |= ImGuiHoveredFlags_AllowWhenOverlapped;
    if (f & TimeGUIHoveredFlags_AllowWhenDisabled)
        out |= ImGuiHoveredFlags_AllowWhenDisabled;
    if (f & TimeGUIHoveredFlags_NoNavOverride)
        out |= ImGuiHoveredFlags_NoNavOverride;
    return out;
}

static ImGuiInputTextFlags TranslateInputTextFlags(TimeGUIInputTextFlags f)
{
    ImGuiInputTextFlags out = 0;
    if (f & TimeGUIInputTextFlags_CharsDecimal)
        out |= ImGuiInputTextFlags_CharsDecimal;
    if (f & TimeGUIInputTextFlags_CharsHexadecimal)
        out |= ImGuiInputTextFlags_CharsHexadecimal;
    if (f & TimeGUIInputTextFlags_CharsUppercase)
        out |= ImGuiInputTextFlags_CharsUppercase;
    if (f & TimeGUIInputTextFlags_CharsNoBlank)
        out |= ImGuiInputTextFlags_CharsNoBlank;
    if (f & TimeGUIInputTextFlags_AutoSelectAll)
        out |= ImGuiInputTextFlags_AutoSelectAll;
    if (f & TimeGUIInputTextFlags_EnterReturnsTrue)
        out |= ImGuiInputTextFlags_EnterReturnsTrue;
    if (f & TimeGUIInputTextFlags_CallbackCompletion)
        out |= ImGuiInputTextFlags_CallbackCompletion;
    if (f & TimeGUIInputTextFlags_CallbackHistory)
        out |= ImGuiInputTextFlags_CallbackHistory;
    if (f & TimeGUIInputTextFlags_CallbackAlways)
        out |= ImGuiInputTextFlags_CallbackAlways;
    if (f & TimeGUIInputTextFlags_CallbackCharFilter)
        out |= ImGuiInputTextFlags_CallbackCharFilter;
    if (f & TimeGUIInputTextFlags_AllowTabInput)
        out |= ImGuiInputTextFlags_AllowTabInput;
    if (f & TimeGUIInputTextFlags_CtrlEnterForNewLine)
        out |= ImGuiInputTextFlags_CtrlEnterForNewLine;
    if (f & TimeGUIInputTextFlags_NoHorizontalScroll)
        out |= ImGuiInputTextFlags_NoHorizontalScroll;
    if (f & TimeGUIInputTextFlags_AlwaysOverwrite)
        out |= ImGuiInputTextFlags_AlwaysOverwrite;
    if (f & TimeGUIInputTextFlags_ReadOnly)
        out |= ImGuiInputTextFlags_ReadOnly;
    if (f & TimeGUIInputTextFlags_Password)
        out |= ImGuiInputTextFlags_Password;
    if (f & TimeGUIInputTextFlags_NoUndoRedo)
        out |= ImGuiInputTextFlags_NoUndoRedo;
    if (f & TimeGUIInputTextFlags_CharsScientific)
        out |= ImGuiInputTextFlags_CharsScientific;
    if (f & TimeGUIInputTextFlags_CallbackResize)
        out |= ImGuiInputTextFlags_CallbackResize;
    if (f & TimeGUIInputTextFlags_CallbackEdit)
        out |= ImGuiInputTextFlags_CallbackEdit;
    if (f & TimeGUIInputTextFlags_EscapeClearsAll)
        out |= ImGuiInputTextFlags_EscapeClearsAll;
    return out;
}

static ImGuiPopupFlags TranslatePopupFlags(TimeGUIPopupFlags f)
{
    ImGuiPopupFlags out = 0;
    if (f & TimeGUIPopupFlags_MouseButtonLeft)
        out |= ImGuiPopupFlags_MouseButtonLeft;
    if (f & TimeGUIPopupFlags_MouseButtonRight)
        out |= ImGuiPopupFlags_MouseButtonRight;
    if (f & TimeGUIPopupFlags_MouseButtonMiddle)
        out |= ImGuiPopupFlags_MouseButtonMiddle;
    if (f & TimeGUIPopupFlags_NoOpenOverExistingPopup)
        out |= ImGuiPopupFlags_NoOpenOverExistingPopup;
    if (f & TimeGUIPopupFlags_NoOpenOverItems)
        out |= ImGuiPopupFlags_NoOpenOverItems;
    if (f & TimeGUIPopupFlags_AnyPopupId)
        out |= ImGuiPopupFlags_AnyPopupId;
    if (f & TimeGUIPopupFlags_AnyPopupLevel)
        out |= ImGuiPopupFlags_AnyPopupLevel;
    return out;
}

static ImGuiDockNodeFlags TranslateDockNodeFlags(TimeGUIDockNodeFlags f)
{
    ImGuiDockNodeFlags out = 0;
    if (f & TimeGUIDockNodeFlags_KeepAliveOnly)
        out |= ImGuiDockNodeFlags_KeepAliveOnly;
    if (f & TimeGUIDockNodeFlags_NoDockingOverCentralNode)
        out |= ImGuiDockNodeFlags_NoDockingOverCentralNode;
    if (f & TimeGUIDockNodeFlags_PassthruCentralNode)
        out |= ImGuiDockNodeFlags_PassthruCentralNode;
    if (f & TimeGUIDockNodeFlags_NoDockingSplit)
        out |= ImGuiDockNodeFlags_NoDockingSplit;
    if (f & TimeGUIDockNodeFlags_NoResize)
        out |= ImGuiDockNodeFlags_NoResize;
    if (f & TimeGUIDockNodeFlags_AutoHideTabBar)
        out |= ImGuiDockNodeFlags_AutoHideTabBar;
    if (f & TimeGUIDockNodeFlags_NoUndocking)
        out |= ImGuiDockNodeFlags_NoUndocking;
    if (f & TimeGUIDockNodeFlags_DockSpace)
        out |= ImGuiDockNodeFlags_DockSpace;
    if (f & TimeGUIDockNodeFlags_NoWindowMenuButton)
        out |= ImGuiDockNodeFlags_NoWindowMenuButton;
    return out;
}

static ImGuiSelectableFlags TranslateSelectableFlags(TimeGUISelectableFlags f)
{
    ImGuiSelectableFlags out = 0;
    if (f & TimeGUISelectableFlags_DontClosePopups)
        out |= ImGuiSelectableFlags_DontClosePopups;
    if (f & TimeGUISelectableFlags_SpanAllColumns)
        out |= ImGuiSelectableFlags_SpanAllColumns;
    if (f & TimeGUISelectableFlags_AllowDoubleClick)
        out |= ImGuiSelectableFlags_AllowDoubleClick;
    if (f & TimeGUISelectableFlags_Disabled)
        out |= ImGuiSelectableFlags_Disabled;
    if (f & TimeGUISelectableFlags_AllowOverlap)
        out |= ImGuiSelectableFlags_AllowOverlap;
    return out;
}

static ImGuiTableFlags TranslateTableFlags(TimeGUITableFlags f)
{
    ImGuiTableFlags out = 0;
    if (f & TimeGUITableFlags_Resizable)
        out |= ImGuiTableFlags_Resizable;
    if (f & TimeGUITableFlags_Reorderable)
        out |= ImGuiTableFlags_Reorderable;
    if (f & TimeGUITableFlags_Hideable)
        out |= ImGuiTableFlags_Hideable;
    if (f & TimeGUITableFlags_Sortable)
        out |= ImGuiTableFlags_Sortable;
    if (f & TimeGUITableFlags_NoSavedSettings)
        out |= ImGuiTableFlags_NoSavedSettings;
    /*
     *
     * ContextMenuInHeader is not defined in this ImGui version
     *
     *   if (f & TimeGUITableFlags_ContextMenuInHeader) out |= ImGuiTableFlags_ContextMenuInHeader;
     **/
    if (f & TimeGUITableFlags_RowBg)
        out |= ImGuiTableFlags_RowBg;
    if (f & TimeGUITableFlags_BordersInnerH)
        out |= ImGuiTableFlags_BordersInnerH;
    if (f & TimeGUITableFlags_BordersOuterH)
        out |= ImGuiTableFlags_BordersOuterH;
    if (f & TimeGUITableFlags_BordersInnerV)
        out |= ImGuiTableFlags_BordersInnerV;
    if (f & TimeGUITableFlags_BordersOuterV)
        out |= ImGuiTableFlags_BordersOuterV;
    if (f & TimeGUITableFlags_NoBordersInBody)
        out |= ImGuiTableFlags_NoBordersInBody;
    if (f & TimeGUITableFlags_NoBordersInBodyUntilHeader)
        out |= ImGuiTableFlags_NoBordersInBodyUntilResize;

    int sizing = f & (7 << 14); // mask for sizing flags
    if (sizing == TimeGUITableFlags_SizingFixedFit)
        out |= ImGuiTableFlags_SizingFixedFit;
    else if (sizing == TimeGUITableFlags_SizingFixedSame)
        out |= ImGuiTableFlags_SizingFixedSame;
    else if (sizing == TimeGUITableFlags_SizingStretchProp)
        out |= ImGuiTableFlags_SizingStretchProp;
    else if (sizing == TimeGUITableFlags_SizingStretchSame)
        out |= ImGuiTableFlags_SizingStretchSame;

    if (f & TimeGUITableFlags_NoHostExtendX)
        out |= ImGuiTableFlags_NoHostExtendX;
    if (f & TimeGUITableFlags_NoHostExtendY)
        out |= ImGuiTableFlags_NoHostExtendY;
    if (f & TimeGUITableFlags_KeepColumnsVisible)
        out |= ImGuiTableFlags_NoKeepColumnsVisible;
    if (f & TimeGUITableFlags_PreciseWidths)
        out |= ImGuiTableFlags_PreciseWidths;
    if (f & TimeGUITableFlags_NoClip)
        out |= ImGuiTableFlags_NoClip;
    if (f & TimeGUITableFlags_PadOuterX)
        out |= ImGuiTableFlags_PadOuterX;
    if (f & TimeGUITableFlags_NoPadOuterX)
        out |= ImGuiTableFlags_NoPadOuterX;
    if (f & TimeGUITableFlags_NoPadInnerX)
        out |= ImGuiTableFlags_NoPadInnerX;
    if (f & TimeGUITableFlags_ScrollX)
        out |= ImGuiTableFlags_ScrollX;
    if (f & TimeGUITableFlags_ScrollY)
        out |= ImGuiTableFlags_ScrollY;
    if (f & TimeGUITableFlags_SortMulti)
        out |= ImGuiTableFlags_SortMulti;
    if (f & TimeGUITableFlags_SortTristate)
        out |= ImGuiTableFlags_SortTristate;
    return out;
}

static ImGuiTableColumnFlags TranslateTableColumnFlags(TimeGUITableColumnFlags f)
{
    ImGuiTableColumnFlags out = 0;
    if (f & TimeGUITableColumnFlags_Disabled)
        out |= ImGuiTableColumnFlags_Disabled;
    if (f & TimeGUITableColumnFlags_DefaultHide)
        out |= ImGuiTableColumnFlags_DefaultHide;
    if (f & TimeGUITableColumnFlags_DefaultSort)
        out |= ImGuiTableColumnFlags_DefaultSort;
    if (f & TimeGUITableColumnFlags_WidthStretch)
        out |= ImGuiTableColumnFlags_WidthStretch;
    if (f & TimeGUITableColumnFlags_WidthFixed)
        out |= ImGuiTableColumnFlags_WidthFixed;
    if (f & TimeGUITableColumnFlags_NoResize)
        out |= ImGuiTableColumnFlags_NoResize;
    if (f & TimeGUITableColumnFlags_NoReorder)
        out |= ImGuiTableColumnFlags_NoReorder;
    if (f & TimeGUITableColumnFlags_NoHide)
        out |= ImGuiTableColumnFlags_NoHide;
    if (f & TimeGUITableColumnFlags_NoClip)
        out |= ImGuiTableColumnFlags_NoClip;
    if (f & TimeGUITableColumnFlags_NoSort)
        out |= ImGuiTableColumnFlags_NoSort;
    if (f & TimeGUITableColumnFlags_NoSortAscending)
        out |= ImGuiTableColumnFlags_NoSortAscending;
    if (f & TimeGUITableColumnFlags_NoSortDescending)
        out |= ImGuiTableColumnFlags_NoSortDescending;
    if (f & TimeGUITableColumnFlags_NoHeaderLabel)
        out |= ImGuiTableColumnFlags_NoHeaderLabel;
    if (f & TimeGUITableColumnFlags_NoHeaderWidth)
        out |= ImGuiTableColumnFlags_NoHeaderWidth;
    if (f & TimeGUITableColumnFlags_PreferSortAscending)
        out |= ImGuiTableColumnFlags_PreferSortAscending;
    if (f & TimeGUITableColumnFlags_PreferSortDescending)
        out |= ImGuiTableColumnFlags_PreferSortDescending;
    if (f & TimeGUITableColumnFlags_IndentEnable)
        out |= ImGuiTableColumnFlags_IndentEnable;
    if (f & TimeGUITableColumnFlags_IndentDisable)
        out |= ImGuiTableColumnFlags_IndentDisable;
    if (f & TimeGUITableColumnFlags_IsEnabled)
        out |= ImGuiTableColumnFlags_IsEnabled;
    if (f & TimeGUITableColumnFlags_IsHovered)
        out |= ImGuiTableColumnFlags_IsHovered;
    if (f & TimeGUITableColumnFlags_IsVisible)
        out |= ImGuiTableColumnFlags_IsVisible;
    if (f & TimeGUITableColumnFlags_IsSorted)
        out |= ImGuiTableColumnFlags_IsSorted;
    // Stream is not defined in this ImGui version
    return out;
}

static ImGuiTreeNodeFlags TranslateTreeNodeFlags(TimeGUITreeNodeFlags f)
{
    ImGuiTreeNodeFlags out = 0;
    if (f & TimeGUITreeNodeFlags_Selected)
        out |= ImGuiTreeNodeFlags_Selected;
    if (f & TimeGUITreeNodeFlags_Framed)
        out |= ImGuiTreeNodeFlags_Framed;
    if (f & TimeGUITreeNodeFlags_AllowOverlap)
        out |= ImGuiTreeNodeFlags_AllowOverlap;
    if (f & TimeGUITreeNodeFlags_NoTreePushOnOpen)
        out |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (f & TimeGUITreeNodeFlags_NoAutoOpenOnLog)
        out |= ImGuiTreeNodeFlags_NoAutoOpenOnLog;
    if (f & TimeGUITreeNodeFlags_DefaultOpen)
        out |= ImGuiTreeNodeFlags_DefaultOpen;
    if (f & TimeGUITreeNodeFlags_OpenOnDoubleCompact)
        out |= ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (f & TimeGUITreeNodeFlags_OpenOnArrow)
        out |= ImGuiTreeNodeFlags_OpenOnArrow;
    if (f & TimeGUITreeNodeFlags_Leaf)
        out |= ImGuiTreeNodeFlags_Leaf;
    if (f & TimeGUITreeNodeFlags_Bullet)
        out |= ImGuiTreeNodeFlags_Bullet;
    if (f & TimeGUITreeNodeFlags_FramePadding)
        out |= ImGuiTreeNodeFlags_FramePadding;
    if (f & TimeGUITreeNodeFlags_SpanAvailWidth)
        out |= ImGuiTreeNodeFlags_SpanAvailWidth;
    if (f & TimeGUITreeNodeFlags_SpanFullWidth)
        out |= ImGuiTreeNodeFlags_SpanFullWidth;
    if (f & TimeGUITreeNodeFlags_SpanAllColumns)
        out |= ImGuiTreeNodeFlags_SpanAllColumns;
    if (f & TimeGUITreeNodeFlags_NavLeftJumpsBackHere)
        out |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
    return out;
}

static ImGuiCol TranslateColorEnum(TimeGUICol c)
{
    switch (c)
    {
    case TimeGUICol_Text:
        return ImGuiCol_Text;
    case TimeGUICol_TextDisabled:
        return ImGuiCol_TextDisabled;
    case TimeGUICol_WindowBg:
        return ImGuiCol_WindowBg;
    case TimeGUICol_ChildBg:
        return ImGuiCol_ChildBg;
    case TimeGUICol_PopupBg:
        return ImGuiCol_PopupBg;
    case TimeGUICol_Border:
        return ImGuiCol_Border;
    case TimeGUICol_BorderShadow:
        return ImGuiCol_BorderShadow;
    case TimeGUICol_FrameBg:
        return ImGuiCol_FrameBg;
    case TimeGUICol_FrameBgHovered:
        return ImGuiCol_FrameBgHovered;
    case TimeGUICol_FrameBgActive:
        return ImGuiCol_FrameBgActive;
    case TimeGUICol_TitleBg:
        return ImGuiCol_TitleBg;
    case TimeGUICol_TitleBgActive:
        return ImGuiCol_TitleBgActive;
    case TimeGUICol_TitleBgCollapsed:
        return ImGuiCol_TitleBgCollapsed;
    case TimeGUICol_MenuBarBg:
        return ImGuiCol_MenuBarBg;
    case TimeGUICol_ScrollbarBg:
        return ImGuiCol_ScrollbarBg;
    case TimeGUICol_ScrollbarGrab:
        return ImGuiCol_ScrollbarGrab;
    case TimeGUICol_ScrollbarGrabHovered:
        return ImGuiCol_ScrollbarGrabHovered;
    case TimeGUICol_ScrollbarGrabActive:
        return ImGuiCol_ScrollbarGrabActive;
    case TimeGUICol_CheckMark:
        return ImGuiCol_CheckMark;
    case TimeGUICol_SliderGrab:
        return ImGuiCol_SliderGrab;
    case TimeGUICol_SliderGrabActive:
        return ImGuiCol_SliderGrabActive;
    case TimeGUICol_Button:
        return ImGuiCol_Button;
    case TimeGUICol_ButtonHovered:
        return ImGuiCol_ButtonHovered;
    case TimeGUICol_ButtonActive:
        return ImGuiCol_ButtonActive;
    case TimeGUICol_Header:
        return ImGuiCol_Header;
    case TimeGUICol_HeaderHovered:
        return ImGuiCol_HeaderHovered;
    case TimeGUICol_HeaderActive:
        return ImGuiCol_HeaderActive;
    case TimeGUICol_Separator:
        return ImGuiCol_Separator;
    case TimeGUICol_SeparatorHovered:
        return ImGuiCol_SeparatorHovered;
    case TimeGUICol_SeparatorActive:
        return ImGuiCol_SeparatorActive;
    case TimeGUICol_ResizeGrip:
        return ImGuiCol_ResizeGrip;
    case TimeGUICol_ResizeGripHovered:
        return ImGuiCol_ResizeGripHovered;
    case TimeGUICol_ResizeGripActive:
        return ImGuiCol_ResizeGripActive;
    case TimeGUICol_InputTextCursor:
        return ImGuiCol_InputTextCursor;
    case TimeGUICol_TabHovered:
        return ImGuiCol_TabHovered;
    case TimeGUICol_Tab:
        return ImGuiCol_Tab;
    case TimeGUICol_TabSelected:
        return ImGuiCol_TabSelected;
    case TimeGUICol_TabSelectedOverline:
        return ImGuiCol_TabSelectedOverline;
    case TimeGUICol_TabDimmed:
        return ImGuiCol_TabDimmed;
    case TimeGUICol_TabDimmedSelected:
        return ImGuiCol_TabDimmedSelected;
    case TimeGUICol_TabDimmedSelectedOverline:
        return ImGuiCol_TabDimmedSelectedOverline;
    case TimeGUICol_DockingPreview:
        return ImGuiCol_DockingPreview;
    case TimeGUICol_DockingEmptyBg:
        return ImGuiCol_DockingEmptyBg;
    case TimeGUICol_PlotLines:
        return ImGuiCol_PlotLines;
    case TimeGUICol_PlotLinesHovered:
        return ImGuiCol_PlotLinesHovered;
    case TimeGUICol_PlotHistogram:
        return ImGuiCol_PlotHistogram;
    case TimeGUICol_PlotHistogramHovered:
        return ImGuiCol_PlotHistogramHovered;
    case TimeGUICol_TableHeaderBg:
        return ImGuiCol_TableHeaderBg;
    case TimeGUICol_TableBorderStrong:
        return ImGuiCol_TableBorderStrong;
    case TimeGUICol_TableBorderLight:
        return ImGuiCol_TableBorderLight;
    case TimeGUICol_TableRowBg:
        return ImGuiCol_TableRowBg;
    case TimeGUICol_TableRowBgAlt:
        return ImGuiCol_TableRowBgAlt;
    case TimeGUICol_TextLink:
        return ImGuiCol_TextLink;
    case TimeGUICol_TextSelectedBg:
        return ImGuiCol_TextSelectedBg;
    case TimeGUICol_TreeLines:
        return ImGuiCol_TreeLines;
    case TimeGUICol_DragDropTarget:
        return ImGuiCol_DragDropTarget;
    case TimeGUICol_DragDropTargetBg:
        return ImGuiCol_DragDropTargetBg;
    case TimeGUICol_UnsavedMarker:
        return ImGuiCol_UnsavedMarker;
    case TimeGUICol_NavCursor:
        return ImGuiCol_NavCursor;
    case TimeGUICol_NavWindowingHighlight:
        return ImGuiCol_NavWindowingHighlight;
    case TimeGUICol_NavWindowingDimBg:
        return ImGuiCol_NavWindowingDimBg;
    case TimeGUICol_ModalWindowDimBg:
        return ImGuiCol_ModalWindowDimBg;
    default:
        return ImGuiCol_Text;
    }
}

static ImGuiStyleVar TranslateStyleVarEnum(TimeGUIStyleVar v)
{
    switch (v)
    {
    case TimeGUIStyleVar_Alpha:
        return ImGuiStyleVar_Alpha;
    case TimeGUIStyleVar_DisabledAlpha:
        return ImGuiStyleVar_DisabledAlpha;
    case TimeGUIStyleVar_WindowPadding:
        return ImGuiStyleVar_WindowPadding;
    case TimeGUIStyleVar_WindowRounding:
        return ImGuiStyleVar_WindowRounding;
    case TimeGUIStyleVar_WindowBorderSize:
        return ImGuiStyleVar_WindowBorderSize;
    case TimeGUIStyleVar_WindowMinSize:
        return ImGuiStyleVar_WindowMinSize;
    case TimeGUIStyleVar_WindowTitleAlign:
        return ImGuiStyleVar_WindowTitleAlign;
    case TimeGUIStyleVar_ChildRounding:
        return ImGuiStyleVar_ChildRounding;
    case TimeGUIStyleVar_ChildBorderSize:
        return ImGuiStyleVar_ChildBorderSize;
    case TimeGUIStyleVar_PopupRounding:
        return ImGuiStyleVar_PopupRounding;
    case TimeGUIStyleVar_PopupBorderSize:
        return ImGuiStyleVar_PopupBorderSize;
    case TimeGUIStyleVar_FramePadding:
        return ImGuiStyleVar_FramePadding;
    case TimeGUIStyleVar_FrameRounding:
        return ImGuiStyleVar_FrameRounding;
    case TimeGUIStyleVar_FrameBorderSize:
        return ImGuiStyleVar_FrameBorderSize;
    case TimeGUIStyleVar_ItemSpacing:
        return ImGuiStyleVar_ItemSpacing;
    case TimeGUIStyleVar_ItemInnerSpacing:
        return ImGuiStyleVar_ItemInnerSpacing;
    case TimeGUIStyleVar_IndentSpacing:
        return ImGuiStyleVar_IndentSpacing;
    case TimeGUIStyleVar_CellPadding:
        return ImGuiStyleVar_CellPadding;
    case TimeGUIStyleVar_ScrollbarSize:
        return ImGuiStyleVar_ScrollbarSize;
    case TimeGUIStyleVar_ScrollbarRounding:
        return ImGuiStyleVar_ScrollbarRounding;
    case TimeGUIStyleVar_ScrollbarPadding:
        return ImGuiStyleVar_ScrollbarPadding;
    case TimeGUIStyleVar_GrabMinSize:
        return ImGuiStyleVar_GrabMinSize;
    case TimeGUIStyleVar_GrabRounding:
        return ImGuiStyleVar_GrabRounding;
    case TimeGUIStyleVar_ImageRounding:
        return ImGuiStyleVar_ImageRounding;
    case TimeGUIStyleVar_ImageBorderSize:
        return ImGuiStyleVar_ImageBorderSize;
    case TimeGUIStyleVar_TabRounding:
        return ImGuiStyleVar_TabRounding;
    case TimeGUIStyleVar_TabBorderSize:
        return ImGuiStyleVar_TabBorderSize;
    case TimeGUIStyleVar_TabMinWidthBase:
        return ImGuiStyleVar_TabMinWidthBase;
    case TimeGUIStyleVar_TabMinWidthShrink:
        return ImGuiStyleVar_TabMinWidthShrink;
    case TimeGUIStyleVar_TabBarBorderSize:
        return ImGuiStyleVar_TabBarBorderSize;
    case TimeGUIStyleVar_TabBarOverlineSize:
        return ImGuiStyleVar_TabBarOverlineSize;
    default:
        return ImGuiStyleVar_Alpha;
    }
}

static ImGuiCond TranslateCond(TimeGUICond c)
{
    ImGuiCond out = 0;
    if (c & TimeGUICond_Always)
        out |= ImGuiCond_Always;
    if (c & TimeGUICond_Once)
        out |= ImGuiCond_Once;
    if (c & TimeGUICond_FirstUseEver)
        out |= ImGuiCond_FirstUseEver;
    if (c & TimeGUICond_Appearing)
        out |= ImGuiCond_Appearing;
    return out;
}

static ImGuiComboFlags TranslateComboFlags(TimeGUIComboFlags f)
{
    ImGuiComboFlags out = 0;
    if (f & TimeGUIComboFlags_PopupAlignLeft)
        out |= ImGuiComboFlags_PopupAlignLeft;
    if (f & TimeGUIComboFlags_HeightSmall)
        out |= ImGuiComboFlags_HeightSmall;
    if (f & TimeGUIComboFlags_HeightRegular)
        out |= ImGuiComboFlags_HeightRegular;
    if (f & TimeGUIComboFlags_HeightLarge)
        out |= ImGuiComboFlags_HeightLarge;
    if (f & TimeGUIComboFlags_HeightLargest)
        out |= ImGuiComboFlags_HeightLargest;
    if (f & TimeGUIComboFlags_NoArrowButton)
        out |= ImGuiComboFlags_NoArrowButton;
    if (f & TimeGUIComboFlags_NoPreview)
        out |= ImGuiComboFlags_NoPreview;
    if (f & TimeGUIComboFlags_WidthFitPreview)
        out |= ImGuiComboFlags_WidthFitPreview;
    return out;
}

void SyncFromImGui()
{
    ImGuiIO &io = ImGui::GetIO();
    s_IOInstance.DeltaTime = io.DeltaTime;
    s_IOInstance.MouseWheel = io.MouseWheel;
    s_IOInstance.MouseDelta = TEVector2(io.MouseDelta.x, io.MouseDelta.y);
    s_IOInstance.DisplaySize = TEVector2(io.DisplaySize.x, io.DisplaySize.y);
    s_IOInstance.KeyShift = io.KeyShift;
    s_IOInstance.KeyCtrl = io.KeyCtrl;
    s_IOInstance.ConfigFlags = io.ConfigFlags;

    ImGuiStyle &style = ImGui::GetStyle();
    s_StyleInstance.WindowRounding = style.WindowRounding;
    s_StyleInstance.ChildRounding = style.ChildRounding;
    s_StyleInstance.FrameRounding = style.FrameRounding;
    s_StyleInstance.PopupRounding = style.PopupRounding;
    s_StyleInstance.TabRounding = style.TabRounding;
    s_StyleInstance.GrabRounding = style.GrabRounding;
    s_StyleInstance.ScrollbarRounding = style.ScrollbarRounding;
    s_StyleInstance.WindowBorderSize = style.WindowBorderSize;
    s_StyleInstance.FrameBorderSize = style.FrameBorderSize;
    s_StyleInstance.PopupBorderSize = style.PopupBorderSize;
    s_StyleInstance.ItemSpacing = style.ItemSpacing;
    s_StyleInstance.FramePadding = style.FramePadding;
    s_StyleInstance.WindowPadding = style.WindowPadding;
}

void SyncToImGui()
{
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags = s_IOInstance.ConfigFlags;
    io.DisplaySize = ImVec2(s_IOInstance.DisplaySize.x, s_IOInstance.DisplaySize.y);

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = s_StyleInstance.WindowRounding;
    style.ChildRounding = s_StyleInstance.ChildRounding;
    style.FrameRounding = s_StyleInstance.FrameRounding;
    style.PopupRounding = s_StyleInstance.PopupRounding;
    style.TabRounding = s_StyleInstance.TabRounding;
    style.GrabRounding = s_StyleInstance.GrabRounding;
    style.ScrollbarRounding = s_StyleInstance.ScrollbarRounding;
    style.WindowBorderSize = s_StyleInstance.WindowBorderSize;
    style.FrameBorderSize = s_StyleInstance.FrameBorderSize;
    style.PopupBorderSize = s_StyleInstance.PopupBorderSize;
    style.ItemSpacing = s_StyleInstance.ItemSpacing;
    style.FramePadding = s_StyleInstance.FramePadding;
    style.WindowPadding = s_StyleInstance.WindowPadding;

    for (int i = 0; i < TimeGUICol_COUNT; ++i)
    {
        style.Colors[i] = ImVec4(s_StyleInstance.Colors[i].r, s_StyleInstance.Colors[i].g, s_StyleInstance.Colors[i].b,
                                 s_StyleInstance.Colors[i].a);
    }
}

TimeGUIIO &GetIO()
{
    SyncFromImGui();
    return s_IOInstance;
}

TimeGUIStyle &GetStyle()
{
    SyncFromImGui();
    return s_StyleInstance;
}

TimeGUIDrawList GetWindowDrawList()
{
    TimeGUIDrawList dl;
    dl.nativeDrawList = (void *)ImGui::GetWindowDrawList();
    return dl;
}

void *GetDrawListSharedData() { return (void *)ImGui::GetDrawListSharedData(); }

void TimeGUIDrawList::AddLine(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float thickness)
{
    ((::ImDrawList *)nativeDrawList)->AddLine(p1, p2, color, thickness);
}

void TimeGUIDrawList::AddRectFilled(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding)
{
    ((::ImDrawList *)nativeDrawList)->AddRectFilled(p1, p2, color, rounding);
}

void TimeGUIDrawList::AddText(const TEVector2 &pos, unsigned int color, const std::string &text)
{
    ((::ImDrawList *)nativeDrawList)->AddText(pos, color, text.c_str());
}

void TimeGUIDrawList::AddRect(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding, int flags,
                              float thickness)
{
    ((::ImDrawList *)nativeDrawList)->AddRect(p1, p2, color, rounding, flags, thickness);
}

void TimeGUIDrawList::AddPolyline(const TEVector2 *points, int num_points, unsigned int color, int flags,
                                  float thickness)
{
    ((::ImDrawList *)nativeDrawList)->AddPolyline((const ImVec2 *)points, num_points, color, flags, thickness);
}

void TimeGUIDrawList::AddConvexPolyFilled(const TEVector2 *points, int num_points, unsigned int color)
{
    ((::ImDrawList *)nativeDrawList)->AddConvexPolyFilled((const ImVec2 *)points, num_points, color);
}

void TimeGUIDrawList::AddTriangleFilled(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                        unsigned int color)
{
    ((::ImDrawList *)nativeDrawList)->AddTriangleFilled(p1, p2, p3, color);
}

void TimeGUIDrawList::AddBezierCubic(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3, const TEVector2 &p4,
                                     unsigned int color, float thickness, int num_segments)
{
    ((::ImDrawList *)nativeDrawList)->AddBezierCubic(p1, p2, p3, p4, color, thickness, num_segments);
}

void TimeGUIDrawList::AddCallback(void (*callback)(TimeGUIDrawList parent_list, const void *cmd), void *callback_data)
{
    ((::ImDrawList *)nativeDrawList)->AddCallback((::ImDrawCallback)callback, callback_data);
}

void TimeGUIDrawList::AddDrawCmd() { ((::ImDrawList *)nativeDrawList)->AddDrawCmd(); }

void TimeGUIDrawList::PushClipRect(const TEVector2 &clip_rect_min, const TEVector2 &clip_rect_max,
                                   bool intersect_with_current_clip_rect)
{
    ((::ImDrawList *)nativeDrawList)->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
}

void TimeGUIDrawList::PopClipRect() { ((::ImDrawList *)nativeDrawList)->PopClipRect(); }

void TimeGUIDrawList::PushTextureID(TimeGUITextureID texture_id)
{
    ((::ImDrawList *)nativeDrawList)->PushTextureID(ImTextureRef((ImTextureID)(uintptr_t)texture_id));
}

void TimeGUIDrawList::ResetForNewFrame() { ((::ImDrawList *)nativeDrawList)->_ResetForNewFrame(); }

void TimeGUIDrawList::AddCircle(const TEVector2 &center, float radius, unsigned int color, int num_segments,
                                float thickness)
{
    ((::ImDrawList *)nativeDrawList)->AddCircle(center, radius, color, num_segments, thickness);
}

void TimeGUIDrawList::AddCircleFilled(const TEVector2 &center, float radius, unsigned int color, int num_segments)
{
    ((::ImDrawList *)nativeDrawList)->AddCircleFilled(center, radius, color, num_segments);
}

void TimeGUIDrawList::AddQuadFilled(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3, const TEVector2 &p4,
                                    unsigned int color)
{
    ((::ImDrawList *)nativeDrawList)->AddQuadFilled(p1, p2, p3, p4, color);
}

void TimeGUIDrawList::AddImage(TimeGUITextureID user_texture_id, const TEVector2 &p_min, const TEVector2 &p_max,
                               const TEVector2 &uv_min, const TEVector2 &uv_max, unsigned int col)
{
    ImDrawList *dl = (ImDrawList *)nativeDrawList;
    dl->AddImage(ImTextureRef((ImTextureID)(uintptr_t)user_texture_id), ImVec2(p_min.x, p_min.y),
                 ImVec2(p_max.x, p_max.y), ImVec2(uv_min.x, uv_min.y), ImVec2(uv_max.x, uv_max.y), col);
}

void TimeGUIDrawList::AddText(const TimeGUIFont &font, float fontSize, const TEVector2 &pos, unsigned int color,
                              const std::string &text)
{
    ImDrawList *dl = (ImDrawList *)nativeDrawList;
    dl->AddText((ImFont *)font.nativeFont, fontSize, ImVec2(pos.x, pos.y), color, text.c_str());
}

int TimeGUIDrawList::GetVertexCount() const { return ((::ImDrawList *)nativeDrawList)->VtxBuffer.Size; }

int TimeGUIDrawList::GetCommandCount() const { return ((::ImDrawList *)nativeDrawList)->CmdBuffer.Size; }

bool Begin(const std::string &name, bool *open, TimeGUIWindowFlags flags)
{
    SyncToImGui();
    return ImGui::Begin(name.c_str(), open, TranslateWindowFlags(flags));
}

void End()
{
    ImGui::End();
    SyncFromImGui();
}

bool BeginChild(const std::string &strId, const TEVector2 &size, bool border, TimeGUIWindowFlags flags)
{
    return ImGui::BeginChild(strId.c_str(), size, border, TranslateWindowFlags(flags));
}

void EndChild() { ImGui::EndChild(); }

bool Button(const std::string &label, float width, float height)
{
    return ImGui::Button(label.c_str(), ImVec2(width, height));
}

bool Button(const std::string &label, const TEVector2 &size) { return ImGui::Button(label.c_str(), size); }

void TextUnformatted(const std::string &text) { ImGui::TextUnformatted(text.c_str()); }

void TextUnformatted(const char *text) { ImGui::TextUnformatted(text); }

void AlignTextToFramePadding() { ImGui::AlignTextToFramePadding(); }

void Text(const std::string &text) { ImGui::TextUnformatted(text.c_str()); }

void Text(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

void TextDisabled(const std::string &text) { ImGui::TextDisabled("%s", text.c_str()); }

void TextDisabled(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextDisabledV(fmt, args);
    va_end(args);
}

void TextColored(const TEColor &color, const std::string &text)
{
    ImGui::TextColored(ImVec4(color.r, color.g, color.b, color.a), "%s", text.c_str());
}

void TextColored(const TEColor &color, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextColoredV(ImVec4(color.r, color.g, color.b, color.a), fmt, args);
    va_end(args);
}

void TextColored(const TEVector4 &color, const std::string &text)
{
    ImGui::TextColored(ImVec4(color.x, color.y, color.z, color.w), "%s", text.c_str());
}

void TextColored(const TEVector4 &color, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextColoredV(ImVec4(color.x, color.y, color.z, color.w), fmt, args);
    va_end(args);
}

void TextWrapped(const std::string &text) { ImGui::TextWrapped("%s", text.c_str()); }

void TextWrapped(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextWrappedV(fmt, args);
    va_end(args);
}

bool Checkbox(const std::string &label, bool *checked) { return ImGui::Checkbox(label.c_str(), checked); }

bool DragFloat(const std::string &label, float *value, float speed, float min, float max, const std::string &format,
               int flags)
{
    return ImGui::DragFloat(label.c_str(), value, speed, min, max, format.c_str(), flags);
}

bool DragFloat2(const std::string &label, float *v, float speed, float min, float max, const std::string &format,
                int flags)
{
    return ImGui::DragFloat2(label.c_str(), v, speed, min, max, format.c_str(), flags);
}

bool DragFloat3(const std::string &label, float *v, float speed, float min, float max, const std::string &format,
                int flags)
{
    return ImGui::DragFloat3(label.c_str(), v, speed, min, max, format.c_str(), flags);
}

bool DragFloat4(const std::string &label, float *v, float speed, float min, float max, const std::string &format,
                int flags)
{
    return ImGui::DragFloat4(label.c_str(), v, speed, min, max, format.c_str(), flags);
}

bool DragInt(const std::string &label, int *v, float speed, int min, int max)
{
    return ImGui::DragInt(label.c_str(), v, speed, min, max);
}

bool InputInt(const std::string &label, int *v, int step, int step_fast, int flags)
{
    return ImGui::InputInt(label.c_str(), v, step, step_fast, flags);
}

bool SliderFloat(const std::string &label, float *v, float v_min, float v_max, const std::string &format, int flags)
{
    return ImGui::SliderFloat(label.c_str(), v, v_min, v_max, format.c_str(), flags);
}

bool ColorEdit3(const std::string &label, float *col) { return ImGui::ColorEdit3(label.c_str(), col); }

bool ColorEdit4(const std::string &label, float *col) { return ImGui::ColorEdit4(label.c_str(), col); }

bool ColorPicker4(const std::string &label, float *col, int flags)
{
    return ImGui::ColorPicker4(label.c_str(), col, TranslateColorEditFlags((TimeGUIColorEditFlags)flags));
}

bool SmallButton(const std::string &label) { return ImGui::SmallButton(label.c_str()); }

bool Combo(const std::string &label, int *currentItem, const char *const items[], int itemsCount,
           int popupMaxHeightInItems)
{
    return ImGui::Combo(label.c_str(), currentItem, items, itemsCount, popupMaxHeightInItems);
}

bool BeginCombo(const std::string &label, const std::string &previewValue, int flags)
{
    return ImGui::BeginCombo(label.c_str(), previewValue.c_str(), TranslateComboFlags((TimeGUIComboFlags)flags));
}

void EndCombo() { ImGui::EndCombo(); }

bool Selectable(const std::string &label, bool selected, int flags, const TEVector2 &size)
{
    return ImGui::Selectable(label.c_str(), selected, TranslateSelectableFlags((TimeGUISelectableFlags)flags), size);
}

bool Selectable(const std::string &label, bool *selected, int flags, const TEVector2 &size)
{
    return ImGui::Selectable(label.c_str(), selected, TranslateSelectableFlags((TimeGUISelectableFlags)flags), size);
}

bool InputText(const std::string &label, std::string &value)
{
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
    if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer)))
    {
        value = buffer;
        return true;
    }
    return false;
}

bool InputText(const std::string &label, char *buf, size_t bufSize)
{
    return ImGui::InputText(label.c_str(), buf, bufSize);
}

bool InputText(const std::string &label, char *buf, size_t bufSize, TimeGUIInputTextFlags flags)
{
    return ImGui::InputText(label.c_str(), buf, bufSize, TranslateInputTextFlags(flags));
}

bool InputTextMultiline(const std::string &label, char *buf, size_t bufSize, const TEVector2 &size, int flags)
{
    return ImGui::InputTextMultiline(label.c_str(), buf, bufSize, size,
                                     TranslateInputTextFlags((TimeGUIInputTextFlags)flags));
}

void Separator() { ImGui::Separator(); }

void SeparatorEx(int flags) { ImGui::SeparatorEx(flags); }

void Spacing() { ImGui::Spacing(); }

void NewLine() { ImGui::NewLine(); }

void SameLine(float offset_from_start_x, float spacing) { ImGui::SameLine(offset_from_start_x, spacing); }

void Dummy(const TEVector2 &size) { ImGui::Dummy(size); }

void BeginGroup() { ImGui::BeginGroup(); }

void EndGroup() { ImGui::EndGroup(); }

void Indent(float indentW) { ImGui::Indent(indentW); }

void Unindent(float indentW) { ImGui::Unindent(indentW); }

void Columns(int count, const char *id, bool border) { ImGui::Columns(count, id, border); }

void NextColumn() { ImGui::NextColumn(); }

void SetColumnWidth(int columnIndex, float width) { ImGui::SetColumnWidth(columnIndex, width); }

void PushID(const std::string &strId) { ImGui::PushID(strId.c_str()); }

void PushID(int intId) { ImGui::PushID(intId); }

void PopID() { ImGui::PopID(); }

bool TreeNodeEx(const std::string &label, int flags)
{
    return ImGui::TreeNodeEx(label.c_str(), TranslateTreeNodeFlags(flags));
}

bool TreeNodeEx(void *ptrId, int flags, const std::string &text)
{
    return ImGui::TreeNodeEx(ptrId, TranslateTreeNodeFlags(flags), "%s", text.c_str());
}

void TreePop() { ImGui::TreePop(); }

bool CollapsingHeader(const std::string &label, int flags)
{
    return ImGui::CollapsingHeader(label.c_str(), TranslateTreeNodeFlags(flags));
}

void OpenPopup(const std::string &strId) { ImGui::OpenPopup(strId.c_str()); }

bool BeginPopup(const std::string &strId, int flags)
{
    return ImGui::BeginPopup(strId.c_str(), TranslateWindowFlags(flags));
}

bool BeginPopupContextWindow(const std::string &strId, int mouseButton, bool alsoOverItems)
{
    ImGuiPopupFlags flags = mouseButton;
    if (!alsoOverItems)
        flags |= ImGuiPopupFlags_NoOpenOverItems;
    return ImGui::BeginPopupContextWindow(strId.empty() ? nullptr : strId.c_str(), flags);
}

bool BeginPopupContextItem(const std::string &strId, int mouseButton)
{
    return ImGui::BeginPopupContextItem(strId.empty() ? nullptr : strId.c_str(), mouseButton);
}

bool BeginPopupModal(const std::string &name, bool *open, TimeGUIWindowFlags flags)
{
    return ImGui::BeginPopupModal(name.c_str(), open, TranslateWindowFlags(flags));
}

void EndPopup() { ImGui::EndPopup(); }

void CloseCurrentPopup() { ImGui::CloseCurrentPopup(); }

void PushStyleColor(TimeGUICol idx, const TEColor &color)
{
    ImGui::PushStyleColor(TranslateColorEnum(idx), ImVec4(color.r, color.g, color.b, color.a));
}

void PopStyleColor(int count) { ImGui::PopStyleColor(count); }

void PushStyleVar(TimeGUIStyleVar idx, float val) { ImGui::PushStyleVar(TranslateStyleVarEnum(idx), val); }

void PushStyleVar(TimeGUIStyleVar idx, const TEVector2 &val) { ImGui::PushStyleVar(TranslateStyleVarEnum(idx), val); }

void PopStyleVar(int count) { ImGui::PopStyleVar(count); }

void SetNextWindowPos(const TEVector2 &pos, TimeGUICond cond, const TEVector2 &pivot)
{
    ImGui::SetNextWindowPos(pos, TranslateCond(cond), pivot);
}

void SetNextWindowSize(const TEVector2 &size, TimeGUICond cond) { ImGui::SetNextWindowSize(size, TranslateCond(cond)); }

void SetNextItemWidth(float itemWidth) { ImGui::SetNextItemWidth(itemWidth); }

void PopItemWidth() { ImGui::PopItemWidth(); }

void SetNextItemAllowOverlap() { ImGui::SetNextItemAllowOverlap(); }

void Image(TimeGUITextureID userTextureId, const TEVector2 &size, const TEVector2 &uv0, const TEVector2 &uv1)
{
    ImGui::Image(ImTextureRef((ImTextureID)(uintptr_t)userTextureId), ImVec2(size.x, size.y), ImVec2(uv0.x, uv0.y),
                 ImVec2(uv1.x, uv1.y));
}

bool ImageButton(const std::string &strId, TimeGUITextureID userTextureId, const TEVector2 &size, const TEVector2 &uv0,
                 const TEVector2 &uv1)
{
    return ImGui::ImageButton(strId.c_str(), ImTextureRef((ImTextureID)(uintptr_t)userTextureId),
                              ImVec2(size.x, size.y), ImVec2(uv0.x, uv0.y), ImVec2(uv1.x, uv1.y));
}

float GetWindowWidth() { return ImGui::GetWindowWidth(); }

float GetWindowHeight() { return ImGui::GetWindowHeight(); }

TEVector2 GetWindowSize() { return ImGui::GetWindowSize(); }

TEVector2 GetWindowPos() { return ImGui::GetWindowPos(); }

TEVector2 GetMousePos() { return ImGui::GetMousePos(); }

void SetCursorScreenPos(const TEVector2 &pos) { ImGui::SetCursorScreenPos(pos); }

void SetCursorPos(const TEVector2 &localPos) { ImGui::SetCursorPos(localPos); }

void SetCursorPosX(float x) { ImGui::SetCursorPosX(x); }

void SetCursorPosY(float y) { ImGui::SetCursorPosY(y); }

float GetCursorPosX() { return ImGui::GetCursorPosX(); }

float GetCursorPosY() { return ImGui::GetCursorPosY(); }

TEVector2 GetCursorScreenPos() { return ImGui::GetCursorScreenPos(); }
void SetScrollHereY(float centerYRatio) { ImGui::SetScrollHereY(centerYRatio); }

bool IsItemHovered(int flags) { return ImGui::IsItemHovered(TranslateHoveredFlags((TimeGUIHoveredFlags)flags)); }

bool IsItemActive() { return ImGui::IsItemActive(); }

bool IsItemClicked(int mouseButton) { return ImGui::IsItemClicked(mouseButton); }

bool IsWindowHovered(int flags) { return ImGui::IsWindowHovered(TranslateHoveredFlags((TimeGUIHoveredFlags)flags)); }

bool IsWindowFocused(int flags) { return ImGui::IsWindowFocused(flags); }

bool IsMouseDown(int button) { return ImGui::IsMouseDown(button); }

bool IsMouseReleased(int button) { return ImGui::IsMouseReleased(button); }

bool IsMouseClicked(int button, bool repeat) { return ImGui::IsMouseClicked(button, repeat); }

bool IsAnyItemHovered() { return ImGui::IsAnyItemHovered(); }

bool IsMouseDoubleClicked(int button) { return ImGui::IsMouseDoubleClicked(button); }

bool IsMouseDragging(int button, float lock_threshold) { return ImGui::IsMouseDragging(button, lock_threshold); }

TEVector2 GetMouseDragDelta(int button, float lock_threshold)
{
    ImVec2 d = ImGui::GetMouseDragDelta(button, lock_threshold);
    return TEVector2(d.x, d.y);
}

double GetTime() { return ImGui::GetTime(); }

TimeGUIViewport GetMainViewport()
{
    auto *vp = ImGui::GetMainViewport();
    TimeGUIViewport tvp;
    tvp.Pos = TEVector2(vp->Pos.x, vp->Pos.y);
    tvp.Size = TEVector2(vp->Size.x, vp->Size.y);
    tvp.ID = vp->ID;
    return tvp;
}

void SetNextWindowViewport(unsigned int viewportId) { ImGui::SetNextWindowViewport(viewportId); }

unsigned int GetID(const std::string &strId) { return ImGui::GetID(strId.c_str()); }

void SetItemTooltip(const std::string &text) { ImGui::SetItemTooltip("%s", text.c_str()); }

void SetItemTooltip(const char *fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ImGui::SetItemTooltip("%s", buf);
}

void SetTooltip(const std::string &text) { ImGui::SetTooltip("%s", text.c_str()); }

void SetItemDefaultFocus() { ImGui::SetItemDefaultFocus(); }

void SetMouseCursor(int cursorType) { ImGui::SetMouseCursor(cursorType); }

unsigned int GetColorU32(const TEColor &color)
{
    return ImGui::GetColorU32(ImVec4(color.r, color.g, color.b, color.a));
}

TEVector2 CalcTextSize(const std::string &text) { return ImGui::CalcTextSize(text.c_str()); }

bool BeginDragDropSource(int flags) { return ImGui::BeginDragDropSource(flags); }

bool SetDragDropPayload(const std::string &type, const void *data, size_t size, int cond)
{
    return ImGui::SetDragDropPayload(type.c_str(), data, size, cond);
}

void EndDragDropSource() { ImGui::EndDragDropSource(); }

unsigned int DockSpace(unsigned int id, const TEVector2 &size, int flags)
{
    return ImGui::DockSpace(id, size, TranslateDockNodeFlags((TimeGUIDockNodeFlags)flags));
}

void DockBuilderRemoveNode(unsigned int nodeId) { ImGui::DockBuilderRemoveNode(nodeId); }

void DockBuilderAddNode(unsigned int nodeId, int flags)
{
    ImGui::DockBuilderAddNode(nodeId, TranslateDockNodeFlags((TimeGUIDockNodeFlags)flags));
}

void DockBuilderSetNodeSize(unsigned int nodeId, const TEVector2 &size) { ImGui::DockBuilderSetNodeSize(nodeId, size); }

unsigned int DockBuilderSplitNode(unsigned int nodeId, int splitDir, float sizeRatio, unsigned int *outIdDir1,
                                  unsigned int *outIdDir2)
{
    return ImGui::DockBuilderSplitNode(nodeId, (ImGuiDir)splitDir, sizeRatio, outIdDir1, outIdDir2);
}

void DockBuilderDockWindow(const std::string &windowName, unsigned int nodeId)
{
    ImGui::DockBuilderDockWindow(windowName.c_str(), nodeId);
}

void DockBuilderFinish(unsigned int nodeId) { ImGui::DockBuilderFinish(nodeId); }

bool BeginTabItem(const std::string &label, bool *open, int flags)
{
    return ImGui::BeginTabItem(label.c_str(), open, flags);
}

void EndTabItem() { ImGui::EndTabItem(); }

bool BeginTabBar(const std::string &strId, int flags) { return ImGui::BeginTabBar(strId.c_str(), flags); }

void EndTabBar() { ImGui::EndTabBar(); }

bool BeginMenuBar() { return ImGui::BeginMenuBar(); }

void EndMenuBar() { ImGui::EndMenuBar(); }

bool BeginMenu(const std::string &label, bool enabled) { return ImGui::BeginMenu(label.c_str(), enabled); }

void EndMenu() { ImGui::EndMenu(); }

bool BeginTable(const std::string &strId, int column, int flags, const TEVector2 &outerSize, float innerWidth)
{
    return ImGui::BeginTable(strId.c_str(), column, TranslateTableFlags((TimeGUITableFlags)flags), outerSize,
                             innerWidth);
}

void EndTable() { ImGui::EndTable(); }

void TableSetupColumn(const std::string &label, int flags, float initWidthOrWeight, unsigned int userId)
{
    ImGui::TableSetupColumn(label.c_str(), TranslateTableColumnFlags((TimeGUITableColumnFlags)flags), initWidthOrWeight,
                            userId);
}

void TableHeadersRow() { ImGui::TableHeadersRow(); }

bool TableNextColumn() { return ImGui::TableNextColumn(); }

void TableNextRow(float rowMinHeight, int rowFlags) { ImGui::TableNextRow(rowMinHeight, rowFlags); }

std::string CleanLabel(const std::string &label)
{
    size_t pos = label.find("###");
    if (pos != std::string::npos)
        return label.substr(0, pos);
    return label;
}

bool DrawVec3Control(const std::string &label, TEVector &values, float resetValue, float columnWidth)
{
    bool changed = false;
    PushID(label);

    Columns(2);
    SetColumnWidth(0, columnWidth);
    Text(CleanLabel(label));
    NextColumn();

    float lineHeight = GetFrameHeight();
    TEVector2 buttonSize = {lineHeight + 3.0f, lineHeight};
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float itemWidth = (ImGui::GetContentRegionAvail().x - buttonSize.x * 3.0f - spacing * 2.0f) / 3.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    // X
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    if (ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    ImGui::SameLine();

    // Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    if (ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    ImGui::SameLine();

    // Z
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    if (ImGui::Button("Z", buttonSize))
    {
        values.z = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;

    ImGui::PopStyleVar();
    Columns(1);
    PopID();

    return changed;
}

bool DrawVec2Control(const std::string &label, TEVector2 &values, float resetValue, float columnWidth)
{
    bool changed = false;
    PushID(label);

    Columns(2);
    SetColumnWidth(0, columnWidth);
    Text(CleanLabel(label));
    NextColumn();

    float lineHeight = GetFrameHeight();
    TEVector2 buttonSize = {lineHeight + 3.0f, lineHeight};
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float itemWidth = (ImGui::GetContentRegionAvail().x - buttonSize.x * 2.0f - spacing * 1.0f) / 2.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    // X
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    if (ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;
    ImGui::SameLine();

    // Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    if (ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        changed = true;

    ImGui::PopStyleVar();
    Columns(1);
    PopID();
    return changed;
}

bool DrawVec4Control(const std::string &label, TEVector4 &values, float resetValue, float columnWidth)
{
    bool changed = false;
    PushID(label);

    Columns(2);
    SetColumnWidth(0, columnWidth);
    Text(CleanLabel(label));
    NextColumn();

    float lineHeight = GetFrameHeight();
    TEVector2 buttonSize = {lineHeight + 3.0f, lineHeight};
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float itemWidth = (ImGui::GetContentRegionAvail().x - buttonSize.x * 4.0f - spacing * 3.0f) / 4.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    static const char *labels[] = {"R", "G", "B", "A"};
    static ImVec4 colors[] = {
        {0.8f, 0.1f, 0.15f, 1.0f}, {0.2f, 0.7f, 0.2f, 1.0f}, {0.1f, 0.25f, 0.8f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}};

    float *vals[] = {&values.x, &values.y, &values.z, &values.w};

    for (int i = 0; i < 4; i++)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, colors[i]);
        if (ImGui::Button(labels[i], buttonSize))
        {
            *vals[i] = resetValue;
            changed = true;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::DragFloat((std::string("##") + labels[i]).c_str(), vals[i], 0.1f, 0.0f, 0.0f, "%.2f"))
            changed = true;
        if (i < 3)
            ImGui::SameLine();
    }

    ImGui::PopStyleVar();
    Columns(1);
    PopID();
    return changed;
}

bool DrawColorControl(const std::string &label, TEColor &values, float columnWidth)
{
    bool changed = false;
    PushID(label);

    Columns(2);
    SetColumnWidth(0, columnWidth);
    Text(CleanLabel(label));
    NextColumn();

    float lineHeight = GetFrameHeight();
    TEVector2 pickerSize = {lineHeight * 1.5f, lineHeight};
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float itemWidth = (ImGui::GetContentRegionAvail().x - pickerSize.x - spacing * 4.0f) / 4.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{spacing, ImGui::GetStyle().ItemSpacing.y});

    ImVec4 valVec = {values.r, values.g, values.b, values.a};
    if (ImGui::ColorButton("##ColorPicker", valVec, ImGuiColorEditFlags_AlphaPreviewHalf, pickerSize))
        ImGui::OpenPopup("##ColorPickerPopup");

    if (ImGui::BeginPopup("##ColorPickerPopup"))
    {
        if (ImGui::ColorPicker4("##Picker", &values.r,
                                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB |
                                    ImGuiColorEditFlags_InputRGB))
            changed = true;
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    float *vals[] = {&values.r, &values.g, &values.b, &values.a};
    for (int i = 0; i < 4; i++)
    {
        ImGui::SetNextItemWidth(itemWidth);
        const char *ids[] = {"##R", "##G", "##B", "##A"};
        if (ImGui::DragFloat(ids[i], vals[i], 0.01f, 0.0f, 1.0f, "%.2f"))
            changed = true;
        if (i < 3)
            ImGui::SameLine();
    }

    ImGui::PopStyleVar();
    Columns(1);
    PopID();
    return changed;
}

bool DrawDeleteButton(const std::string &id, float size, float fontScale)
{
    if (size == 0.0f)
        size = GetFrameHeight() * 0.8f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool pressed = ImGui::InvisibleButton(id.c_str(), ImVec2{size, size});

    ImVec4 color = {0.7f, 0.1f, 0.1f, 0.6f};
    if (ImGui::IsItemActive())
        color = {0.5f, 0.1f, 0.1f, 0.7f};
    else if (ImGui::IsItemHovered())
        color = {0.9f, 0.2f, 0.2f, 0.8f};

    auto *drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(pos, {pos.x + size, pos.y + size}, ImGui::GetColorU32(color), 3.0f);

    const char *text = "x";
    ImGui::SetWindowFontScale(fontScale);
    ImVec2 textSize = ImGui::CalcTextSize(text);
    drawList->AddText({pos.x + (size - textSize.x) * 0.5f, pos.y + (size - textSize.y) * 0.5f + 1.0f},
                      ImGui::GetColorU32(ImGuiCol_Text), text);
    ImGui::SetWindowFontScale(1.0f);

    return pressed;
}

bool DrawPlusButton(const std::string &id, float size, float fontScale)
{
    if (size == 0.0f)
        size = GetFrameHeight() * 0.8f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool pressed = ImGui::InvisibleButton(id.c_str(), ImVec2{size, size});

    ImVec4 color = {0.1f, 0.7f, 0.1f, 0.6f};
    if (ImGui::IsItemActive())
        color = {0.1f, 0.5f, 0.1f, 0.7f};
    else if (ImGui::IsItemHovered())
        color = {0.2f, 0.9f, 0.2f, 0.8f};

    auto *drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(pos, {pos.x + size, pos.y + size}, ImGui::GetColorU32(color), 3.0f);

    const char *text = "+";
    ImGui::SetWindowFontScale(fontScale);
    ImVec2 textSize = ImGui::CalcTextSize(text);
    drawList->AddText({pos.x + (size - textSize.x) * 0.5f, pos.y + (size - textSize.y) * 0.5f + 1.0f},
                      ImGui::GetColorU32(ImGuiCol_Text), text);
    ImGui::SetWindowFontScale(1.0f);

    return pressed;
}

float GetFrameHeight() { return ImGui::GetFrameHeight(); }

TEVector2 GetContentRegionAvail() { return ImGui::GetContentRegionAvail(); }

bool IsKeyPressed(int key) { return ImGui::IsKeyPressed((ImGuiKey)key); }

bool RadioButton(const std::string &label, bool active) { return ImGui::RadioButton(label.c_str(), active); }

bool RadioButton(const std::string &label, int *v, int v_button)
{
    return ImGui::RadioButton(label.c_str(), v, v_button);
}

bool SliderInt(const std::string &label, int *v, int v_min, int v_max, const std::string &format, int flags)
{
    return ImGui::SliderInt(label.c_str(), v, v_min, v_max, format.c_str(), flags);
}

void PushFont(const TimeGUIFont &font) { ImGui::PushFont((ImFont *)font.nativeFont); }

void PopFont() { ImGui::PopFont(); }

TEVector4 ColorConvertU32ToFloat4(unsigned int in) { return ImGui::ColorConvertU32ToFloat4(in); }

unsigned int ColorConvertFloat4ToU32(const TEVector4 &in) { return ImGui::ColorConvertFloat4ToU32(in); }

unsigned int GetColorU32(TimeGUICol idx, float alpha_mul) { return ImGui::GetColorU32((ImGuiCol)idx, alpha_mul); }

bool ColorEdit4(const std::string &label, float *col, int flags)
{
    return ImGui::ColorEdit4(label.c_str(), col, flags);
}

void TextFormatted(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

TimeGUIDrawList CreateDrawList()
{
    TimeGUIDrawList dl;
    ImDrawList *native = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
    native->_ResetForNewFrame();
    dl.nativeDrawList = native;
    return dl;
}

void DestroyDrawList(TimeGUIDrawList &dl)
{
    ImDrawList *native = (ImDrawList *)dl.nativeDrawList;
    IM_DELETE(native);
    dl.nativeDrawList = nullptr;
}

void RenderDrawList(const TimeGUIDrawList &dl, const TEVector2 &displaySize)
{
    ImDrawData drawData;
    drawData.Valid = true;
    drawData.Textures = nullptr;
    drawData.AddDrawList((ImDrawList *)dl.nativeDrawList);
    drawData.DisplayPos = ImVec2(0.0f, 0.0f);
    drawData.DisplaySize = ImVec2(displaySize.x, displaySize.y);
    drawData.FramebufferScale = ImVec2(1.0f, 1.0f);

    ImGui_ImplOpenGL3_RenderDrawData(&drawData);
}

void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b)
{
    ImGui::ColorConvertHSVtoRGB(h, s, v, out_r, out_g, out_b);
}

void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v)
{
    ImGui::ColorConvertRGBtoHSV(r, g, b, out_h, out_s, out_v);
}

bool IsItemDeactivatedAfterEdit() { return ImGui::IsItemDeactivatedAfterEdit(); }

TimeGUIDrawList GetBackgroundDrawList()
{
    TimeGUIDrawList dl;
    dl.nativeDrawList = ImGui::GetBackgroundDrawList();
    return dl;
}

TimeGUIFont GetDefaultFont()
{
    TimeGUIFont f;
    f.nativeFont = ImGui::GetFont();
    return f;
}

TimeGUITextureID GetFontAtlasTextureID() { return (TimeGUITextureID)(uintptr_t)ImGui::GetIO().Fonts->TexID.GetTexID(); }

TEVector2 TimeGUIFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char *text_begin) const
{
    ImFont *f = (ImFont *)nativeFont;
    ImVec2 sz = f->CalcTextSizeA(size, max_width, wrap_width, text_begin);
    return TEVector2(sz.x, sz.y);
}

::ImFont *TimeGUIFont::operator->() const { return (::ImFont *)nativeFont; }

bool BeginProjectCard(const std::string &id, const TEVector2 &size, bool &hovered)
{
    ImGui::PushID(id.c_str());
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, ImGui::GetID(id.c_str())))
        return false;

    bool held;
    bool pressed = ImGui::ButtonBehavior(bb, ImGui::GetID(id.c_str()), &hovered, &held);

    // Glass Card Background
    ImU32 bgCol = ImGui::GetColorU32(hovered ? ImGuiCol_HeaderHovered : ImGuiCol_WindowBg);
    float alpha = hovered ? 0.6f : 0.4f;
    ImVec4 bgVec = ImGui::ColorConvertU32ToFloat4(bgCol);
    bgVec.w = alpha;

    window->DrawList->AddRectFilled(pos, bb.Max, ImGui::ColorConvertFloat4ToU32(bgVec), 12.0f);
    window->DrawList->AddRect(pos, bb.Max, ImGui::GetColorU32(ImGuiCol_Border, 0.5f), 12.0f, 0, 1.5f);

    return pressed;
}

void EndProjectCard() { ImGui::PopID(); }

void PushItemWidth(float itemWidth) { ImGui::PushItemWidth(itemWidth); }

void BeginDisabled(bool disabled) { ImGui::BeginDisabled(disabled); }

void EndDisabled() { ImGui::EndDisabled(); }

static ImVec2 s_SavedMousePos;
static bool s_SavedMouseDown[5];
static bool s_InputSuspended = false;

void PushSuspendedInput(const TEVector2 &stubMousePos)
{
    if (s_InputSuspended)
        return;
    ImGuiIO &io = ImGui::GetIO();
    s_SavedMousePos = io.MousePos;
    for (int i = 0; i < 5; i++)
    {
        s_SavedMouseDown[i] = io.MouseDown[i];
        io.MouseDown[i] = false;
    }
    io.MousePos = ImVec2(stubMousePos.x, stubMousePos.y);
    s_InputSuspended = true;
}

void PopSuspendedInput()
{
    if (!s_InputSuspended)
        return;
    ImGuiIO &io = ImGui::GetIO();
    io.MousePos = s_SavedMousePos;
    for (int i = 0; i < 5; i++)
    {
        io.MouseDown[i] = s_SavedMouseDown[i];
    }
    s_InputSuspended = false;
}

void PushMultiItemsWidths(int components, float width_full) { ImGui::PushMultiItemsWidths(components, width_full); }

float CalcItemWidth() { return ImGui::CalcItemWidth(); }

bool MenuItem(const std::string &label, const std::string &shortcut, bool selected, bool enabled)
{
    return ImGui::MenuItem(label.c_str(), shortcut.empty() ? nullptr : shortcut.c_str(), selected, enabled);
}

bool MenuItem(const std::string &label, const std::string &shortcut, bool *p_selected, bool enabled)
{
    return ImGui::MenuItem(label.c_str(), shortcut.empty() ? nullptr : shortcut.c_str(), p_selected, enabled);
}

TEVector2 GetCursorPos()
{
    ImVec2 p = ImGui::GetCursorPos();
    return TEVector2(p.x, p.y);
}

TEVector2 GetWindowContentRegionMin()
{
    ImVec2 p = ImGui::GetWindowContentRegionMin();
    return TEVector2(p.x, p.y);
}

bool IsItemFocused() { return ImGui::IsItemFocused(); }

TEVector2 GetItemRectMin()
{
    ImVec2 p = ImGui::GetItemRectMin();
    return TEVector2(p.x, p.y);
}

TEVector2 GetItemRectSize()
{
    ImVec2 p = ImGui::GetItemRectSize();
    return TEVector2(p.x, p.y);
}
} // namespace TimeGUI

} // namespace TE

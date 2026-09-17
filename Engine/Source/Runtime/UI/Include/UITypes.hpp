#pragma once
#include "PreRequisites.h"
#include "TEColor.hpp"
#include "MathUtils.hpp"
#include "EngineTypes/TEString.hpp"

// Independent font wrapper
struct TE_API UIFont
{
    void *nativeFont = nullptr;
    TEVector2 CalcTextSizeA(float size, float max_width, float wrap_width, const char *text_begin) const
    {
        return TEVector2(0.0f, 0.0f);
    }
};

// Independent IO wrapper structure
struct TE_API UIIO
{
    float DeltaTime = 0.0f;
    float MouseWheel = 0.0f;
    TEVector2 MouseDelta;
    TEVector2 DisplaySize;
    bool KeyShift = false;
    bool KeyCtrl = false;
    bool KeyAlt = false;
    bool KeySuper = false;
    bool WantTextInput = false;
    bool WantCaptureKeyboard = false;
    bool WantCaptureMouse = false;
    int ConfigFlags = 0;
    UIFont DefaultFont;
};

// Independent style colors enumeration
enum UIStyleCol_
{
    UIStyleCol_Text,
    UIStyleCol_TextDisabled,
    UIStyleCol_WindowBg,
    UIStyleCol_ChildBg,
    UIStyleCol_PopupBg,
    UIStyleCol_Border,
    UIStyleCol_BorderShadow,
    UIStyleCol_FrameBg,
    UIStyleCol_FrameBgHovered,
    UIStyleCol_FrameBgActive,
    UIStyleCol_TitleBg,
    UIStyleCol_TitleBgActive,
    UIStyleCol_TitleBgCollapsed,
    UIStyleCol_MenuBarBg,
    UIStyleCol_ScrollbarBg,
    UIStyleCol_ScrollbarGrab,
    UIStyleCol_ScrollbarGrabHovered,
    UIStyleCol_ScrollbarGrabActive,
    UIStyleCol_CheckMark,
    UIStyleCol_CheckboxSelectedBg,
    UIStyleCol_SliderGrab,
    UIStyleCol_SliderGrabActive,
    UIStyleCol_Button,
    UIStyleCol_ButtonHovered,
    UIStyleCol_ButtonActive,
    UIStyleCol_Header,
    UIStyleCol_HeaderHovered,
    UIStyleCol_HeaderActive,
    UIStyleCol_Separator,
    UIStyleCol_SeparatorHovered,
    UIStyleCol_SeparatorActive,
    UIStyleCol_ResizeGrip,
    UIStyleCol_ResizeGripHovered,
    UIStyleCol_ResizeGripActive,
    UIStyleCol_InputTextCursor,
    UIStyleCol_TabHovered,
    UIStyleCol_Tab,
    UIStyleCol_TabSelected,
    UIStyleCol_TabSelectedOverline,
    UIStyleCol_TabDimmed,
    UIStyleCol_TabDimmedSelected,
    UIStyleCol_TabDimmedSelectedOverline,
    UIStyleCol_DockingPreview,
    UIStyleCol_DockingEmptyBg,
    UIStyleCol_PlotLines,
    UIStyleCol_PlotLinesHovered,
    UIStyleCol_PlotHistogram,
    UIStyleCol_PlotHistogramHovered,
    UIStyleCol_TableHeaderBg,
    UIStyleCol_TableBorderStrong,
    UIStyleCol_TableBorderLight,
    UIStyleCol_TableRowBg,
    UIStyleCol_TableRowBgAlt,
    UIStyleCol_TextLink,
    UIStyleCol_TextSelectedBg,
    UIStyleCol_TreeLines,
    UIStyleCol_DragDropTarget,
    UIStyleCol_DragDropTargetBg,
    UIStyleCol_UnsavedMarker,
    UIStyleCol_NavCursor,
    UIStyleCol_NavWindowingHighlight,
    UIStyleCol_NavWindowingDimBg,
    UIStyleCol_ModalWindowDimBg,
    UIStyleCol_COUNT,

    UIStyleCol_TabActive = UIStyleCol_TabSelected,
    UIStyleCol_TabUnfocused = UIStyleCol_TabDimmed,
    UIStyleCol_TabUnfocusedActive = UIStyleCol_TabDimmedSelected,
    UIStyleCol_NavHighlight = UIStyleCol_NavCursor,
};
typedef int UIStyleCol;

// Independent clean Style wrapper structure
struct TE_API UIStyle
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

    TEColor Colors[UIStyleCol_COUNT];
};

enum TimeGUIDragDropFlags_
{
    TimeGUIDragDropFlags_None                         = 0,
    // Source flags
    TimeGUIDragDropFlags_SourceNoPreviewTooltip       = 1 << 0,
    TimeGUIDragDropFlags_SourceNoDisableHover         = 1 << 1,
    TimeGUIDragDropFlags_SourceNoHoldToOpenOthers     = 1 << 2,
    TimeGUIDragDropFlags_SourceAllowNullID            = 1 << 3,
    TimeGUIDragDropFlags_SourceExtern                 = 1 << 4,
    TimeGUIDragDropFlags_SourceAutoExpirePayload      = 1 << 5,
    // Target flags
    TimeGUIDragDropFlags_AcceptBeforeDelivery         = 1 << 10,
    TimeGUIDragDropFlags_AcceptNoDrawDefaultRect      = 1 << 11,
    TimeGUIDragDropFlags_AcceptNoPreviewTooltip       = 1 << 12,
    TimeGUIDragDropFlags_AcceptPeekOnly               = TimeGUIDragDropFlags_AcceptBeforeDelivery | TimeGUIDragDropFlags_AcceptNoDrawDefaultRect
};
typedef int TimeGUIDragDropFlags;

struct TE_API TimeGUIPayload
{
    void *Data = nullptr;
    int DataSize = 0;
    TEString DataType;
    bool IsPreview = false;
    bool IsDelivery = false;

    bool IsDataType(const TEString &type) const
    {
        return DataType == type;
    }
};


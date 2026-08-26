#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/Texture.hpp"
#include <functional>

enum class EditorToolbarAlignment
{
    Left,
    Center,
    Right
};

struct EditorToolbarItem
{
    TEString id;
    TEString label;
    TEString tooltip;
    EditorToolbarAlignment alignment = EditorToolbarAlignment::Center;
    int priority = 0; // Lower numbers render first within alignment group

    TERef<Texture> icon = nullptr;
    float width = 0.0f;

    std::function<bool()> isVisible = nullptr;
    std::function<bool()> isEnabled = nullptr;
    std::function<bool()> isActive = nullptr;
    std::function<void()> onClick = nullptr;

    // Optional custom TimeGUI render delegate (for combos, sliders, popups)
    std::function<void()> onCustomRender = nullptr;

    float GetWidth() const;
};

class TE_API EditorToolbarRegistry
{
public:
    static void RegisterItem(const EditorToolbarItem &item);
    static void UnregisterItem(const TEString &id);
    static TEArray<EditorToolbarItem> GetItems(EditorToolbarAlignment alignment);
    static void Clear();
};

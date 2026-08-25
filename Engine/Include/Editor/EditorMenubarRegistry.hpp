#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <functional>


struct EditorMenubarItem
{
    TEString id;
    TEString category; // "File", "Edit", "Window", "Tools", "Help", etc.
    TEString label;
    TEString shortcut;
    int priority = 0;

    std::function<bool()> isVisible = nullptr;
    std::function<bool()> isChecked = nullptr;
    std::function<bool()> isEnabled = nullptr;
    std::function<void()> onClick = nullptr;
};

class TE_API EditorMenubarRegistry
{
public:
    static void RegisterItem(const EditorMenubarItem &item);
    static void UnregisterItem(const TEString &id);
    static TEArray<TEString> GetCategories();
    static TEArray<EditorMenubarItem> GetItemsInCategory(const TEString &category);
    static void Clear();
    static void OnTimeGUIRender();
};


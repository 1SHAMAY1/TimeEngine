#include "Editor/EditorMenubarRegistry.hpp"
#include "Core/PreRequisites.h"
#include "Utils/TimeGUI.hpp"
#include <algorithm>

static TEMap<TEString, EditorMenubarItem> &GetItemsMap()
{
    static TEMap<TEString, EditorMenubarItem> s_Items;
    return s_Items;
}

void EditorMenubarRegistry::RegisterItem(const EditorMenubarItem &item)
{
    if (!item.id.empty())
    {
        GetItemsMap()[item.id] = item;
    }
}

void EditorMenubarRegistry::UnregisterItem(const TEString &id) { GetItemsMap().Remove(id); }

TEArray<TEString> EditorMenubarRegistry::GetCategories()
{
    static const TEArray<TEString> kOrder = {"File", "Edit", "Window", "Tools", "Help"};
    TEArray<TEString> present;
    for (const auto &cat : kOrder)
    {
        for (const auto &[id, item] : GetItemsMap())
        {
            if (item.category == cat)
            {
                if (!item.isVisible || item.isVisible())
                {
                    present.Add(cat);
                    break;
                }
            }
        }
    }
    for (const auto &[id, item] : GetItemsMap())
    {
        if (std::find(present.begin(), present.end(), item.category) == present.end())
        {
            if (!item.isVisible || item.isVisible())
            {
                present.Add(item.category);
            }
        }
    }
    return present;
}

TEArray<EditorMenubarItem> EditorMenubarRegistry::GetItemsInCategory(const TEString &category)
{
    TEArray<EditorMenubarItem> result;
    for (const auto &[id, item] : GetItemsMap())
    {
        if (item.category == category)
        {
            if (!item.isVisible || item.isVisible())
            {
                result.Add(item);
            }
        }
    }

    std::sort(result.begin(), result.end(),
              [](const EditorMenubarItem &a, const EditorMenubarItem &b) { return a.priority < b.priority; });

    return result;
}

void EditorMenubarRegistry::Clear() { GetItemsMap().Clear(); }

void EditorMenubarRegistry::OnTimeGUIRender()
{
    if (TimeGUI::BeginMenuBar())
    {
        for (const auto &cat : GetCategories())
        {
            if (TimeGUI::BeginMenu(cat))
            {
                for (const auto &item : GetItemsInCategory(cat))
                {
                    bool selected = item.isChecked ? item.isChecked() : false;
                    bool enabled = item.isEnabled ? item.isEnabled() : true;
                    if (TimeGUI::MenuItem(item.label, item.shortcut, &selected, enabled))
                    {
                        if (item.onClick)
                            item.onClick();
                    }
                }
                TimeGUI::EndMenu();
            }
        }
        TimeGUI::EndMenuBar();
    }
}

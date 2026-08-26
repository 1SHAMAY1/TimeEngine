#include "Editor/EditorToolbarRegistry.hpp"
#include "Core/PreRequisites.h"
#include "Utils/TimeGUI.hpp"
#include <algorithm>

float EditorToolbarItem::GetWidth() const
{
    if (width > 0.0f)
        return width;
    if (icon)
        return 32.0f;
    if (!label.empty() && !label.StartsWith("##"))
        return TimeGUI::CalcTextSize(label.c_str()).x + 20.0f;
    return 32.0f;
}

static TEMap<TEString, EditorToolbarItem> &GetItemsMap()
{
    static TEMap<TEString, EditorToolbarItem> s_Items;
    return s_Items;
}

void EditorToolbarRegistry::RegisterItem(const EditorToolbarItem &item)
{
    if (!item.id.empty())
    {
        GetItemsMap()[item.id] = item;
    }
}

void EditorToolbarRegistry::UnregisterItem(const TEString &id) { GetItemsMap().Remove(id); }

TEArray<EditorToolbarItem> EditorToolbarRegistry::GetItems(EditorToolbarAlignment alignment)
{
    TEArray<EditorToolbarItem> result;
    for (const auto &[id, item] : GetItemsMap())
    {
        if (item.alignment == alignment)
        {
            if (!item.isVisible || item.isVisible())
            {
                result.Add(item);
            }
        }
    }

    std::sort(result.begin(), result.end(),
              [](const EditorToolbarItem &a, const EditorToolbarItem &b) { return a.priority < b.priority; });

    return result;
}

void EditorToolbarRegistry::Clear() { GetItemsMap().Clear(); }

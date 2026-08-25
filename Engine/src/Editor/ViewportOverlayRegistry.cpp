#include "Core/PreRequisites.h"
#include "Editor/ViewportOverlayRegistry.hpp"
#include <algorithm>


static TEMap<TEString, ViewportOverlayItem> &GetItemsMap()
{
    static TEMap<TEString, ViewportOverlayItem> s_Items;
    return s_Items;
}

void ViewportOverlayRegistry::RegisterItem(const ViewportOverlayItem &item)
{
    if (!item.id.empty())
    {
        GetItemsMap()[item.id] = item;
    }
}

void ViewportOverlayRegistry::UnregisterItem(const TEString &id)
{
    GetItemsMap().Remove(id);
}

TEArray<ViewportOverlayItem> ViewportOverlayRegistry::GetItems(ViewportOverlayCorner corner,
                                                               ViewportOverlayAlignment align)
{
    TEArray<ViewportOverlayItem> result;
    for (const auto &[id, item] : GetItemsMap())
    {
        if (item.corner == corner && item.alignment == align)
        {
            if (!item.isVisible || item.isVisible())
            {
                result.Add(item);
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const ViewportOverlayItem &a, const ViewportOverlayItem &b) {
        return a.priority < b.priority;
    });

    return result;
}

void ViewportOverlayRegistry::Clear()
{
    GetItemsMap().Clear();
}

static TEArray<TERef<IViewportOverlay>> s_ViewportOverlays;

void ViewportOverlayOwnerRegistry::RegisterOverlay(TERef<IViewportOverlay> overlay)
{
    if (overlay)
        s_ViewportOverlays.Add(overlay);
}

TEArray<TERef<IViewportOverlay>> ViewportOverlayOwnerRegistry::GetOverlays()
{
    return s_ViewportOverlays;
}

void ViewportOverlayOwnerRegistry::Clear()
{
    s_ViewportOverlays.Clear();
}


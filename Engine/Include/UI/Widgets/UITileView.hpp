#pragma once
#include "Renderer/Texture.hpp"
#include "UI/UIWidget.hpp"
#include <functional>

struct UITileItem
{
    TEString ID;
    TEString Title;
    TEString Subtitle;
    TEString BadgeText;
    TEVector4 BadgeColor = {0.3f, 0.5f, 0.9f, 1.0f};
    TERef<Texture> IconTexture = nullptr;
    std::function<void(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max)> CustomDrawIcon = nullptr;
    void *UserData = nullptr;
};

class TE_API UITileView : public UIWidget
{
public:
    UITileView(const TEVector2 &tileSize = {96.0f, 110.0f}, const TEString &id = "##UITileView");

    void SetItems(const TEArray<UITileItem> &items) { m_Items = items; }
    const TEArray<UITileItem> &GetItems() const { return m_Items; }

    void SetTileSize(const TEVector2 &size) { m_TileSize = size; }
    const TEVector2 &GetTileSize() const { return m_TileSize; }

    void SetSelectedItem(const TEString &id)
    {
        m_SelectedIDs.Clear();
        if (!id.empty())
            m_SelectedIDs.Add(id);
        m_LastClickedID = id;
    }
    void AddSelectedItem(const TEString &id)
    {
        if (!id.empty())
            m_SelectedIDs.Add(id);
        m_LastClickedID = id;
    }
    void RemoveSelectedItem(const TEString &id) { m_SelectedIDs.Remove(id); }
    void ToggleSelectedItem(const TEString &id)
    {
        if (m_SelectedIDs.Contains(id))
            m_SelectedIDs.Remove(id);
        else
            m_SelectedIDs.Add(id);
        m_LastClickedID = id;
    }
    void SelectAll()
    {
        m_SelectedIDs.Clear();
        for (const auto &item : m_Items)
            m_SelectedIDs.Add(item.ID);
    }
    void ClearSelection()
    {
        m_SelectedIDs.Clear();
        m_LastClickedID = "";
    }
    bool IsItemSelected(const TEString &id) const { return m_SelectedIDs.Contains(id); }
    const TESet<TEString> &GetSelectedItems() const { return m_SelectedIDs; }
    TEString GetSelectedItem() const
    {
        if (m_SelectedIDs.IsEmpty())
            return "";
        return *m_SelectedIDs.begin();
    }
    void SelectRange(const TEString &fromID, const TEString &toID);

    void DrawSelf() override;

    std::function<void(const UITileItem &)> OnItemClicked;
    std::function<void(const UITileItem &)> OnItemDoubleClicked;
    std::function<void(const UITileItem &)> OnItemContextMenu;

private:
    TEVector2 m_TileSize;
    TEArray<UITileItem> m_Items;
    TESet<TEString> m_SelectedIDs;
    TEString m_LastClickedID;
};

#include "UI/Widgets/UITileView.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/PreRequisites.h"
#include "Input/Input.hpp"
#include <algorithm>
#include <cmath>

UITileView::UITileView(const TEVector2 &tileSize, const TEString &id) : UIWidget(id), m_TileSize(tileSize) {}

void UITileView::SelectRange(const TEString &fromID, const TEString &toID)
{
    if (fromID.empty())
    {
        SetSelectedItem(toID);
        return;
    }

    int fromIdx = -1;
    int toIdx = -1;
    for (size_t i = 0; i < m_Items.Num(); ++i)
    {
        if (m_Items[i].ID == fromID)
            fromIdx = (int)i;
        if (m_Items[i].ID == toID)
            toIdx = (int)i;
    }

    if (fromIdx == -1 || toIdx == -1)
    {
        SetSelectedItem(toID);
        return;
    }

    int start = std::min(fromIdx, toIdx);
    int end = std::max(fromIdx, toIdx);

    m_SelectedIDs.Clear();
    for (int i = start; i <= end; ++i)
    {
        m_SelectedIDs.Add(m_Items[i].ID);
    }
}

void UITileView::DrawSelf()
{
    float panelWidth = TimeGUI::GetContentRegionAvail().x;
    float padding = 10.0f;
    float effectiveTileWidth = m_TileSize.x + padding;
    int columns = (int)(panelWidth / effectiveTileWidth);
    if (columns < 1)
        columns = 1;

    int currentColumn = 0;

    for (const auto &item : m_Items)
    {
        TimeGUI::PushID(item.ID);

        bool isSelected = m_SelectedIDs.Contains(item.ID);
        if (isSelected)
        {
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.2f, 0.35f, 0.6f, 0.8f));
        }

        // Draw card button container
        TEString btnLabel = "##Tile_" + item.ID;
        if (TimeGUI::Button(btnLabel, m_TileSize))
        {
            bool ctrlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
            bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

            if (shiftPressed && !m_LastClickedID.empty())
            {
                SelectRange(m_LastClickedID, item.ID);
            }
            else if (ctrlPressed)
            {
                ToggleSelectedItem(item.ID);
            }
            else
            {
                SetSelectedItem(item.ID);
            }

            if (OnItemClicked)
                OnItemClicked(item);
        }

        if (TimeGUI::IsItemHovered() && TimeGUI::IsMouseDoubleClicked(0))
        {
            if (OnItemDoubleClicked)
                OnItemDoubleClicked(item);
        }

        if (TimeGUI::IsItemHovered() && TimeGUI::IsMouseClicked(1))
        {
            if (OnItemContextMenu)
                OnItemContextMenu(item);
        }

        if (isSelected)
        {
            TimeGUI::PopStyleColor();
        }

        // Draw icon, text, and badge inside the button rect
        TEVector2 itemMin = TimeGUI::GetItemRectMin();
        TEVector2 itemSize = TimeGUI::GetItemRectSize();
        TEVector2 itemMax = TEVector2(itemMin.x + itemSize.x, itemMin.y + itemSize.y);
        TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

        // Dark Glass Card Background & Border
        unsigned int cardBg = isSelected ? 0xEE1E2E4A : (TimeGUI::IsItemHovered() ? 0xDD1B202A : 0xBB13161C);
        unsigned int cardBorder = isSelected ? 0xFF4A88E8 : (TimeGUI::IsItemHovered() ? 0x90384458 : 0x55222834);
        dl.AddRectFilled(itemMin, itemMax, cardBg, 6.0f);
        dl.AddRect(itemMin, itemMax, cardBorder, 6.0f, 0, 1.0f);

        // Icon area
        float iconSize = 48.0f;
        TEVector2 iconPos = TEVector2(itemMin.x + (itemSize.x - iconSize) * 0.5f, itemMin.y + 8.0f);
        TEVector2 iconMax = TEVector2(iconPos.x + iconSize, iconPos.y + iconSize);
        if (item.IconTexture)
        {
            TimeGUITextureID texID = (TimeGUITextureID)(uintptr_t)item.IconTexture->GetRendererID();
            dl.AddImage(texID, iconPos, iconMax, TEVector2(0, 1), TEVector2(1, 0));
        }
        else if (item.CustomDrawIcon)
        {
            item.CustomDrawIcon(dl, iconPos, iconMax);
        }
        else
        {
            // Placeholder box
            dl.AddRectFilled(iconPos, iconMax, 0x22FFFFFF, 4.0f);
        }

        // Title text (Truncated with ...)
        TEString truncatedTitle = item.Title;
        if (truncatedTitle.length() > 9)
            truncatedTitle = truncatedTitle.substr(0, 7) + "..";

        TEVector2 textSize = TimeGUI::CalcTextSize(truncatedTitle);
        TEVector2 textPos = TEVector2(itemMin.x + (itemSize.x - textSize.x) * 0.5f, itemMin.y + iconSize + 12.0f);
        dl.AddText(textPos, 0xFFE0E2E8, truncatedTitle);

        // Type badge pill at bottom
        if (!item.BadgeText.empty())
        {
            TEVector2 badgeSize = TimeGUI::CalcTextSize(item.BadgeText);
            float badgeWidth = badgeSize.x + 10.0f;
            float badgeHeight = badgeSize.y + 3.0f;
            TEVector2 badgeMin =
                TEVector2(itemMin.x + (itemSize.x - badgeWidth) * 0.5f, itemMax.y - badgeHeight - 6.0f);
            TEVector2 badgeMax = TEVector2(badgeMin.x + badgeWidth, badgeMin.y + badgeHeight);

            unsigned int col = TimeGUI::GetColorU32(
                TEColor(item.BadgeColor.x, item.BadgeColor.y, item.BadgeColor.z, item.BadgeColor.w));
            dl.AddRectFilled(badgeMin, badgeMax, col, 3.0f);
            dl.AddText(TEVector2(badgeMin.x + 5.0f, badgeMin.y + 1.5f), 0xFFFFFFFF, item.BadgeText);
        }

        TimeGUI::PopID();

        // Layout wrapping
        currentColumn++;
        if (currentColumn < columns)
        {
            TimeGUI::SameLine(0.0f, padding);
        }
        else
        {
            currentColumn = 0;
        }
    }
}

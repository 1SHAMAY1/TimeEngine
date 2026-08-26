#pragma once
#include "Editor/EditorMode.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorToolbarRegistry.hpp"
#include "Editor/EditorUtils.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/TimeGUI.hpp"

class EditorToolbar
{
public:
    static void OnTimeGUIRender(const TERef<Texture> &saveIcon = nullptr, const TERef<Texture> &playIcon = nullptr,
                                const TERef<Texture> &brandIcon = nullptr)
    {
        float toolbarHeight = 36.0f;

        TimeGUI::PushStyleVar(TimeGUI::TimeGUIStyleVar_FramePadding, TEVector2(6, 4));
        TimeGUI::PushStyleVar(TimeGUI::TimeGUIStyleVar_WindowPadding, TEVector2(6, 4));
        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEVector4(0.08f, 0.09f, 0.11f, 1.0f));

        if (TimeGUI::BeginChild("##ToolbarRegion", TEVector2(0, toolbarHeight), false,
                                TimeGUI::TimeGUIWindowFlags_NoScrollbar |
                                    TimeGUI::TimeGUIWindowFlags_NoScrollWithMouse))
        {
            // 0. Branding Icon (Thumbnail)
            if (brandIcon)
            {
                TimeGUI::SetCursorPos(TEVector2(8, 4));
                TimeGUI::TimeGUITextureID brandIconID =
                    (TimeGUI::TimeGUITextureID)(uintptr_t)brandIcon->GetRendererID();
                TimeGUI::Image(brandIconID, TEVector2(26, 26));
                TimeGUI::SameLine(0, 10);
            }

            TimeGUI::SetCursorPosY(4.0f);

            // Render Left Region Registered Items
            auto leftItems = EditorToolbarRegistry::GetItems(EditorToolbarAlignment::Left);
            for (size_t i = 0; i < leftItems.Num(); ++i)
            {
                RenderItem(leftItems[i]);
                if (i + 1 < leftItems.Num())
                    TimeGUI::SameLine(0, 8);
            }

            // Render Center Region Registered Items (Simulation Controls - Pure Math Centering)
            auto centerItems = EditorToolbarRegistry::GetItems(EditorToolbarAlignment::Center);
            if (!centerItems.IsEmpty())
            {
                float winWidth = TimeGUI::GetWindowWidth();
                float totalCenterW = CalculateGroupWidth(centerItems, 8.0f);
                float targetCenterPos = (winWidth - totalCenterW) * 0.5f;
                TimeGUI::SameLine(0, 0);
                if (targetCenterPos > TimeGUI::GetCursorPosX())
                    TimeGUI::SetCursorPosX(targetCenterPos);

                for (size_t i = 0; i < centerItems.Num(); ++i)
                {
                    RenderItem(centerItems[i]);
                    if (i + 1 < centerItems.Num())
                        TimeGUI::SameLine(0, 8);
                }
            }

            // Render Right Region Registered Items (Standalone & Restart - Pure Math Right Alignment)
            auto rightItems = EditorToolbarRegistry::GetItems(EditorToolbarAlignment::Right);
            if (!rightItems.IsEmpty())
            {
                float winWidth = TimeGUI::GetWindowWidth();
                float totalRightW = CalculateGroupWidth(rightItems, 8.0f);
                float targetRightPos = winWidth - totalRightW - 16.0f;
                TimeGUI::SameLine(0, 0);
                if (targetRightPos > TimeGUI::GetCursorPosX())
                    TimeGUI::SetCursorPosX(targetRightPos);

                for (size_t i = 0; i < rightItems.Num(); ++i)
                {
                    RenderItem(rightItems[i]);
                    if (i + 1 < rightItems.Num())
                        TimeGUI::SameLine(0, 8);
                }
            }
        }
        TimeGUI::EndChild();
        TimeGUI::PopStyleColor();
        TimeGUI::PopStyleVar(2);
    }

private:
    static float CalculateGroupWidth(const TEArray<EditorToolbarItem> &items, float spacing = 8.0f)
    {
        if (items.IsEmpty())
            return 0.0f;
        float total = 0.0f;
        for (size_t i = 0; i < items.Num(); ++i)
        {
            total += items[i].GetWidth();
            if (i + 1 < items.Num())
                total += spacing;
        }
        return total;
    }
    static void RenderItem(const EditorToolbarItem &item)
    {
        if (item.onCustomRender)
        {
            item.onCustomRender();
            return;
        }

        bool enabled = !item.isEnabled || item.isEnabled();
        bool active = item.isActive && item.isActive();

        if (!enabled)
        {
            TimeGUI::PushStyleVar(TimeGUI::TimeGUIStyleVar_Alpha, 0.5f);
        }

        if (active)
        {
            TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Button, TEColor(0.2f, 0.55f, 0.9f, 1.0f));
        }

        float btnSize = TimeGUI::GetFrameHeight();
        TEString btnId = "##" + item.id;
        bool clicked = false;

        if (item.icon)
        {
            TimeGUI::TimeGUITextureID iconID = (TimeGUI::TimeGUITextureID)(uintptr_t)item.icon->GetRendererID();
            clicked = TimeGUI::ImageButton(btnId.c_str(), iconID, TEVector2(btnSize, btnSize));
        }
        else if (item.id == "PlayButton" || item.id == "PauseButton" || item.id == "StopButton")
        {
            // Procedural vector icon buttons
            float btnW = 32.0f;
            float btnH = 26.0f;

            if (item.id == "PlayButton")
                TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.52f, 0.28f, 0.90f));
            else if (item.id == "PauseButton")
                TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.60f, 0.45f, 0.15f, 0.90f));
            else if (item.id == "StopButton")
                TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.65f, 0.20f, 0.20f, 0.90f));

            clicked = TimeGUI::Button(btnId.c_str(), TEVector2(btnW, btnH));
            TimeGUI::PopStyleColor();

            TEVector2 bMin = TimeGUI::GetItemRectMin();
            TEVector2 bCenter = TEVector2(bMin.x + btnW * 0.5f, bMin.y + btnH * 0.5f);
            TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

            if (item.id == "PlayButton")
                EditorUtils::DrawPlayIcon(dl, bCenter, 16.0f, 0xFFFFFFFF);
            else if (item.id == "PauseButton")
                EditorUtils::DrawPauseIcon(dl, bCenter, 16.0f, 0xFFFFFFFF);
            else if (item.id == "StopButton")
                EditorUtils::DrawStopIcon(dl, bCenter, 16.0f, 0xFFFFFFFF);
        }
        else
        {
            TEString btnText = item.label.empty() ? item.id : item.label;
            clicked = TimeGUI::Button(btnText.c_str());
        }

        if (clicked && enabled && item.onClick)
        {
            item.onClick();
        }

        if (!item.tooltip.empty() && TimeGUI::IsItemHovered())
        {
            TimeGUI::SetTooltip(item.tooltip.c_str());
        }

        if (active)
        {
            TimeGUI::PopStyleColor();
        }

        if (!enabled)
        {
            TimeGUI::PopStyleVar();
        }
    }
};

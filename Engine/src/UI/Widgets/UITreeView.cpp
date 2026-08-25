#include "Core/PreRequisites.h"
#include "UI/Widgets/UITreeView.hpp"


UITreeView::UITreeView(const TEString &id)
    : UIWidget(id)
{
}

void UITreeView::DrawSelf()
{
    for (const auto &root : m_Roots)
    {
        RenderNodeRecursive(root);
    }
}

void UITreeView::RenderNodeRecursive(const UITreeNode &node)
{
    TimeGUI::PushID(node.ID);

    int flags = TimeGUITreeNodeFlags_OpenOnArrow | TimeGUITreeNodeFlags_SpanAvailWidth;
    if (node.IsLeaf || node.Children.IsEmpty())
        flags |= TimeGUITreeNodeFlags_Leaf;

    if (m_SelectedNodeID == node.ID)
        flags |= TimeGUITreeNodeFlags_Selected;

    TEString displayLabel = (node.IconGlyph.empty() ? "" : (node.IconGlyph + " ")) + node.Label + "##" + node.ID;
    bool open = TimeGUI::TreeNodeEx(displayLabel, flags);

    if (TimeGUI::IsItemClicked())
    {
        m_SelectedNodeID = node.ID;
        if (OnNodeSelected)
            OnNodeSelected(node);
    }

    if (TimeGUI::IsItemHovered() && TimeGUI::IsMouseDoubleClicked(0))
    {
        if (OnNodeDoubleClicked)
            OnNodeDoubleClicked(node);
    }

    if (TimeGUI::IsItemHovered() && TimeGUI::IsMouseClicked(1))
    {
        if (OnNodeContextMenu)
            OnNodeContextMenu(node);
    }

    if (open)
    {
        if (!node.IsLeaf)
        {
            for (const auto &child : node.Children)
            {
                RenderNodeRecursive(child);
            }
        }
        TimeGUI::TreePop();
    }

    TimeGUI::PopID();
}


#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "UI/UIWidget.hpp"
#include <functional>

struct UITreeNode
{
    TEString ID;
    TEString Label;
    TEString IconGlyph;
    bool IsLeaf = false;
    void *UserData = nullptr;
    TEArray<UITreeNode> Children;
};

class TE_API UITreeView : public UIWidget
{
public:
    UITreeView(const TEString &id = "##UITreeView");

    void SetRootNodes(const TEArray<UITreeNode> &roots) { m_Roots = roots; }
    const TEArray<UITreeNode> &GetRootNodes() const { return m_Roots; }

    void SetSelectedNode(const TEString &nodeID) { m_SelectedNodeID = nodeID; }
    const TEString &GetSelectedNode() const { return m_SelectedNodeID; }

    void DrawSelf() override;

    std::function<void(const UITreeNode &)> OnNodeSelected;
    std::function<void(const UITreeNode &)> OnNodeDoubleClicked;
    std::function<void(const UITreeNode &)> OnNodeContextMenu;

private:
    void RenderNodeRecursive(const UITreeNode &node);

    TEArray<UITreeNode> m_Roots;
    TEString m_SelectedNodeID;
};

#pragma once
#include "Core/PreRequisites.h"
#include "Core/Scene/EntityManager.hpp"
#include "Layers/Layer.hpp"
#include "Utils/TimeGUI.hpp"

class EditorLayer;

class TE_API ViewportContextMenu : public Layer
{
public:
    ViewportContextMenu(EditorLayer *editor = nullptr);
    virtual ~ViewportContextMenu() = default;

    void OnTimeGUIRender() override;
    void OnTimeGUIRender(EditorLayer *editor);

    static void OpenAt(const TEVector2 &screenPos, Entity hoveredEntity = Entity());
    static bool IsOpen();
    static void Close();

    void SetEditor(EditorLayer *editor) { m_Editor = editor; }

private:
    EditorLayer *m_Editor = nullptr;
    static bool s_IsOpen;
    static bool s_ShouldOpen;
    static TEVector2 s_SpawnPos;
    static Entity s_TargetEntity;
};

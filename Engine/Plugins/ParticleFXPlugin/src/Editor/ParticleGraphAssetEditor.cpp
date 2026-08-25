#include "Editor/ParticleGraphAssetEditor.hpp"
#include "Graph/ParticleGraphNodeRegistry.hpp"
#include "Graph/ParticleGraphNodes.hpp"
#include "Utils/TimeGUI.hpp"


ParticleGraphAssetEditor::ParticleGraphAssetEditor()
    : m_Graph("Particle System Graph")
{
}

void ParticleGraphAssetEditor::DrawEditor(EditorTab &tab)
{
    if (!m_Initialized)
    {
        ParticleGraphNodeRegistry::RegisterAllNodes(m_Canvas.GetPalette());

        if (m_Graph.GetNodes().IsEmpty())
        {
            auto rootNode = CreateRef<ParticleEmitterRootNode>();
            rootNode->Position = {300.0f, 150.0f};
            m_Graph.AddNode(rootNode);

            auto spawnRate = CreateRef<ParticleSpawnRateNode>();
            spawnRate->Position = {50.0f, 100.0f};
            m_Graph.AddNode(spawnRate);

            auto colorLife = CreateRef<ParticleColorOverLifeNode>();
            colorLife->Position = {50.0f, 250.0f};
            m_Graph.AddNode(colorLife);

            // Connect SpawnRate -> Root.Spawn
            auto *pOut = spawnRate->FindPinByName("Out", PinDirection::Output);
            auto *pIn = rootNode->FindPinByName("Spawn", PinDirection::Input);
            if (pOut && pIn)
                m_Graph.AddConnection(pOut->ID, pIn->ID);
        }

        m_Initialized = true;
    }

    TimeGUI::Text("Particle System Graph Studio");
    TimeGUI::SameLine();
    if (TimeGUI::Button("Zoom to Fit"))
    {
        m_Canvas.ZoomToFit(m_Graph);
    }

    TimeGUI::Separator();

    // Render Canvas
    m_Canvas.Draw(m_Graph);
}

void ParticleGraphAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Golden Amber
    dl.AddRectFilled(min, max, IM_COL32(230, 160, 20, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(35, 28, 16, 255), 2.0f);

    // Glowing Particle Burst Dots
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;
    dl.AddCircleFilled(TEVector2(cx, cy), 5.0f, IM_COL32(255, 240, 100, 255));
    dl.AddCircleFilled(TEVector2(cx - 7.0f, cy - 6.0f), 3.0f, IM_COL32(255, 180, 40, 240));
    dl.AddCircleFilled(TEVector2(cx + 8.0f, cy - 4.0f), 3.5f, IM_COL32(255, 140, 30, 240));
    dl.AddCircleFilled(TEVector2(cx - 5.0f, cy + 7.0f), 2.5f, IM_COL32(255, 200, 60, 240));
    dl.AddCircleFilled(TEVector2(cx + 6.0f, cy + 6.0f), 2.0f, IM_COL32(255, 230, 90, 240));
}

#include "NeuralAIPlugin.hpp"

TE_REGISTER_PLUGIN(NeuralAIPlugin)

void NeuralAIPlugin::OnLoad()
{
    // TODO: Register STNeuralActionTask and STNeuralEvaluatorCondition into StateTreeRegistry
}

void NeuralAIPlugin::OnUnload()
{
    // TODO: Cleanup neural runtime registrations
}

void NeuralAIPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF141923, 6.0f);
    dl.AddRect(min, max, 0xFF3D5A80, 6.0f, 0, 1.0f);

    // Neural nodes
    dl.AddCircleFilled(TEVector2(c.x - 12.0f, c.y - 8.0f), 4.0f, 0xFF98C1D9);
    dl.AddCircleFilled(TEVector2(c.x - 12.0f, c.y + 8.0f), 4.0f, 0xFF98C1D9);
    dl.AddCircleFilled(TEVector2(c.x + 12.0f, c.y), 5.0f, 0xFFEE6C4D);
    dl.AddLine(TEVector2(c.x - 12.0f, c.y - 8.0f), TEVector2(c.x + 12.0f, c.y), 0xFF98C1D9, 1.5f);
    dl.AddLine(TEVector2(c.x - 12.0f, c.y + 8.0f), TEVector2(c.x + 12.0f, c.y), 0xFF98C1D9, 1.5f);
}

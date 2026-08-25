#include "SpriteMode.hpp"
#include "Core/Log.h"
#include "SpriteEditorLayer.hpp"
#include "Submodes/CodeEditorSubmode.hpp"
#include "Submodes/PixelPaintSubmode.hpp"
#include "Submodes/VectorEditorSubmode.hpp"
#include <algorithm>
#include <cstring>


SpriteMode::SpriteMode()
{
    m_ExportLayer = CreateScope<SpriteExportLayer>(this);
    m_ScriptRuntime = CreateScope<SpriteScriptRuntime>();

    // Seed default procedural script template
    auto templates = SpriteModeLibrary::GetPresetTemplates();
    if (!templates.empty())
    {
        m_ProcBuffer = templates[0].Code;
        m_ScriptRuntime->Compile(m_ProcBuffer);
    }

    // Modular submodes
    m_Submodes.Add(CreateScope<PixelPaintSubmode>());
    m_Submodes.Add(CreateScope<VectorEditorSubmode>());
    m_Submodes.Add(CreateScope<CodeEditorSubmode>());
    m_ActiveSubmodeIndex = 0; // Default to Pixel Paint studio suite

    // Seed default color history
    m_ColorHistory.Add(TEVector4(1.0f, 1.0f, 1.0f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.0f, 0.0f, 0.0f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.85f, 0.15f, 0.15f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.15f, 0.75f, 0.15f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.15f, 0.15f, 0.85f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.9f, 0.85f, 0.15f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.15f, 0.75f, 0.75f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.75f, 0.15f, 0.75f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.5f, 0.5f, 0.5f, 1.0f));
    m_ColorHistory.Add(TEVector4(0.75f, 0.75f, 0.75f, 1.0f));

    // Initialize initial frame with default Layer 1
    PixelFrame initialFrame;
    PixelLayer initialLayer;
    initialLayer.Name = "Layer 1";
    initialLayer.Pixels.resize(m_PixelGridWidth * m_PixelGridHeight, TEVector4(0, 0, 0, 0));
    initialFrame.Layers.Add(initialLayer);
    m_PixelFrames.Add(initialFrame);
    m_ActiveFrameIndex = 0;
    m_ActiveLayerIndex = 0;

    SaveUndoState();
}

SpriteMode::~SpriteMode()
{
}

void SpriteMode::OnEnter()
{
    if (m_ActiveSubmodeIndex >= 0 && m_ActiveSubmodeIndex < (int)m_Submodes.Size())
    {
        m_Submodes[m_ActiveSubmodeIndex]->OnEnter(this);
    }
}

void SpriteMode::OnUpdate(float dt)
{
    if (m_ActiveSubmodeIndex >= 0 && m_ActiveSubmodeIndex < (int)m_Submodes.Size())
    {
        m_Submodes[m_ActiveSubmodeIndex]->OnUpdate(dt, this);
    }

    if (m_ExportLayer && m_ExportLayer->IsOpen())
    {
        m_ExportLayer->OnUpdate();
    }

    // Advance procedural animation timeline
    if (m_ProcAnimPlaying && m_ProcFPS > 0.0f)
    {
        m_ProcAnimTime += dt;
        int nextFrame = (int)(m_ProcAnimTime * m_ProcFPS);
        if (m_ProcTotalFrames > 0)
        {
            if (m_ProcAnimLoop)
                m_ProcAnimFrame = nextFrame % m_ProcTotalFrames;
            else
                m_ProcAnimFrame = std::min(nextFrame, m_ProcTotalFrames - 1);
        }
    }
}

void SpriteMode::OnTimeGUIRender()
{
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(8, 8));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FrameRounding, 6.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(8, 8));

    TimeGUI::Begin("Sprite Studio", nullptr, TimeGUIWindowFlags_NoCollapse | TimeGUIWindowFlags_NoMove);

    // ── Top Submode Navigation Header ──────────────────────────────────────────
    TimeGUI::BeginChild("##SubmodeNavHeader", TEVector2(0, 38), false, TimeGUIWindowFlags_NoScrollbar);

    TEVector4 activeCol(0.20f, 0.45f, 0.85f, 1.0f);
    TEVector4 inactiveCol(0.18f, 0.18f, 0.22f, 1.0f);

    for (int i = 0; i < (int)m_Submodes.Size(); i++)
    {
        const auto &submode = m_Submodes[i];
        if (!submode)
            continue;

        bool isActive = (m_ActiveSubmodeIndex == i);
        TimeGUI::PushStyleColor(TimeGUICol_Button, isActive ? activeCol : inactiveCol);

        TEString btnText = TEString(submode->GetIcon()) + " " + submode->GetName();
        if (TimeGUI::Button(btnText.c_str(), TEVector2(140, 30)))
        {
            if (m_ActiveSubmodeIndex != i)
            {
                if (m_ActiveSubmodeIndex >= 0 && m_ActiveSubmodeIndex < (int)m_Submodes.Size())
                {
                    m_Submodes[m_ActiveSubmodeIndex]->OnExit(this);
                }
                m_ActiveSubmodeIndex = i;
                submode->OnEnter(this);
            }
        }
        TimeGUI::PopStyleColor();
        TimeGUI::SameLine();
    }

    // Right-aligned Export Action
    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 130);
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.15f, 0.6f, 0.3f, 1.0f));
    if (TimeGUI::Button("Export PNG", TEVector2(110, 30)))
    {
        if (m_ExportLayer)
            m_ExportLayer->Open(this);
    }
    TimeGUI::PopStyleColor();

    TimeGUI::EndChild();
    TimeGUI::Separator();

    // ── Render Active Submode ──────────────────────────────────────────────────
    if (m_ActiveSubmodeIndex >= 0 && m_ActiveSubmodeIndex < (int)m_Submodes.Size())
    {
        m_Submodes[m_ActiveSubmodeIndex]->OnTimeGUIRender(nullptr, this);
    }

    // ── Render Export Popup Modal Layer ───────────────────────────────────────
    if (m_ExportLayer && m_ExportLayer->IsOpen())
    {
        m_ExportLayer->OnTimeGUIRender();
    }

    TimeGUI::End();
    TimeGUI::PopStyleVar(3);
}

void SpriteMode::OnExit()
{
    if (m_ActiveSubmodeIndex >= 0 && m_ActiveSubmodeIndex < (int)m_Submodes.Size())
    {
        m_Submodes[m_ActiveSubmodeIndex]->OnExit(this);
    }
}

bool SpriteMode::OnShortcut(const TEString &shortcutId)
{
    if (shortcutId == "Editor_Undo")
    {
        Undo();
        return true;
    }
    if (shortcutId == "Editor_Redo")
    {
        Redo();
        return true;
    }

    if (m_ActiveSubmodeIndex >= 0 && m_ActiveSubmodeIndex < (int)m_Submodes.Size())
    {
        return m_Submodes[m_ActiveSubmodeIndex]->OnShortcut(shortcutId, this);
    }
    return false;
}

void SpriteMode::AddColorToHistory(TEVector4 color)
{
    if (color.w < 0.001f)
        return;
    m_ColorHistory.RemoveBy([&](const TEVector4 &c) {
        return std::abs(c.x - color.x) < 0.001f && std::abs(c.y - color.y) < 0.001f &&
               std::abs(c.z - color.z) < 0.001f && std::abs(c.w - color.w) < 0.001f;
    });
    m_ColorHistory.Insert(0, color);
    if (m_ColorHistory.Size() > 16)
        m_ColorHistory.RemoveAt(m_ColorHistory.Size() - 1);
}

void SpriteMode::SaveUndoState()
{
    if (m_IsUndoingRedoing)
        return;

    m_RedoStack.clear();

    SpriteModeState state;
    state.VectorElements = m_VectorElements;
    state.ProcBuffer = TEString(m_ProcBuffer);
    state.Keywords = m_Keywords;
    state.PixelFrames = m_PixelFrames;
    state.ActiveFrameIndex = m_ActiveFrameIndex;
    state.ActiveLayerIndex = m_ActiveLayerIndex;

    m_UndoStack.Add(state);
    if (m_UndoStack.Size() > 50)
        m_UndoStack.RemoveAt(0);
}

void SpriteMode::Undo()
{
    if (m_UndoStack.Size() <= 1)
        return;

    m_IsUndoingRedoing = true;
    m_RedoStack.Add(m_UndoStack.Last());
    m_UndoStack.RemoveAt(m_UndoStack.Size() - 1);

    auto &state = m_UndoStack.Last();
    m_VectorElements = state.VectorElements;
    m_ProcBuffer = state.ProcBuffer;
    m_Keywords = state.Keywords;
    m_PixelFrames = state.PixelFrames;
    m_ActiveFrameIndex = state.ActiveFrameIndex;
    m_ActiveLayerIndex = state.ActiveLayerIndex;

    if (m_ScriptRuntime)
        m_ScriptRuntime->Compile(m_ProcBuffer);

    m_SelectedElementIdx = -1;
    m_PreviewDirty = true;
    m_IsUndoingRedoing = false;
}

void SpriteMode::Redo()
{
    if (m_RedoStack.IsEmpty())
        return;

    m_IsUndoingRedoing = true;
    auto state = m_RedoStack.Last();
    m_RedoStack.RemoveAt(m_RedoStack.Size() - 1);
    m_UndoStack.Add(state);

    m_VectorElements = state.VectorElements;
    m_ProcBuffer = state.ProcBuffer;
    m_Keywords = state.Keywords;
    m_PixelFrames = state.PixelFrames;
    m_ActiveFrameIndex = state.ActiveFrameIndex;
    m_ActiveLayerIndex = state.ActiveLayerIndex;

    if (m_ScriptRuntime)
        m_ScriptRuntime->Compile(m_ProcBuffer);

    m_SelectedElementIdx = -1;
    m_PreviewDirty = true;
    m_IsUndoingRedoing = false;
}

void SpriteMode::RefreshPreview()
{
}

void SpriteMode::PerformExport()
{
}

void SpriteMode::ExecuteProceduralCode(TimeGUI::TimeGUIDrawList dl, TEVector2 origin, TEVector2 cellSize, float dt)
{
    if (m_ScriptRuntime)
    {
        m_ScriptRuntime->Execute(dl, origin, cellSize, m_ProcAnimTime, dt, m_ProcAnimFrame, m_ProcTotalFrames,
                                 m_PixelGridWidth, m_PixelGridHeight);
    }
}

void SpriteMode::RenderVectorShapes(TimeGUI::TimeGUIDrawList dl, TEVector2 origin, TEVector2 cellSize, float zoom,
                                    TEVector2 pan, int hoveredIdx, int selectedIdx)
{
    for (int i = 0; i < (int)m_VectorElements.size(); i++)
    {
        const auto &elem = m_VectorElements[i];
        unsigned int fillCol = TimeGUI::ColorConvertFloat4ToU32(elem.FillColor);
        unsigned int strokeCol = TimeGUI::ColorConvertFloat4ToU32(elem.StrokeColor);

        if (elem.Type == VectorShapeType::Rectangle && elem.Points.size() >= 2)
        {
            TEVector2 p1 = TEVector2(origin.x + (elem.Points[0].x * cellSize.x + pan.x) * zoom,
                                     origin.y + (elem.Points[0].y * cellSize.y + pan.y) * zoom);
            TEVector2 p2 = TEVector2(origin.x + (elem.Points[1].x * cellSize.x + pan.x) * zoom,
                                     origin.y + (elem.Points[1].y * cellSize.y + pan.y) * zoom);
            dl.AddRectFilled(p1, p2, fillCol, elem.FillRounding);
            if (elem.StrokeThickness > 0.0f)
                dl.AddRect(p1, p2, strokeCol, elem.StrokeRounding, 0, elem.StrokeThickness * zoom);
        }
        else if (elem.Type == VectorShapeType::Circle && !elem.Points.empty())
        {
            TEVector2 center = TEVector2(origin.x + (elem.Points[0].x * cellSize.x + pan.x) * zoom,
                                         origin.y + (elem.Points[0].y * cellSize.y + pan.y) * zoom);
            float r = elem.Radius * cellSize.x * zoom;
            dl.AddCircleFilled(center, r, fillCol);
            if (elem.StrokeThickness > 0.0f)
                dl.AddCircle(center, r, strokeCol, 0, elem.StrokeThickness * zoom);
        }
    }
}

void SpriteMode::RenderPixelGrid(TimeGUI::TimeGUIDrawList dl, TEVector2 origin, TEVector2 cellSize, float zoom,
                                 TEVector2 pan, int frameIndex)
{
    int idx = (frameIndex >= 0) ? frameIndex : m_ActiveFrameIndex;
    if (idx < 0 || idx >= (int)m_PixelFrames.size())
        return;

    const auto &frame = m_PixelFrames[idx];
    float pixelW = cellSize.x / m_PixelGridWidth;
    float pixelH = cellSize.y / m_PixelGridHeight;

    // Composite all visible layers
    for (const auto &layer : frame.Layers)
    {
        if (!layer.Visible || layer.Pixels.size() < (size_t)(m_PixelGridWidth * m_PixelGridHeight))
            continue;

        for (int y = 0; y < m_PixelGridHeight; y++)
        {
            for (int x = 0; x < m_PixelGridWidth; x++)
            {
                TEVector4 col = layer.Pixels[y * m_PixelGridWidth + x];
                if (col.w > 0.001f)
                {
                    col.w *= layer.Opacity;
                    TEVector2 p1 =
                        TEVector2(origin.x + (x * pixelW + pan.x) * zoom, origin.y + (y * pixelH + pan.y) * zoom);
                    TEVector2 p2 = TEVector2(origin.x + ((x + 1) * pixelW + pan.x) * zoom,
                                             origin.y + ((y + 1) * pixelH + pan.y) * zoom);
                    dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(col));
                }
            }
        }
    }
}

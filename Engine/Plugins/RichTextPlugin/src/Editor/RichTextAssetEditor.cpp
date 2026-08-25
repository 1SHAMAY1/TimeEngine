#include "Editor/RichTextAssetEditor.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"
#include "Utils/TEFileSystem.hpp"

RichTextAssetEditor::RichTextAssetEditor()
{
    const char *defaultSample =
        "<color=gold><b>TimeEngine</b></color> <color=#00e0ff><b>Rich Text Studio</b></color>\n"
        "Welcome to the <wave amp=3 speed=3><color=cyan><b>modular rich text subsystem</b></color></wave>!\n"
        "Features include <shake intensity=2><b>shake effects</b></shake>, <rainbow speed=2>rainbow styling</rainbow>, and <link=ShopPrompt><u>interactive hyperlinks</u></link>.\n"
        "Supports <size=20><color=orange>custom font sizes</color></size>, <i>italics</i>, <u>underlines</u>, and <s>strikethroughs</s>.";

    m_InputBuffer = defaultSample;
}

TEString RichTextAssetEditor::CreateDefaultTemplate(const TEString &name) const
{
    return "<color=gold><b>" + name + "</b></color>\n"
           "Welcome to <wave amp=3 speed=3><color=cyan><b>RichText</b></color></wave> in TimeEngine!\n"
           "Use <shake intensity=2>shake</shake>, <rainbow speed=2>rainbow</rainbow>, and <link=Action><u>links</u></link>.";
}

void RichTextAssetEditor::DrawEditor(EditorTab &tab)
{
    static TEString s_LastPath = "";
    if (s_LastPath != tab.AssetPath)
    {
        s_LastPath = tab.AssetPath;
        if (TEFileSystem::Exists(tab.AssetPath))
        {
            m_InputBuffer = TEFileSystem::ReadAllText(tab.AssetPath);
            m_bDirty = true;
        }
    }

    // Top Toolbar
    TimeGUI::TextColored(TEVector4(0.3f, 0.85f, 0.95f, 1.0f), "RichText Document: %s", tab.AssetPath.c_str());
    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Asset##RichSave"))
    {
        if (TEFileSystem::WriteAllText(tab.AssetPath, m_InputBuffer))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }
    TimeGUI::Separator();

    // Quick Tag Insertion Buttons Toolbar
    TimeGUI::Text("Quick Tags:");
    TimeGUI::SameLine();
    if (TimeGUI::Button("<b>Bold</b>")) { m_InputBuffer += "<b>text</b>"; m_bDirty = true; AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true); }
    TimeGUI::SameLine();
    if (TimeGUI::Button("<i>Italic</i>")) { m_InputBuffer += "<i>text</i>"; m_bDirty = true; AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true); }
    TimeGUI::SameLine();
    if (TimeGUI::Button("<u>Under</u>")) { m_InputBuffer += "<u>text</u>"; m_bDirty = true; AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true); }
    TimeGUI::SameLine();
    if (TimeGUI::Button("<color>")) { m_InputBuffer += "<color=gold>text</color>"; m_bDirty = true; AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true); }
    TimeGUI::SameLine();
    if (TimeGUI::Button("<wave>")) { m_InputBuffer += "<wave amp=4 speed=3>wavy</wave>"; m_bDirty = true; AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true); }
    TimeGUI::SameLine();
    if (TimeGUI::Button("<shake>")) { m_InputBuffer += "<shake intensity=2>shaking</shake>"; m_bDirty = true; AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true); }
    TimeGUI::SameLine();
    if (TimeGUI::Button("<rainbow>")) { m_InputBuffer += "<rainbow speed=2>rainbow</rainbow>"; m_bDirty = true; AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true); }
    TimeGUI::SameLine();
    if (TimeGUI::Button("<link>")) { m_InputBuffer += "<link=action_id>click me</link>"; m_bDirty = true; AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true); }

    TimeGUI::Separator();

    // Two Columns: Left Editor, Right Live Render Output
    TimeGUI::Columns(2, "RichTextStudioCols", true);

    // --- Left: Input Area & Properties ---
    TimeGUI::Text("Markup Source:");
    if (TimeGUI::InputTextMultiline("##RichTextInput", m_InputBuffer, TEVector2(-1.0f, 180.0f)))
    {
        m_bDirty = true;
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Spacing();
    TimeGUI::Text("Formatting Options:");
    if (TimeGUI::SliderFloat("Wrap Width", &m_WrapWidth, 200.0f, 1200.0f, "%.0f px"))
        m_bDirty = true;

    if (TimeGUI::SliderFloat("Line Spacing", &m_LineSpacing, 0.8f, 2.5f, "%.2fx"))
        m_bDirty = true;

    TimeGUI::Checkbox("Enable Realtime Animations", &m_EnableAnim);

    if (TimeGUI::SliderFloat("Typewriter Progress", &m_TypewriterProgress, 0.0f, 1.0f, "%.2f"))
    {
        // Force update animated layout
    }

    if (!m_LastClickedLink.empty())
    {
        TimeGUI::TextColored(TEVector4(0.2f, 0.9f, 0.4f, 1.0f), "Last Clicked Link: %s", m_LastClickedLink.c_str());
    }

    TimeGUI::NextColumn();

    // --- Right: Live Canvas Preview ---
    TimeGUI::Text("Live Render Output:");

    TERef<FontAsset> defaultFont = nullptr;

    if (m_bDirty)
    {
        auto spans = RichTextParser::Parse(m_InputBuffer);
        m_Layout = RichTextLayoutEngine::ComputeLayout(spans, defaultFont, m_WrapWidth, m_LineSpacing);
        m_AnimatedLayout = m_Layout;
        m_bDirty = false;
    }

    // Tick animations
    float dt = TimeGUI::GetIO().DeltaTime;
    if (m_EnableAnim)
    {
        m_TotalTime += dt;
        m_AnimatedLayout = m_Layout;
        RichTextAnimator::AnimateLayout(m_AnimatedLayout, m_TotalTime, dt, m_TypewriterProgress);
    }
    else
    {
        m_AnimatedLayout = m_Layout;
    }

    // Canvas Area
    TEVector2 canvasP0 = TimeGUI::GetCursorScreenPos();
    TEVector2 canvasSz = TEVector2(TimeGUI::GetContentRegionAvail().x, 320.0f);
    TEVector2 canvasP1 = TEVector2(canvasP0.x + canvasSz.x, canvasP0.y + canvasSz.y);

    TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();
    drawList.AddRectFilled(canvasP0, canvasP1, TIMEGUI_COL32(18, 22, 28, 255));
    drawList.AddRect(canvasP0, canvasP1, TIMEGUI_COL32(50, 60, 75, 255));

    // Draw animated layout
    RichTextRenderer::DrawToImGui(m_AnimatedLayout, defaultFont, TEVector2(canvasP0.x + 15.0f, canvasP0.y + 15.0f),
                                 [this](const TEString &link) {
                                     m_LastClickedLink = link;
                                 });

    TimeGUI::Dummy(canvasSz);
    TimeGUI::Columns(1);
}

void RichTextAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Background in Dark Violet
    dl.AddRectFilled(min, max, IM_COL32(75, 40, 120, 230), 4.0f);

    TEVector2 cMin(min.x + pad, min.y + pad);
    TEVector2 cMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(cMin, cMax, IM_COL32(25, 18, 38, 255), 2.0f);

    // Stylized "T" text icon
    float cx = (cMin.x + cMax.x) * 0.5f;
    float topY = cMin.y + (cMax.y - cMin.y) * 0.22f;
    float botY = cMax.y - (cMax.y - cMin.y) * 0.22f;
    float barW = (cMax.x - cMin.x) * 0.55f;

    // Top horizontal bar
    dl.AddLine(TEVector2(cx - barW * 0.5f, topY), TEVector2(cx + barW * 0.5f, topY), IM_COL32(230, 140, 255, 255), 3.0f);
    // Vertical stem
    dl.AddLine(TEVector2(cx, topY), TEVector2(cx, botY), IM_COL32(180, 80, 255, 255), 3.0f);
}

TE_REGISTER_ASSET_EDITOR(RichTextAssetEditor);

#include "CodeEditorSubmode.hpp"
#include "../SpriteEditorLayer.hpp"
#include "../SpriteMode.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>

void CodeEditorSubmode::OnEnter(SpriteMode *mode)
{
    m_CodeEditor.SetLanguage(ECodeLanguage::TScript);
    if (mode)
    {
        m_CodeEditor.SetText(mode->m_ProcBuffer);
        if (mode->m_ScriptRuntime)
        {
            mode->m_ScriptRuntime->Compile(mode->m_ProcBuffer);
        }
    }
}

void CodeEditorSubmode::OnUpdate(float dt, SpriteMode *mode) {}

void CodeEditorSubmode::OnExit(SpriteMode *mode) {}

bool CodeEditorSubmode::OnShortcut(const TEString &shortcutId, SpriteMode *mode)
{
    if (m_CodeEditor.IsFocused())
    {
        if (m_CodeEditor.OnShortcut(shortcutId))
        {
            if (mode && m_CodeEditor.GetText() != mode->m_ProcBuffer)
            {
                mode->m_ProcBuffer = m_CodeEditor.GetText();
                if (mode->m_ScriptRuntime)
                    mode->m_ScriptRuntime->Compile(mode->m_ProcBuffer);
            }
            return true;
        }
    }

    if (shortcutId == "Editor_Save" || shortcutId == "Script_Run")
    {
        if (mode && mode->m_ScriptRuntime)
        {
            mode->m_ScriptRuntime->Compile(mode->m_ProcBuffer);
            mode->SaveUndoState();
            return true;
        }
    }
    return false;
}

void CodeEditorSubmode::OnTimeGUIRender(SpriteEditorLayer *layer, SpriteMode *mode)
{
    if (!mode)
        return;

    // Ensure code editor is initialized with mode's script buffer
    if (m_CodeEditor.GetText().empty() && !mode->m_ProcBuffer.empty())
    {
        m_CodeEditor.SetText(mode->m_ProcBuffer);
    }

    // ── 2-Panel Clean Layout (Left: Code Panel, Right: Live Preview + Export) ──
    if (TimeGUI::BeginTable("##CodeStudioLayout", 2, TimeGUITableFlags_Resizable))
    {
        TimeGUI::TableSetupColumn("##CodeCol", TimeGUITableColumnFlags_WidthStretch, 0.55f);
        TimeGUI::TableSetupColumn("##PreviewCol", TimeGUITableColumnFlags_WidthStretch, 0.45f);

        TimeGUI::TableNextColumn();
        DrawCodeEditorPanel(mode);

        TimeGUI::TableNextColumn();
        DrawPreviewAndTimelinePanel(mode);

        TimeGUI::EndTable();
    }

    if (m_ShowApiHelp)
    {
        DrawApiHelpModal(mode);
    }
}

void CodeEditorSubmode::DrawCodeEditorPanel(SpriteMode *mode)
{
    TimeGUI::BeginChild("##CodeEditorContainer", TEVector2(0, 0), true);

    // ── Top Toolbar: Templates, Compile Status, Run, API Docs ─────────────────
    auto templates = SpriteModeLibrary::GetPresetTemplates();
    TEArray<const char *> templateNames;
    for (const auto &t : templates)
        templateNames.push_back(t.Name.c_str());

    TimeGUI::TextColored(TEVector4(0.3f, 0.8f, 1.0f, 1.0f), "TScript Code Editor");
    TimeGUI::SameLine();
    TimeGUI::Spacing();
    TimeGUI::SameLine();

    // Import Template Action Button
    if (TimeGUI::Button("Import Template \xE2\x96\xBE", TEVector2(145, 24)))
    {
        TimeGUI::OpenPopup("##ImportTemplatePopup");
    }

    if (TimeGUI::BeginPopup("##ImportTemplatePopup"))
    {
        TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "Choose Template to Import:");
        TimeGUI::Separator();
        for (int i = 0; i < (int)templates.size(); ++i)
        {
            if (TimeGUI::Selectable(templates[i].Name))
            {
                mode->m_ProcBuffer = templates[i].Code;
                m_CodeEditor.Clear();
                m_CodeEditor.SetText(mode->m_ProcBuffer);
                if (mode->m_ScriptRuntime)
                {
                    mode->m_ScriptRuntime->Compile(mode->m_ProcBuffer);
                    int req = mode->m_ScriptRuntime->GetRequestedTotalFrames();
                    if (req >= 1)
                    {
                        mode->m_ProcTotalFrames = req;
                        mode->m_ProcAnimFrame = 0;
                    }
                }
                mode->SaveUndoState();
            }
            if (TimeGUI::IsItemHovered())
            {
                TimeGUI::SetTooltip(templates[i].Description);
            }
        }
        TimeGUI::EndPopup();
    }

    TimeGUI::SameLine();
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.2f, 0.6f, 0.3f, 1.0f));
    if (TimeGUI::Button("Run / Recompile", TEVector2(120, 24)))
    {
        mode->m_ProcBuffer = m_CodeEditor.GetText();
        if (mode->m_ScriptRuntime)
            mode->m_ScriptRuntime->Compile(mode->m_ProcBuffer);
        mode->SaveUndoState();
    }
    TimeGUI::PopStyleColor();

    TimeGUI::SameLine();
    if (TimeGUI::Button("API Cheatsheet", TEVector2(110, 24)))
    {
        m_ShowApiHelp = !m_ShowApiHelp;
    }

    // Status Badge
    TimeGUI::SameLine();
    if (mode->m_ScriptRuntime && mode->m_ScriptRuntime->IsValid())
    {
        TimeGUI::TextColored(TEVector4(0.2f, 0.9f, 0.3f, 1.0f), "[Ready]");
        m_CodeEditor.ClearErrorMarkers();
    }
    else
    {
        TimeGUI::TextColored(TEVector4(1.0f, 0.25f, 0.25f, 1.0f), "[Error]");
        if (mode->m_ScriptRuntime)
        {
            TimeGUI::SameLine();
            TimeGUI::TextColored(TEVector4(1.0f, 0.6f, 0.6f, 1.0f), "(Hover for details)");
            if (TimeGUI::IsItemHovered())
            {
                TimeGUI::SetTooltip(mode->m_ScriptRuntime->GetCompileError());
            }
        }
    }

    TimeGUI::Separator();

    // ── Syntax-Colored UICodeEdit Block ───────────────────────────────────────
    TEVector2 avail = TimeGUI::GetContentRegionAvail();
    m_CodeEditor.SetSize(TEVector2(avail.x, avail.y - 4.0f));
    m_CodeEditor.Draw();

    if (m_CodeEditor.GetText() != mode->m_ProcBuffer)
    {
        mode->m_ProcBuffer = m_CodeEditor.GetText();
        if (mode->m_ScriptRuntime)
        {
            mode->m_ScriptRuntime->Compile(mode->m_ProcBuffer);
        }
    }

    TimeGUI::EndChild();
}

void CodeEditorSubmode::DrawPreviewAndTimelinePanel(SpriteMode *mode)
{
    TimeGUI::BeginChild("##PreviewTimelineContainer", TEVector2(0, 0), true);

    // ── Top Header: Title, Canvas Info ───────────────────────────────────────
    TimeGUI::TextColored(TEVector4(0.4f, 0.85f, 1.0f, 1.0f), "LIVE ANIMATION PREVIEW");
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(Frame %d / %d)", mode->m_ProcAnimFrame + 1, std::max(1, mode->m_ProcTotalFrames));

    TimeGUI::Separator();

    // ── Live Canvas Viewport ──────────────────────────────────────────────────
    TEVector2 avail = TimeGUI::GetContentRegionAvail();
    float timelineHeight = 72.0f;
    float previewMax = std::max(64.0f, std::min(avail.x - 16.0f, avail.y - timelineHeight - 16.0f));

    TEVector2 canvasSize = TEVector2(previewMax, previewMax);

    // Center the preview
    float padX = std::max(0.0f, (avail.x - canvasSize.x) * 0.5f);
    if (padX > 0)
    {
        TimeGUI::SetCursorPosX(TimeGUI::GetCursorPosX() + padX);
    }

    TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    // Background Container
    dl.AddRectFilled(canvasPos, TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                     IM_COL32(20, 20, 24, 255), 4.0f);
    dl.AddRect(canvasPos, TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(40, 40, 50, 255),
               4.0f);

    // Checkerboard Background
    float checkSize = 16.0f;
    for (float y = 0; y < canvasSize.y; y += checkSize)
    {
        for (float x = 0; x < canvasSize.x; x += checkSize)
        {
            int ix = (int)(x / checkSize);
            int iy = (int)(y / checkSize);
            unsigned int col = ((ix + iy) % 2 == 0) ? IM_COL32(26, 26, 32, 255) : IM_COL32(34, 34, 42, 255);
            dl.AddRectFilled(TEVector2(canvasPos.x + x, canvasPos.y + y),
                             TEVector2(canvasPos.x + std::min(x + checkSize, canvasSize.x),
                                       canvasPos.y + std::min(y + checkSize, canvasSize.y)),
                             col);
        }
    }

    // Clamp frame to configured loop boundary
    if (mode->m_ProcTotalFrames >= 1)
    {
        if (mode->m_ProcAnimFrame >= mode->m_ProcTotalFrames)
        {
            mode->m_ProcAnimFrame = mode->m_ProcAnimLoop ? 0 : mode->m_ProcTotalFrames - 1;
            mode->m_ProcAnimTime = (float)mode->m_ProcAnimFrame / std::max(1.0f, mode->m_ProcFPS);
        }
        else if (mode->m_ProcAnimFrame < 0)
        {
            mode->m_ProcAnimFrame = 0;
            mode->m_ProcAnimTime = 0.0f;
        }
    }

    // Procedural Render
    if (mode->m_ScriptRuntime)
    {
        float dt = TimeGUI::GetIO().DeltaTime;
        mode->ExecuteProceduralCode(dl, canvasPos, canvasSize, dt);
    }

    // Dummy element to capture layout space
    TimeGUI::Dummy(canvasSize);

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // ── Timeline Controls ─────────────────────────────────────────────────────
    TimeGUI::BeginChild("##TimelineControlsPane", TEVector2(0, 54), false);

    // Play / Pause
    if (TimeGUI::Button(mode->m_ProcAnimPlaying ? "Pause" : "Play", TEVector2(60, 26)))
    {
        mode->m_ProcAnimPlaying = !mode->m_ProcAnimPlaying;
    }
    TimeGUI::SameLine();

    // Step Back
    if (TimeGUI::Button("|<", TEVector2(28, 26)))
    {
        mode->m_ProcAnimFrame = (mode->m_ProcAnimFrame - 1 + mode->m_ProcTotalFrames) % mode->m_ProcTotalFrames;
        mode->m_ProcAnimTime = (float)mode->m_ProcAnimFrame / std::max(1.0f, mode->m_ProcFPS);
    }
    TimeGUI::SameLine();

    // Step Forward
    if (TimeGUI::Button(">|", TEVector2(28, 26)))
    {
        mode->m_ProcAnimFrame = (mode->m_ProcAnimFrame + 1) % mode->m_ProcTotalFrames;
        mode->m_ProcAnimTime = (float)mode->m_ProcAnimFrame / std::max(1.0f, mode->m_ProcFPS);
    }
    TimeGUI::SameLine();

    // Frame Scrubber
    int frameSlider = mode->m_ProcAnimFrame + 1;
    TimeGUI::SetNextItemWidth(120);
    if (TimeGUI::SliderInt("##FrameSlider", &frameSlider, 1, std::max(1, mode->m_ProcTotalFrames)))
    {
        mode->m_ProcAnimFrame = frameSlider - 1;
        mode->m_ProcAnimTime = (float)mode->m_ProcAnimFrame / std::max(1.0f, mode->m_ProcFPS);
    }
    TimeGUI::SameLine();

    // Total Frames (Read-only status driven by loop/script)
    TimeGUI::Text("Frame:");
    TimeGUI::SameLine();
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "%d / %d", frameSlider, mode->m_ProcTotalFrames);
    TimeGUI::SameLine(0, 16.0f);

    // Loop toggle
    TimeGUI::Checkbox("Loop", &mode->m_ProcAnimLoop);

    TimeGUI::EndChild();
    TimeGUI::EndChild();
}

void CodeEditorSubmode::DrawApiHelpModal(SpriteMode *mode)
{
    TimeGUI::SetNextWindowSize(TEVector2(580, 480), TimeGUICond_Appearing);
    if (TimeGUI::Begin("TScript Sprite Studio - API Reference", &m_ShowApiHelp))
    {
        TimeGUI::TextColored(TEVector4(0.3f, 0.85f, 1.0f, 1.0f), "Quick Syntax & Function Reference");
        TimeGUI::Separator();

        if (TimeGUI::BeginTabBar("##ApiHelpTabs"))
        {
            if (TimeGUI::BeginTabItem("Vector Shapes"))
            {
                TimeGUI::TextColored(TEVector4(1.0f, 0.85f, 0.2f, 1.0f), "Vector Primitives:");
                TimeGUI::Text("- draw_rect(x, y, w, h, col, [rounding], [thickness])");
                TimeGUI::Text("- draw_rect_filled(x, y, w, h, col, [rounding])");
                TimeGUI::Text("- draw_circle(x, y, radius, col, [thickness])");
                TimeGUI::Text("- draw_circle_filled(x, y, radius, col)");
                TimeGUI::Text("- draw_ring(x, y, outer_r, inner_r, col)");
                TimeGUI::Text("- draw_line(x1, y1, x2, y2, col, [thickness])");
                TimeGUI::Text("- draw_triangle(x1, y1, x2, y2, x3, y3, col, [thickness])");
                TimeGUI::Text("- draw_quad(x1, y1, x2, y2, x3, y3, x4, y4, col)");
                TimeGUI::Text("- draw_ellipse(cx, cy, rx, ry, col, [rot], [thickness])");
                TimeGUI::Text("- draw_star(cx, cy, spikes, outer_r, inner_r, col, [rot])");
                TimeGUI::Text("- draw_polygon(cx, cy, radius, sides, col, [rot], [thickness])");
                TimeGUI::Text("- draw_bezier(x1, y1, x2, y2, x3, y3, x4, y4, col, [thickness])");
                TimeGUI::EndTabItem();
            }

            if (TimeGUI::BeginTabItem("Text & Typography"))
            {
                TimeGUI::TextColored(TEVector4(1.0f, 0.85f, 0.2f, 1.0f), "Text Functions:");
                TimeGUI::Text("- draw_text(x, y, text, col)");
                TimeGUI::Text("- draw_text_outlined(x, y, text, col, outline_col, [thickness])");
                TimeGUI::Text("- draw_text_shadowed(x, y, text, col, shadow_col, [offX], [offY])");
                TimeGUI::EndTabItem();
            }

            if (TimeGUI::BeginTabItem("Pixel Art"))
            {
                TimeGUI::TextColored(TEVector4(1.0f, 0.85f, 0.2f, 1.0f), "Pixel Art Tools:");
                TimeGUI::Text("- set_grid_size(width, height) - sets raster resolution (e.g. 16, 16)");
                TimeGUI::Text("- set_pixel(x, y, col) / pixel(x, y, col)");
                TimeGUI::Text("- get_pixel(x, y)");
                TimeGUI::Text("- draw_pixel_rect(x, y, w, h, col)");
                TimeGUI::Text("- draw_pixel_line(x1, y1, x2, y2, col)");
                TimeGUI::Text("- draw_pixel_circle(cx, cy, r, col)");
                TimeGUI::Text("- clear_pixels(col) / fill_pixels(col)");
                TimeGUI::EndTabItem();
            }

            if (TimeGUI::BeginTabItem("Animation & Colors"))
            {
                TimeGUI::TextColored(TEVector4(1.0f, 0.85f, 0.2f, 1.0f), "Animation:");
                TimeGUI::Text("- time() - continuous time in seconds");
                TimeGUI::Text("- frame() - current frame (0..total_frames-1)");
                TimeGUI::Text("- total_frames() - total animation frames count");
                TimeGUI::Text("- frame_progress() - normalized progress (0.0 to 1.0)");
                TimeGUI::Text("- animate_wave(speed, min, max) - sinusoidal wave");
                TimeGUI::Text("- animate_pingpong(speed, min, max) - bounce wave");
                TimeGUI::Text("- animate_rotate(speed) - rotating angle in radians");
                TimeGUI::Spacing();
                TimeGUI::TextColored(TEVector4(1.0f, 0.85f, 0.2f, 1.0f), "Colors & Math:");
                TimeGUI::Text("- rgb(r, g, b), rgba(r, g, b, a), hsv(h, s, v, [a]), hex(\"#FF0000\")");
                TimeGUI::Text("- WHITE, BLACK, RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, ORANGE, PURPLE");
                TimeGUI::Text("- sin(a), cos(a), tan(a), atan2(y, x), sqrt(v), abs(v), lerp(a, b, t)");
                TimeGUI::EndTabItem();
            }
            TimeGUI::EndTabBar();
        }

        TimeGUI::Separator();
        if (TimeGUI::Button("Close", TEVector2(80, 24)))
        {
            m_ShowApiHelp = false;
        }
    }
    TimeGUI::End();
}

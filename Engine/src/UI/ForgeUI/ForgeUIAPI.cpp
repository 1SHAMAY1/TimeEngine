#include "Core/PreRequisites.h"
#include "UI/ForgeUI/ForgeUIAPI.hpp"
#include "Core/Log.h"
#include "Input/Input.hpp"
#include "Renderer/RendererContext.hpp"
#include "ForgeUI/Core/ForgeContext.h"
#include "ForgeUI/Core/ForgeTypes.h"
#include "ForgeUI/Core/ForgeTheme.h"
#include "ForgeUI/Widgets/ForgeWidgets.h"
#include "ForgeUI/Render/ForgeDrawList.h"
#include "ForgeUI/Backend/ForgeBackend_Null.h"
#include "ForgeUI/Backend/ForgeBackend_OpenGL.h"
#include "ForgeUI/Platform/ForgePlatform_GLFW.h"
#include <algorithm>

// -------------------------------------------------------------------------
// Helper Conversions (Strictly Local)
// -------------------------------------------------------------------------
static inline forge::ForgeVec2 ToForgeVec2(const TEVector2 &v) { return forge::ForgeVec2(v.x, v.y); }
static inline TEVector2 FromForgeVec2(const forge::ForgeVec2 &v) { return TEVector2(v.x, v.y); }
static inline forge::ForgeVec4 ToForgeVec4(const TEVector4 &v) { return forge::ForgeVec4(v.x, v.y, v.z, v.w); }
static inline forge::ForgeColor ToForgeColor(const TEColor &c) { return forge::ForgeColor(c.r, c.g, c.b, c.a); }
static inline uint32_t ToRGBA(const TEColor &c) { return forge::ForgeColor(c.r, c.g, c.b, c.a).ToRGBA8(); }
static inline uint32_t ToRGBA(const TEVector4 &c) { return forge::ForgeColor(c.x, c.y, c.z, c.w).ToRGBA8(); }

bool ForgeUIAPI::Init(void *nativeWindow)
{
    if (m_Initialized)
    {
        TE_CORE_WARN("ForgeUIAPI: already initialized.");
        return true;
    }

    m_NativeWindow = nativeWindow;
    m_Context = static_cast<void *>(forge::CreateContext());

    if (!m_Context)
    {
        TE_CORE_ERROR("ForgeUIAPI: forge::CreateContext() returned nullptr.");
        return false;
    }

    forge::SetCurrentContext(static_cast<forge::ForgeContext *>(m_Context));

    m_Initialized = true;
    TE_CORE_INFO("ForgeUIAPI: initialized.");
    return true;
}

void ForgeUIAPI::Shutdown()
{
    if (!m_Initialized)
        return;

    if (m_RenderInitialized)
        ShutdownRenderBackend();

    if (m_Context)
    {
        forge::DestroyContext(static_cast<forge::ForgeContext *>(m_Context));
        m_Context = nullptr;
    }

    m_Initialized = false;
    TE_CORE_INFO("ForgeUIAPI: shut down.");
}

static const uint8_t s_ForgeFont5x7[96][7] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00},
    {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00}, {0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A},
    {0x04, 0x1E, 0x05, 0x0E, 0x14, 0x0F, 0x04}, {0x12, 0x15, 0x12, 0x08, 0x09, 0x15, 0x09},
    {0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D}, {0x06, 0x06, 0x04, 0x00, 0x00, 0x00, 0x00},
    {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}, {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
    {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00}, {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x04}, {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06}, {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00},
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
    {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F}, {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
    {0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00}, {0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x04},
    {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02}, {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00},
    {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}, {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04},
    {0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E}, {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
    {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F},
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F},
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11},
    {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D},
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E},
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A},
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E},
    {0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00}, {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E},
    {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},
    {0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F},
    {0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1E}, {0x00, 0x00, 0x0E, 0x10, 0x10, 0x11, 0x0E},
    {0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x0F}, {0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E},
    {0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08}, {0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x0E},
    {0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11}, {0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E},
    {0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C}, {0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12},
    {0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, {0x00, 0x00, 0x1A, 0x15, 0x15, 0x11, 0x11},
    {0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11}, {0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E},
    {0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10}, {0x00, 0x00, 0x0D, 0x13, 0x0F, 0x01, 0x01},
    {0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10}, {0x00, 0x00, 0x0E, 0x10, 0x0E, 0x01, 0x1E},
    {0x08, 0x08, 0x1C, 0x08, 0x08, 0x09, 0x06}, {0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D},
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04}, {0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A},
    {0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11}, {0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E},
    {0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F}, {0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02},
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, {0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

static uint64_t CreateForgeDefaultFontAtlas(forge::IForgeBackend *backend)
{
    if (!backend)
        return 0;

    const uint32_t atlasSize = 512;
    TEArray<uint8_t> pixels;
    pixels.Resize(atlasSize * atlasSize * 4);
    std::fill(pixels.begin(), pixels.end(), static_cast<uint8_t>(0));

    for (int c = 32; c < 127; ++c)
    {
        int row = (c - 32) / 16;
        int col = (c - 32) % 16;
        int gx = col * 32 + 5;
        int gy = row * 32 + 4;

        int fontIdx = c - 32;
        for (int r = 0; r < 7; ++r)
        {
            uint8_t bits = s_ForgeFont5x7[fontIdx][r];
            for (int b = 0; b < 5; ++b)
            {
                bool on = (bits & (1 << (4 - b))) != 0;
                if (on)
                {
                    for (int sy = 0; sy < 3; ++sy)
                    {
                        for (int sx = 0; sx < 3; ++sx)
                        {
                            int px = gx + b * 3 + sx;
                            int py = gy + r * 3 + sy;
                            if (px < static_cast<int>(atlasSize) && py < static_cast<int>(atlasSize))
                            {
                                size_t idx = (py * atlasSize + px) * 4;
                                pixels[idx + 0] = 255;
                                pixels[idx + 1] = 255;
                                pixels[idx + 2] = 255;
                                pixels[idx + 3] = 255;
                            }
                        }
                    }
                }
            }
        }
    }

    return backend->CreateTexture(atlasSize, atlasSize, pixels.Data(), true);
}

bool ForgeUIAPI::InitRenderBackend()
{
    if (m_RenderInitialized)
        return true;

    auto backend = CreateScope<forge::ForgeBackend_OpenGL>();
    if (!backend->Initialize())
    {
        TE_CORE_ERROR("ForgeUIAPI: failed to initialize render backend.");
        return false;
    }

    if (m_Context)
    {
        auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
        uint64_t fontAtlasHandle = CreateForgeDefaultFontAtlas(backend.get());
        ctx->GetFont().SetAtlasTextureHandle(fontAtlasHandle);
    }

    m_Backend = static_cast<void *>(backend.release());

    m_RenderInitialized = true;
    TE_CORE_INFO("ForgeUIAPI: render backend initialized.");
    return true;
}

void ForgeUIAPI::ShutdownRenderBackend()
{
    if (!m_RenderInitialized)
        return;

    if (m_Backend)
    {
        TEScope<forge::IForgeBackend> backend(static_cast<forge::IForgeBackend *>(m_Backend));
        backend->Shutdown();
        m_Backend = nullptr;
    }

    m_RenderInitialized = false;
}

void ForgeUIAPI::BindThreadContext()
{
    if (m_Context)
        forge::SetCurrentContext(static_cast<forge::ForgeContext *>(m_Context));
}

void ForgeUIAPI::PrepareFrame() { BindThreadContext(); }

void ForgeUIAPI::BeginFrame(uint32_t width, uint32_t height)
{
    BindThreadContext();
    m_Width = width;
    m_Height = height;

    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return;

    // Sync input from TimeEngine Input system
    auto [mx, my] = Input::GetMousePosition();
    auto &input = ctx->GetInput();
    input.mouseDelta.x = mx - input.mousePos.x;
    input.mouseDelta.y = my - input.mousePos.y;
    input.mousePos.x = mx;
    input.mousePos.y = my;

    bool currentLeft = Input::IsMouseButtonPressed(Mouse::ButtonLeft);
    bool currentRight = Input::IsMouseButtonPressed(Mouse::ButtonRight);
    bool currentMiddle = Input::IsMouseButtonPressed(Mouse::ButtonMiddle);

    input.mouseDown[0] = currentLeft;
    input.mouseDown[1] = currentRight;
    input.mouseDown[2] = currentMiddle;

    input.mousePressed[0] = currentLeft && !m_PrevMouseDown[0];
    input.mousePressed[1] = currentRight && !m_PrevMouseDown[1];
    input.mousePressed[2] = currentMiddle && !m_PrevMouseDown[2];

    input.mouseReleased[0] = !currentLeft && m_PrevMouseDown[0];
    input.mouseReleased[1] = !currentRight && m_PrevMouseDown[1];
    input.mouseReleased[2] = !currentMiddle && m_PrevMouseDown[2];

    m_PrevMouseDown[0] = currentLeft;
    m_PrevMouseDown[1] = currentRight;
    m_PrevMouseDown[2] = currentMiddle;

    MouseDelta scroll = Input::GetMouseScrollDelta();
    input.scrollDelta.x = scroll.x;
    input.scrollDelta.y = scroll.y;

    ctx->SetRootDimensions(static_cast<float>(width), static_cast<float>(height));
    ctx->BeginFrame();

    // Ensure initial clip rect is bounded to [0, width] x [0, height] to avoid driver scissor overflow
    ctx->GetDrawList().PushClipRect(
        forge::ForgeVec4(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)));

    if (m_Backend)
    {
        auto *backend = static_cast<forge::IForgeBackend *>(m_Backend);
        backend->BeginFrame(width, height);
    }
}

void *ForgeUIAPI::EndFrame(uint32_t width, uint32_t height)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return nullptr;

    ctx->GetDrawList().PopClipRect();
    ctx->ComputeLayout(static_cast<float>(width), static_cast<float>(height));
    ctx->EndFrame();
    return &ctx->GetDrawList();
}

void ForgeUIAPI::RenderDrawData(void *drawData)
{
    if (!m_Backend || !drawData)
        return;

    auto *backend = static_cast<forge::IForgeBackend *>(m_Backend);
    auto *drawList = static_cast<forge::ForgeDrawList *>(drawData);
    backend->RenderDrawList(drawList);
    backend->EndFrame();
}

TEVector2 ForgeUIAPI::GetMainViewportPos() { return TEVector2(0.0f, 0.0f); }

TEVector2 ForgeUIAPI::GetMainViewportSize()
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
    {
        float w = ctx->RootWidth();
        float h = ctx->RootHeight();
        return TEVector2(w, h);
    }
    return TEVector2(static_cast<float>(m_Width), static_cast<float>(m_Height));
}

uint32_t ForgeUIAPI::GetMainViewportID() { return 0; }

void *ForgeUIAPI::GetBackgroundDrawList()
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
        return &ctx->GetDrawList();
    return nullptr;
}

void *ForgeUIAPI::GetForegroundDrawList()
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
        return &ctx->GetDrawList();
    return nullptr;
}

void *ForgeUIAPI::GetWindowDrawList()
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
        return &ctx->GetDrawList();
    return nullptr;
}

// -------------------------------------------------------------------------
// Layout Helpers
// -------------------------------------------------------------------------
void ForgeUIAPI::DrawString(float x, float y, unsigned int color, const TEString &text, float fontSize)
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
    {
        ctx->GetFont().AppendTextToDrawList(&ctx->GetDrawList(), text.c_str(), forge::ForgeVec2(x, y), fontSize, color);
    }
}

ForgeUIAPI::WindowLayoutState *ForgeUIAPI::CurrentWindowLayout()
{
    if (m_WindowStack.IsEmpty())
        return nullptr;
    return &m_WindowStack.Back();
}

TEVector2 ForgeUIAPI::AllocateItem(float width, float height)
{
    auto *win = CurrentWindowLayout();
    if (!win)
    {
        m_LastItemMin = {0.0f, 0.0f};
        m_LastItemMax = {width, height};
        m_LastItemSize = {width, height};
        return {0.0f, 0.0f};
    }

    TEVector2 itemPos;
    if (win->sameLine)
    {
        itemPos = win->cursor;
        win->cursor.x += width + win->sameLineSpacing;
        win->lineHeight = Max(win->lineHeight, height);
        win->sameLine = false;
    }
    else
    {
        itemPos = win->cursor;
        win->cursor.y += height + 6.0f;
        win->cursor.x = win->columns.active ? win->columns.startX[win->columns.current] : win->startX;
        win->lineHeight = height;
    }

    m_LastItemMin = itemPos;
    m_LastItemMax = itemPos + TEVector2(width, height);
    m_LastItemSize = {width, height};
    return itemPos;
}

// -------------------------------------------------------------------------
// Windows & Containers
// -------------------------------------------------------------------------
bool ForgeUIAPI::Begin(const TEString &name, bool *open, int flags)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return false;

    float posX = m_HasNextWindowPos ? m_NextWindowPos.x : 0.0f;
    float posY = m_HasNextWindowPos ? m_NextWindowPos.y : 0.0f;
    float width = m_HasNextWindowSize ? m_NextWindowSize.x : 400.0f;
    float height = m_HasNextWindowSize ? m_NextWindowSize.y : 300.0f;

    m_HasNextWindowPos = false;
    m_HasNextWindowSize = false;

    bool noTitleBar = (flags & (1 << 0)) != 0;   // TimeGUIWindowFlags_NoTitleBar
    bool noBackground = (flags & (1 << 7)) != 0; // TimeGUIWindowFlags_NoBackground

    WindowLayoutState winState{};
    winState.pos = {posX, posY};
    winState.size = {width, height};
    winState.startX = posX + (noTitleBar ? 0.0f : 12.0f);
    winState.startY = posY + (noTitleBar ? 0.0f : 36.0f);
    winState.cursor = {winState.startX, winState.startY};
    winState.lineHeight = 0.0f;
    winState.sameLine = false;
    winState.isChild = false;
    m_WindowStack.Add(winState);

    if (!noBackground)
    {
        ctx->GetDrawList().AddRoundedRect(forge::ForgeVec2(posX, posY), forge::ForgeVec2(posX + width, posY + height),
                                          noTitleBar ? 0.0f : 6.0f, 0xFF1E2024);
    }
    if (!noTitleBar)
    {
        ctx->GetDrawList().AddRoundedRect(forge::ForgeVec2(posX, posY), forge::ForgeVec2(posX + width, posY + 28.0f),
                                          6.0f, 0xFF282C34);
        DrawString(posX + 10.0f, posY + 7.0f, 0xFFFFFFFF, name);
    }

    return true;
}

void ForgeUIAPI::End()
{
    if (!m_WindowStack.IsEmpty())
        m_WindowStack.pop_back();
}

bool ForgeUIAPI::BeginChild(const TEString &strId, const TEVector2 &size, bool border, int flags)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return false;

    WindowLayoutState *parent = CurrentWindowLayout();
    float posX = parent ? parent->cursor.x : 0.0f;
    float posY = parent ? parent->cursor.y : 0.0f;

    float availW = 300.0f;
    float availH = 200.0f;
    if (parent)
    {
        TEVector2 avail = GetContentRegionAvail();
        availW = avail.x;
        availH = avail.y;
    }

    float childW = (size.x > 0.0f) ? size.x : ((size.x < 0.0f) ? Max(10.0f, availW + size.x) : availW);
    float childH = (size.y > 0.0f) ? size.y : ((size.y < 0.0f) ? Max(10.0f, availH + size.y) : availH);

    WindowLayoutState childState{};
    childState.pos = {posX, posY};
    childState.size = {childW, childH};
    childState.startX = posX + 8.0f;
    childState.startY = posY + 8.0f;
    childState.cursor = {childState.startX, childState.startY};
    childState.isChild = true;
    m_WindowStack.Add(childState);

    // Draw background if childBg is pushed
    if (!m_ChildBgColors.IsEmpty())
    {
        TEVector4 bgCol = m_ChildBgColors.Back();
        if (bgCol.w > 0.0f)
        {
            ctx->GetDrawList().AddRoundedRect(forge::ForgeVec2(posX, posY),
                                              forge::ForgeVec2(posX + childW, posY + childH), 4.0f, ToRGBA(bgCol));
        }
    }
    if (border)
    {
        ctx->GetDrawList().AddBorder(forge::ForgeVec2(posX, posY), forge::ForgeVec2(posX + childW, posY + childH), 4.0f,
                                     1.0f, 0x44FFFFFF);
    }

    ctx->GetDrawList().PushClipRect(forge::ForgeVec4(posX, posY, posX + childW, posY + childH));
    return true;
}

void ForgeUIAPI::EndChild()
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (ctx)
    {
        ctx->GetDrawList().PopClipRect();
    }
    if (!m_WindowStack.IsEmpty())
    {
        WindowLayoutState finishedChild = m_WindowStack.Back();
        m_WindowStack.pop_back();

        auto *parent = CurrentWindowLayout();
        if (parent)
        {
            parent->cursor.y = Max(parent->cursor.y, finishedChild.pos.y + finishedChild.size.y);
        }
    }
}

bool ForgeUIAPI::BeginMenuBar() { return true; }

void ForgeUIAPI::EndMenuBar() {}

bool ForgeUIAPI::BeginMenu(const TEString &label, bool enabled) { return false; }

void ForgeUIAPI::EndMenu() {}

bool ForgeUIAPI::MenuItem(const TEString &label, const TEString &shortcut, bool selected, bool enabled)
{
    if (!enabled)
        return false;
    return Button(label, TEVector2(0, 24.0f));
}

bool ForgeUIAPI::MenuItem(const TEString &label, const TEString &shortcut, bool *p_selected, bool enabled)
{
    bool clicked = MenuItem(label, shortcut, p_selected ? *p_selected : false, enabled);
    if (clicked && p_selected)
        *p_selected = !(*p_selected);
    return clicked;
}

// -------------------------------------------------------------------------
// Basic Widgets
// -------------------------------------------------------------------------
bool ForgeUIAPI::Button(const TEString &label, const TEVector2 &size)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return false;

    TEVector2 avail = GetContentRegionAvail();
    float btnW =
        (size.x > 0.0f) ? size.x : ((size.x < 0.0f) ? Max(10.0f, avail.x + (size.x < -1.0f ? size.x : 0.0f)) : 100.0f);
    float btnH = (size.y > 0.0f) ? size.y : 28.0f;
    TEVector2 pos = AllocateItem(btnW, btnH);

    TEVector2 mousePos = GetMousePos();
    bool hovered =
        (mousePos.x >= pos.x && mousePos.x <= pos.x + btnW && mousePos.y >= pos.y && mousePos.y <= pos.y + btnH);

    auto &input = ctx->GetInput();
    bool clicked = hovered && input.mousePressed[0];

    // Determine button background color
    uint32_t bgCol = hovered ? (input.mouseDown[0] ? 0xFF263248 : 0xFF3D4A5E) : 0xFF2A2E35;
    if (!m_ButtonColors.IsEmpty())
    {
        bgCol = ToRGBA(m_ButtonColors.Back());
    }

    float rounding = m_FrameRoundings.IsEmpty() ? 4.0f : m_FrameRoundings.Back();

    if ((bgCol >> 24) > 0)
    {
        ctx->GetDrawList().AddRoundedRect(ToForgeVec2(pos), ToForgeVec2(pos + TEVector2(btnW, btnH)), rounding, bgCol);
        ctx->GetDrawList().AddBorder(ToForgeVec2(pos), ToForgeVec2(pos + TEVector2(btnW, btnH)), rounding, 1.0f,
                                     hovered ? 0xFF667799 : 0xFF3E4654);
    }

    // Strip ## id suffix for rendering
    TEString displayLabel = label;
    int hashIdx = displayLabel.Find("##");
    if (hashIdx >= 0)
        displayLabel = displayLabel.Left(static_cast<size_t>(hashIdx));

    float textW = static_cast<float>(displayLabel.Length() * 8);
    float textX = pos.x + Max(8.0f, (btnW - textW) * 0.5f);
    float textY = pos.y + Max(4.0f, (btnH - 14.0f) * 0.5f);

    DrawString(textX, textY, 0xFFFFFFFF, displayLabel);
    return clicked;
}

bool ForgeUIAPI::SmallButton(const TEString &label) { return Button(label, TEVector2(0, 20.0f)); }

bool ForgeUIAPI::InvisibleButton(const TEString &strId, const TEVector2 &size, int flags)
{
    AllocateItem(size.x, size.y);
    return false;
}

void ForgeUIAPI::Text(const TEString &text) { TextColored(TEColor(1.0f, 1.0f, 1.0f, 1.0f), text); }

void ForgeUIAPI::TextUnformatted(const TEString &text) { Text(text); }

void ForgeUIAPI::TextColored(const TEColor &color, const TEString &text)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return;

    float textW = static_cast<float>(text.Length() * 8 + 4);
    float textH = 18.0f;
    TEVector2 pos = AllocateItem(textW, textH);

    DrawString(pos.x, pos.y, ToRGBA(color), text);
}

void ForgeUIAPI::TextDisabled(const TEString &text) { TextColored(TEColor(0.5f, 0.5f, 0.5f, 1.0f), text); }

void ForgeUIAPI::TextWrapped(const TEString &text) { Text(text); }

bool ForgeUIAPI::Checkbox(const TEString &label, bool *checked)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return false;

    float boxSize = 18.0f;
    float totalW = boxSize + 8.0f + static_cast<float>(label.Length() * 8);
    TEVector2 pos = AllocateItem(totalW, boxSize);

    TEVector2 mousePos = GetMousePos();
    bool hovered =
        (mousePos.x >= pos.x && mousePos.x <= pos.x + totalW && mousePos.y >= pos.y && mousePos.y <= pos.y + boxSize);

    auto &input = ctx->GetInput();
    bool clicked = hovered && input.mousePressed[0];
    if (clicked && checked)
        *checked = !(*checked);

    // Draw checkbox box
    ctx->GetDrawList().AddRoundedRect(ToForgeVec2(pos), ToForgeVec2(pos + TEVector2(boxSize, boxSize)), 3.0f,
                                      0xFF1E222A);
    ctx->GetDrawList().AddBorder(ToForgeVec2(pos), ToForgeVec2(pos + TEVector2(boxSize, boxSize)), 3.0f, 1.0f,
                                 hovered ? 0xFF5090FF : 0xFF404855);

    if (checked && *checked)
    {
        ctx->GetDrawList().AddRect(ToForgeVec2(pos + TEVector2(4.0f, 4.0f)),
                                   ToForgeVec2(pos + TEVector2(boxSize - 4.0f, boxSize - 4.0f)), 0xFF5090FF);
    }

    DrawString(pos.x + boxSize + 6.0f, pos.y + 2.0f, 0xFFFFFFFF, label);
    return clicked;
}

bool ForgeUIAPI::RadioButton(const TEString &label, bool active)
{
    bool val = active;
    return Checkbox(label, &val);
}

bool ForgeUIAPI::RadioButton(const TEString &label, int *v, int v_button)
{
    if (!v)
        return false;
    bool active = (*v == v_button);
    if (Checkbox(label, &active) && active)
    {
        *v = v_button;
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------
// Sliders & Drags
// -------------------------------------------------------------------------
bool ForgeUIAPI::SliderFloat(const TEString &label, float *v, float v_min, float v_max, const TEString &format,
                             int flags)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    return forge::SliderFloat(std::string_view(label.c_str(), label.Length()), v, v_min, v_max,
                              forge::ForgeDimension::Px(200.0f), {}, ctx);
}

bool ForgeUIAPI::SliderInt(const TEString &label, int *v, int v_min, int v_max, const TEString &format, int flags)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    return forge::SliderInt(std::string_view(label.c_str(), label.Length()), reinterpret_cast<int32_t *>(v), v_min,
                            v_max, forge::ForgeDimension::Px(200.0f), {}, ctx);
}

bool ForgeUIAPI::DragFloat(const TEString &label, float *value, float speed, float min, float max,
                           const TEString &format, int flags)
{
    return SliderFloat(label, value, min, max, format, flags);
}

bool ForgeUIAPI::DragFloat2(const TEString &label, float *v, float speed, float min, float max, const TEString &format,
                            int flags)
{
    if (!v)
        return false;
    bool c1 = SliderFloat(label + ".x", &v[0], min, max, format, flags);
    bool c2 = SliderFloat(label + ".y", &v[1], min, max, format, flags);
    return c1 || c2;
}

bool ForgeUIAPI::DragFloat3(const TEString &label, float *v, float speed, float min, float max, const TEString &format,
                            int flags)
{
    if (!v)
        return false;
    bool c1 = SliderFloat(label + ".x", &v[0], min, max, format, flags);
    bool c2 = SliderFloat(label + ".y", &v[1], min, max, format, flags);
    bool c3 = SliderFloat(label + ".z", &v[2], min, max, format, flags);
    return c1 || c2 || c3;
}

bool ForgeUIAPI::DragFloat4(const TEString &label, float *v, float speed, float min, float max, const TEString &format,
                            int flags)
{
    if (!v)
        return false;
    bool c1 = SliderFloat(label + ".x", &v[0], min, max, format, flags);
    bool c2 = SliderFloat(label + ".y", &v[1], min, max, format, flags);
    bool c3 = SliderFloat(label + ".z", &v[2], min, max, format, flags);
    bool c4 = SliderFloat(label + ".w", &v[3], min, max, format, flags);
    return c1 || c2 || c3 || c4;
}

bool ForgeUIAPI::DragInt(const TEString &label, int *v, float speed, int min, int max)
{
    return SliderInt(label, v, min, max);
}

// -------------------------------------------------------------------------
// Input Fields
// -------------------------------------------------------------------------
bool ForgeUIAPI::InputText(const TEString &label, TEString &value, int flags)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return false;

    TEVector2 avail = GetContentRegionAvail();
    float boxW = Min(220.0f, avail.x);
    float boxH = 28.0f;
    TEVector2 pos = AllocateItem(boxW, boxH);

    TEVector2 mousePos = GetMousePos();
    bool hovered =
        (mousePos.x >= pos.x && mousePos.x <= pos.x + boxW && mousePos.y >= pos.y && mousePos.y <= pos.y + boxH);

    // Draw input box background & border
    ctx->GetDrawList().AddRoundedRect(ToForgeVec2(pos), ToForgeVec2(pos + TEVector2(boxW, boxH)), 4.0f, 0xFF1A1C20);
    ctx->GetDrawList().AddBorder(ToForgeVec2(pos), ToForgeVec2(pos + TEVector2(boxW, boxH)), 4.0f, 1.0f,
                                 hovered ? 0xFF4080DF : 0xFF353A45);

    // Draw value string inside box
    DrawString(pos.x + 8.0f, pos.y + 7.0f, 0xFFFFFFFF, value);

    // Draw label outside box (strip ## id)
    TEString displayLabel = label;
    int hashIdx = displayLabel.Find("##");
    if (hashIdx >= 0)
        displayLabel = displayLabel.Left(static_cast<size_t>(hashIdx));

    if (!displayLabel.IsEmpty())
    {
        DrawString(pos.x + boxW + 8.0f, pos.y + 7.0f, 0xFFCCCCCC, displayLabel);
    }

    return false;
}

bool ForgeUIAPI::InputText(const TEString &label, char *buf, size_t bufSize, int flags)
{
    if (!buf)
        return false;
    TEString strVal(buf);
    bool changed = InputText(label, strVal, flags);
    if (changed)
    {
        size_t len = Min(strVal.Length(), bufSize - 1);
        std::memcpy(buf, strVal.c_str(), len);
        buf[len] = '\0';
    }
    return changed;
}

bool ForgeUIAPI::InputTextWithHint(const TEString &label, const TEString &hint, TEString &value, int flags)
{
    return InputText(label, value, flags);
}

bool ForgeUIAPI::InputTextWithHint(const TEString &label, const char *hint, char *buf, size_t bufSize, int flags)
{
    return InputText(label, buf, bufSize, flags);
}

bool ForgeUIAPI::InputTextMultiline(const TEString &label, TEString &value, const TEVector2 &size, int flags)
{
    return InputText(label, value, flags);
}

bool ForgeUIAPI::InputTextMultiline(const TEString &label, char *buf, size_t bufSize, const TEVector2 &size, int flags)
{
    return InputText(label, buf, bufSize, flags);
}

bool ForgeUIAPI::InputInt(const TEString &label, int *v, int step, int step_fast, int flags)
{
    return SliderInt(label, v, -10000, 10000);
}

// -------------------------------------------------------------------------
// Color Pickers & Edits
// -------------------------------------------------------------------------
bool ForgeUIAPI::ColorEdit3(const TEString &label, float *col)
{
    if (!col)
        return false;
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    forge::ForgeColor fc(col[0], col[1], col[2], 1.0f);
    bool changed = forge::ColorPicker(std::string_view(label.c_str(), label.Length()), &fc,
                                      forge::ForgeDimension::Px(200.0f), ctx);
    if (changed)
    {
        col[0] = fc.r;
        col[1] = fc.g;
        col[2] = fc.b;
    }
    return changed;
}

bool ForgeUIAPI::ColorEdit4(const TEString &label, float *col, int flags) { return ColorEdit3(label, col); }

bool ForgeUIAPI::ColorPicker4(const TEString &label, float *col, int flags) { return ColorEdit3(label, col); }

bool ForgeUIAPI::ColorButton(const TEString &desc_id, const TEVector4 &col, int flags, const TEVector2 &size)
{
    return false;
}

// -------------------------------------------------------------------------
// Combos & Selectables
// -------------------------------------------------------------------------
bool ForgeUIAPI::Combo(const TEString &label, int *currentItem, const char *const items[], int itemsCount,
                       int popupMaxHeightInItems)
{
    static bool s_DropdownOpen = false;
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    return forge::Dropdown(std::string_view(label.c_str(), label.Length()), reinterpret_cast<int32_t *>(currentItem),
                           items, static_cast<size_t>(itemsCount), &s_DropdownOpen, forge::ForgeDimension::Px(200.0f),
                           ctx);
}

bool ForgeUIAPI::BeginCombo(const TEString &label, const TEString &previewValue, int flags) { return false; }

void ForgeUIAPI::EndCombo() {}

bool ForgeUIAPI::Selectable(const TEString &label, bool selected, int flags, const TEVector2 &size)
{
    return Button(label, size);
}

bool ForgeUIAPI::Selectable(const TEString &label, bool *selected, int flags, const TEVector2 &size)
{
    if (!selected)
        return false;
    if (Button(label, size))
    {
        *selected = !(*selected);
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------
// Images
// -------------------------------------------------------------------------
void ForgeUIAPI::Image(void *userTextureId, const TEVector2 &size, const TEVector2 &uv0, const TEVector2 &uv1,
                       const TEVector4 &tintCol)
{
    TEVector2 pos = AllocateItem(size.x, size.y);
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
    {
        uint64_t handle = reinterpret_cast<uintptr_t>(userTextureId);
        ctx->GetDrawList().AddTexturedQuad(ToForgeVec2(pos), ToForgeVec2(pos + size), ToForgeVec2(uv0),
                                           ToForgeVec2(uv1), handle);
    }
}

bool ForgeUIAPI::ImageButton(const TEString &strId, void *userTextureId, const TEVector2 &size, const TEVector2 &uv0,
                             const TEVector2 &uv1)
{
    return false;
}

// -------------------------------------------------------------------------
// Layout, Groups, Spacing
// -------------------------------------------------------------------------
void ForgeUIAPI::Separator()
{
    auto *win = CurrentWindowLayout();
    if (win)
    {
        TEVector2 pos = AllocateItem(win->size.x - 24.0f, 2.0f);
        if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
        {
            ctx->GetDrawList().AddRect(ToForgeVec2(pos), ToForgeVec2(pos + TEVector2(win->size.x - 24.0f, 1.0f)),
                                       0x33FFFFFF);
        }
    }
}

void ForgeUIAPI::SeparatorEx(int flags) { Separator(); }

void ForgeUIAPI::Spacing()
{
    auto *win = CurrentWindowLayout();
    if (win)
        win->cursor.y += 8.0f;
}

void ForgeUIAPI::NewLine()
{
    auto *win = CurrentWindowLayout();
    if (win)
    {
        win->cursor.y += (win->lineHeight > 0.0f ? win->lineHeight : 18.0f) + 6.0f;
        win->cursor.x = win->columns.active ? win->columns.startX[win->columns.current] : win->startX;
        win->lineHeight = 0.0f;
        win->sameLine = false;
    }
}

void ForgeUIAPI::SameLine(float offset_from_start_x, float spacing)
{
    auto *win = CurrentWindowLayout();
    if (win)
    {
        win->sameLine = true;
        win->sameLineSpacing = (spacing >= 0.0f) ? spacing : 8.0f;
        if (offset_from_start_x > 0.0f)
            win->cursor.x = win->startX + offset_from_start_x;
    }
}

void ForgeUIAPI::Dummy(const TEVector2 &size) { AllocateItem(size.x, size.y); }

void ForgeUIAPI::BeginGroup() {}
void ForgeUIAPI::EndGroup() {}
void ForgeUIAPI::Indent(float indentW)
{
    auto *win = CurrentWindowLayout();
    if (win)
    {
        win->startX += (indentW > 0.0f ? indentW : 20.0f);
        win->cursor.x = win->startX;
    }
}

void ForgeUIAPI::Unindent(float indentW)
{
    auto *win = CurrentWindowLayout();
    if (win)
    {
        win->startX = Max(0.0f, win->startX - (indentW > 0.0f ? indentW : 20.0f));
        win->cursor.x = win->startX;
    }
}

void ForgeUIAPI::Columns(int count, const char *id, bool border)
{
    auto *win = CurrentWindowLayout();
    if (!win)
        return;
    if (count <= 1)
    {
        win->columns.active = false;
        return;
    }
    win->columns.active = true;
    win->columns.count = Min(count, 16);
    win->columns.current = 0;
    win->columns.columnStartY = win->cursor.y;
    win->columns.maxCursorY = win->cursor.y;
    float availW = win->size.x - (win->startX - win->pos.x) - 12.0f;
    float colW = availW / static_cast<float>(win->columns.count);
    float curX = win->startX;
    for (int i = 0; i < win->columns.count; ++i)
    {
        win->columns.widths[i] = colW;
        win->columns.startX[i] = curX;
        curX += colW;
    }
    win->cursor.x = win->columns.startX[0];
}

void ForgeUIAPI::NextColumn()
{
    auto *win = CurrentWindowLayout();
    if (!win || !win->columns.active)
        return;
    win->columns.maxCursorY = Max(win->columns.maxCursorY, win->cursor.y);
    win->columns.current++;
    if (win->columns.current >= win->columns.count)
    {
        win->columns.current = 0;
        win->columns.columnStartY = win->columns.maxCursorY;
        win->cursor.y = win->columns.columnStartY;
    }
    else
    {
        win->cursor.y = win->columns.columnStartY;
    }
    win->cursor.x = win->columns.startX[win->columns.current];
    win->lineHeight = 0.0f;
    win->sameLine = false;
}

void ForgeUIAPI::SetColumnWidth(int columnIndex, float width)
{
    auto *win = CurrentWindowLayout();
    if (!win || !win->columns.active || columnIndex < 0 || columnIndex >= win->columns.count)
        return;
    win->columns.widths[columnIndex] = width;
    float curX = win->columns.startX[0];
    for (int i = 0; i < win->columns.count; ++i)
    {
        win->columns.startX[i] = curX;
        curX += win->columns.widths[i];
    }
    win->cursor.x = win->columns.startX[win->columns.current];
}

void ForgeUIAPI::AlignTextToFramePadding() {}

// -------------------------------------------------------------------------
// ID & Item Stack
// -------------------------------------------------------------------------
void ForgeUIAPI::PushID(const TEString &strId)
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
        ctx->PushID(std::string_view(strId.c_str(), strId.Length()));
}

void ForgeUIAPI::PushID(int intId)
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
        ctx->PushID(intId);
}

void ForgeUIAPI::PopID()
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
        ctx->PopID();
}

unsigned int ForgeUIAPI::GetID(const TEString &strId)
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
        return static_cast<unsigned int>(ctx->GetID(std::string_view(strId.c_str(), strId.Length())));
    return 0;
}

void ForgeUIAPI::PushItemWidth(float itemWidth) {}
void ForgeUIAPI::PopItemWidth() {}
void ForgeUIAPI::PushMultiItemsWidths(int components, float width_full) {}
float ForgeUIAPI::CalcItemWidth() { return 200.0f; }
void ForgeUIAPI::BeginDisabled(bool disabled) {}
void ForgeUIAPI::EndDisabled() {}

// -------------------------------------------------------------------------
// Styling & Fonts
// -------------------------------------------------------------------------
void ForgeUIAPI::PushStyleColor(int idx, const TEColor &color)
{
    if (idx == 3) // TimeGUICol_ChildBg
    {
        m_ChildBgColors.Add(color);
    }
    else if (idx == 21) // TimeGUICol_Button
    {
        m_ButtonColors.Add(color);
    }
}

void ForgeUIAPI::PopStyleColor(int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (!m_ChildBgColors.IsEmpty())
            m_ChildBgColors.pop_back();
        if (!m_ButtonColors.IsEmpty())
            m_ButtonColors.pop_back();
    }
}

void ForgeUIAPI::PushStyleVar(int idx, float val)
{
    if (idx == 12) // TimeGUIStyleVar_FrameRounding
    {
        m_FrameRoundings.Add(val);
    }
}

void ForgeUIAPI::PushStyleVar(int idx, const TEVector2 &val) {}

void ForgeUIAPI::PopStyleVar(int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (!m_FrameRoundings.IsEmpty())
            m_FrameRoundings.pop_back();
    }
}

void ForgeUIAPI::PushFont(void *font) {}
void ForgeUIAPI::PopFont() {}
void *ForgeUIAPI::GetDefaultFont() { return nullptr; }
void *ForgeUIAPI::GetFontAtlasTextureID() { return nullptr; }

// -------------------------------------------------------------------------
// Tables
// -------------------------------------------------------------------------
bool ForgeUIAPI::BeginTable(const TEString &strId, int column, int flags, const TEVector2 &outerSize, float innerWidth)
{
    return false;
}
void ForgeUIAPI::EndTable() {}
void ForgeUIAPI::TableSetupColumn(const TEString &label, int flags, float initWidthOrWeight, unsigned int userId) {}
void ForgeUIAPI::TableHeadersRow() {}
bool ForgeUIAPI::TableNextColumn() { return false; }
void ForgeUIAPI::TableNextRow(float rowMinHeight, int rowFlags) {}

// -------------------------------------------------------------------------
// Trees & Collapsing Headers
// -------------------------------------------------------------------------
bool ForgeUIAPI::TreeNodeEx(const TEString &label, int flags) { return false; }
bool ForgeUIAPI::TreeNodeEx(void *ptrId, int flags, const TEString &text) { return false; }
void ForgeUIAPI::TreePop() {}

bool ForgeUIAPI::CollapsingHeader(const TEString &label, int flags)
{
    static bool s_Expanded = true;
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    return forge::CollapsingHeader(std::string_view(label.c_str(), label.Length()), &s_Expanded, ctx);
}

void ForgeUIAPI::SetNextItemOpen(bool isOpen) {}

// -------------------------------------------------------------------------
// Popups, Tooltips, Focus & Cursor
// -------------------------------------------------------------------------
void ForgeUIAPI::OpenPopup(const TEString &strId) {}
bool ForgeUIAPI::BeginPopup(const TEString &strId, int flags) { return false; }
bool ForgeUIAPI::BeginPopupContextWindow(const TEString &strId, int mouseButton, bool alsoOverItems) { return false; }
bool ForgeUIAPI::BeginPopupContextItem(const TEString &strId, int mouseButton) { return false; }
bool ForgeUIAPI::BeginPopupModal(const TEString &name, bool *open, int flags) { return false; }
void ForgeUIAPI::EndPopup() {}
void ForgeUIAPI::CloseCurrentPopup() {}
bool ForgeUIAPI::IsPopupOpen(const TEString &strId) { return false; }
void ForgeUIAPI::BeginTooltip() {}
void ForgeUIAPI::EndTooltip() {}
void ForgeUIAPI::SetTooltip(const TEString &text) {}
void ForgeUIAPI::SetItemTooltip(const TEString &text) {}
void ForgeUIAPI::SetItemDefaultFocus() {}
void ForgeUIAPI::SetMouseCursor(int cursorType) {}

// -------------------------------------------------------------------------
// Drag & Drop
// -------------------------------------------------------------------------
bool ForgeUIAPI::BeginDragDropSource(int flags) { return false; }
bool ForgeUIAPI::SetDragDropPayload(const TEString &type, const void *data, size_t size, int cond) { return false; }
void ForgeUIAPI::EndDragDropSource() {}

// -------------------------------------------------------------------------
// Docking (No-op in ForgeUI)
// -------------------------------------------------------------------------
unsigned int ForgeUIAPI::DockSpace(unsigned int id, const TEVector2 &size, int flags) { return 0; }
void ForgeUIAPI::DockBuilderRemoveNode(unsigned int nodeId) {}
void ForgeUIAPI::DockBuilderAddNode(unsigned int nodeId, int flags) {}
void ForgeUIAPI::DockBuilderSetNodeSize(unsigned int nodeId, const TEVector2 &size) {}
unsigned int ForgeUIAPI::DockBuilderSplitNode(unsigned int nodeId, int splitDir, float sizeRatio,
                                              unsigned int *outIdDir1, unsigned int *outIdDir2)
{
    return 0;
}
void ForgeUIAPI::DockBuilderDockWindow(const TEString &windowName, unsigned int nodeId) {}
void ForgeUIAPI::DockBuilderFinish(unsigned int nodeId) {}

// -------------------------------------------------------------------------
// Tabs & Cards
// -------------------------------------------------------------------------
bool ForgeUIAPI::BeginTabItem(const TEString &label, bool *open, int flags) { return false; }
void ForgeUIAPI::EndTabItem() {}
bool ForgeUIAPI::BeginTabBar(const TEString &strId, int flags) { return false; }
void ForgeUIAPI::EndTabBar() {}
bool ForgeUIAPI::BeginProjectCard(const TEString &id, const TEVector2 &size, bool &hovered)
{
    auto *ctx = static_cast<forge::ForgeContext *>(m_Context);
    if (!ctx)
        return false;

    auto *win = CurrentWindowLayout();
    TEVector2 pos = win ? win->cursor : TEVector2(0, 0);

    TEVector2 mousePos = GetMousePos();
    hovered =
        (mousePos.x >= pos.x && mousePos.x <= pos.x + size.x && mousePos.y >= pos.y && mousePos.y <= pos.y + size.y);

    auto &input = ctx->GetInput();
    bool clicked = hovered && input.mousePressed[0];

    // Card background & border
    uint32_t bgCol = hovered ? 0xFF282E3A : 0xFF181B22;
    ctx->GetDrawList().AddRoundedRect(ToForgeVec2(pos), ToForgeVec2(pos + size), 8.0f, bgCol);
    ctx->GetDrawList().AddBorder(ToForgeVec2(pos), ToForgeVec2(pos + size), 8.0f, 1.0f,
                                 hovered ? 0xFF5090F0 : 0xFF2A303C);

    return clicked;
}

void ForgeUIAPI::EndProjectCard() {}

// -------------------------------------------------------------------------
// Next Window & Item Controls
// -------------------------------------------------------------------------
void ForgeUIAPI::SetNextWindowPos(const TEVector2 &pos, int cond, const TEVector2 &pivot)
{
    m_NextWindowPos = pos;
    m_HasNextWindowPos = true;
}

void ForgeUIAPI::SetNextWindowSize(const TEVector2 &size, int cond)
{
    m_NextWindowSize = size;
    m_HasNextWindowSize = true;
}

void ForgeUIAPI::SetNextWindowContentSize(const TEVector2 &size) {}
void ForgeUIAPI::SetNextWindowDockID(unsigned int dockId, int cond) {}
void ForgeUIAPI::SetNextWindowViewport(unsigned int viewportId) {}
void ForgeUIAPI::SetNextItemWidth(float itemWidth) {}
void ForgeUIAPI::SetNextItemAllowOverlap() {}

// -------------------------------------------------------------------------
// State & Queries
// -------------------------------------------------------------------------
TEVector2 ForgeUIAPI::GetCursorPos()
{
    auto *win = CurrentWindowLayout();
    if (win)
        return win->cursor - win->pos;
    return {0, 0};
}

void ForgeUIAPI::SetCursorPos(const TEVector2 &pos)
{
    auto *win = CurrentWindowLayout();
    if (win)
        win->cursor = win->pos + pos;
}

float ForgeUIAPI::GetCursorPosX()
{
    auto *win = CurrentWindowLayout();
    if (win)
        return win->cursor.x - win->pos.x;
    return 0.0f;
}

float ForgeUIAPI::GetCursorPosY()
{
    auto *win = CurrentWindowLayout();
    if (win)
        return win->cursor.y - win->pos.y;
    return 0.0f;
}

void ForgeUIAPI::SetCursorPosX(float x)
{
    auto *win = CurrentWindowLayout();
    if (win)
        win->cursor.x = win->pos.x + x;
}

void ForgeUIAPI::SetCursorPosY(float y)
{
    auto *win = CurrentWindowLayout();
    if (win)
        win->cursor.y = win->pos.y + y;
}

TEVector2 ForgeUIAPI::GetCursorScreenPos()
{
    auto *win = CurrentWindowLayout();
    if (win)
        return win->cursor;
    return {0, 0};
}

void ForgeUIAPI::SetCursorScreenPos(const TEVector2 &pos)
{
    auto *win = CurrentWindowLayout();
    if (win)
        win->cursor = pos;
}

TEVector2 ForgeUIAPI::GetWindowSize()
{
    auto *win = CurrentWindowLayout();
    if (win)
        return win->size;
    return m_NextWindowSize;
}

TEVector2 ForgeUIAPI::GetWindowPos()
{
    auto *win = CurrentWindowLayout();
    if (win)
        return win->pos;
    return m_NextWindowPos;
}

float ForgeUIAPI::GetWindowWidth()
{
    auto *win = CurrentWindowLayout();
    if (win)
        return win->size.x;
    return m_NextWindowSize.x;
}

float ForgeUIAPI::GetWindowHeight()
{
    auto *win = CurrentWindowLayout();
    if (win)
        return win->size.y;
    return m_NextWindowSize.y;
}

TEVector2 ForgeUIAPI::GetContentRegionAvail()
{
    auto *win = CurrentWindowLayout();
    if (!win)
        return TEVector2(static_cast<float>(m_Width), static_cast<float>(m_Height));

    float availW = 0.0f;
    if (win->columns.active)
        availW =
            win->columns.widths[win->columns.current] - (win->cursor.x - win->columns.startX[win->columns.current]);
    else
        availW = (win->pos.x + win->size.x) - win->cursor.x - 12.0f;

    float availH = (win->pos.y + win->size.y) - win->cursor.y - 12.0f;
    return TEVector2(Max(0.0f, availW), Max(0.0f, availH));
}

float ForgeUIAPI::GetFrameHeight() { return 20.0f; }
TEVector2 ForgeUIAPI::GetWindowContentRegionMin() { return {0, 0}; }
TEVector2 ForgeUIAPI::GetWindowContentRegionMax()
{
    return TEVector2(static_cast<float>(m_Width), static_cast<float>(m_Height));
}
void ForgeUIAPI::SetScrollHereY(float centerYRatio) {}
float ForgeUIAPI::GetScrollY() { return 0.0f; }
void ForgeUIAPI::SetScrollY(float scrollY) {}
TEArray<unsigned int> ForgeUIAPI::GetInputQueueCharacters() { return {}; }
void ForgeUIAPI::ClearInputQueueCharacters() {}
TEVector2 ForgeUIAPI::GetMousePos() { return TEVector2(Input::GetMouseX(), Input::GetMouseY()); }

bool ForgeUIAPI::IsMouseDown(int button)
{
    if (button >= 0 && button < 3)
        return Input::IsMouseButtonPressed(static_cast<MouseCode>(button));
    return false;
}

bool ForgeUIAPI::IsMouseReleased(int button)
{
    if (button >= 0 && button < 3)
        return Input::GetMouseButtonUp(button);
    return false;
}

bool ForgeUIAPI::IsMouseClicked(int button, bool repeat)
{
    if (button >= 0 && button < 3)
        return Input::GetMouseButtonDown(button);
    return false;
}

bool ForgeUIAPI::IsMouseDoubleClicked(int button) { return false; }
bool ForgeUIAPI::IsMouseDragging(int button, float lock_threshold) { return false; }
TEVector2 ForgeUIAPI::GetMouseDragDelta(int button, float lock_threshold) { return {0, 0}; }
void ForgeUIAPI::ResetMouseDragDelta(int button) {}
bool ForgeUIAPI::IsAnyItemHovered() { return false; }
bool ForgeUIAPI::IsKeyPressed(int key) { return Input::IsKeyPressed(static_cast<KeyCode>(key)); }
double ForgeUIAPI::GetTime() { return 0.0; }

TEVector2 ForgeUIAPI::CalcTextSize(const TEString &text)
{
    if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
    {
        forge::ForgeVec2 sz = ctx->GetFont().MeasureText(text.c_str(), 14.0f);
        return FromForgeVec2(sz);
    }
    return {static_cast<float>(text.Length() * 7), 14.0f};
}

bool ForgeUIAPI::IsItemDeactivatedAfterEdit() { return false; }
bool ForgeUIAPI::IsItemHovered(int flags) { return false; }
bool ForgeUIAPI::IsItemActive() { return false; }
bool ForgeUIAPI::IsItemFocused() { return false; }
bool ForgeUIAPI::IsItemClicked(int mouse_button) { return false; }
bool ForgeUIAPI::IsWindowHovered(int flags) { return false; }
bool ForgeUIAPI::IsWindowFocused(int flags) { return false; }
bool ForgeUIAPI::IsWindowAppearing() { return false; }
TEVector2 ForgeUIAPI::GetItemRectMin() { return {0, 0}; }
TEVector2 ForgeUIAPI::GetItemRectSize() { return {0, 0}; }
TEVector2 ForgeUIAPI::GetItemRectMax() { return {0, 0}; }
void ForgeUIAPI::SetKeyboardFocusHere(int offset) {}
void ForgeUIAPI::SetClipboardText(const TEString &text) {}
TEString ForgeUIAPI::GetClipboardText() { return ""; }
void ForgeUIAPI::SetCaretColor(const TEVector4 &color) {}
TEVector4 ForgeUIAPI::GetCaretColor() { return {1, 1, 1, 1}; }
void ForgeUIAPI::SaveIniSettingsToDisk(const char *ini_filename) {}
void ForgeUIAPI::LoadIniSettingsFromDisk(const char *ini_filename) {}

// -------------------------------------------------------------------------
// DrawList Commands
// -------------------------------------------------------------------------
void ForgeUIAPI::DrawListAddLine(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                 float thickness)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (!dl)
        return;

    float th = (thickness > 0.0f) ? thickness : 1.0f;
    TEVector2 dir = p2 - p1;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.0001f)
        return;

    TEVector2 normal(-dir.y / len, dir.x / len);
    TEVector2 offset = normal * (th * 0.5f);

    TEVector2 v0 = p1 - offset;
    TEVector2 v1 = p1 + offset;
    TEVector2 v2 = p2 + offset;
    TEVector2 v3 = p2 - offset;

    // Use min/max bounding quad for the line segment
    TEVector2 minPos(Min(Min(v0.x, v1.x), Min(v2.x, v3.x)), Min(Min(v0.y, v1.y), Min(v2.y, v3.y)));
    TEVector2 maxPos(Max(Max(v0.x, v1.x), Max(v2.x, v3.x)), Max(Max(v0.y, v1.y), Max(v2.y, v3.y)));
    dl->AddRect(ToForgeVec2(minPos), ToForgeVec2(maxPos), color);
}

void ForgeUIAPI::DrawListAddRect(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                 float rounding, int flags, float thickness)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
    {
        float th = (thickness > 0.0f) ? thickness : 1.0f;
        dl->AddBorder(ToForgeVec2(p1), ToForgeVec2(p2), rounding, th, color);
    }
}

void ForgeUIAPI::DrawListAddRectFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, unsigned int color,
                                       float rounding)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
    {
        if (rounding > 0.0f)
            dl->AddRoundedRect(ToForgeVec2(p1), ToForgeVec2(p2), rounding, color);
        else
            dl->AddRect(ToForgeVec2(p1), ToForgeVec2(p2), color);
    }
}

void ForgeUIAPI::DrawListAddCircle(void *drawList, const TEVector2 &center, float radius, unsigned int color,
                                   int num_segments, float thickness)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
    {
        float th = (thickness > 0.0f) ? thickness : 1.0f;
        TEVector2 minPos(center.x - radius, center.y - radius);
        TEVector2 maxPos(center.x + radius, center.y + radius);
        dl->AddBorder(ToForgeVec2(minPos), ToForgeVec2(maxPos), radius, th, color);
    }
}

void ForgeUIAPI::DrawListAddCircleFilled(void *drawList, const TEVector2 &center, float radius, unsigned int color,
                                         int num_segments)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
        dl->AddCircle(ToForgeVec2(center), radius, color);
}

void ForgeUIAPI::DrawListAddText(void *drawList, const TEVector2 &pos, unsigned int color, const TEString &text)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
    {
        if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
            ctx->GetFont().AppendTextToDrawList(dl, text.c_str(), ToForgeVec2(pos), 14.0f, color);
    }
}

void ForgeUIAPI::DrawListAddText(void *drawList, void *font, float fontSize, const TEVector2 &pos, unsigned int color,
                                 const TEString &text)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
    {
        if (auto *ctx = static_cast<forge::ForgeContext *>(m_Context))
            ctx->GetFont().AppendTextToDrawList(dl, text.c_str(), ToForgeVec2(pos), fontSize, color);
    }
}

void ForgeUIAPI::DrawListAddImage(void *drawList, void *texture_id, const TEVector2 &p_min, const TEVector2 &p_max,
                                  const TEVector2 &uv_min, const TEVector2 &uv_max, unsigned int col)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
    {
        uint64_t handle = reinterpret_cast<uintptr_t>(texture_id);
        dl->AddTexturedQuad(ToForgeVec2(p_min), ToForgeVec2(p_max), ToForgeVec2(uv_min), ToForgeVec2(uv_max), handle,
                            col);
    }
}

void ForgeUIAPI::DrawListAddBezierCubic(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                        const TEVector2 &p4, unsigned int color, float thickness, int num_segments)
{
}

void ForgeUIAPI::DrawListPushClipRect(void *drawList, const TEVector2 &clip_rect_min, const TEVector2 &clip_rect_max,
                                      bool intersect_with_current_clip_rect)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
    {
        float minX = Clamp(clip_rect_min.x, 0.0f, static_cast<float>(m_Width));
        float minY = Clamp(clip_rect_min.y, 0.0f, static_cast<float>(m_Height));
        float maxX = Clamp(clip_rect_max.x, 0.0f, static_cast<float>(m_Width));
        float maxY = Clamp(clip_rect_max.y, 0.0f, static_cast<float>(m_Height));
        dl->PushClipRect(forge::ForgeVec4(minX, minY, maxX, maxY));
    }
}

void ForgeUIAPI::DrawListPopClipRect(void *drawList)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    if (dl)
        dl->PopClipRect();
}

void ForgeUIAPI::DrawListAddPolyline(void *drawList, const TEVector2 *points, int num_points, unsigned int color,
                                     int flags, float thickness)
{
}
void ForgeUIAPI::DrawListAddConvexPolyFilled(void *drawList, const TEVector2 *points, int num_points,
                                             unsigned int color)
{
}
void ForgeUIAPI::DrawListAddTriangleFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2,
                                           const TEVector2 &p3, unsigned int color)
{
}
void ForgeUIAPI::DrawListAddQuadFilled(void *drawList, const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                       const TEVector2 &p4, unsigned int color)
{
}
void ForgeUIAPI::DrawListPushTextureID(void *drawList, void *texture_id) {}
void *ForgeUIAPI::DrawListCreate() { return nullptr; }
void ForgeUIAPI::DrawListDestroy(void *drawList) {}
void ForgeUIAPI::DrawListRender(void *drawList, const TEVector2 &displaySize) {}
int ForgeUIAPI::DrawListGetVertexCount(void *drawList)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    return dl ? static_cast<int>(dl->GetVertexCount()) : 0;
}

int ForgeUIAPI::DrawListGetCommandCount(void *drawList)
{
    auto *dl = static_cast<forge::ForgeDrawList *>(drawList ? drawList : GetWindowDrawList());
    return dl ? static_cast<int>(dl->GetCommandCount()) : 0;
}

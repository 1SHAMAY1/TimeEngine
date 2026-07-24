#include "Editor/MaterialAssetEditor.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/MaterialSerializer.hpp"
#include "Renderer/SpriteSerializer.hpp"
#include "Renderer/SpriteSheetSerializer.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <imgui.h>
#include <filesystem>
#include <vector>
#include <string>

namespace TE
{

void MaterialAssetEditor::DrawEditor(EditorTab &tab)
{
    auto mat = std::dynamic_pointer_cast<Material>(tab.LoadedAsset);
    if (!mat) return;

    static float s_MatZoom = 1.0f;
    static TEVector2 s_LightPos2D = { 150.0f, 150.0f };
    static bool s_EnableLightTester = true;
    static float s_LightIntensity = 1.5f;
    static int s_TargetQueueIdx = 0;
    static std::vector<std::string> s_CustomQueues = { "Queue 1: Surface Slabs", "Queue 2: Lighting & Normal Coats", "Queue 3: FX & Modifiers", "Queue 4: Blending & Output" };
    static bool s_ExpandAllNodes = false;
    static bool s_CollapseAllNodes = false;

    TimeGUI::Columns(2, "MaterialMainSplitter", true);

    // --- LEFT COLUMN: Interactive Material Viewport & 2D Light Tester ---
    TimeGUI::BeginChild("MaterialLeftViewportPanel", TEVector2(0.0f, 0.0f), false);

    TimeGUI::Text("Material Render Output Preview");
    TimeGUI::Separator();

    TimeGUI::Checkbox("2D Light Tester", &s_EnableLightTester);
    TimeGUI::SameLine();
    TimeGUI::SliderFloat("Light Intensity", &s_LightIntensity, 0.1f, 5.0f, "%.1f");
    TimeGUI::SameLine();
    TimeGUI::SliderFloat("Zoom", &s_MatZoom, 0.2f, 3.0f, "%.0f%%");

    TimeGUI::Separator();

    auto &stack = mat->GetPassStack();

    // Find active pass node properties
    std::shared_ptr<Texture> albedoTex = nullptr;
    std::shared_ptr<Texture> normalTex = nullptr;
    std::shared_ptr<Texture> emissiveTex = nullptr;
    TEVector4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    TEVector4 glowColor = { 1.0f, 1.0f, 0.5f, 1.0f };
    float bumpDepth = 1.0f;
    float glowMult = 1.0f;
    float uTiling = 1.0f, vTiling = 1.0f;

    for (auto &node : stack)
    {
        if (!node.Enabled) continue;
        if (node.Type == MaterialPassNodeType::BaseSurfaceSlab)
        {
            albedoTex = node.TextureRef;
            baseColor = node.Color;
        }
        else if (node.Type == MaterialPassNodeType::CoatNormalSlab)
        {
            normalTex = node.TextureRef;
            bumpDepth = node.FloatVal1;
        }
        else if (node.Type == MaterialPassNodeType::EmissiveGlowSlab)
        {
            emissiveTex = node.TextureRef;
            glowColor = node.Color;
            glowMult = node.FloatVal1;
        }
        else if (node.Type == MaterialPassNodeType::UVScrollerModifier)
        {
            uTiling = node.FloatVal3;
            vTiling = node.FloatVal4;
        }
    }

    float previewW = 320.0f * s_MatZoom;
    float previewH = 320.0f * s_MatZoom;
    TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();

    if (albedoTex)
    {
        TimeGUI::Image((void *)(uintptr_t)albedoTex->GetRendererID(), TEVector2(previewW, previewH),
                       TEVector2(0, 0), TEVector2(uTiling, vTiling), baseColor);
    }
    else
    {
        auto drawList = TimeGUI::GetWindowDrawList();
        drawList->AddRectFilled(ImVec2(canvasPos.x, canvasPos.y), ImVec2(canvasPos.x + previewW, canvasPos.y + previewH),
                                IM_COL32((int)(baseColor.x * 255), (int)(baseColor.y * 255), (int)(baseColor.z * 255), (int)(baseColor.w * 255)));
        drawList->AddRect(ImVec2(canvasPos.x, canvasPos.y), ImVec2(canvasPos.x + previewW, canvasPos.y + previewH),
                          IM_COL32(255, 255, 255, 120), 0.0f, 0, 1.5f);
        TimeGUI::Dummy(TEVector2(previewW, previewH));
    }

    bool isCanvasHovered = TimeGUI::IsItemHovered();
    bool isMouseDown = TimeGUI::IsMouseDown(0);
    TEVector2 mousePos = TimeGUI::GetMousePos();

    if (s_EnableLightTester && isCanvasHovered && isMouseDown)
    {
        s_LightPos2D = { mousePos.x - canvasPos.x, mousePos.y - canvasPos.y };
    }

    auto drawList = TimeGUI::GetWindowDrawList();

    if (s_EnableLightTester)
    {
        float lx = canvasPos.x + s_LightPos2D.x;
        float ly = canvasPos.y + s_LightPos2D.y;

        drawList->AddCircleFilled(ImVec2(lx, ly), 10.0f, IM_COL32(255, 240, 150, 255));
        drawList->AddCircle(ImVec2(lx, ly), 20.0f * s_LightIntensity, IM_COL32(255, 220, 100, 180), 0, 2.0f);
        drawList->AddCircle(ImVec2(lx, ly), 45.0f * s_LightIntensity, IM_COL32(255, 200, 50, 80), 0, 1.5f);

        if (normalTex)
        {
            drawList->AddCircleFilled(ImVec2(lx, ly), 30.0f * bumpDepth, IM_COL32(255, 255, 255, (int)(80 * s_LightIntensity)));
        }

        if (glowMult > 0.0f)
        {
            drawList->AddRect(ImVec2(canvasPos.x, canvasPos.y), ImVec2(canvasPos.x + previewW, canvasPos.y + previewH),
                              IM_COL32((int)(glowColor.x * 255), (int)(glowColor.y * 255), (int)(glowColor.z * 255), (int)(40 * glowMult)), 0.0f, 0, 2.0f);
        }
    }

    TimeGUI::EndChild();

    TimeGUI::NextColumn();

    // --- RIGHT COLUMN: ECS Hierarchy-Style Queue & Stack Inspector ---
    TimeGUI::BeginChild("MaterialRightPipelinePanel", TEVector2(0.0f, 0.0f), false);

    TimeGUI::Text("Material Name:");
    TimeGUI::SameLine();
    char nameBuffer[256];
    strcpy_s(nameBuffer, mat->GetName().c_str());
    if (TimeGUI::InputText("##MatName", nameBuffer, sizeof(nameBuffer)))
    {
        mat->SetName(nameBuffer);
        MaterialSerializer serializer(mat);
        serializer.Serialize(tab.AssetPath);
        tab.Title = nameBuffer;
    }

    TimeGUI::Separator();

    auto resolveAnyTextureAsset = [](const std::string &filepath) -> std::shared_ptr<Texture> {
        if (filepath.empty() || !std::filesystem::exists(filepath)) return nullptr;
        std::string ext = std::filesystem::path(filepath).extension().string();
        if (ext == ".tesprite")
        {
            auto sprite = std::make_shared<Sprite>();
            SpriteSerializer serializer(sprite);
            if (serializer.Deserialize(filepath))
                return sprite->GetTexture();
        }
        else if (ext == ".tespritesheet")
        {
            auto sheet = std::make_shared<SpriteSheet>();
            SpriteSheetSerializer serializer(sheet);
            if (serializer.Deserialize(filepath))
                return sheet->GetTexture();
        }
        return std::make_shared<Texture>(filepath);
    };

    TimeGUI::Text("Target Queue:");
    TimeGUI::SameLine();

    std::vector<const char*> queueItemPtrs;
    for (auto &qName : s_CustomQueues) queueItemPtrs.push_back(qName.c_str());
    queueItemPtrs.push_back("+ Create New Queue...");

    int currentQIdx = s_TargetQueueIdx;
    if (TimeGUI::Combo("##TargetQueueCombo", &currentQIdx, queueItemPtrs.data(), (int)queueItemPtrs.size()))
    {
        if (currentQIdx == (int)queueItemPtrs.size() - 1)
        {
            std::string newQName = "Queue " + std::to_string(s_CustomQueues.size() + 1) + ": Custom FX";
            s_CustomQueues.push_back(newQName);
            s_TargetQueueIdx = (int)s_CustomQueues.size() - 1;
        }
        else
        {
            s_TargetQueueIdx = currentQIdx;
        }
    }

    TimeGUI::SameLine();

    static int s_SelectedFuncIdx = 0;
    const char *catalogItems[] = {
        "+ Add Function...",
        "Base Surface Slab",
        "Coat / Normal Slab",
        "Emissive Glow Slab",
        "PBR Metallic & Roughness Slab",
        "Fresnel Rim Light Modifier",
        "Color Grading Modifier",
        "Subsurface Scattering Slab",
        "Dissolve Mask Modifier",
        "Triplanar World Mapping Slab",
        "Parallax Occlusion Depth Slab",
        "Gradient Ramp Colorizer Modifier",
        "Chromatic Aberration & UV Distortion",
        "Sprite / SpriteSheet Frame Sampler",
        "UV Scroller Modifier",
        "Blend Output State"
    };

    if (TimeGUI::Combo("##AddFuncCombo", &s_SelectedFuncIdx, catalogItems, 16) && s_SelectedFuncIdx > 0)
    {
        MaterialPassNode node;
        node.Enabled = true;
        node.TargetQueueIndex = s_TargetQueueIdx;
        if (s_TargetQueueIdx < (int)s_CustomQueues.size())
            node.QueueName = s_CustomQueues[s_TargetQueueIdx];

        switch (s_SelectedFuncIdx)
        {
        case 1:
            node.Name = "Base Surface Slab";
            node.Type = MaterialPassNodeType::BaseSurfaceSlab;
            break;
        case 2:
            node.Name = "Coat / Normal Slab";
            node.Type = MaterialPassNodeType::CoatNormalSlab;
            break;
        case 3:
            node.Name = "Emissive Glow Slab";
            node.Type = MaterialPassNodeType::EmissiveGlowSlab;
            break;
        case 4:
            node.Name = "PBR Metallic & Roughness";
            node.Type = MaterialPassNodeType::PBRMetallicRoughnessSlab;
            break;
        case 5:
            node.Name = "Fresnel Rim Light";
            node.Type = MaterialPassNodeType::FresnelRimLightModifier;
            break;
        case 6:
            node.Name = "Color Grading Modifier";
            node.Type = MaterialPassNodeType::ColorGradingModifier;
            break;
        case 7:
            node.Name = "Subsurface Scattering";
            node.Type = MaterialPassNodeType::SubsurfaceScatteringSlab;
            break;
        case 8:
            node.Name = "Dissolve Mask Modifier";
            node.Type = MaterialPassNodeType::DissolveMaskModifier;
            break;
        case 9:
            node.Name = "Triplanar World Mapping";
            node.Type = MaterialPassNodeType::TriplanarMappingSlab;
            break;
        case 10:
            node.Name = "Parallax Occlusion Depth";
            node.Type = MaterialPassNodeType::ParallaxOcclusionSlab;
            break;
        case 11:
            node.Name = "Gradient Ramp Colorizer";
            node.Type = MaterialPassNodeType::GradientRampColorizerModifier;
            break;
        case 12:
            node.Name = "Chromatic Aberration & Distortion";
            node.Type = MaterialPassNodeType::ChromaticAberrationModifier;
            break;
        case 13:
            node.Name = "Sprite / SpriteSheet Sampler";
            node.Type = MaterialPassNodeType::SpriteSheetFrameSamplerSlab;
            break;
        case 14:
            node.Name = "UV Scroller Modifier";
            node.Type = MaterialPassNodeType::UVScrollerModifier;
            break;
        case 15:
            node.Name = "Blend Output State";
            node.Type = MaterialPassNodeType::BlendOutputState;
            break;
        }

        mat->AddPassNode(node);
        MaterialSerializer serializer(mat);
        serializer.Serialize(tab.AssetPath);
        s_SelectedFuncIdx = 0;
    }

    TimeGUI::Separator();

    if (TimeGUI::Button("Expand All")) s_ExpandAllNodes = true;
    TimeGUI::SameLine();
    if (TimeGUI::Button("Collapse All")) s_CollapseAllNodes = true;

    TimeGUI::Separator();

    // --- ECS HIERARCHY TREE VIEW: Left to Right Resizable Queue Columns ---
    ImGui::SetNextWindowContentSize(ImVec2((float)s_CustomQueues.size() * 320.0f, 0.0f));
    TimeGUI::BeginChild("MaterialECSHierarchy", TEVector2(0.0f, 0.0f), true, TimeGUIWindowFlags_HorizontalScrollbar);

    TimeGUI::Columns((int)s_CustomQueues.size(), "MaterialQueueResizableCols", true);

    for (size_t q = 0; q < s_CustomQueues.size(); ++q)
    {
        std::vector<size_t> nodeIndicesInQueue;
        for (size_t i = 0; i < stack.size(); ++i)
        {
            if (stack[i].TargetQueueIndex == (int)q)
                nodeIndicesInQueue.push_back(i);
        }

        std::string qHeaderLabel = "[Q" + std::to_string(q + 1) + "] " + s_CustomQueues[q] + " (" + std::to_string(nodeIndicesInQueue.size()) + ")###QHeader_" + std::to_string(q);

        if (s_ExpandAllNodes) TimeGUI::SetNextItemOpen(true);
        if (s_CollapseAllNodes) TimeGUI::SetNextItemOpen(false);

        if (TimeGUI::CollapsingHeader(qHeaderLabel.c_str()))
        {
            if (nodeIndicesInQueue.empty())
            {
                TimeGUI::TextDisabled(" (Queue Empty)");
            }

            for (size_t idx : nodeIndicesInQueue)
            {
                TimeGUI::PushID((int)idx);
                auto &node = stack[idx];
                bool nodeChanged = false;

                if (s_ExpandAllNodes) TimeGUI::SetNextItemOpen(true);
                if (s_CollapseAllNodes) TimeGUI::SetNextItemOpen(false);

                // RED DELETE BUTTON (Centered X with zero frame padding)
                TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(0.0f, 0.0f));
                TimeGUI::PushStyleColor(TimeGUICol_Button, TEColor(0.75f, 0.15f, 0.15f, 1.0f));
                TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEColor(0.95f, 0.25f, 0.25f, 1.0f));
                TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEColor(0.55f, 0.1f, 0.1f, 1.0f));
                bool deleteClicked = TimeGUI::Button("X", 18.0f, 18.0f);
                TimeGUI::PopStyleColor(3);
                TimeGUI::PopStyleVar();

                if (deleteClicked)
                {
                    mat->RemovePassNode(idx);
                    MaterialSerializer serializer(mat);
                    serializer.Serialize(tab.AssetPath);
                    TimeGUI::PopID();
                    break;
                }

                TimeGUI::SameLine();
                if (TimeGUI::Checkbox("##Enable", &node.Enabled)) nodeChanged = true;
                TimeGUI::SameLine();

                std::string nodeHeaderLabel = node.Name + "###NodeTree_" + std::to_string(idx);
                bool nodeOpen = TimeGUI::TreeNodeEx(nodeHeaderLabel.c_str());

                if (nodeOpen)
                {
                    if (node.Enabled)
                    {
                        switch (node.Type)
                        {
                        case MaterialPassNodeType::BaseSurfaceSlab:
                        {
                            static char texBuf[256] = "";
                            strcpy_s(texBuf, node.TexturePath.c_str());
                            if (TimeGUI::InputText("Asset (Tex/Sprite)", texBuf, sizeof(texBuf)))
                            {
                                node.TexturePath = texBuf;
                                node.TextureRef = resolveAnyTextureAsset(node.TexturePath);
                                nodeChanged = true;
                            }
                            TimeGUI::SameLine();
                            if (TimeGUI::Button("..."))
                            {
                                std::string filepath = PlatformUtils::OpenFile("Supported Assets (*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite)\0*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite\0All Files (*.*)\0*.*\0");
                                if (!filepath.empty())
                                {
                                    node.TexturePath = filepath;
                                    node.TextureRef = resolveAnyTextureAsset(filepath);
                                    nodeChanged = true;
                                }
                            }

                            float colArr[4] = { node.Color.x, node.Color.y, node.Color.z, node.Color.w };
                            if (TimeGUI::ColorEdit4("Color Tint", colArr))
                            {
                                node.Color = { colArr[0], colArr[1], colArr[2], colArr[3] };
                                nodeChanged = true;
                            }

                            if (TimeGUI::SliderFloat("Roughness", &node.FloatVal1, 0.0f, 1.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Metallic", &node.FloatVal2, 0.0f, 1.0f, "%.2f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::CoatNormalSlab:
                        {
                            static char normBuf[256] = "";
                            strcpy_s(normBuf, node.TexturePath.c_str());
                            if (TimeGUI::InputText("Normal Asset", normBuf, sizeof(normBuf)))
                            {
                                node.TexturePath = normBuf;
                                node.TextureRef = resolveAnyTextureAsset(node.TexturePath);
                                nodeChanged = true;
                            }
                            TimeGUI::SameLine();
                            if (TimeGUI::Button("..."))
                            {
                                std::string filepath = PlatformUtils::OpenFile("Supported Assets (*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite)\0*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite\0All Files (*.*)\0*.*\0");
                                if (!filepath.empty())
                                {
                                    node.TexturePath = filepath;
                                    node.TextureRef = resolveAnyTextureAsset(filepath);
                                    nodeChanged = true;
                                }
                            }

                            if (TimeGUI::SliderFloat("Bump Depth", &node.FloatVal1, 0.0f, 5.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Clear Coat", &node.FloatVal2, 0.0f, 1.0f, "%.2f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::EmissiveGlowSlab:
                        {
                            static char emBuf[256] = "";
                            strcpy_s(emBuf, node.TexturePath.c_str());
                            if (TimeGUI::InputText("Glow Asset", emBuf, sizeof(emBuf)))
                            {
                                node.TexturePath = emBuf;
                                node.TextureRef = resolveAnyTextureAsset(node.TexturePath);
                                nodeChanged = true;
                            }
                            TimeGUI::SameLine();
                            if (TimeGUI::Button("..."))
                            {
                                std::string filepath = PlatformUtils::OpenFile("Supported Assets (*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite)\0*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite\0All Files (*.*)\0*.*\0");
                                if (!filepath.empty())
                                {
                                    node.TexturePath = filepath;
                                    node.TextureRef = resolveAnyTextureAsset(filepath);
                                    nodeChanged = true;
                                }
                            }

                            float colArr[4] = { node.Color.x, node.Color.y, node.Color.z, node.Color.w };
                            if (TimeGUI::ColorEdit4("Glow Tint", colArr))
                            {
                                node.Color = { colArr[0], colArr[1], colArr[2], colArr[3] };
                                nodeChanged = true;
                            }

                            if (TimeGUI::SliderFloat("Intensity", &node.FloatVal1, 0.0f, 20.0f, "%.1fx")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::PBRMetallicRoughnessSlab:
                        {
                            if (TimeGUI::SliderFloat("Metallic", &node.FloatVal1, 0.0f, 1.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Roughness", &node.FloatVal2, 0.0f, 1.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("AO Strength", &node.FloatVal3, 0.0f, 1.0f, "%.2f")) nodeChanged = true;
                            float colArr[4] = { node.Color.x, node.Color.y, node.Color.z, node.Color.w };
                            if (TimeGUI::ColorEdit4("Specular Tint", colArr))
                            {
                                node.Color = { colArr[0], colArr[1], colArr[2], colArr[3] };
                                nodeChanged = true;
                            }
                            break;
                        }
                        case MaterialPassNodeType::FresnelRimLightModifier:
                        {
                            float colArr[4] = { node.Color.x, node.Color.y, node.Color.z, node.Color.w };
                            if (TimeGUI::ColorEdit4("Rim Color", colArr))
                            {
                                node.Color = { colArr[0], colArr[1], colArr[2], colArr[3] };
                                nodeChanged = true;
                            }
                            if (TimeGUI::SliderFloat("Rim Exponent", &node.FloatVal1, 0.5f, 10.0f, "%.1f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Rim Multiplier", &node.FloatVal2, 0.0f, 5.0f, "%.1f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::ColorGradingModifier:
                        {
                            if (TimeGUI::SliderFloat("Saturation", &node.FloatVal1, 0.0f, 2.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Contrast", &node.FloatVal2, 0.5f, 2.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Brightness", &node.FloatVal3, -1.0f, 1.0f, "%.2f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::SubsurfaceScatteringSlab:
                        {
                            float colArr[4] = { node.Color.x, node.Color.y, node.Color.z, node.Color.w };
                            if (TimeGUI::ColorEdit4("Scattering Tint", colArr))
                            {
                                node.Color = { colArr[0], colArr[1], colArr[2], colArr[3] };
                                nodeChanged = true;
                            }
                            if (TimeGUI::SliderFloat("Translucency Depth", &node.FloatVal1, 0.0f, 5.0f, "%.2f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::DissolveMaskModifier:
                        {
                            static char disBuf[256] = "";
                            strcpy_s(disBuf, node.TexturePath.c_str());
                            if (TimeGUI::InputText("Noise Asset", disBuf, sizeof(disBuf)))
                            {
                                node.TexturePath = disBuf;
                                node.TextureRef = resolveAnyTextureAsset(node.TexturePath);
                                nodeChanged = true;
                            }
                            TimeGUI::SameLine();
                            if (TimeGUI::Button("..."))
                            {
                                std::string filepath = PlatformUtils::OpenFile("Supported Assets (*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite)\0*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite\0All Files (*.*)\0*.*\0");
                                if (!filepath.empty())
                                {
                                    node.TexturePath = filepath;
                                    node.TextureRef = resolveAnyTextureAsset(filepath);
                                    nodeChanged = true;
                                }
                            }
                            if (TimeGUI::SliderFloat("Dissolve Amount", &node.FloatVal1, 0.0f, 1.0f, "%.2f")) nodeChanged = true;
                            float colArr[4] = { node.Color.x, node.Color.y, node.Color.z, node.Color.w };
                            if (TimeGUI::ColorEdit4("Edge Burn Color", colArr))
                            {
                                node.Color = { colArr[0], colArr[1], colArr[2], colArr[3] };
                                nodeChanged = true;
                            }
                            break;
                        }
                        case MaterialPassNodeType::TriplanarMappingSlab:
                        {
                            static char triBuf[256] = "";
                            strcpy_s(triBuf, node.TexturePath.c_str());
                            if (TimeGUI::InputText("Projection Asset", triBuf, sizeof(triBuf)))
                            {
                                node.TexturePath = triBuf;
                                node.TextureRef = resolveAnyTextureAsset(node.TexturePath);
                                nodeChanged = true;
                            }
                            TimeGUI::SameLine();
                            if (TimeGUI::Button("..."))
                            {
                                std::string filepath = PlatformUtils::OpenFile("Supported Assets (*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite)\0*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite\0All Files (*.*)\0*.*\0");
                                if (!filepath.empty())
                                {
                                    node.TexturePath = filepath;
                                    node.TextureRef = resolveAnyTextureAsset(filepath);
                                    nodeChanged = true;
                                }
                            }
                            if (TimeGUI::SliderFloat("Blend Sharpness", &node.FloatVal1, 1.0f, 10.0f, "%.1f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("World Tile Scale", &node.FloatVal2, 0.1f, 10.0f, "%.2f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::ParallaxOcclusionSlab:
                        {
                            static char parBuf[256] = "";
                            strcpy_s(parBuf, node.TexturePath.c_str());
                            if (TimeGUI::InputText("Heightmap Asset", parBuf, sizeof(parBuf)))
                            {
                                node.TexturePath = parBuf;
                                node.TextureRef = resolveAnyTextureAsset(node.TexturePath);
                                nodeChanged = true;
                            }
                            TimeGUI::SameLine();
                            if (TimeGUI::Button("..."))
                            {
                                std::string filepath = PlatformUtils::OpenFile("Supported Assets (*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite)\0*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite\0All Files (*.*)\0*.*\0");
                                if (!filepath.empty())
                                {
                                    node.TexturePath = filepath;
                                    node.TextureRef = resolveAnyTextureAsset(filepath);
                                    nodeChanged = true;
                                }
                            }
                            if (TimeGUI::SliderFloat("Parallax Scale", &node.FloatVal1, 0.0f, 0.2f, "%.3f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Min/Max Steps", &node.FloatVal2, 8.0f, 64.0f, "%.0f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::GradientRampColorizerModifier:
                        {
                            float colArr[4] = { node.Color.x, node.Color.y, node.Color.z, node.Color.w };
                            if (TimeGUI::ColorEdit4("Ramp Color A", colArr))
                            {
                                node.Color = { colArr[0], colArr[1], colArr[2], colArr[3] };
                                nodeChanged = true;
                            }
                            if (TimeGUI::SliderFloat("Blend Exponent", &node.FloatVal1, 0.1f, 5.0f, "%.2f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::ChromaticAberrationModifier:
                        {
                            if (TimeGUI::SliderFloat("RGB Shift Offset", &node.FloatVal1, 0.0f, 0.1f, "%.3f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Ripple Frequency", &node.FloatVal2, 0.0f, 20.0f, "%.1f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::SpriteSheetFrameSamplerSlab:
                        {
                            static char spBuf[256] = "";
                            strcpy_s(spBuf, node.TexturePath.c_str());
                            if (TimeGUI::InputText("Sprite/Sheet Asset", spBuf, sizeof(spBuf)))
                            {
                                node.TexturePath = spBuf;
                                node.TextureRef = resolveAnyTextureAsset(node.TexturePath);
                                nodeChanged = true;
                            }
                            TimeGUI::SameLine();
                            if (TimeGUI::Button("..."))
                            {
                                std::string filepath = PlatformUtils::OpenFile("Supported Assets (*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite)\0*.png;*.jpg;*.tetexture;*.tespritesheet;*.tesprite\0All Files (*.*)\0*.*\0");
                                if (!filepath.empty())
                                {
                                    node.TexturePath = filepath;
                                    node.TextureRef = resolveAnyTextureAsset(filepath);
                                    nodeChanged = true;
                                }
                            }
                            if (TimeGUI::SliderFloat("Frame Index", &node.FloatVal1, 0.0f, 64.0f, "%.0f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::UVScrollerModifier:
                        {
                            if (TimeGUI::SliderFloat("Speed U", &node.FloatVal1, -5.0f, 5.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Speed V", &node.FloatVal2, -5.0f, 5.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Tiling U", &node.FloatVal3, 0.1f, 10.0f, "%.2f")) nodeChanged = true;
                            if (TimeGUI::SliderFloat("Tiling V", &node.FloatVal4, 0.1f, 10.0f, "%.2f")) nodeChanged = true;
                            break;
                        }
                        case MaterialPassNodeType::BlendOutputState:
                        {
                            const char *blendItems[] = { "Opaque", "Alpha Blend", "Additive", "Multiply" };
                            if (TimeGUI::Combo("Blend Mode", &node.BlendMode, blendItems, 4)) nodeChanged = true;
                            break;
                        }
                        default:
                            break;
                        }
                    }

                    TimeGUI::TreePop();
                }

                if (nodeChanged)
                {
                    MaterialSerializer serializer(mat);
                    serializer.Serialize(tab.AssetPath);
                }

                TimeGUI::PopID();
            }
        }

        TimeGUI::NextColumn();
    }

    TimeGUI::Columns(1);

    s_ExpandAllNodes = false;
    s_CollapseAllNodes = false;

    TimeGUI::EndChild();
    TimeGUI::EndChild();
    TimeGUI::Columns(1);
}

TE_REGISTER_ASSET_EDITOR(MaterialAssetEditor);

} // namespace TE

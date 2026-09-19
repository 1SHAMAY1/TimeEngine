#include "PreRequisites.h"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"
#include "Editor/DragDrop/DragDropTypes.hpp"
#include "Editor/DragDrop/IDragDropHandler.hpp"
#include "Editor/DragDrop/DragDropRegistry.hpp"
#include "Editor/DragDrop/DragDropManager.hpp"
#include "Editor/DragDrop/StandardDragDropHandlers.hpp"
#include "Utils/TEFileSystem.hpp"

// Custom Plugin Simulation Handler
class CustomVFXPluginDropHandler : public IDragDropHandler
{
public:
    TEString GetSupportedPayloadType() const override { return "TE_DND_VFX_PRESET"; }
    TEString GetSupportedTargetContext() const override { return DragDropTargetContext::Viewport; }
    TEString GetSupportedExtensions() const override { return ".vfx"; }
    int GetPriority() const override { return 50; }

    bool CanHandle(const DragDropContext &context, const DragDropPayload &payload) const override
    {
        return payload.IsType("TE_DND_VFX_PRESET");
    }

    bool OnDrop(const DragDropContext &context, const DragDropPayload &payload) override
    {
        m_SpawnedVFXCount++;
        m_LastPresetName = payload.AsString();
        return true;
    }

    int m_SpawnedVFXCount = 0;
    TEString m_LastPresetName;
};

// -----------------------------------------------------------------------------
// Integration Tests
// -----------------------------------------------------------------------------

TE_TEST_CASE(DragDropIntegration, CustomPluginHandlerIntegration)
{
    DragDropRegistry::Clear();

    auto vfxPluginHandler = CreateRef<CustomVFXPluginDropHandler>();
    DragDropRegistry::RegisterHandler(vfxPluginHandler);

    DragDropContext vpContext;
    vpContext.TargetContextId = DragDropTargetContext::Viewport;
    vpContext.ScreenPosition = TEVector2(500.0f, 350.0f);

    TEString vfxName = "Fireball_Explosion.vfx";
    DragDropPayload payload;
    payload.PayloadType = "TE_DND_VFX_PRESET";
    payload.Data = vfxName.c_str();
    payload.DataSize = vfxName.Length() + 1;
    payload.IsDelivery = true;

    bool dropped = DragDropRegistry::HandleDrop(vpContext, payload);
    TE_CHECK(dropped);
    TE_CHECK_EQ(vfxPluginHandler->m_SpawnedVFXCount, 1);
    TE_CHECK_EQ(vfxPluginHandler->m_LastPresetName, "Fireball_Explosion.vfx");

    DragDropRegistry::Clear();
}

TE_TEST_CASE(DragDropIntegration, SimulatedMemoryDragDropPipeline)
{
    DragDropRegistry::Clear();

    auto spriteHandler = CreateRef<Sprite2DViewportDropHandler>();
    DragDropRegistry::RegisterHandler(spriteHandler);

    DragDropContext context;
    context.TargetContextId = DragDropTargetContext::Viewport;

    // 1. Incompatible payload type
    DragDropPayload badPayload;
    badPayload.PayloadType = "RANDOM_UNHANDLED_TYPE";
    TE_CHECK(!DragDropRegistry::HandleDrop(context, badPayload));

    // 2. Incompatible file extension
    TEString badExtPath = "Assets/Shaders/PostProcess.glsl";
    DragDropPayload badExtPayload;
    badExtPayload.PayloadType = TE_DND_ASSET_PATH;
    badExtPayload.Data = badExtPath.c_str();
    badExtPayload.DataSize = badExtPath.Length() + 1;
    TE_CHECK(!DragDropRegistry::HandleDrop(context, badExtPayload));

    // 3. Compatible file extension check
    TEString goodExtPath = "Assets/Textures/Player_Idle.png";
    DragDropPayload goodPayload;
    goodPayload.PayloadType = TE_DND_ASSET_PATH;
    goodPayload.Data = goodExtPath.c_str();
    goodPayload.DataSize = goodExtPath.Length() + 1;

    auto matchedHandler = DragDropRegistry::FindHandler(context, goodPayload);
    TE_CHECK(matchedHandler != nullptr);
    TE_CHECK_EQ(matchedHandler->GetSupportedPayloadType(), TE_DND_ASSET_PATH);
    TE_CHECK_EQ(matchedHandler->GetTooltipText(context, goodPayload), "Spawn Sprite: Player_Idle.png");

    DragDropRegistry::Clear();
}

TE_TEST_CASE(DragDropIntegration, FolderAssetMoveHandlerIntegration)
{
    DragDropRegistry::Clear();

    auto folderMoveHandler = CreateRef<FolderAssetMoveDropHandler>();
    DragDropRegistry::RegisterHandler(folderMoveHandler);

    TEString targetFolder = "Artifacts/TestAudio";
    TEFileSystem::CreateDirectories(targetFolder);
    DragDropContext context;
    context.TargetContextId = DragDropTargetContext::ContentBrowserFolder;
    context.TargetUserData = &targetFolder;

    TEString draggedFile = "Assets/Textures/Logo.png";
    DragDropPayload payload;
    payload.PayloadType = TE_DND_ASSET_PATH;
    payload.Data = draggedFile.c_str();
    payload.DataSize = draggedFile.Length() + 1;

    auto matched = DragDropRegistry::FindHandler(context, payload);
    TE_CHECK(matched != nullptr);
    TE_CHECK_EQ(matched->GetTooltipText(context, payload), "Move into TestAudio");

    TEFileSystem::RemoveAll(targetFolder);
    DragDropRegistry::Clear();
}

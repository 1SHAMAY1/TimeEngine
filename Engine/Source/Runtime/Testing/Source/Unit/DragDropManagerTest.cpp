#include "PreRequisites.h"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"
#include "Editor/DragDrop/DragDropTypes.hpp"
#include "Editor/DragDrop/IDragDropHandler.hpp"
#include "Editor/DragDrop/DragDropRegistry.hpp"
#include "Editor/DragDrop/DragDropManager.hpp"
#include "Events/ApplicationEvent.h"
#include "Events/Event.h"

// Mock Handler for unit testing
class MockTestDropHandler : public IDragDropHandler
{
public:
    MockTestDropHandler(const TEString &pType, const TEString &ctx, int priority = 0)
        : m_PayloadType(pType), m_Context(ctx), m_Priority(priority)
    {
    }

    TEString GetSupportedPayloadType() const override { return m_PayloadType; }
    TEString GetSupportedTargetContext() const override { return m_Context; }
    int GetPriority() const override { return m_Priority; }

    bool CanHandle(const DragDropContext &context, const DragDropPayload &payload) const override
    {
        return m_AllowHandle;
    }

    bool OnDrop(const DragDropContext &context, const DragDropPayload &payload) override
    {
        m_DropHandled = true;
        m_LastReceivedString = payload.AsString();
        return true;
    }

    bool m_AllowHandle = true;
    bool m_DropHandled = false;
    TEString m_LastReceivedString;

private:
    TEString m_PayloadType;
    TEString m_Context;
    int m_Priority;
};

// -----------------------------------------------------------------------------
// Unit Tests
// -----------------------------------------------------------------------------

TE_TEST_CASE(DragDropManager, RegistryDynamicRegistration)
{
    DragDropRegistry::Clear();

    auto handlerA = CreateRef<MockTestDropHandler>("TEST_PAYLOAD_A", "TestViewport", 10);
    auto handlerB = CreateRef<MockTestDropHandler>("TEST_PAYLOAD_B", "TestInspector", 20);

    DragDropRegistry::RegisterHandler(handlerA);
    DragDropRegistry::RegisterHandler(handlerB);

    auto handlers = DragDropRegistry::GetHandlers();
    TE_CHECK_EQ(handlers.Size(), 2);
    // Highest priority should be first
    TE_CHECK_EQ(handlers[0]->GetPriority(), 20);
    TE_CHECK_EQ(handlers[1]->GetPriority(), 10);

    DragDropContext ddContextA;
    ddContextA.TargetContextId = "TestViewport";
    DragDropPayload ddPayloadA;
    ddPayloadA.PayloadType = "TEST_PAYLOAD_A";

    auto found = DragDropRegistry::FindHandler(ddContextA, ddPayloadA);
    TE_CHECK(found != nullptr);
    TE_CHECK_EQ(found->GetPriority(), 10);

    DragDropRegistry::Clear();
    TE_CHECK_EQ(DragDropRegistry::GetHandlers().Size(), 0);
}

TE_TEST_CASE(DragDropManager, RegistryPriorityOverride)
{
    DragDropRegistry::Clear();

    auto defaultHandler = CreateRef<MockTestDropHandler>(TE_DND_ASSET_PATH, "Viewport", 10);
    auto pluginOverride = CreateRef<MockTestDropHandler>(TE_DND_ASSET_PATH, "Viewport", 100);

    DragDropRegistry::RegisterHandler(defaultHandler);
    DragDropRegistry::RegisterHandler(pluginOverride);

    DragDropContext ddContext;
    ddContext.TargetContextId = "Viewport";
    DragDropPayload ddPayload;
    ddPayload.PayloadType = TE_DND_ASSET_PATH;

    auto found = DragDropRegistry::FindHandler(ddContext, ddPayload);
    TE_CHECK(found != nullptr);
    TE_CHECK_EQ(found->GetPriority(), 100);

    DragDropRegistry::Clear();
}

TE_TEST_CASE(DragDropManager, RegistryWildcardMatching)
{
    DragDropRegistry::Clear();

    auto universalHandler = CreateRef<MockTestDropHandler>("*", "*", 5);
    DragDropRegistry::RegisterHandler(universalHandler);

    DragDropContext ddContext;
    ddContext.TargetContextId = "AnyRandomPanel";
    DragDropPayload ddPayload;
    ddPayload.PayloadType = "CUSTOM_PAYLOAD";

    auto found = DragDropRegistry::FindHandler(ddContext, ddPayload);
    TE_CHECK(found != nullptr);
    TE_CHECK_EQ(found->GetPriority(), 5);

    DragDropRegistry::Clear();
}

TE_TEST_CASE(DragDropManager, SinglePathEncodingDecoding)
{
    TEArray<TEString> paths;
    paths.Add("Assets/Textures/My Sprite With Spaces.png");

    auto encoded = DragDropManager::EncodeStringArray(paths);
    TE_CHECK(!encoded.IsEmpty());

    auto decoded = DragDropManager::DecodeStringArray(encoded.GetData(), encoded.Size());
    TE_CHECK_EQ(decoded.Size(), 1);
    TE_CHECK_EQ(decoded[0], "Assets/Textures/My Sprite With Spaces.png");
}

TE_TEST_CASE(DragDropManager, BatchPathArrayEncodingDecoding)
{
    TEArray<TEString> paths;
    paths.Add("Assets/Scenes/Main.tescene");
    paths.Add("Assets/Textures/Player.png");
    paths.Add("Assets/Audio/BGM/Track_01.wav");
    paths.Add("Assets/Prefabs/Enemy_Boss_v2.teprefab");

    auto encoded = DragDropManager::EncodeStringArray(paths);
    TE_CHECK(!encoded.IsEmpty());

    auto decoded = DragDropManager::DecodeStringArray(encoded.GetData(), encoded.Size());
    TE_CHECK_EQ(decoded.Size(), 4);
    TE_CHECK_EQ(decoded[0], "Assets/Scenes/Main.tescene");
    TE_CHECK_EQ(decoded[1], "Assets/Textures/Player.png");
    TE_CHECK_EQ(decoded[2], "Assets/Audio/BGM/Track_01.wav");
    TE_CHECK_EQ(decoded[3], "Assets/Prefabs/Enemy_Boss_v2.teprefab");
}

TE_TEST_CASE(DragDropManager, BatchEntityArrayEncodingDecoding)
{
    TEArray<uint64_t> entities;
    entities.Add(1001);
    entities.Add(2048);
    entities.Add(999999);

    auto encoded = DragDropManager::EncodeEntityArray(entities);
    TE_CHECK(!encoded.IsEmpty());

    auto decoded = DragDropManager::DecodeEntityArray(encoded.GetData(), encoded.Size());
    TE_CHECK_EQ(decoded.Size(), 3);
    TE_CHECK_EQ(decoded[0], 1001);
    TE_CHECK_EQ(decoded[1], 2048);
    TE_CHECK_EQ(decoded[2], 999999);
}

TE_TEST_CASE(DragDropManager, EmptyAndNullPayloadHandling)
{
    auto decodedNull = DragDropManager::DecodeStringArray(nullptr, 0);
    TE_CHECK(decodedNull.IsEmpty());

    TEArray<uint8_t> garbageData;
    garbageData.Add(0x00);
    garbageData.Add(0x01);
    auto decodedGarbage = DragDropManager::DecodeStringArray(garbageData.GetData(), garbageData.Size());
    TE_CHECK(decodedGarbage.IsEmpty());

    auto decodedEntityNull = DragDropManager::DecodeEntityArray(nullptr, 0);
    TE_CHECK(decodedEntityNull.IsEmpty());
}

TE_TEST_CASE(DragDropManager, ExtensionValidationSingleAndMulti)
{
    TE_CHECK(DragDropManager::ValidateExtension("Player.png", ".png"));
    TE_CHECK(DragDropManager::ValidateExtension("Player.PNG", ".png"));
    TE_CHECK(!DragDropManager::ValidateExtension("Player.png", ".tescene"));

    TE_CHECK(DragDropManager::ValidateExtension("icon.jpg", ".png,.jpg,.jpeg,.tga"));
    TE_CHECK(DragDropManager::ValidateExtension("texture.TGA", ".png,.jpg,.jpeg,.tga"));
    TE_CHECK(!DragDropManager::ValidateExtension("sound.wav", ".png,.jpg,.jpeg,.tga"));

    TE_CHECK(DragDropManager::ValidateExtension("file.xyz", "*"));
    TE_CHECK(DragDropManager::ValidateExtension("file.xyz", ""));
}

TE_TEST_CASE(DragDropManager, WindowDropEventDispatching)
{
    TEArray<TEString> droppedPaths;
    droppedPaths.Add("C:/Users/User/Desktop/CoolTexture.png");
    droppedPaths.Add("C:/Users/User/Desktop/Sound.wav");

    WindowDropEvent dropEvent(droppedPaths, TEVector2(400.0f, 300.0f));
    TE_CHECK_EQ(dropEvent.GetPathCount(), 2);
    TE_CHECK_EQ(dropEvent.GetDropPos().x, 400.0f);
    TE_CHECK_EQ(dropEvent.GetDropPos().y, 300.0f);
    TE_CHECK(dropEvent.IsInCategory(EventCategoryApplication));
    TE_CHECK(!dropEvent.Handled());

    bool handled = false;
    EventDispatcher dispatcher(dropEvent);
    dispatcher.Dispatch<WindowDropEvent>(
        [&](WindowDropEvent &e) -> bool
        {
            handled = true;
            TE_CHECK_EQ(e.GetPathCount(), 2);
            TE_CHECK_EQ(e.GetPaths()[0], "C:/Users/User/Desktop/CoolTexture.png");
            return true;
        });

    TE_CHECK(handled);
    TE_CHECK(dropEvent.Handled());
}

TE_TEST_CASE(DragDropManager, FolderMoveSelfDropValidation)
{
    TEString folderA = "Assets/Textures";
    TEString subFolder = "Assets/Textures/Icons";
    TEString parentFolder = "Assets";

    // Cannot drop folder into itself
    TE_CHECK(subFolder.StartsWith(folderA));
    // Cannot drop into same parent
    TE_CHECK_EQ(subFolder.GetParentPath(), folderA);
}

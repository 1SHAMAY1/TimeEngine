#include "PreRequisites.h"
#include "Editor/DragDrop/DragDropManager.hpp"
#include "Editor/DragDrop/DragDropRegistry.hpp"
#include "Layers/EditorLayer.hpp"
#include "Scene.hpp"
#include "EntityManager.hpp"
#include "TransformComponent.hpp"
#include "Log.h"


// -----------------------------------------------------------------------------
// DragDropPayload Member Implementations
// -----------------------------------------------------------------------------
TEArray<TEString> DragDropPayload::AsPathArray() const
{
    if (!Data || DataSize == 0)
        return {};

    if (PayloadType == TE_DND_CONTENT_BROWSER_ITEMS || PayloadType == TE_DND_OS_FILES)
    {
        return DragDropManager::DecodeStringArray(Data, DataSize);
    }
    else if (PayloadType == TE_DND_ASSET_PATH)
    {
        TEArray<TEString> arr;
        arr.Add(AsString());
        return arr;
    }
    return {};
}

uint64_t DragDropPayload::AsEntityID() const
{
    if (!Data || DataSize == 0)
        return 0;

    if (DataSize == sizeof(uint64_t))
    {
        uint64_t id = 0;
        memcpy(&id, Data, sizeof(uint64_t));
        return id;
    }

    TEString str = AsString();
    return (uint64_t)strtoull(str.c_str(), nullptr, 10);
}

TEArray<uint64_t> DragDropPayload::AsEntityIDArray() const
{
    if (!Data || DataSize == 0)
        return {};

    if (PayloadType == TE_DND_ENTITY_LIST)
    {
        return DragDropManager::DecodeEntityArray(Data, DataSize);
    }
    else if (PayloadType == TE_DND_ENTITY_ID)
    {
        TEArray<uint64_t> arr;
        arr.Add(AsEntityID());
        return arr;
    }
    return {};
}

// -----------------------------------------------------------------------------
// DragDropManager Sources
// -----------------------------------------------------------------------------
bool DragDropManager::BeginSource(const TEString &type, const void *data, size_t size, const TEString &tooltip,
                                  int flags)
{
    if (TimeGUI::BeginDragDropSource(flags))
    {
        TimeGUI::SetDragDropPayload(type, data, size);

        if (!tooltip.empty())
        {
            TimeGUI::Text(tooltip);
        }

        TimeGUI::EndDragDropSource();
        return true;
    }
    return false;
}

bool DragDropManager::BeginSourceString(const TEString &type, const TEString &str, const TEString &tooltip,
                                        int flags)
{
    return BeginSource(type, str.c_str(), str.Length() + 1, tooltip.empty() ? str : tooltip, flags);
}

bool DragDropManager::BeginSourcePathArray(const TEArray<TEString> &paths, const TEString &tooltip, int flags)
{
    if (paths.IsEmpty())
        return false;

    if (paths.Size() == 1)
    {
        TEString t = tooltip.empty() ? paths[0] : tooltip;
        return BeginSourceString(TE_DND_ASSET_PATH, paths[0], t, flags);
    }

    auto encoded = EncodeStringArray(paths);
    TEString t = tooltip.empty() ? (TEString::FromInt((int)paths.Size()) + " items") : tooltip;
    return BeginSource(TE_DND_CONTENT_BROWSER_ITEMS, encoded.GetData(), encoded.Size(), t, flags);
}

bool DragDropManager::BeginSourceEntity(uint64_t entityId, const TEString &entityName, int flags)
{
    TEString tooltip = entityName.empty() ? ("Entity " + TEString::FromInt((int)entityId)) : entityName;
    return BeginSource(TE_DND_ENTITY_ID, &entityId, sizeof(uint64_t), tooltip, flags);
}

bool DragDropManager::BeginSourceEntityArray(const TEArray<uint64_t> &entityIds, const TEString &tooltip,
                                            int flags)
{
    if (entityIds.IsEmpty())
        return false;

    if (entityIds.Size() == 1)
    {
        return BeginSourceEntity(entityIds[0], tooltip, flags);
    }

    auto encoded = EncodeEntityArray(entityIds);
    TEString t = tooltip.empty() ? (TEString::FromInt((int)entityIds.Size()) + " Entities") : tooltip;
    return BeginSource(TE_DND_ENTITY_LIST, encoded.GetData(), encoded.Size(), t, flags);
}

// -----------------------------------------------------------------------------
// DragDropManager Targets
// -----------------------------------------------------------------------------
bool DragDropManager::ExecuteTarget(const TEString &targetContextId, void *userData, Ref<EditorLayer> editor,
                                    const TEString &allowedPayloadType)
{
    if (TimeGUI::BeginDragDropTarget())
    {
        const TimeGUIPayload *guiPayload = TimeGUI::GetDragDropPayload();
        if (guiPayload && guiPayload->Data)
        {
            if (allowedPayloadType.empty() || guiPayload->IsDataType(allowedPayloadType))
            {
                DragDropContext context;
                context.TargetContextId = targetContextId;
                context.ScreenPosition = TimeGUI::GetMousePos();
                context.LocalTargetPosition = context.ScreenPosition - TimeGUI::GetItemRectMin();
                context.TargetUserData = userData;
                context.Editor = editor;
                if (editor)
                    context.ActiveScene = editor->GetActiveScene();

                DragDropPayload payload;
                payload.PayloadType = guiPayload->DataType;
                payload.Data = guiPayload->Data;
                payload.DataSize = guiPayload->DataSize;
                payload.IsPreview = guiPayload->IsPreview;
                payload.IsDelivery = guiPayload->IsDelivery;

                auto handler = DragDropRegistry::FindHandler(context, payload);
                if (handler)
                {
                    // Render hover feedback or tooltip override
                    handler->OnHover(context, payload);
                    TEString tip = handler->GetTooltipText(context, payload);
                    if (!tip.empty())
                    {
                        TimeGUI::SetTooltip(tip);
                    }

                    // Accept payload when user releases mouse (delivery)
                    const TimeGUIPayload *accepted = TimeGUI::AcceptDragDropPayload(guiPayload->DataType);
                    if (accepted && accepted->Data)
                    {
                        payload.Data = accepted->Data;
                        payload.DataSize = accepted->DataSize;
                        payload.IsDelivery = true;
                        TE_CORE_INFO("DragDropManager: Delivered payload '{0}' (Size: {1} B) to target '{2}' (Supported Type: '{3}')",
                                     payload.PayloadType, payload.DataSize, targetContextId, handler->GetSupportedPayloadType());
                        bool handled = handler->OnDrop(context, payload);
                        TimeGUI::EndDragDropTarget();
                        return handled;
                    }
                }
            }
        }
        TimeGUI::EndDragDropTarget();
    }
    return false;
}

// -----------------------------------------------------------------------------
// UI Feedback Helpers
// -----------------------------------------------------------------------------
void DragDropManager::DrawDropTargetHighlight(const TEVector2 &min, const TEVector2 &max, bool isValid,
                                              float rounding)
{
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    unsigned int fillColor = isValid ? 0x254A88E8 : 0x25E84A4A;
    unsigned int borderColor = isValid ? 0xFF4A88E8 : 0xFFE84A4A;

    dl.AddRectFilled(min, max, fillColor, rounding);
    dl.AddRect(min, max, borderColor, rounding, 0, 2.0f);
}

// -----------------------------------------------------------------------------
// Utility & Codecs
// -----------------------------------------------------------------------------
TEArray<uint8_t> DragDropManager::EncodeStringArray(const TEArray<TEString> &strings)
{
    TEArray<uint8_t> buffer;
    uint32_t count = (uint32_t)strings.Size();

    size_t totalBytes = sizeof(uint32_t);
    for (size_t i = 0; i < strings.Size(); ++i)
    {
        totalBytes += sizeof(uint32_t) + strings[i].Length();
    }

    buffer.Resize(totalBytes);
    size_t offset = 0;

    memcpy(buffer.GetData() + offset, &count, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for (size_t i = 0; i < strings.Size(); ++i)
    {
        uint32_t len = (uint32_t)strings[i].Length();
        memcpy(buffer.GetData() + offset, &len, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (len > 0)
        {
            memcpy(buffer.GetData() + offset, strings[i].c_str(), len);
            offset += len;
        }
    }

    return buffer;
}

TEArray<TEString> DragDropManager::DecodeStringArray(const void *data, size_t size)
{
    if (!data || size < sizeof(uint32_t))
        return {};

    size_t offset = 0;
    uint32_t count = 0;
    memcpy(&count, (const char *)data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    TEArray<TEString> result;
    result.Reserve(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        if (offset + sizeof(uint32_t) > size)
            break;

        uint32_t len = 0;
        memcpy(&len, (const char *)data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (offset + len > size)
            break;

        TEString s((const char *)data + offset, len);
        offset += len;
        result.Add(s);
    }

    return result;
}

TEArray<uint8_t> DragDropManager::EncodeEntityArray(const TEArray<uint64_t> &entities)
{
    TEArray<uint8_t> buffer;
    uint32_t count = (uint32_t)entities.Size();

    size_t totalBytes = sizeof(uint32_t) + count * sizeof(uint64_t);
    buffer.Resize(totalBytes);

    size_t offset = 0;
    memcpy(buffer.GetData() + offset, &count, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (count > 0)
    {
        memcpy(buffer.GetData() + offset, entities.GetData(), count * sizeof(uint64_t));
    }

    return buffer;
}

TEArray<uint64_t> DragDropManager::DecodeEntityArray(const void *data, size_t size)
{
    if (!data || size < sizeof(uint32_t))
        return {};

    size_t offset = 0;
    uint32_t count = 0;
    memcpy(&count, (const char *)data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    TEArray<uint64_t> result;
    result.Reserve(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        if (offset + sizeof(uint64_t) > size)
            break;

        uint64_t id = 0;
        memcpy(&id, (const char *)data + offset, sizeof(uint64_t));
        offset += sizeof(uint64_t);
        result.Add(id);
    }

    return result;
}


bool DragDropManager::ValidateExtension(const TEString &path, const TEString &expectedExtensions)
{
    if (expectedExtensions.empty() || expectedExtensions == "*")
        return true;

    TEString lowerPath = path.ToLower();

    // Comma-separated extension list (e.g. ".png,.jpg,.jpeg,.tga")
    TEArray<TEString> exts = expectedExtensions.ToLower().Split(',');
    for (size_t i = 0; i < exts.Size(); ++i)
    {
        TEString ext = exts[i].Trim();
        if (ext.empty() || ext == "*")
            return true;

        if (lowerPath.EndsWith(ext))
            return true;
    }

    return false;
}

bool DragDropManager::IsDescendantOf(Ref<Scene> scene, uint64_t candidateChildId, uint64_t candidateParentId)
{
    if (!scene || candidateChildId == 0 || candidateParentId == 0)
        return false;

    if (candidateChildId == candidateParentId)
        return true;

    EntityManager &em = scene->GetEntityManager();
    EntityID currentID = candidateParentId;
    while (currentID != 0 && em.IsValid(currentID))
    {
        Entity current(currentID, &em);
        auto *tc = current.GetComponent<TransformComponent>();
        if (!tc || tc->Parent == 0)
            break;

        if (tc->Parent == candidateChildId)
            return true;

        currentID = tc->Parent;
    }

    return false;
}


#include "PreRequisites.h"
#include "Editor/DragDrop/DragDropRegistry.hpp"
#include "Log.h"

TEArray<Ref<IDragDropHandler>> &DragDropRegistry::GetHandlersList()
{
    static TEArray<Ref<IDragDropHandler>> s_Handlers;
    return s_Handlers;
}

void DragDropRegistry::RegisterHandler(Ref<IDragDropHandler> handler)
{
    if (!handler)
        return;

    auto &handlers = GetHandlersList();
    for (size_t i = 0; i < handlers.Size(); ++i)
    {
        if (handlers[i] == handler)
            return;
    }

    // Insert sorted by Priority descending
    int p = handler->GetPriority();
    size_t insertIdx = handlers.Size();
    for (size_t i = 0; i < handlers.Size(); ++i)
    {
        if (handlers[i]->GetPriority() < p)
        {
            insertIdx = i;
            break;
        }
    }

    if (insertIdx == handlers.Size())
        handlers.Add(handler);
    else
        handlers.Insert(insertIdx, handler);

    TE_CORE_DEBUG("DragDropRegistry: Registered handler for Context '{0}', Payload '{1}', Priority {2}",
                  handler->GetSupportedTargetContext(), handler->GetSupportedPayloadType(), handler->GetPriority());

}

void DragDropRegistry::UnregisterHandler(Ref<IDragDropHandler> handler)
{
    if (!handler)
        return;

    auto &handlers = GetHandlersList();
    for (size_t i = 0; i < handlers.Size(); ++i)
    {
        if (handlers[i] == handler)
        {
            handlers.RemoveAt(i);
            break;
        }
    }
}

TEArray<Ref<IDragDropHandler>> DragDropRegistry::GetHandlers()
{
    return GetHandlersList();
}

Ref<IDragDropHandler> DragDropRegistry::FindHandler(const DragDropContext &context, const DragDropPayload &payload)
{
    auto &handlers = GetHandlersList();
    for (size_t i = 0; i < handlers.Size(); ++i)
    {
        auto &handler = handlers[i];
        if (!handler)
            continue;

        // Check context match (exact or wildcard)
        TEString ctx = handler->GetSupportedTargetContext();
        if (ctx != "*" && ctx != context.TargetContextId)
            continue;

        // Check payload type match (exact or wildcard)
        TEString pType = handler->GetSupportedPayloadType();
        if (pType != "*" && pType != payload.PayloadType)
            continue;

        // Check handler custom validation
        if (handler->CanHandle(context, payload))
            return handler;
    }
    return nullptr;
}

bool DragDropRegistry::HandleDrop(const DragDropContext &context, const DragDropPayload &payload)
{
    auto handler = FindHandler(context, payload);
    if (handler)
    {
        TE_CORE_INFO("DragDropRegistry: Executing Drop on Context '{0}' with Handler (Priority {1})",
                     context.TargetContextId, handler->GetPriority());
        return handler->OnDrop(context, payload);
    }
    return false;
}

bool DragDropRegistry::HandleHover(const DragDropContext &context, const DragDropPayload &payload)
{
    auto handler = FindHandler(context, payload);
    if (handler)
    {
        handler->OnHover(context, payload);
        return true;
    }
    return false;
}

void DragDropRegistry::Clear()
{
    GetHandlersList().Clear();
}

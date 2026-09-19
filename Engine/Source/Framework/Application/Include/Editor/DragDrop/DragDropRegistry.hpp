#pragma once

#include "PreRequisites.h"
#include "Editor/DragDrop/IDragDropHandler.hpp"

class TE_API DragDropRegistry
{
public:
    static void RegisterHandler(Ref<IDragDropHandler> handler);
    static void UnregisterHandler(Ref<IDragDropHandler> handler);
    static TEArray<Ref<IDragDropHandler>> GetHandlers();
    static Ref<IDragDropHandler> FindHandler(const DragDropContext &context, const DragDropPayload &payload);
    static bool HandleDrop(const DragDropContext &context, const DragDropPayload &payload);
    static bool HandleHover(const DragDropContext &context, const DragDropPayload &payload);
    static void Clear();

private:
    static TEArray<Ref<IDragDropHandler>> &GetHandlersList();
};

template <typename T> struct DragDropHandlerAutoRegister
{
    DragDropHandlerAutoRegister() { DragDropRegistry::RegisterHandler(CreateRef<T>()); }
};

#define TE_REGISTER_DRAG_DROP_HANDLER(HandlerClass)                                                                    \
    inline DragDropHandlerAutoRegister<HandlerClass> s_AutoRegister_##HandlerClass;

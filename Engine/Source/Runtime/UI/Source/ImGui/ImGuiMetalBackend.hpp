#pragma once
#include "PreRequisites.h"

#ifdef TE_SUPPORT_METAL

#ifdef __cplusplus
extern "C"
{
#endif

    bool ImGui_InitMetalBackend(void *nativeWindow);
    void ImGui_ShutdownMetalBackend();
    void ImGui_PrepareMetalFrame();
    void ImGui_RenderMetalDrawData(void *drawData);

#ifdef __cplusplus
}
#endif

#endif

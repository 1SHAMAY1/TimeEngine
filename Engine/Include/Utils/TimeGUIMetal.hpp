#pragma once
#include "Core/PreRequisites.h"

#ifdef TE_SUPPORT_METAL

#ifdef __cplusplus
extern "C"
{
#endif

    bool TimeGUI_InitMetalBackend(void *nativeWindow);
    void TimeGUI_ShutdownMetalBackend();
    void TimeGUI_PrepareMetalFrame();
    void TimeGUI_RenderMetalDrawData(void *drawData);

#ifdef __cplusplus
}
#endif

#endif

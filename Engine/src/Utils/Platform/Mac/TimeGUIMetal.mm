#include "Core/PreRequisites.h"
#include "Utils/TimeGUIMetal.hpp"
#include "Core/Log.h"

#ifdef TE_SUPPORT_METAL

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "backends/imgui_impl_metal.h"

static id<MTLDevice> s_MetalDevice = nil;
static id<MTLCommandQueue> s_CommandQueue = nil;
static CAMetalLayer *s_MetalLayer = nil;
static GLFWwindow *s_Window = nullptr;
static id<MTLTexture> s_DummyTexture = nil;

extern "C" {

bool TimeGUI_InitMetalBackend(void *nativeWindow)
{
    @autoreleasepool {
        s_MetalDevice = MTLCreateSystemDefaultDevice();
        if (!s_MetalDevice)
        {
            TE_CORE_ERROR("[TimeGUI] Failed to create default Metal device.");
            return false;
        }

        s_CommandQueue = [s_MetalDevice newCommandQueue];

        // Create 1x1 dummy texture to supply pipeline format to ImGui NewFrame without holding a live drawable
        MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                          width:1
                                                                                         height:1
                                                                                      mipmapped:NO];
        texDesc.usage = MTLTextureUsageRenderTarget;
        s_DummyTexture = [s_MetalDevice newTextureWithDescriptor:texDesc];

        s_Window = static_cast<GLFWwindow *>(nativeWindow);
        if (s_Window)
        {
            NSWindow *nswin = glfwGetCocoaWindow(s_Window);
            if (nswin && nswin.contentView)
            {
                s_MetalLayer = [CAMetalLayer layer];
                s_MetalLayer.device = s_MetalDevice;
                s_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
                int fbW = 0, fbH = 0;
                glfwGetFramebufferSize(s_Window, &fbW, &fbH);
                if (fbW > 0 && fbH > 0)
                    s_MetalLayer.drawableSize = CGSizeMake(fbW, fbH);
                nswin.contentView.layer = s_MetalLayer;
                nswin.contentView.wantsLayer = YES;
            }
        }

        if (!ImGui_ImplMetal_Init(s_MetalDevice))
        {
            TE_CORE_ERROR("[TimeGUI] ImGui_ImplMetal_Init failed.");
            return false;
        }

        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->Build();

        TE_CORE_INFO("[TimeGUI] Metal backend initialized successfully.");
        return true;
    }
}

void TimeGUI_ShutdownMetalBackend()
{
    @autoreleasepool {
        ImGui_ImplMetal_Shutdown();
        s_DummyTexture = nil;
        s_CommandQueue = nil;
        s_MetalDevice = nil;
        s_MetalLayer = nil;
        s_Window = nullptr;
    }
}

void TimeGUI_PrepareMetalFrame()
{
    @autoreleasepool {
        MTLRenderPassDescriptor *renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDesc.colorAttachments[0].texture = s_DummyTexture;
        renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
        renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.1, 0.1, 0.1, 1.0);
        renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

        ImGui_ImplMetal_NewFrame(renderPassDesc);
    }
}

void TimeGUI_RenderMetalDrawData(void *drawData)
{
    if (!drawData || !s_CommandQueue || !s_MetalLayer)
        return;

    @autoreleasepool {
        ImDrawData *data = static_cast<ImDrawData *>(drawData);
        if (data->DisplaySize.x <= 0.0f || data->DisplaySize.y <= 0.0f)
            return;

        if (s_Window)
        {
            int fbW = 0, fbH = 0;
            glfwGetFramebufferSize(s_Window, &fbW, &fbH);
            if (fbW > 0 && fbH > 0)
            {
                CGSize currentSize = s_MetalLayer.drawableSize;
                if (currentSize.width != fbW || currentSize.height != fbH)
                {
                    s_MetalLayer.drawableSize = CGSizeMake(fbW, fbH);
                }
            }
        }

        id<CAMetalDrawable> drawable = [s_MetalLayer nextDrawable];
        if (!drawable)
            return;

        MTLRenderPassDescriptor *renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDesc.colorAttachments[0].texture = drawable.texture;
        renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
        renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.1, 0.1, 0.1, 1.0);
        renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLCommandBuffer> commandBuffer = [s_CommandQueue commandBuffer];
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];

        ImGui_ImplMetal_RenderDrawData(data, commandBuffer, renderEncoder);

        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];
    }
}

}

#endif

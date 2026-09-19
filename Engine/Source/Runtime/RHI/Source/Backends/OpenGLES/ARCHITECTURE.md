# OpenGL ES Renderer Backend Architecture

The OpenGL ES (GLES) graphics backend in TimeEngine provides embedded and mobile graphics pipeline support (`OpenGLESRendererAPI`), GLAD function pointer loading (`gladLoadGLLoader`), GLES shader compilation ([`OpenGLESShader`](OpenGLESShader.cpp)), Framebuffers ([`OpenGLESFramebuffer`](OpenGLESFramebuffer.cpp)), and embedded buffer resources (`OpenGLESVertexArray`, `OpenGLESVertexBuffer`, `OpenGLESIndexBuffer`).

> [!NOTE]
> In short, think of the **OpenGL ES Backend** as TimeEngine's lightweight mobile graphics driver: `OpenGLESRendererAPI` mirrors desktop OpenGL commands (`glViewport`, `glClear`, `glDrawElements`, `glBlendFunc`), tailored specifically for embedded hardware platforms (Android, iOS, WebGL) with mobile shader precision specs (`precision mediump float`).

---

## Architecture & Component Breakdown

1. **[`TE::OpenGLESRendererAPI`](OpenGLESRendererAPI.cpp)**:
   - Derives from `RendererAPI`.
   - Manages mobile viewport bounds, clearing, and alpha blending modes (Normal, Additive, Multiplicative).
2. **[`TE::OpenGLESShader`](OpenGLESShader.cpp)**:
   - Compiles ES 3.0 GLSL shader strings with embedded precision specifiers (`#version 300 es`).
3. **Resource Objects**: `OpenGLESVertexArray`, `OpenGLESVertexBuffer`, `OpenGLESIndexBuffer`, `OpenGLESFramebuffer`.

---

## Related Architectural Documentation

- [Renderer Subsystem Architecture](../ARCHITECTURE.md) — Multi-backend 2D batching pipeline.
- [OpenGL Graphics Backend Architecture](../OpenGL/ARCHITECTURE.md) — Desktop OpenGL backend implementation.


# TextureUpscalerPlugin Architecture

The `TextureUpscalerPlugin` provides deep-learning-based 2D asset super-resolution (ESRGAN / Real-ESRGAN / SRGAN) inside **TimeEditor** with cross-platform hardware acceleration.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph EditorInspector ["TimeEditor Asset Inspector"]
        Asset["Texture2D Asset (.png / .jpg)"]
        Button["Inspector Button: '↑ 4x AI Upscale'"]
        Asset --> Button
    end

    subgraph PlatformBackends ["TextureUpscalerPlugin Hardware Backends"]
        DirectML["Windows: DirectML / ONNX Runtime (GPU Accelerated)"]
        CoreML["macOS: Apple CoreML / Metal Performance Shaders (Apple Silicon NPU)"]
        VulkanCompute["Linux: Vulkan Compute Shader / OpenVINO"]
    end

    subgraph TilingPipeline ["Tiled Image Inference Pipeline"]
        Splitter["Tile Splitter (512x512 with 32px Overlap)"]
        Inference["Deep Neural Forward Pass"]
        Blender["Seam Blender & PNG Exporter"]
        
        Splitter --> Inference --> Blender
    end

    Button --> DirectML & CoreML & VulkanCompute
    DirectML & CoreML & VulkanCompute --> Splitter
    Blender --> OutAsset["New Upscaled Asset: <TextureName>_4x.png"]
```

---

## 🛠️ Contributor Implementation Guide

- Implement `TextureUpscalerBackend` dispatchers for Windows (DirectML), macOS (Metal/CoreML), and Linux (Vulkan).
- Implement tiled inference with overlap blending to maintain low VRAM footprint ($< 500\text{ MB}$).
- Connect `#if defined(TE_HAS_PLUGIN_SPRITEEDITORPLUGIN)` to add upscale actions to Sprite Studio.

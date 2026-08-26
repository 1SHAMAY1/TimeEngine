---
trigger: always_on
---

# TimeEngine Development Priorities

1. **Leverage Existing Systems & Engine Types**:
   - Use established engine classes and types (`TEString`, `TEArray<T>`, `TEOption<T>`, `TEResult<T, E>`, `TEVector2`, `TEVector`, `TEVector4`, `TEMatrix4`, `Texture2D`, etc.) instead of raw `std::string`, raw STL containers, raw GLM math, or raw OpenGL/DirectX/Vulkan calls.
   - **No Raw Owning Pointers**: Always use smart pointer aliases (`TE::Scope<T>` / `CreateScope`, `TE::Ref<T>` / `CreateRef`, `TE::WeakRef<T>`) instead of managing memory with raw `new`/`delete` or unmanaged pointers.
2. **Local Workflow & Documentation Compliance**: Always follow local documentation and rules: `llms.md`, `llms.txt`, all `ARCHITECTURE.md` files across the codebase, and `.agentsrules`. Avoid using Chrome; perform searches and lookups locally.
3. **Build & Test Validation**: For all build, test, and generation steps, strictly use project scripts only (e.g., scripts under `Scripts/Windows/MSVC/` such as `BuildDebug.bat`). Do not run direct MSBuild.exe commands.
4. **Offline-First Research**: Check local headers and project documentation before searching external resources.
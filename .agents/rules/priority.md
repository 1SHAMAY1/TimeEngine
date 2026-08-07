---
trigger: always_on
---

# TimeEngine Development Priorities

1. **Leverage Existing Systems**: Use established classes and types (`Texture2D`, `Ref<T>`, etc.) instead of re-implementing or using raw OpenGL.
2. **Local Workflow & Documentation Compliance**: Always follow local documentation and rules: `llms.md`, `llms.txt`, all `ARCHITECTURE.md` files across the codebase, and `.agentsrules`. Avoid using Chrome; perform searches and lookups locally.
3. **Build & Test Validation**: For all build, test, and generation steps, strictly use project scripts only (e.g., scripts under `Scripts/Windows/MSVC/` such as `BuildDebug.bat`). Do not run direct MSBuild.exe commands.
4. **Offline-First Research**: Check local headers and project documentation before searching external resources.
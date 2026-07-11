---
trigger: always_on
---

# TimeEngine Development Priorities

1. **Leverage Existing Systems**: Use established classes and types (`Texture2D`, `Ref<T>`, etc.) instead of re-implementing or using raw OpenGL.
2. **Local Workflow**: Avoid using Chrome. Perform all searches and documentation lookups via the local environment or backend tools.
3. **Build Validation**: Always verify changes by building the project:
   `D:\Installed\VisualStudio\VisualStudioIDE\MSBuild\Current\Bin\amd64\MSBuild.exe Engine\Engine.vcxproj /p:Configuration=Debug /p:Platform=x64`
4. **Offline-First Research**: Check local headers and project documentation (like `llms.txt`) before searching the web.
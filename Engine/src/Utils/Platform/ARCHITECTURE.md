# Platform Utilities Architecture

The `Platform` module under `Utils/Platform/` contains platform-specific implementations of the [`TE::PlatformUtils`](file:///e:/TimeEngine/Engine/Include/Utils/PlatformUtils.hpp) interface.

## Platform Implementations

- [Windows Platform Architecture](file:///e:/TimeEngine/Engine/src/Utils/Platform/Windows/ARCHITECTURE.md) — Windows OS implementation using COM shell dialogs (`IFileDialog`), `GetOpenFileNameA`, Windows Registry (`RegCreateKeyExA`), and `GetModuleFileNameA`.
- [Unix Platform Architecture](file:///e:/TimeEngine/Engine/src/Utils/Platform/Unix/ARCHITECTURE.md) — POSIX / Linux implementation details.

# Platform Utilities Architecture

The `Platform` module under `Utils/Platform/` contains platform-specific implementations of the `TE::PlatformUtils` (`Engine/Include/Utils/PlatformUtils.hpp`) interface.

## Platform Implementations

- [Windows Platform Architecture](Windows/ARCHITECTURE.md) — Windows OS implementation using COM shell dialogs (`IFileDialog`), `GetOpenFileNameA`, Windows Registry (`RegCreateKeyExA`), and `GetModuleFileNameA`.
- [Unix Platform Architecture](Unix/ARCHITECTURE.md) — POSIX / Linux implementation details.


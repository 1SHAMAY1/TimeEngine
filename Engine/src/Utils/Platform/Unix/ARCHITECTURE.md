# Unix Platform Utilities Architecture

`UnixPlatformUtils.cpp` provides POSIX and Unix platform implementations of the [`TE::PlatformUtils`](file:///e:/TimeEngine/Engine/Include/Utils/PlatformUtils.hpp) interface.

> [!NOTE]
> In short, this module acts as the Unix-specific diplomat for TimeEngine: it talks directly to Unix/macOS operating systems to resolve executable paths via `/proc/self/exe`, open system dialogs via native desktop utilities, and register file types with Unix MIME databases.

---

## Implementation Details

On Unix operating systems, native file pickers can be spawned using desktop environment dialog utilities like `zenity` or `kdialog`, and binary paths are resolved via `/proc/self/exe`.

### 1. Executable Path Resolution (`GetExecutablePath`)
```cpp
std::string PlatformUtils::GetExecutablePath()
{
    char buffer[1024];
    ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        return std::string(buffer);
    }
    return "";
}
```

### 2. Dialog & File Association Stubs
File associations (`RegisterFileAssociation`) on Unix targets interface with MIME type databases (`xdg-mime` / `.desktop` files).

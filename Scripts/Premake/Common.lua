-- ========== Common Toolset, Include Directories & Workspace Configuration ==========

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

filter { "system:windows", "action:vs*" }
    local customToolset = os.getenv("PlatformToolset")
    if customToolset and customToolset ~= "" then
        toolset(customToolset)
    end

    local numProcs = tonumber(os.getenv("NUMBER_OF_PROCESSORS")) or 4
    local safeProcs = math.min(4, math.max(1, math.floor(numProcs / 4)))
    if customToolset == "ClangCL" then
        buildoptions { "/FC", "-gcodeview", "-gcodeview-ghash", "-Wno-unused-command-line-argument" }
        disablewarnings { "4251", "4275" }
    else
        buildoptions { "/MP" .. safeProcs, "/FC", "/we26409", "/we26400", "/we26401", "/bigobj", "/FS" }
        disablewarnings { "4251", "4275" }
    end
filter {}

filter { "system:windows", "action:vs*", "configurations:Debug", "kind:not StaticLib" }
    local customToolset = os.getenv("PlatformToolset")
    if customToolset == "ClangCL" then
        linkoptions { "/DEBUG:GHASH" }
    else
        linkoptions { "/DEBUG:FASTLINK" }
    end
filter {}

filter { "action:gmake*" }
    buildoptions { "-pipe" }
filter {}

filter { "system:not windows" }
    pic "On"
filter {}

filter { "system:linux", "action:gmake*" }
    -- Prefer mold (ultra-fast multi-threaded linker), fallback gracefully if not installed
    local hasMold = (os.execute("command -v mold >/dev/null 2>&1") == 0)
    local hasLld = (os.execute("command -v ld.lld >/dev/null 2>&1") == 0)
    if hasMold then
        linkoptions { "-fuse-ld=mold" }
    elseif hasLld then
        linkoptions { "-fuse-ld=lld" }
    end
filter {}

filter { "system:macosx", "action:xcode*" }
    cppdialect "C++20"
    xcodebuildsettings {
        ["ONLY_ACTIVE_ARCH"] = "YES",
        ["CLANG_CXX_LANGUAGE_STANDARD"] = "c++20"
    }
filter {}

filter { "system:macosx", "action:xcode*", "configurations:Debug" }
    xcodebuildsettings {
        ["DEBUG_INFORMATION_FORMAT"] = "dwarf"
    }
filter {}

-- ========== Include Directories Map ==========

IncludeDir = {}
IncludeDir["Engine_Source"]       = "%{wks.location}/Engine/Source"
IncludeDir["Engine_Runtime"]      = "%{wks.location}/Engine/Source/Runtime"
IncludeDir["Engine_Framework"]    = "%{wks.location}/Engine/Source/Framework"

IncludeDir["Engine_Core"]         = "%{wks.location}/Engine/Source/Runtime/Core/Include"
IncludeDir["Engine_Math"]         = "%{wks.location}/Engine/Source/Runtime/Math/Include"
IncludeDir["Engine_RHI"]          = "%{wks.location}/Engine/Source/Runtime/RHI/Include"
IncludeDir["Engine_Renderer2D"]   = "%{wks.location}/Engine/Source/Runtime/Renderer2D/Include"
IncludeDir["Engine_Physics"]      = "%{wks.location}/Engine/Source/Runtime/Physics/Include"
IncludeDir["Engine_Scripting"]    = "%{wks.location}/Engine/Source/Runtime/Scripting/Include"
IncludeDir["Engine_Scene"]        = "%{wks.location}/Engine/Source/Runtime/Scene/Include"
IncludeDir["Engine_Testing"]      = "%{wks.location}/Engine/Source/Runtime/Testing/Include"
IncludeDir["Engine_UI"]           = "%{wks.location}/Engine/Source/Runtime/UI/Include"
IncludeDir["Engine_Window"]       = "%{wks.location}/Engine/Source/Runtime/Window/Include"

IncludeDir["Engine_Asset"]        = "%{wks.location}/Engine/Source/Framework/Asset/Include"
IncludeDir["Engine_Input"]        = "%{wks.location}/Engine/Source/Framework/Input/Include"
IncludeDir["Engine_Gameplay"]     = "%{wks.location}/Engine/Source/Framework/Gameplay/Include"
IncludeDir["Engine_App"]          = "%{wks.location}/Engine/Source/Framework/Application/Include"


IncludeDir["ThirdParty_Logger"]   = "%{wks.location}/ThirdParty/Customizable_Logger/Include"
IncludeDir["ThirdParty_GLFW"]     = "%{wks.location}/ThirdParty/GLFW/glfw/include"
IncludeDir["ThirdParty_GLAD"]     = "%{wks.location}/ThirdParty/GLAD/include"
IncludeDir["ThirdParty_ImGui"]    = "%{wks.location}/ThirdParty/IMGUI/ImGui"
IncludeDir["ThirdParty_GLM"]      = "%{wks.location}/ThirdParty/GLM"
IncludeDir["ThirdParty_stb"]      = "%{wks.location}/ThirdParty/stb_image"
IncludeDir["ThirdParty_Velox"]    = "%{wks.location}/ThirdParty/Velox/include"
IncludeDir["ThirdParty_Vulkan"]   = "%{wks.location}/ThirdParty/Vulkan/include"
IncludeDir["ThirdParty_volk"]     = "%{wks.location}/ThirdParty/volk"
IncludeDir["ThirdParty_OpenGLES"] = "%{wks.location}/ThirdParty/OpenGL-Registry/api"
IncludeDir["ThirdParty_miniaudio"]= "%{wks.location}/ThirdParty/miniaudio"
IncludeDir["ThirdParty_ForgeUI"]  = "%{wks.location}/ThirdParty/ForgeUI/include"

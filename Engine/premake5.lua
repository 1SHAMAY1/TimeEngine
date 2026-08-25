-- ========== Engine Project ==========

project "Engine"
    location "."
    kind "SharedLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "off"

    targetdir ("%{wks.location}/Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    pchheader "Core/PreRequisites.h"
    pchsource "src/Core/PreRequisites.cpp"

    filter "system:macosx"
        xcodebuildsettings {
            ["LD_DYLIB_INSTALL_NAME"] = "@rpath/libEngine.dylib"
        }
        files { "src/**.mm" }
    filter {}

    files {
        -- Core Engine
        "src/**.h",
        "src/**.cpp",
        "Include/**.h",
        "Include/**.hpp",

        -- Editor & Profiling Layers
        "Include/Layers/**.hpp",
        "src/Core/Layers/**.cpp",
        "src/Layers/**.cpp",
        "src/Core/Project/**.cpp",
        "Include/Core/Project/**.hpp",
        "src/Utils/Platform/Windows/**.cpp",

        -- GLAD
        "%{wks.location}/Vendor/GLAD/src/glad.c",

        -- ImGui core
        "%{wks.location}/Vendor/IMGUI/ImGui/*.cpp",
        "%{wks.location}/Vendor/IMGUI/ImGui/*.h",

        -- ImGui backends
        "%{wks.location}/Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.cpp",
        "%{wks.location}/Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.h",
        "%{wks.location}/Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.cpp",
        "%{wks.location}/Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.h",

        -- volk
        "%{wks.location}/Vendor/volk/volk.c"
    }

    filter "files:**.c"
        flags { "NoPCH" }
    filter "files:**/Vendor/**"
        flags { "NoPCH" }
    filter "files:%{wks.location}/Vendor/**"
        flags { "NoPCH" }
    filter {}

    -- Exclude Windows and DirectX11 specific source files on non-Windows platforms
    filter "system:not windows"
        removefiles {
            "src/Renderer/DirectX11/**",
            "Include/Renderer/DirectX11/**",
            "src/Utils/Platform/Windows/**"
        }
    filter {}

    -- Exclude Metal specific source files on non-macOS platforms
    filter "system:not macosx"
        removefiles {
            "src/Renderer/Metal/**",
            "Include/Renderer/Metal/**",
            "src/**.mm"
        }
    filter {}
    
    -- Exclude non-Metal renderers on macOS
    filter "system:macosx"
        removefiles {
            "src/Renderer/OpenGLES/**",
            "Include/Renderer/OpenGLES/**",
            "src/Renderer/Vulkan/**",
            "Include/Renderer/Vulkan/**",
            "%{wks.location}/Vendor/volk/**"
        }
    filter {}

    vpaths {
        ["Header Files/*"] = {
            "Include/**.h",
            "Include/**.hpp"
        },
        ["Source Files/*"] = {
            "src/**.cpp",
            "%{wks.location}/Vendor/GLAD/src/glad.c"
        },
        ["Editor Layer/*"] = {
            "src/Core/Layers/**.cpp",
            "src/Layers/**.cpp",
            "Include/Layers/**.hpp"
        }
    }

    includedirs {
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.Engine}",
        "%{IncludeDir.Engine_Include}",
        "%{IncludeDir.Logger}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.Velox}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.volk}",
        "%{IncludeDir.OpenGLES}",
        "%{IncludeDir.miniaudio}"
    }

    externalincludedirs {
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.Engine}",
        "%{IncludeDir.Engine_Include}",
        "%{IncludeDir.Logger}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.Velox}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.volk}",
        "%{IncludeDir.OpenGLES}",
        "%{IncludeDir.miniaudio}"
    }

    filter "action:vs*"
        libdirs {
            "%{wks.location}/Vendor/Customizable_Logger/build/lib",
            "%{wks.location}/Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
            "%{wks.location}/Vendor/GLFW/build/src",
            "%{wks.location}/Vendor/GLFW/build/src/%{cfg.buildcfg}"
        }
    filter "action:gmake*"
        libdirs {
            "%{wks.location}/Vendor/Customizable_Logger/build/lib",
            "%{wks.location}/Vendor/GLFW/build/src"
        }
    filter {}

    links {
        "Customizable_Logger",
        "Velox",
        "glfw3"
    }

    filter "system:windows"
        links { "opengl32" }
    filter "system:linux"
        links { "GL" }
    filter "system:macosx"
        links {
            "Cocoa.framework",
            "IOKit.framework",
            "CoreFoundation.framework",
            "CoreVideo.framework",
            "QuartzCore.framework",
            "Metal.framework"
        }
    filter {}

    dependson { "Logger", "Velox" }

    local rootDir = _MAIN_SCRIPT_DIR or _WORKING_DIR or "."
    for _, pluginPath in ipairs(os.matchfiles(rootDir .. "/Engine/Plugins/*/*.teplugin")) do
        local pName = path.getbasename(pluginPath)
        defines {
            "TE_PLUGIN_" .. pName .. "_ENABLED=1",
            "TE_HAS_PLUGIN_" .. string.upper(pName)
        }
    end

    filter "action:vs*"
        buildoptions { "/utf-8", "/FS", "/bigobj" }
    filter {}

    -- Force-run rule checker before any compilation begins
    filter { "system:windows", "action:vs*" }
        prebuildcommands {
            '"$(SolutionDir)Vendor\\Premake\\Windows\\premake5.exe" --file="$(SolutionDir)premake5.lua" check-rules'
        }
    filter { "system:windows", "action:gmake*" }
        prebuildcommands {
            '"%{wks.location}/Vendor/Premake/Windows/premake5.exe" --file="%{wks.location}/premake5.lua" check-rules'
        }
    filter { "system:linux" }
        prebuildcommands {
            '"%{wks.location}/Vendor/Premake/Linux/premake5" --file="%{wks.location}/premake5.lua" check-rules'
        }
    filter { "system:macosx" }
        prebuildcommands {
            '"%{wks.location}/Vendor/Premake/Mac/premake5" --file="%{wks.location}/premake5.lua" check-rules'
        }
    filter {}

    filter "system:windows"
        systemversion "latest"
        defines {
            "TE_PLATFORM_WINDOWS",
            "TE_BUILD_DLL",
            "IMGUI_IMPL_OPENGL_LOADER_GLAD"
        }
        links {
            "d3d11",
            "dxgi",
            "d3dcompiler",
            "gdi32",
            "comdlg32",
            "ole32",
            "uuid",
            "pdh",
            "ws2_32"
        }

    filter { "system:windows", "action:vs*" }
        postbuildcommands {
            -- Copy DLL and LIB to TimeEditor
            'xcopy /Y /D /Q "%{wks.location}\\Bin\\' .. outputdir .. '\\Engine\\Engine.dll" "%{wks.location}\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul',
            'xcopy /Y /D /Q "%{wks.location}\\Bin\\' .. outputdir .. '\\Engine\\Engine.lib" "%{wks.location}\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul'
        }

    filter { "system:windows", "action:gmake*" }
        postbuildcommands {
            '{COPY} "%{wks.location}/Bin/' .. outputdir .. '/Engine/Engine.dll" "%{wks.location}/Bin/' .. outputdir .. '/TimeEditor/"',
            '{COPY} "%{wks.location}/Bin/' .. outputdir .. '/Engine/Engine.lib" "%{wks.location}/Bin/' .. outputdir .. '/TimeEditor/"'
        }

    filter "configurations:Debug"
        defines { "TE_DEBUG", "TE_EDITOR" }
        symbols "On"

    filter "configurations:Release"
        defines { "TE_RELEASE", "TE_EDITOR" }
        optimize "On"

    filter "configurations:Dist"
        defines { "TE_DIST", "TE_PACKAGED", "TE_MINIMIZED" }
        optimize "On"
        
    filter "system:windows"
        icon "%{wks.location}/Resources/Branding/TimeEngineIcon.ico"

-- Load dynamic plugins
include "Plugins"

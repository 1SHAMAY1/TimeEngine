-- ========== Engine Project ==========

project "Engine"
    location "."
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/Artifacts/Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Artifacts/Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    filter "action:vs*"
        pchheader "PreRequisites.h"
        pchsource "Source/Runtime/Core/Source/PreRequisites.cpp"
    filter "action:xcode*"
        pchheader "PreRequisites.h"
    filter "action:gmake*"
        pchheader "PreRequisites.h"
        pchsource "Source/Runtime/Core/Source/PreRequisites.cpp"
    filter {}


    filter "system:macosx"
        xcodebuildsettings {
            ["LD_DYLIB_INSTALL_NAME"] = "@rpath/libEngine.dylib"
        }
        files { "Source/**.mm" }
    filter {}

    files {
        -- Core Runtime Subsystems
        "Source/Runtime/**.h",
        "Source/Runtime/**.hpp",
        "Source/Runtime/**.cpp",

        -- Framework Subsystems
        "Source/Framework/**.h",
        "Source/Framework/**.hpp",
        "Source/Framework/**.cpp",

        -- ThirdParty Glue
        "%{wks.location}/ThirdParty/GLAD/src/glad.c",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/*.cpp",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/*.h",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_glfw.cpp",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_glfw.h",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_opengl3.cpp",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_opengl3.h",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_dx11.cpp",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_dx11.h",
        "%{wks.location}/ThirdParty/volk/volk.c"
    }

    filter "files:**.c"
        flags { "NoPCH" }
    filter { "system:macosx", "action:xcode*", "files:**.c" }
        buildoptions { "-x c++" }
    filter "files:**.mm"
        flags { "NoPCH" }
    filter "files:**/ThirdParty/**"
        flags { "NoPCH" }
    filter "files:%{wks.location}/ThirdParty/**"
        flags { "NoPCH" }
    filter {}

    -- Platform-specific exclusions
    filter "system:not windows"
        removefiles {
            "Source/Runtime/RHI/Source/Backends/DirectX11/**",
            "Source/Runtime/Core/Source/Platform/Windows/**",
            "Source/Runtime/Window/Source/WindowsWindow.cpp",
            "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_dx11.cpp",
            "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_dx11.h",
            "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_win32.cpp",
            "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_win32.h"
        }
    filter {}

    filter "system:not linux"
        removefiles {
            "Source/Runtime/Core/Source/Platform/Linux/**",
            "Source/Runtime/Window/Source/LinuxWindow.cpp"
        }
    filter {}

    filter "system:not macosx"
        removefiles {
            "Source/Runtime/Core/Source/Platform/Mac/**",
            "Source/Runtime/RHI/Source/Backends/Metal/**",
            "Source/**.mm",
            "Source/Runtime/Window/Source/MacWindow.cpp"
        }
    filter {}

    filter "system:macosx"
        removefiles {
            "Source/Runtime/RHI/Source/Backends/OpenGLES/**",
            "Source/Runtime/RHI/Source/Backends/Vulkan/**",
            "Source/Runtime/UI/Source/ForgeUI/**",
            "%{wks.location}/ThirdParty/volk/**"
        }
    filter {}

    includedirs {
        "%{IncludeDir.Engine_Source}",
        "%{IncludeDir.Engine_Runtime}",
        "%{IncludeDir.Engine_Framework}",
        "%{IncludeDir.Engine_Core}",
        "%{IncludeDir.Engine_Math}",
        "%{IncludeDir.Engine_RHI}",
        "%{IncludeDir.Engine_Renderer2D}",
        "%{IncludeDir.Engine_Physics}",
        "%{IncludeDir.Engine_Scripting}",
        "%{IncludeDir.Engine_Scene}",
        "%{IncludeDir.Engine_Testing}",
        "%{IncludeDir.Engine_UI}",
        "%{IncludeDir.Engine_Window}",
        "%{IncludeDir.Engine_Framework}",
        "%{IncludeDir.Engine_Asset}",
        "%{IncludeDir.Engine_Input}",
        "%{IncludeDir.Engine_Gameplay}",
        "%{IncludeDir.Engine_App}",
        "%{IncludeDir.ThirdParty_ImGui}",
        "%{IncludeDir.ThirdParty_ForgeUI}",
        "%{IncludeDir.ThirdParty_Logger}",
        "%{IncludeDir.ThirdParty_GLFW}",
        "%{IncludeDir.ThirdParty_GLAD}",
        "%{IncludeDir.ThirdParty_GLM}",
        "%{IncludeDir.ThirdParty_stb}",
        "%{IncludeDir.ThirdParty_Velox}",
        "%{IncludeDir.ThirdParty_Vulkan}",
        "%{IncludeDir.ThirdParty_volk}",
        "%{IncludeDir.ThirdParty_OpenGLES}",
        "%{IncludeDir.ThirdParty_miniaudio}"
    }

    externalincludedirs {
        "%{IncludeDir.ThirdParty_ImGui}",
        "%{IncludeDir.ThirdParty_ForgeUI}",
        "%{IncludeDir.ThirdParty_Logger}",
        "%{IncludeDir.ThirdParty_GLFW}",
        "%{IncludeDir.ThirdParty_GLAD}",
        "%{IncludeDir.ThirdParty_GLM}",
        "%{IncludeDir.ThirdParty_stb}",
        "%{IncludeDir.ThirdParty_Velox}",
        "%{IncludeDir.ThirdParty_Vulkan}",
        "%{IncludeDir.ThirdParty_volk}",
        "%{IncludeDir.ThirdParty_OpenGLES}",
        "%{IncludeDir.ThirdParty_miniaudio}"
    }

    filter "action:vs*"
        libdirs {
            "%{wks.location}/ThirdParty/Customizable_Logger/build/lib",
            "%{wks.location}/ThirdParty/Customizable_Logger/build/lib/%{cfg.buildcfg}",
            "%{wks.location}/ThirdParty/GLFW/build/src",
            "%{wks.location}/ThirdParty/GLFW/build/src/%{cfg.buildcfg}"
        }
    filter "action:gmake* or action:xcode*"
        libdirs {
            "%{wks.location}/ThirdParty/Customizable_Logger/build/lib",
            "%{wks.location}/ThirdParty/GLFW/build/src"
        }
    filter {}

    links {
        "Customizable_Logger",
        "Velox",
        "glfw3"
    }

    filter "system:not macosx"
        links { "ForgeUI" }
        dependson { "ForgeUI" }
    filter {}

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

    filter "action:vs*"
        buildoptions { "/utf-8", "/FS", "/bigobj" }
    filter {}

    -- Master rule checker runs once before Engine compiles (Engine is the core dependency for TimeEditor and all Plugins)
    filter { "system:windows", "action:vs*" }
        prebuildcommands {
            '"$(SolutionDir)ThirdParty\\Premake\\Windows\\premake5.exe" --file="$(SolutionDir)Premake5.lua" check-rules'
        }
    filter { "system:windows", "action:gmake*" }
        prebuildcommands {
            '"%{wks.location}/ThirdParty/Premake/Windows/premake5.exe" --file="%{wks.location}/Premake5.lua" check-rules'
        }
    filter { "system:linux" }
        prebuildcommands {
            '"%{wks.location}/ThirdParty/Premake/Linux/premake5" --file="%{wks.location}/Premake5.lua" check-rules'
        }
    filter { "system:macosx", "action:gmake*" }
        prebuildcommands {
            'premake5 --file="%{wks.location}/Premake5.lua" check-rules'
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
            "ws2_32",
            "dwmapi"
        }

    filter { "system:windows", "action:vs*" }
        postbuildcommands {
            'xcopy /Y /D /Q "%{wks.location}\\Artifacts\\Bin\\' .. outputdir .. '\\Engine\\Engine.dll" "%{wks.location}\\Artifacts\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul',
            'xcopy /Y /D /Q "%{wks.location}\\Artifacts\\Bin\\' .. outputdir .. '\\Engine\\Engine.lib" "%{wks.location}\\Artifacts\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul'
        }

    filter { "system:windows", "action:gmake*" }
        postbuildcommands {
            '{COPY} "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/Engine/Engine.dll" "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/TimeEditor/"',
            '{COPY} "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/Engine/Engine.lib" "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/TimeEditor/"'
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
    filter {}

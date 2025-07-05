workspace "TimeEngine"
    architecture "x64"
    startproject "Sandbox"

    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- ========== Include Directories ==========

IncludeDir = {}
IncludeDir["Engine"]         = "Engine/src"
IncludeDir["Engine_Include"] = "Engine/Include"
IncludeDir["Logger"]         = "Logger/Customizable_Logger/Include"
IncludeDir["GLFW"]           = "Vendor/GLFW/glfw/include"
IncludeDir["GLAD"]           = "Vendor/GLAD/include"
IncludeDir["ImGui"]          = "Vendor/IMGUI/ImGui"
IncludeDir["GLM"]            = "Vendor/GLM"

-- ========== Vendor Group ==========

group "Vendor"
    -- GLFW is built via CMake
group ""

-- ========== Logger Project ==========

project "Logger"
    location "Logger/Customizable_Logger"
    kind "None"
    language "C++"
    files { }
    includedirs { "%{IncludeDir.Logger}" }

-- ========== Engine Project ==========

project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        -- Core Engine
        "Engine/src/**.h",
        "Engine/src/**.cpp",
        "Engine/Include/**.h",
        "Engine/Include/**.hpp",

        -- Editor Layer
        "Engine/Include/Layers/**.hpp",
        "Engine/src/Core/Layers/**.cpp",

        -- GLAD
        "Vendor/GLAD/src/glad.c",

        -- ImGui core
        "Vendor/IMGUI/ImGui/*.cpp",
        "Vendor/IMGUI/ImGui/*.h",

        -- ImGui backends
        "Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.cpp",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.h",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.cpp",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.h"
    }

    vpaths {
        ["Header Files/*"] = {
            "Engine/Include/**.h",
            "Engine/Include/**.hpp"
        },
        ["Source Files/*"] = {
            "Engine/src/**.cpp",
            "Vendor/GLAD/src/glad.c"
        },
        ["Editor Layer/*"] = {
            "Engine/src/Core/Layers/**.cpp",
            "Engine/Include/Layers/**.hpp"
        }
    }

    includedirs {
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.Engine}",
        "%{IncludeDir.Engine_Include}",
        "%{IncludeDir.Logger}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.GLM}"
    }

    libdirs {
        "Logger/Customizable_Logger/build/lib/%{cfg.buildcfg}",
        "Vendor/GLFW/build/src/%{cfg.buildcfg}"
    }

    links {
        "Customizable_Logger",
        "glfw3",
        "opengl32.lib"
    }

    dependson { "Logger" }

    buildoptions { "/utf-8" }

    filter "system:windows"
        systemversion "latest"
        defines {
            "TE_PLATFORM_WINDOWS",
            "TE_BUILD_DLL",
            "IMGUI_IMPL_OPENGL_LOADER_GLAD"
        }

        postbuildcommands {
            -- Copy DLL and LIB to Sandbox
            'xcopy /Y /D /Q "%{wks.location}Bin\\' .. outputdir .. '\\Engine\\Engine.dll" "%{wks.location}Bin\\' .. outputdir .. '\\Sandbox\\" > nul',
            'xcopy /Y /D /Q "%{wks.location}Bin\\' .. outputdir .. '\\Engine\\Engine.lib" "%{wks.location}Bin\\' .. outputdir .. '\\Sandbox\\" > nul'
        }

    filter "configurations:Debug"
        defines { "TE_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "TE_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "TE_DIST" }
        optimize "On"

-- ========== Sandbox Project ==========

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "Sandbox/src/**.h",
        "Sandbox/src/**.cpp",
        "Sandbox/Include/Layers/**.h",
        "Sandbox/src/Core/Layers/**.cpp"
    }

    includedirs {
        "%{IncludeDir.Engine}",
        "%{IncludeDir.Engine_Include}",
        "%{IncludeDir.Logger}",
        "%{IncludeDir.GLM}"
    }

    libdirs {
        "Logger/Customizable_Logger/build/lib/%{cfg.buildcfg}"
    }

    links {
        "Engine",
        "Customizable_Logger"
    }

    dependson { "Engine", "Logger" }

    buildoptions { "/utf-8" }

    filter "system:windows"
        systemversion "latest"
        defines { "TE_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "TE_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "TE_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "TE_DIST" }
        optimize "On"

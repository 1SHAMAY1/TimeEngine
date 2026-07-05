workspace "TimeEngine"
    architecture "x64"
    startproject "TimeEditor"

    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- ========== Include Directories ==========

IncludeDir = {}
IncludeDir["Engine"]         = "Engine/src"
IncludeDir["Engine_Include"] = "Engine/Include"
IncludeDir["Logger"]         = "Vendor/Customizable_Logger/Include"
IncludeDir["GLFW"]           = "Vendor/GLFW/glfw/include"
IncludeDir["GLAD"]           = "Vendor/GLAD/include"
IncludeDir["ImGui"]          = "Vendor/IMGUI/ImGui"
IncludeDir["GLM"]            = "Vendor/GLM"
IncludeDir["stb_image"]      = "Vendor/stb_image"
IncludeDir["Velox"]          = "Vendor/Velox/include"
IncludeDir["Vulkan"]         = "Vendor/Vulkan/include"
IncludeDir["volk"]           = "Vendor/volk"
IncludeDir["OpenGLES"]       = "Vendor/OpenGL-Registry/api"


-- ========== Vendor Group ==========

group "Vendor"
    -- GLFW is built via CMake

    project "Velox"
        location "Vendor/Velox"
        kind "SharedLib"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"

        targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
        objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

        files {
            "Vendor/Velox/include/velox/**.h",
            "Vendor/Velox/src/core/**.cpp",
            "Vendor/Velox/src/core/**.h",
            "Vendor/Velox/src/math/**.cpp",
            "Vendor/Velox/src/math/**.h",
            "Vendor/Velox/src/physics/**.cpp",
            "Vendor/Velox/src/physics/**.h",
            "Vendor/Velox/src/api/**.cpp",
            "Vendor/Velox/src/api/**.h"
        }

        includedirs {
            "Vendor/Velox/include",
            "Vendor/Velox/src",
            "Vendor/Velox/src/core",
            "Vendor/Velox/src/math",
            "Vendor/Velox/src/physics",
            "Vendor/Velox/src/api"
        }

        filter "system:windows"
            systemversion "latest"
            defines {
                "WIN32",
                "VELOX_EXPORTS"
            }

            postbuildcommands {
                -- Copy DLL and LIB to TimeEditor
                'xcopy /Y /D /Q "..\\..\\Bin\\' .. outputdir .. '\\Velox\\Velox.dll" "..\\..\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul',
                'xcopy /Y /D /Q "..\\..\\Bin\\' .. outputdir .. '\\Velox\\Velox.lib" "..\\..\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul'
            }

        filter "configurations:Debug"
            symbols "On"

        filter "configurations:Release"
            optimize "On"

        filter "configurations:Dist"
            optimize "On"
group ""

-- ========== Logger Project ==========

project "Logger"
    location "Vendor/Customizable_Logger"
    kind "Utility"
    language "C++"
    files { }
    includedirs { "%{IncludeDir.Logger}" }
    
-- ========== Docs / Misc Project (visibility only, not compiled) ==========

project "Docs"
    location "Docs"
    kind "None"
    language "C++"
    objdir "Bin-Intermediate/Docs"

    files {
        -- Root markdown & text docs
        "*.md",
        "llms.md",
        "README.md",
        "LICENSE",
        "CONTRIBUTING.md",
        "ROADMAP.md",
        "SECURITY.md",

        -- Config / meta files
        ".agentsrules",
        ".clang-format",
        ".gitattributes",
        ".gitignore",
        ".gitmodules",

        -- .github and .agents folders
        ".github/**",
        ".agents/**",

        -- Build script itself
        "Premake5.lua",

        -- Scripts folder (adjust pattern/extension as needed)
        "Scripts/**.bat",
        "Scripts/**.sh",
        "Scripts/**.lua",
        "Scripts/**.py"
    }

    vpaths {
        ["Docs/*"] = { "*.md", "README*", "LICENSE*", "CONTRIBUTING*", "ROADMAP*", "SECURITY*", "llms*" },
        ["Config/*"] = { ".agentsrules", ".clang-format", ".gitattributes", ".gitignore", ".gitmodules" },
        ["Scripts/*"] = { "Scripts/**.bat", "Scripts/**.sh", "Scripts/**.lua", "Scripts/**.py" },
        ["Build/*"] = { "Premake5.lua" },
        ["Github/*"] = { ".github/**" },
        ["Agents/*"] = { ".agents/**" }
    }

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

        -- Editor & Profiling Layers
        "Engine/Include/Layers/**.hpp",
        "Engine/src/Core/Layers/**.cpp",
        "Engine/src/Layers/**.cpp",
        "Engine/src/Core/Project/**.cpp",
        "Engine/Include/Core/Project/**.hpp",
        "Engine/src/Platform/Windows/**.cpp",        
        
        -- GLAD
        "Vendor/GLAD/src/glad.c",

        -- ImGui core
        "Vendor/IMGUI/ImGui/*.cpp",
        "Vendor/IMGUI/ImGui/*.h",

        -- ImGui backends
        "Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.cpp",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.h",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.cpp",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.h",

        -- volk
        "Vendor/volk/volk.c"
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
            "Engine/src/Layers/**.cpp",
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
        "%{IncludeDir.GLM}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.Velox}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.volk}",
        "%{IncludeDir.OpenGLES}"
    }

    libdirs {
        "Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
        "Vendor/GLFW/build/src/%{cfg.buildcfg}"
    }

    links {
        "Customizable_Logger",
        "Velox",
        "glfw3",
        "opengl32.lib"
    }

    dependson { "Logger", "Velox" }

    buildoptions { "/utf-8", "/FS" }

    filter "system:windows"
        systemversion "latest"
        defines {
            "TE_PLATFORM_WINDOWS",
            "TE_BUILD_DLL",
            "IMGUI_IMPL_OPENGL_LOADER_GLAD"
        }
        links {
            "d3d11.lib",
            "dxgi.lib",
            "d3dcompiler.lib"
        }

        postbuildcommands {
            -- Copy DLL and LIB to TimeEditor
            'xcopy /Y /D /Q "..\\Bin\\' .. outputdir .. '\\Engine\\Engine.dll" "..\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul',
            'xcopy /Y /D /Q "..\\Bin\\' .. outputdir .. '\\Engine\\Engine.lib" "..\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul'
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
        icon "Resources/Branding/Icon.ico"

-- ========== TimeEditor Project ==========

project "TimeEditor"
    location "TimeEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "TimeEditor/src/**.h",
        "TimeEditor/src/**.cpp",
        "TimeEditor/Include/Layers/**.h",
        "TimeEditor/src/Core/Layers/**.cpp",

        -- ImGui core
        "Vendor/IMGUI/ImGui/*.cpp",
        "Vendor/IMGUI/ImGui/*.h",

        -- ImGui backends
        "Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.cpp",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.h",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.cpp",
        "Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.h"
    }

    includedirs {
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.Engine}",
        "%{IncludeDir.Engine_Include}",
        "%{IncludeDir.Logger}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Velox}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.volk}"
    }

    libdirs {
        "Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
        "Vendor/GLFW/build/src/%{cfg.buildcfg}"
    }

    links {
        "Engine",
        "Customizable_Logger",
        "Velox",
        "glfw3",
        "opengl32.lib"
    }

    dependson { "Engine", "Logger", "Velox" }

    buildoptions { "/utf-8" }

    filter "system:windows"
        systemversion "latest"
        defines { "TE_PLATFORM_WINDOWS" }
        links {
            "d3d11.lib",
            "dxgi.lib",
            "d3dcompiler.lib"
        }

        postbuildcommands {
            '\"%{wks.location}Bin\\' .. outputdir .. '\\%{prj.name}\\%{prj.name}.exe\" --register'
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

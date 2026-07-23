workspace "TimeEngine"
    filter "system:not macosx"
        architecture "x64"
    filter {}
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

        filter { "system:windows", "action:vs*" }
            postbuildcommands {
                -- Copy DLL and LIB to TimeEditor
                'xcopy /Y /D /Q "..\\..\\Bin\\' .. outputdir .. '\\Velox\\Velox.dll" "..\\..\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul',
                'xcopy /Y /D /Q "..\\..\\Bin\\' .. outputdir .. '\\Velox\\Velox.lib" "..\\..\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul'
            }

        filter { "system:windows", "action:gmake*" }
            postbuildcommands {
                '{COPY} "../../Bin/' .. outputdir .. '/Velox/Velox.dll" "../../Bin/' .. outputdir .. '/TimeEditor/"',
                '{COPY} "../../Bin/' .. outputdir .. '/Velox/Velox.lib" "../../Bin/' .. outputdir .. '/TimeEditor/"'
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
        "LICENSE",

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
        "Engine/src/Utils/Platform/Windows/**.cpp",        
        
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

    -- Exclude Windows and DirectX11 specific source files on non-Windows platforms
    filter "system:not windows"
        removefiles {
            "Engine/src/Renderer/DirectX11/**",
            "Engine/Include/Renderer/DirectX11/**",
            "Engine/src/Utils/Platform/Windows/**"
        }
    filter {}

    -- Exclude non-Metal renderers on macOS (OpenGL, OpenGLES, Vulkan) since Metal is not yet present
    filter "system:macosx"
        removefiles {
            --"Engine/src/Renderer/OpenGL/**",
            --"Engine/Include/Renderer/OpenGL/**",
            "Engine/src/Renderer/OpenGLES/**",
            "Engine/Include/Renderer/OpenGLES/**",
            "Engine/src/Renderer/Vulkan/**",
            "Engine/Include/Renderer/Vulkan/**",
            "Vendor/volk/**"
        }
    filter {}


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

    filter "action:vs*"
        libdirs {
            "Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
            "Vendor/GLFW/build/src/%{cfg.buildcfg}"
        }
    filter "action:gmake*"
        libdirs {
            "Vendor/Customizable_Logger/build/lib",
            "Vendor/GLFW/build/src"
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
            "QuartzCore.framework"
        }
    filter {}

    dependson { "Logger", "Velox" }

    filter "action:vs*"
        buildoptions { "/utf-8", "/FS" }
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
            "pdh"
        }

    filter { "system:windows", "action:vs*" }
        postbuildcommands {
            -- Copy DLL and LIB to TimeEditor
            'xcopy /Y /D /Q "..\\Bin\\' .. outputdir .. '\\Engine\\Engine.dll" "..\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul',
            'xcopy /Y /D /Q "..\\Bin\\' .. outputdir .. '\\Engine\\Engine.lib" "..\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul'
        }

    filter { "system:windows", "action:gmake*" }
        postbuildcommands {
            '{COPY} "../Bin/' .. outputdir .. '/Engine/Engine.dll" "../Bin/' .. outputdir .. '/TimeEditor/"',
            '{COPY} "../Bin/' .. outputdir .. '/Engine/Engine.lib" "../Bin/' .. outputdir .. '/TimeEditor/"'
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

    filter "action:vs*"
        libdirs {
            "Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
            "Vendor/GLFW/build/src/%{cfg.buildcfg}"
        }
    filter "action:gmake*"
        libdirs {
            "Vendor/Customizable_Logger/build/lib",
            "Vendor/GLFW/build/src"
        }
    filter {}

    links {
        "Engine",
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
            "QuartzCore.framework"
        }
    filter {}

    dependson { "Engine", "Logger", "Velox" }

    filter "action:vs*"
        buildoptions { "/utf-8" }
    filter {}

    filter "system:windows"
        systemversion "latest"
        defines { "TE_PLATFORM_WINDOWS" }
        links {
            "d3d11",
            "dxgi",
            "d3dcompiler",
            "gdi32",
            "comdlg32",
            "ole32",
            "uuid"
        }

        if os.getenv("CI") ~= "true" and os.getenv("GITHUB_ACTIONS") ~= "true" then
            filter { "system:windows", "action:vs*" }
                postbuildcommands {
                    '\"%{wks.location}Bin\\' .. outputdir .. '\\%{prj.name}\\%{prj.name}.exe\" --register'
                }

            filter { "system:windows", "action:gmake*" }
                postbuildcommands {
                    '\"%{cfg.targetdir}/%{prj.name}.exe\" --register'
                }
        end

    filter "configurations:Debug"
        defines { "TE_DEBUG", "TE_EDITOR" }
        symbols "On"

    filter "configurations:Release"
        defines { "TE_RELEASE", "TE_EDITOR" }
        optimize "On"

    filter "configurations:Dist"
        defines { "TE_DIST", "TE_PACKAGED", "TE_MINIMIZED" }
        optimize "On"

-- ========== Dynamic Plugin Projects Discovery & Generation ==========

group "Plugins"

-- Find and configure all engine-level plugins
local enginePlugins = os.matchfiles("Engine/Plugins/*/*.teplugin")
for _, pluginPath in ipairs(enginePlugins) do
    local pluginDir = path.getdirectory(pluginPath)
    local pluginName = path.getbasename(pluginPath)
    
    project (pluginName)
        location (pluginDir)
        kind "SharedLib"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"

        targetdir ("Bin/" .. outputdir .. "/TimeEditor/Plugins/" .. pluginName)
        objdir ("Bin-Intermediate/" .. outputdir .. "/Plugins/" .. pluginName)

        files {
            pluginDir .. "/src/**.h",
            pluginDir .. "/src/**.hpp",
            pluginDir .. "/src/**.cpp"
        }

        includedirs {
            "Engine/src",
            "Engine/Include",
            "Vendor/IMGUI/ImGui",
            "Vendor/Customizable_Logger/Include",
            "Vendor/GLM",
            "Vendor/GLFW/glfw/include",
            "Vendor/Velox/include",
            "Vendor/Vulkan/include",
            "Vendor/volk"
        }

        filter "action:vs*"
            libdirs {
                "Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
                "Vendor/GLFW/build/src/%{cfg.buildcfg}"
            }
        filter "action:gmake*"
            libdirs {
                "Vendor/Customizable_Logger/build/lib",
                "Vendor/GLFW/build/src"
            }
        filter {}

        links {
            "Engine",
            "Customizable_Logger"
        }

        filter "system:windows"
            systemversion "latest"
            defines {
                "TE_PLATFORM_WINDOWS"
            }
            links {
                "ws2_32"
            }

        filter { "system:windows", "action:vs*" }
            postbuildcommands {
                'xcopy /Y /D /Q "$(ProjectDir)*.teplugin" "$(OutDir)" > nul'
            }

        filter { "system:windows", "action:gmake*" }
            postbuildcommands {
                '{COPY} "' .. pluginName .. '.teplugin" "%{cfg.targetdir}/"'
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
end

-- Find and configure all project-level plugins
local projectPlugins = os.matchfiles("Projects/*/Plugins/*/*.teplugin")
for _, pluginPath in ipairs(projectPlugins) do
    local pluginDir = path.getdirectory(pluginPath)
    local pluginName = path.getbasename(pluginPath)
    
    project (pluginName)
        location (pluginDir)
        kind "SharedLib"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"

        targetdir ("Bin/" .. outputdir .. "/TimeEditor/Plugins/" .. pluginName)
        objdir ("Bin-Intermediate/" .. outputdir .. "/Plugins/" .. pluginName)

        files {
            pluginDir .. "/src/**.h",
            pluginDir .. "/src/**.hpp",
            pluginDir .. "/src/**.cpp"
        }

        includedirs {
            "Engine/src",
            "Engine/Include",
            "Vendor/IMGUI/ImGui",
            "Vendor/Customizable_Logger/Include",
            "Vendor/GLM",
            "Vendor/GLFW/glfw/include",
            "Vendor/Velox/include",
            "Vendor/Vulkan/include",
            "Vendor/volk"
        }

        filter "action:vs*"
            libdirs {
                "Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
                "Vendor/GLFW/build/src/%{cfg.buildcfg}"
            }
        filter "action:gmake*"
            libdirs {
                "Vendor/Customizable_Logger/build/lib",
                "Vendor/GLFW/build/src"
            }
        filter {}

        links {
            "Engine",
            "Customizable_Logger"
        }

        filter "system:windows"
            systemversion "latest"
            defines {
                "TE_PLATFORM_WINDOWS"
            }
            links {
                "ws2_32"
            }

        filter { "system:windows", "action:vs*" }
            postbuildcommands {
                'xcopy /Y /D /Q "$(ProjectDir)*.teplugin" "$(OutDir)" > nul'
            }

        filter { "system:windows", "action:gmake*" }
            postbuildcommands {
                '{COPY} "' .. pluginName .. '.teplugin" "%{cfg.targetdir}/"'
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
end

group ""

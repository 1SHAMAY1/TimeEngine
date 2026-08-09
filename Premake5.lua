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

        filter "system:macosx"
            xcodebuildsettings {
                ["LD_DYLIB_INSTALL_NAME"] = "@rpath/libVelox.dylib"
            }
        filter {}

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

        externalincludedirs {
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

    filter "system:macosx"
        xcodebuildsettings {
            ["LD_DYLIB_INSTALL_NAME"] = "@rpath/libEngine.dylib"
        }
        files { "Engine/src/**.mm" }
    filter {}

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

    -- Exclude Metal specific source files on non-macOS platforms
    filter "system:not macosx"
        removefiles {
            "Engine/src/Renderer/Metal/**",
            "Engine/Include/Renderer/Metal/**",
            "Engine/src/**.mm"
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
            "QuartzCore.framework",
            "Metal.framework"
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

-- Auto-generate TimeEditor/Info.plist on macOS if missing
if os.target() == "macosx" and not os.isfile("TimeEditor/Info.plist") then
    local plistFile = io.open("TimeEditor/Info.plist", "w")
    if plistFile then
        plistFile:write([[<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>TimeEditor</string>
    <key>CFBundleIdentifier</key>
    <string>com.timeengine.timeeditor</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>TimeEditor</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.14</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>CFBundleDocumentTypes</key>
    <array>
        <dict>
            <key>CFBundleTypeExtensions</key>
            <array>
                <string>teproj</string>
            </array>
            <key>CFBundleTypeName</key>
            <string>TimeEngine Project</string>
            <key>CFBundleTypeRole</key>
            <string>Editor</string>
            <key>LSHandlerRank</key>
            <string>Owner</string>
        </dict>
    </array>
</dict>
</plist>
]])
        plistFile:close()
    end
end

project "TimeEditor"
    location "TimeEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    filter "system:macosx"
        kind "WindowedApp"
        files { "Info.plist" }
        xcodebuildsettings {
            ["INFOPLIST_FILE"] = "Info.plist",
            ["LD_RUNPATH_SEARCH_PATHS"] = "@executable_path"
        }
        postbuildcommands {
            'mkdir -p "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS/"',
            'cp -f "../Bin/' .. outputdir .. '/Engine/libEngine.dylib" "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS/"',
            'cp -f "../Bin/' .. outputdir .. '/Velox/libVelox.dylib" "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS/"',
            'codesign --force --deep --sign - "%{cfg.targetdir}/TimeEditor.app" 2>/dev/null || true'
        }
    filter {}

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

    externalincludedirs {
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

    filter "system:windows or linux"
        links {
            "Engine",
            "Customizable_Logger",
            "Velox",
            "glfw3"
        }

    filter "system:macosx"
        links {
            "Engine",
            "Customizable_Logger",
            "Velox",
            "glfw3",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreFoundation.framework",
            "CoreVideo.framework",
            "QuartzCore.framework",
            "Metal.framework"
        }

    filter "system:windows"
        links { "opengl32" }
    filter "system:linux"
        links { "GL" }
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

        externalincludedirs {
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

        externalincludedirs {
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

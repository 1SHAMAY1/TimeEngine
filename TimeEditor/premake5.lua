-- ========== TimeEditor Project ==========

-- Auto-generate TimeEditor/src/TimeEditor.rc on Windows
if os.target() == "windows" and not os.isfile("src/TimeEditor.rc") then
    local rcFile = io.open("src/TimeEditor.rc", "w")
    if rcFile then
        rcFile:write([[// TimeEngine TimeEditor Windows Resource Script
#define IDI_ICON1 101

IDI_ICON1 ICON "../../Resources/Branding/TimeEngineIcon.ico"
]])
        rcFile:close()
    end
end

-- Auto-generate TimeEditor/Info.plist on macOS if missing
if os.target() == "macosx" and not os.isfile("Info.plist") then
    local plistFile = io.open("Info.plist", "w")
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
    <key>CFBundleIconFile</key>
    <string>TimeEngineIcon.png</string>
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
    location "."
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "off"

    filter { "system:macosx", "action:xcode*" }
        kind "WindowedApp"
        files { "Info.plist" }
        xcodebuildsettings {
            ["INFOPLIST_FILE"] = "Info.plist",
            ["LD_RUNPATH_SEARCH_PATHS"] = "@executable_path"
        }

    filter { "system:macosx", "action:gmake*" }
        kind "ConsoleApp"
        targetname "TimeEditor"
        postbuildcommands {
            'mkdir -p "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS"',
            'mkdir -p "%{cfg.targetdir}/TimeEditor.app/Contents/Resources"',
            'cp -f "%{cfg.targetdir}/TimeEditor" "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS/TimeEditor"',
            'cp -f "%{wks.location}/TimeEditor/Info.plist" "%{cfg.targetdir}/TimeEditor.app/Contents/Info.plist" 2>/dev/null || true',
            'cp -f "%{wks.location}/Resources/Branding/TimeEngineIcon.png" "%{cfg.targetdir}/TimeEditor.app/Contents/Resources/" 2>/dev/null || true',
            'cp -f "%{wks.location}/Bin/' .. outputdir .. '/Engine/libEngine.dylib" "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS/" 2>/dev/null || true',
            'cp -f "%{wks.location}/Bin/' .. outputdir .. '/Velox/libVelox.dylib" "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS/" 2>/dev/null || true',
            'codesign --force --deep --sign - "%{cfg.targetdir}/TimeEditor.app" 2>/dev/null || true'
        }
    filter {}

    targetdir ("%{wks.location}/Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    local rootDir = _MAIN_SCRIPT_DIR or _WORKING_DIR or "."
    local editorDeps = { "Engine", "Logger", "Velox" }
    for _, pluginPath in ipairs(os.matchfiles(rootDir .. "/Engine/Plugins/*/*.teplugin")) do
        table.insert(editorDeps, path.getbasename(pluginPath))
        local pName = path.getbasename(pluginPath)
        defines {
            "TE_PLUGIN_" .. pName .. "_ENABLED=1",
            "TE_HAS_PLUGIN_" .. string.upper(pName)
        }
    end
    dependson(editorDeps)

    files {
        "src/**.h",
        "src/**.cpp",
        "src/**.rc",
        "Include/Layers/**.h",
        "src/Core/Layers/**.cpp",

        -- ImGui core
        "%{wks.location}/Vendor/IMGUI/ImGui/*.cpp",
        "%{wks.location}/Vendor/IMGUI/ImGui/*.h",

        -- ImGui backends
        "%{wks.location}/Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.cpp",
        "%{wks.location}/Vendor/IMGUI/ImGui/backends/imgui_impl_glfw.h",
        "%{wks.location}/Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.cpp",
        "%{wks.location}/Vendor/IMGUI/ImGui/backends/imgui_impl_opengl3.h"
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
            "%{wks.location}/Vendor/Customizable_Logger/build/lib",
            "%{wks.location}/Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
            "%{wks.location}/Vendor/GLFW/build/src",
            "%{wks.location}/Vendor/GLFW/build/src/%{cfg.buildcfg}",
            "%{wks.location}/Bin/" .. outputdir .. "/Engine",
            "%{wks.location}/Bin/" .. outputdir .. "/Velox"
        }
    filter "action:gmake* or action:xcode*"
        libdirs {
            "%{wks.location}/Vendor/Customizable_Logger/build/lib",
            "%{wks.location}/Vendor/GLFW/build/src"
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

    -- Force-run rule checker before any compilation begins
    filter { "system:windows", "action:vs*" }
        prebuildcommands {
            '"$(SolutionDir)Vendor\\Premake\\Windows\\premake5.exe" --file="$(SolutionDir)Premake5.lua" check-rules'
        }
    filter { "system:windows", "action:gmake*" }
        prebuildcommands {
            '"%{wks.location}/Vendor/Premake/Windows/premake5.exe" --file="%{wks.location}/Premake5.lua" check-rules'
        }
    filter { "system:linux" }
        prebuildcommands {
            '"%{wks.location}/Vendor/Premake/Linux/premake5" --file="%{wks.location}/Premake5.lua" check-rules'
        }
    filter { "system:macosx" }
        prebuildcommands {
            '"%{wks.location}/Vendor/Premake/Mac/premake5" --file="%{wks.location}/Premake5.lua" check-rules'
        }
    filter {}

    filter "system:windows"
        systemversion "latest"
        icon "%{wks.location}/Resources/Branding/TimeEngineIcon.ico"
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

    filter "configurations:Debug"
        defines { "TE_DEBUG", "TE_EDITOR" }
        symbols "On"

    filter "configurations:Release"
        defines { "TE_RELEASE", "TE_EDITOR" }
        optimize "On"

    filter "configurations:Dist"
        defines { "TE_DIST", "TE_PACKAGED", "TE_MINIMIZED" }
        optimize "On"

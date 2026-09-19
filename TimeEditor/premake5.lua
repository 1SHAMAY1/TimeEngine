local scriptDir = path.getdirectory(_SCRIPT) or "."

-- Generate macOS Info.plist dynamically for Xcode application bundle
if os.target() == "macosx" or _ACTION == "xcode4" then
    local plistFile = io.open(scriptDir .. "/Info.plist", "w")
    if plistFile then
        plistFile:write([[<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>TimeEditor</string>
    <key>CFBundleIconFile</key>
    <string>TimeEngineIcon</string>
    <key>CFBundleIdentifier</key>
    <string>com.timeengine.editor</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>TimeEditor</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>12.0</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSHumanReadableCopyright</key>
    <string>Copyright © 2026 TimeEngine. All rights reserved.</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
]])
        plistFile:close()
    end
end

-- Generate Windows Resource Script (.rc) dynamically
if os.target() == "windows" or (_ACTION and string.startswith(_ACTION, "vs")) then
    local rcFile = io.open(scriptDir .. "/TimeEditor.rc", "w")
    if rcFile then
        rcFile:write([[// TimeEngine TimeEditor Windows Resource Script (Auto-Generated)
#define IDI_ICON1 101
IDI_ICON1 ICON "../Resources/Branding/TimeEngineIcon.ico"
]])
        rcFile:close()
    end
end

project "TimeEditor"
    location "."
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
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
            'cp -f "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/Engine/libEngine.dylib" "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS/" 2>/dev/null || true',
            'cp -f "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/Velox/libVelox.dylib" "%{cfg.targetdir}/TimeEditor.app/Contents/MacOS/" 2>/dev/null || true',
            'codesign --force --deep --sign - "%{cfg.targetdir}/TimeEditor.app" 2>/dev/null || true'
        }
    filter {}

    targetdir ("%{wks.location}/Artifacts/Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/Artifacts/Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    local rootDir = _MAIN_SCRIPT_DIR or _WORKING_DIR or "."
    local editorDeps = { "Engine", "Logger", "Velox" }

    for _, p in ipairs(os.matchfiles(rootDir .. "/Plugins/Shipping/*/*.teplugin")) do table.insert(editorDeps, path.getbasename(p)) end
    for _, p in ipairs(os.matchfiles(rootDir .. "/Plugins/Experimental/*/*.teplugin")) do table.insert(editorDeps, path.getbasename(p)) end

    dependson(editorDeps)

    files {
        "Source/**.h",
        "Source/**.hpp",
        "Source/**.cpp",

        -- ThirdParty ImGui core & backends
        "%{wks.location}/ThirdParty/IMGUI/ImGui/*.cpp",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/*.h",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_glfw.cpp",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_glfw.h",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_opengl3.cpp",
        "%{wks.location}/ThirdParty/IMGUI/ImGui/backends/imgui_impl_opengl3.h"
    }

    filter { "system:windows", "action:vs*" }
        files { "TimeEditor.rc" }
    filter {}

    includedirs {
        "Source",
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
        "%{IncludeDir.Engine_Asset}",
        "%{IncludeDir.Engine_Input}",
        "%{IncludeDir.Engine_Gameplay}",
        "%{IncludeDir.Engine_App}",
        "%{IncludeDir.ThirdParty_ImGui}",
        "%{IncludeDir.ThirdParty_Logger}",
        "%{IncludeDir.ThirdParty_GLM}",
        "%{IncludeDir.ThirdParty_GLFW}",
        "%{IncludeDir.ThirdParty_Velox}",
        "%{IncludeDir.ThirdParty_Vulkan}",
        "%{IncludeDir.ThirdParty_volk}"
    }

    externalincludedirs {
        "%{IncludeDir.ThirdParty_ImGui}",
        "%{IncludeDir.ThirdParty_Logger}",
        "%{IncludeDir.ThirdParty_GLM}",
        "%{IncludeDir.ThirdParty_GLFW}",
        "%{IncludeDir.ThirdParty_Velox}",
        "%{IncludeDir.ThirdParty_Vulkan}",
        "%{IncludeDir.ThirdParty_volk}"
    }

    filter "action:vs*"
        libdirs {
            "%{wks.location}/ThirdParty/Customizable_Logger/build/lib",
            "%{wks.location}/ThirdParty/Customizable_Logger/build/lib/%{cfg.buildcfg}",
            "%{wks.location}/ThirdParty/GLFW/build/src",
            "%{wks.location}/ThirdParty/GLFW/build/src/%{cfg.buildcfg}",
            "%{wks.location}/Artifacts/Bin/" .. outputdir .. "/Engine",
            "%{wks.location}/Artifacts/Bin/" .. outputdir .. "/Velox"
        }
    filter "action:gmake* or action:xcode*"
        libdirs {
            "%{wks.location}/ThirdParty/Customizable_Logger/build/lib",
            "%{wks.location}/ThirdParty/GLFW/build/src",
            "%{wks.location}/Artifacts/Bin/" .. outputdir .. "/Engine",
            "%{wks.location}/Artifacts/Bin/" .. outputdir .. "/Velox"
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
        icon "%{wks.location}/Resources/Branding/TimeEngineIcon.ico"
        defines { "TE_PLATFORM_WINDOWS" }
        links {
            "d3d11",
            "dxgi",
            "d3dcompiler",
            "gdi32",
            "comdlg32",
            "ole32",
            "uuid",
            "dwmapi"
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

-- ========== Dynamic Marketplace Plugins Generator ==========

local rootDir = _MAIN_SCRIPT_DIR or _WORKING_DIR or "."
local allPlugins = {}

for _, p in ipairs(os.matchfiles(rootDir .. "/Plugins/Shipping/*/*.teplugin")) do table.insert(allPlugins, p) end
for _, p in ipairs(os.matchfiles(rootDir .. "/Plugins/Experimental/*/*.teplugin")) do table.insert(allPlugins, p) end

for _, pluginDescriptor in ipairs(allPlugins) do
    local pluginDir = path.getdirectory(pluginDescriptor)
    local pluginName = path.getbasename(pluginDescriptor)

    project(pluginName)
        location(pluginDir)
        kind "SharedLib"
        language "C++"
        cppdialect "C++20"
        staticruntime "off"

        targetdir ("%{wks.location}/Artifacts/Bin/" .. outputdir .. "/TimeEditor/Plugins/" .. pluginName)
        objdir ("%{wks.location}/Artifacts/Bin-Intermediate/" .. outputdir .. "/Plugins/" .. pluginName)

        files {
            pluginDir .. "/Include/**.h",
            pluginDir .. "/Include/**.hpp",
            pluginDir .. "/Source/**.cpp",
            pluginDir .. "/Source/**.h",
            pluginDir .. "/Source/**.hpp",
            pluginDir .. "/src/**.cpp",
            pluginDir .. "/src/**.hpp",
            pluginDir .. "/src/**.h",
            pluginDir .. "/*.cpp",
            pluginDir .. "/*.hpp",
            pluginDir .. "/*.h"
        }

        includedirs {
            pluginDir,
            pluginDir .. "/Include",
            pluginDir .. "/Source",
            pluginDir .. "/src",
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
            "%{IncludeDir.ThirdParty_Logger}",
            "%{IncludeDir.ThirdParty_ImGui}",
            "%{IncludeDir.ThirdParty_GLM}",
            "%{IncludeDir.ThirdParty_GLFW}",
            "%{IncludeDir.ThirdParty_GLAD}",
            "%{IncludeDir.ThirdParty_Velox}",
            "%{IncludeDir.ThirdParty_ForgeUI}"
        }

        externalincludedirs {
            "%{IncludeDir.ThirdParty_Logger}",
            "%{IncludeDir.ThirdParty_ImGui}",
            "%{IncludeDir.ThirdParty_GLM}",
            "%{IncludeDir.ThirdParty_GLFW}",
            "%{IncludeDir.ThirdParty_GLAD}",
            "%{IncludeDir.ThirdParty_Velox}",
            "%{IncludeDir.ThirdParty_ForgeUI}"
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

        links {
            "Engine",
            "Customizable_Logger",
            "Velox",
            "glfw3"
        }

        dependson { "Engine", "Logger", "Velox" }

        filter "action:vs*"
            buildoptions { "/utf-8", "/FS", "/bigobj" }
        filter {}

        filter "system:windows"
            systemversion "latest"
            defines { "TE_PLATFORM_WINDOWS", "TE_BUILD_PLUGIN" }
            links { "ws2_32", "ole32", "uuid", "oleaut32" }

        filter { "system:windows", "action:vs*" }
            postbuildcommands {
                'xcopy /Y /Q "' .. pluginDir .. '\\*.teplugin" "$(OutDir)" > nul'
            }

        filter { "system:windows", "action:gmake*" }
            postbuildcommands {
                '{COPY} "' .. pluginDescriptor .. '" "%{cfg.targetdir}/"'
            }

        filter "configurations:Debug"
            defines { "TE_DEBUG", "TE_EDITOR" }
            symbols "On"

        filter "configurations:Release"
            defines { "TE_RELEASE", "TE_EDITOR" }
            optimize "On"

        filter "configurations:Dist"
            defines { "TE_DIST", "TE_PACKAGED" }
            optimize "On"
end

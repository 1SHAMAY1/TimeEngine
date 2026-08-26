-- ========== Dynamic Standalone Plugin Projects Discovery & Generation ==========

group "Plugins"

local rootDir = _MAIN_SCRIPT_DIR or _WORKING_DIR or "."
local allPlugins = {}
for _, p in ipairs(os.matchfiles(rootDir .. "/Engine/Plugins/*/*.teplugin")) do table.insert(allPlugins, p) end
for _, p in ipairs(os.matchfiles(rootDir .. "/Projects/*/Plugins/*/*.teplugin")) do table.insert(allPlugins, p) end

for _, pluginPath in ipairs(allPlugins) do
    local pluginDir = path.getdirectory(pluginPath)
    local pluginName = path.getbasename(pluginPath)
    
    project (pluginName)
        location (pluginDir)
        kind "SharedLib"
        language "C++"
        cppdialect "C++latest"
        staticruntime "off"

        targetdir ("%{wks.location}/Bin/" .. outputdir .. "/TimeEditor/Plugins/" .. pluginName)
        objdir ("%{wks.location}/Bin-Intermediate/" .. outputdir .. "/Plugins/" .. pluginName)

        files {
            pluginDir .. "/src/**.h",
            pluginDir .. "/src/**.hpp",
            pluginDir .. "/src/**.cpp"
        }

        includedirs {
            pluginDir .. "/src",
            "%{wks.location}/Engine/src",
            "%{wks.location}/Engine/Include",
            "%{wks.location}/Vendor/IMGUI/ImGui",
            "%{wks.location}/Vendor/Customizable_Logger/Include",
            "%{wks.location}/Vendor/GLM",
            "%{wks.location}/Vendor/GLFW/glfw/include",
            "%{wks.location}/Vendor/Velox/include",
            "%{wks.location}/Vendor/Vulkan/include",
            "%{wks.location}/Vendor/volk"
        }

        externalincludedirs {
            "%{wks.location}/Engine/src",
            "%{wks.location}/Engine/Include",
            "%{wks.location}/Vendor/IMGUI/ImGui",
            "%{wks.location}/Vendor/Customizable_Logger/Include",
            "%{wks.location}/Vendor/GLM",
            "%{wks.location}/Vendor/GLFW/glfw/include",
            "%{wks.location}/Vendor/Velox/include",
            "%{wks.location}/Vendor/Vulkan/include",
            "%{wks.location}/Vendor/volk"
        }

        dependson { "Engine" }

        filter "action:vs*"
            libdirs {
                "%{wks.location}/Vendor/Customizable_Logger/build/lib",
                "%{wks.location}/Vendor/Customizable_Logger/build/lib/%{cfg.buildcfg}",
                "%{wks.location}/Vendor/GLFW/build/src",
                "%{wks.location}/Vendor/GLFW/build/src/%{cfg.buildcfg}",
                "%{wks.location}/Bin/" .. outputdir .. "/Engine",
                "%{wks.location}/Bin/" .. outputdir .. "/Velox"
            }
        filter "action:gmake*"
            libdirs {
                "%{wks.location}/Vendor/Customizable_Logger/build/lib",
                "%{wks.location}/Vendor/GLFW/build/src"
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
                "ws2_32",
                "ole32",
                "oleaut32",
                "uuid"
            }

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

        filter { "system:windows", "action:vs*" }
            postbuildcommands {
                'xcopy /Y /Q "$(ProjectDir)*.teplugin" "$(OutDir)" > nul'
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

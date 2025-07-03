workspace "TimeEngine"
    architecture "x64"
    startproject "Sandbox"
    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["Engine"] = "Engine/src"
IncludeDir["Logger"] = "Logger/Customizable_Logger/Include"

-- Logger Project (SharedLib)
project "Logger"
    location "Logger/Customizable_Logger"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.location}/Include/**.hpp",
        "%{prj.location}/Source/**.cpp"
    }

    includedirs {
        "%{prj.location}/Include"
    }

    filter "system:windows"
        systemversion "latest"
        defines {
            "TE_PLATFORM_WINDOWS",
            "TE_LOGGER_BUILD_DLL"  -- export Logger DLL symbols
        }
        postbuildcommands {
            -- Copy Logger DLL and LIB to Sandbox Bin folder
            'xcopy /Y /D /Q "%{wks.location}Bin\\' .. outputdir .. '\\Logger\\%{prj.name}.dll" "%{wks.location}Bin\\' .. outputdir .. '\\Sandbox\\" > nul',
            'xcopy /Y /D /Q "%{wks.location}Bin\\' .. outputdir .. '\\Logger\\%{prj.name}.lib" "%{wks.location}Bin\\' .. outputdir .. '\\Sandbox\\" > nul'
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

-- Engine Project (SharedLib)
project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "Engine/src/**.h",
        "Engine/src/**.cpp"
    }

    includedirs {
        "%{IncludeDir.Engine}",
        "%{IncludeDir.Logger}"
    }

    links {
        "Logger"
    }

    dependson { "Logger" }

    buildoptions { "/utf-8" }

    filter "system:windows"
        systemversion "latest"
        defines {
            "TE_PLATFORM_WINDOWS",
            "TE_BUILD_DLL"  -- export Engine DLL symbols
        }
        postbuildcommands {
            -- Copy Engine DLL and LIB to Sandbox Bin folder
            'xcopy /Y /D /Q "%{wks.location}Bin\\' .. outputdir .. '\\Engine\\%{prj.name}.dll" "%{wks.location}Bin\\' .. outputdir .. '\\Sandbox\\" > nul',
            'xcopy /Y /D /Q "%{wks.location}Bin\\' .. outputdir .. '\\Engine\\%{prj.name}.lib" "%{wks.location}Bin\\' .. outputdir .. '\\Sandbox\\" > nul'
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

-- Sandbox Project (ConsoleApp)
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
        "Sandbox/src/**.cpp"
    }

    includedirs {
        "%{IncludeDir.Engine}",
        "%{IncludeDir.Logger}"
    }

    links {
        "Engine",
        "Logger"
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

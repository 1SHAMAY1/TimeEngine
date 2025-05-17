workspace "TimeEngine"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- TimeEngine Shared Library
project "TimeEngine"
    location "TimeEngine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "%{prj.name}/vendor/Log/include"
    }

    filter "system:windows"
        systemversion "latest"
        defines { "TE_PLATFORM_WINDOWS", "TE_BUILD_DLL", "_WINDLL" }

    postbuildcommands
    {
        ("if not exist \"../Bin/" .. outputdir .. "/Sandbox\" mkdir \"../Bin/" .. outputdir .. "/Sandbox\""),
        ("{COPY} \"%{cfg.buildtarget.relpath}\" \"../Bin/" .. outputdir .. "/Sandbox\"")
    }


    buildoptions{ 
        "/utf-8"
     } 
    filter "configurations:Debug"
        defines "TE_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "TE_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "TE_DIST"
        optimize "On"

-- SideScroller Console App
project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "Sandbox/src/**.h",
        "Sandbox/src/**.cpp"
    }

    includedirs {
        "TimeEngine/vendor/Log/include",
        "TimeEngine/src"
    }

    links {
        "TimeEngine"
    }

    buildoptions
     { 
        "/utf-8"
     } 

    filter "system:windows"
        systemversion "latest"
        defines { "TE_PLATFORM_WINDOWS", "_WINDLL" }

    filter "configurations:Debug"
        defines "TE_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "TE_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "TE_DIST"
        optimize "On"

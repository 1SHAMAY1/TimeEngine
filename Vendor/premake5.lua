-- ========== Vendor Projects ==========

group "Vendor"
    project "Velox"
        location "Velox"
        kind "SharedLib"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"

        targetdir ("%{wks.location}/Bin/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/Bin-Intermediate/" .. outputdir .. "/%{prj.name}")
        disablewarnings { "4267" }

        filter "system:macosx"
            xcodebuildsettings {
                ["LD_DYLIB_INSTALL_NAME"] = "@rpath/libVelox.dylib"
            }
        filter {}

        files {
            "Velox/include/velox/**.h",
            "Velox/src/core/**.cpp",
            "Velox/src/core/**.h",
            "Velox/src/math/**.cpp",
            "Velox/src/math/**.h",
            "Velox/src/physics/**.cpp",
            "Velox/src/physics/**.h",
            "Velox/src/api/**.cpp",
            "Velox/src/api/**.h"
        }

        externalincludedirs {
            "Velox/include",
            "Velox/src",
            "Velox/src/core",
            "Velox/src/math",
            "Velox/src/physics",
            "Velox/src/api"
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
                'xcopy /Y /D /Q "%{wks.location}\\Bin\\' .. outputdir .. '\\Velox\\Velox.dll" "%{wks.location}\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul',
                'xcopy /Y /D /Q "%{wks.location}\\Bin\\' .. outputdir .. '\\Velox\\Velox.lib" "%{wks.location}\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul'
            }

        filter { "system:windows", "action:gmake*" }
            postbuildcommands {
                '{COPY} "%{wks.location}/Bin/' .. outputdir .. '/Velox/Velox.dll" "%{wks.location}/Bin/' .. outputdir .. '/TimeEditor/"',
                '{COPY} "%{wks.location}/Bin/' .. outputdir .. '/Velox/Velox.lib" "%{wks.location}/Bin/' .. outputdir .. '/TimeEditor/"'
            }

        filter "configurations:Debug"
            symbols "On"

        filter "configurations:Release"
            optimize "On"

        filter "configurations:Dist"
            optimize "On"

    project "Logger"
        location "Customizable_Logger"
        kind "Utility"
        language "C++"
        files { }
        includedirs { "%{IncludeDir.Logger}" }

    project "ForgeUI"
        location "ForgeUI"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"
        staticruntime "off"

        targetdir ("%{wks.location}/Bin/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

        files {
            "ForgeUI/include/**.h",
            "ForgeUI/src/**.cpp",
            "ForgeUI/src/**.h"
        }

        includedirs {
            "%{IncludeDir.ForgeUI}",
            "%{IncludeDir.GLAD}",
            "%{IncludeDir.GLFW}"
        }

        externalincludedirs {
            "%{IncludeDir.ForgeUI}",
            "%{IncludeDir.GLAD}",
            "%{IncludeDir.GLFW}"
        }

        filter "system:windows"
            systemversion "latest"
            defines { "WIN32", "_CRT_SECURE_NO_WARNINGS" }

        filter { "system:windows", "action:gmake*" }
            defines { 'posix_memalign(p, a, s)=(*(p) = _aligned_malloc((s), (a)), *(p) ? 0 : 12)' }

        filter "configurations:Debug"
            symbols "On"

        filter "configurations:Release"
            optimize "On"

        filter "configurations:Dist"
            optimize "On"
group ""

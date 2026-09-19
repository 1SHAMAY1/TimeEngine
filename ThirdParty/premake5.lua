-- ========== ThirdParty Projects ==========

group "ThirdParty"
    project "Velox"
        location "Velox"
        kind "SharedLib"
        language "C++"
        cppdialect "C++20"
        staticruntime "off"

        targetdir ("%{wks.location}/Artifacts/Bin/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/Artifacts/Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

        filter "system:windows"
            disablewarnings { "4267" }
        filter {}

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
                'xcopy /Y /D /Q "%{wks.location}\\Artifacts\\Bin\\' .. outputdir .. '\\Velox\\Velox.dll" "%{wks.location}\\Artifacts\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul',
                'xcopy /Y /D /Q "%{wks.location}\\Artifacts\\Bin\\' .. outputdir .. '\\Velox\\Velox.lib" "%{wks.location}\\Artifacts\\Bin\\' .. outputdir .. '\\TimeEditor\\" > nul'
            }

        filter { "system:windows", "action:gmake*" }
            postbuildcommands {
                '{COPY} "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/Velox/Velox.dll" "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/TimeEditor/"',
                '{COPY} "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/Velox/Velox.lib" "%{wks.location}/Artifacts/Bin/' .. outputdir .. '/TimeEditor/"'
            }

        filter "configurations:Debug"
            symbols "On"

        filter "configurations:Release"
            optimize "On"

        filter "configurations:Dist"
            optimize "On"
        filter {}

    project "Logger"
        location "Customizable_Logger"
        kind "Utility"
        language "C++"
        files { }
        includedirs { "%{IncludeDir.ThirdParty_Logger}" }

    project "ForgeUI"
        location "ForgeUI"
        filter "system:macosx"
            kind "None"
        filter "system:not macosx"
            kind "StaticLib"
        filter {}
        language "C++"
        cppdialect "C++20"
        staticruntime "off"

        targetdir ("%{wks.location}/Artifacts/Bin/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/Artifacts/Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

        files {
            "ForgeUI/include/**.h",
            "ForgeUI/src/**.cpp",
            "ForgeUI/src/**.h"
        }

        includedirs {
            "%{IncludeDir.ThirdParty_ForgeUI}",
            "%{IncludeDir.ThirdParty_GLAD}",
            "%{IncludeDir.ThirdParty_GLFW}"
        }

        externalincludedirs {
            "%{IncludeDir.ThirdParty_ForgeUI}",
            "%{IncludeDir.ThirdParty_GLAD}",
            "%{IncludeDir.ThirdParty_GLFW}"
        }

        filter "system:windows"
            systemversion "latest"
            defines { "WIN32", "_CRT_SECURE_NO_WARNINGS" }

        filter "configurations:Debug"
            symbols "On"

        filter "configurations:Release"
            optimize "On"

        filter "configurations:Dist"
            optimize "On"
        filter {}

group ""

-- ==============================================================================
-- TimeEngine Master Premake5 Workspace
-- ==============================================================================

workspace "TimeEngine"
    filter "system:not macosx"
        architecture "x64"
    filter {}
    startproject "TimeEditor"

    configurations { "Debug", "Release", "Dist" }

-- Load Common Setup (Toolsets, Flags, Include Directories, IDE configs)
dofile "Scripts/Premake/Common.lua"

-- Load Codebase Rule Checker ('premake5 check-rules')
dofile "Scripts/Premake/Rules.lua"

-- Include Sub-Projects
include "Vendor"
include "Engine"
include "TimeEditor"

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

        -- Build scripts
        "Premake5.lua",
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

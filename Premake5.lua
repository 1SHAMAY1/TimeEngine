-- ==============================================================================
-- TimeEngine Master Modular Premake5 Workspace
-- ==============================================================================

workspace "TimeEngine"
    cppdialect "C++20"
    filter "system:not macosx"
        architecture "x64"
    filter {}
    startproject "TimeEditor"

    configurations { "Debug", "Release", "Dist" }

-- Load Common Setup (Toolsets, Flags, Output paths, Include Directories)
dofile "Scripts/Premake/Common.lua"

-- Load Codebase Rule Checker ('premake5 check-rules')
dofile "Scripts/Premake/Rules.lua"

-- Include Sub-Projects in Tiered Order
include "ThirdParty"
include "Engine"
include "Plugins"
include "TimeEditor"

-- ========== Docs Solution Project (in-IDE Markdown browsing) ==========
project "Docs"
    location "Docs"
    kind "None"
    language "C++"
    objdir "Artifacts/Bin-Intermediate/Docs"

    files {
        "*.md",
        "LICENSE",
        ".agentsrules",
        ".clang-format",
        ".gitattributes",
        ".gitignore",
        ".gitmodules",
        ".github/**",
        ".agents/**",
        "Docs/**",
        "Premake5.lua",
        "Scripts/**.bat",
        "Scripts/**.sh",
        "Scripts/**.lua",
        "Scripts/**.py"
    }

    vpaths {
        ["Docs/*"] = { "Docs/**", "*.md", "README*", "LICENSE*", "CONTRIBUTING*", "ROADMAP*", "SECURITY*", "llms*" },
        ["Config/*"] = { ".agentsrules", ".clang-format", ".gitattributes", ".gitignore", ".gitmodules" },
        ["Scripts/*"] = { "Scripts/**.bat", "Scripts/**.sh", "Scripts/**.lua", "Scripts/**.py" },
        ["Build/*"] = { "Premake5.lua" },
        ["Github/*"] = { ".github/**" },
        ["Agents/*"] = { ".agents/**" }
    }

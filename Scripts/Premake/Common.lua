-- ========== Common Toolset, Include Directories & Workspace Configuration ==========

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

filter { "system:windows", "action:vs*" }
    local numProcs = tonumber(os.getenv("NUMBER_OF_PROCESSORS")) or 4
    local safeProcs = math.max(1, numProcs - 2)
    buildoptions { "/MP" .. safeProcs, "/FC", "/we26409", "/we26400", "/we26401" }
    disablewarnings { "4251", "4275" }
filter {}

filter { "system:windows", "action:vs*", "configurations:Debug" }
    linkoptions { "/DEBUG:FULL" }
filter {}

filter { "action:gmake*" }
    buildoptions { "-pipe" }
filter {}

filter { "system:macosx", "action:xcode*" }
    xcodebuildsettings {
        ["ONLY_ACTIVE_ARCH"] = "YES"
    }
filter {}

filter { "system:macosx", "action:xcode*", "configurations:Debug" }
    xcodebuildsettings {
        ["DEBUG_INFORMATION_FORMAT"] = "dwarf"
    }
filter {}

-- ========== Include Directories Map ==========

IncludeDir = {}
IncludeDir["Engine"]         = "%{wks.location}/Engine/src"
IncludeDir["Engine_Include"] = "%{wks.location}/Engine/Include"
IncludeDir["Logger"]         = "%{wks.location}/Vendor/Customizable_Logger/Include"
IncludeDir["GLFW"]           = "%{wks.location}/Vendor/GLFW/glfw/include"
IncludeDir["GLAD"]           = "%{wks.location}/Vendor/GLAD/include"
IncludeDir["ImGui"]          = "%{wks.location}/Vendor/IMGUI/ImGui"
IncludeDir["GLM"]            = "%{wks.location}/Vendor/GLM"
IncludeDir["stb_image"]      = "%{wks.location}/Vendor/stb_image"
IncludeDir["Velox"]          = "%{wks.location}/Vendor/Velox/include"
IncludeDir["Vulkan"]         = "%{wks.location}/Vendor/Vulkan/include"
IncludeDir["volk"]           = "%{wks.location}/Vendor/volk"
IncludeDir["OpenGLES"]       = "%{wks.location}/Vendor/OpenGL-Registry/api"
IncludeDir["miniaudio"]      = "%{wks.location}/Vendor/miniaudio"

-- ========== Automated VS Code / IDE Configuration Generator ==========

function GenerateVSCodeConfig()
    local vscodeDir = ".vscode"
    local schemasDir = ".vscode/schemas"
    if not os.isdir(vscodeDir) then os.mkdir(vscodeDir) end
    if not os.isdir(schemasDir) then os.mkdir(schemasDir) end

    -- 1. c_cpp_properties.json
    local cppProps = [[{
  "configurations": [
    {
      "name": "Win32",
      "includePath": [
        "${workspaceFolder}/Engine/Include",
        "${workspaceFolder}/Engine/src",
        "${workspaceFolder}/TimeEditor/src",
        "${workspaceFolder}/TimeEditor/Include",
        "${workspaceFolder}/Vendor/Customizable_Logger/Include",
        "${workspaceFolder}/Vendor/GLFW/glfw/include",
        "${workspaceFolder}/Vendor/GLAD/include",
        "${workspaceFolder}/Vendor/IMGUI/ImGui",
        "${workspaceFolder}/Vendor/GLM",
        "${workspaceFolder}/Vendor/stb_image",
        "${workspaceFolder}/Vendor/Velox/include",
        "${workspaceFolder}/Vendor/Vulkan/include",
        "${workspaceFolder}/Vendor/volk",
        "${workspaceFolder}/Vendor/OpenGL-Registry/api"
      ],
      "defines": [
        "TE_PLATFORM_WINDOWS",
        "TE_DEBUG",
        "TE_BUILD_DLL",
        "IMGUI_ENABLE_DOCKING",
        "IMGUI_ENABLE_VIEWPORTS",
        "_CRT_SECURE_NO_WARNINGS"
      ],
      "cStandard": "c17",
      "cppStandard": "c++20",
      "intelliSenseMode": "windows-msvc-x64",
      "browse": {
        "path": [
          "${workspaceFolder}/Engine/Include",
          "${workspaceFolder}/Engine/src",
          "${workspaceFolder}/TimeEditor",
          "${workspaceFolder}/Vendor"
        ],
        "limitSymbolsToIncludedHeaders": false
      }
    }
  ],
  "version": 4
}]]
    local f = io.open(vscodeDir .. "/c_cpp_properties.json", "w")
    if f then f:write(cppProps) f:close() end

    -- 2. timeengine.code-snippets
    local snippets = [[{
  "TimeEngine Component Skeleton": {
    "prefix": "te-comp",
    "body": [
      "#pragma once",
      "#include \"Core/Scene/TComponent.hpp\"",
      "#include \"Core/Scene/ComponentRegistry.hpp\"",
      "",
      "namespace TE",
      "{",
      "",
      "struct ${1:MyComponent} : public TComponent",
      "{",
      "    GENERATED_BODY(${1:MyComponent})",
      "",
      "    T_PROPERTY(${2:float}, ${3:Value}, \"${4:Display Name}\", ${5:0.0f});",
      "};",
      "",
      "T_REGISTER_COMPONENT(${1:MyComponent}, \"${4:Display Name}\")",
      "T_REGISTER_PROPERTY(${1:MyComponent}, ${2:float}, ${3:Value}, \"${4:Display Name}\")",
      "",
      "} // namespace TE"
    ],
    "description": "Create a new TimeEngine Component with reflection macros"
  },
  "TimeEngine Property Registration": {
    "prefix": "te-prop",
    "body": [
      "T_PROPERTY(${1:float}, ${2:PropertyName}, \"${3:Display Name}\", ${4:0.0f});"
    ],
    "description": "Add a reflected TimeEngine property with default value and display label"
  },
  "TimeEngine Enum Registration": {
    "prefix": "te-enum",
    "body": [
      "enum class ${1:MyEnum}",
      "{",
      "    ${2:OptionA},",
      "    ${3:OptionB}",
      "};",
      "",
      "T_ENUM(${1:MyEnum}, {",
      "    {\"${2:OptionA}\", ${1:MyEnum}::${2:OptionA}},",
      "    {\"${3:OptionB}\", ${1:MyEnum}::${3:OptionB}}",
      "});"
    ],
    "description": "Declare and register an inspectable TimeEngine Enum"
  },
  "TimeEngine Entity Preset": {
    "prefix": "te-preset",
    "body": [
      "T_REGISTER_PRESET(${1:PresetName}, \"${2:Display Name}\", \"${3:Category}\", [](EntityID id, EntityManager* em) {",
      "    ${4:// Add and configure preset components here}",
      "});"
    ],
    "description": "Register an entity creation preset in the Editor Add menu"
  },
  "TimeEngine Core Log": {
    "prefix": "te-log",
    "body": [
      "TE_CORE_INFO(\"${1:Message}\"${2:, args});"
    ],
    "description": "TimeEngine Core log statement"
  },
  "TimeEngine Client Log": {
    "prefix": "te-logclient",
    "body": [
      "TE_CLIENT_INFO(\"${1:Message}\"${2:, args});"
    ],
    "description": "TimeEngine Client log statement"
  },
  "TimeEngine Smart Pointer": {
    "prefix": "te-ref",
    "body": [
      "Ref<${1:Type}> ${2:name} = CreateRef<${1:Type}>(${3:args});"
    ],
    "description": "Declare and allocate a TimeEngine smart pointer (Ref<T>)"
  }
}]]
    f = io.open(vscodeDir .. "/timeengine.code-snippets", "w")
    if f then f:write(snippets) f:close() end

    -- 3. settings.json
    local settings = [[{
  "files.associations": {
    "*.tescene": "yaml",
    "*.teproj": "yaml",
    "*.hpp": "cpp",
    "*.h": "cpp"
  },
  "yaml.schemas": {
    "./.vscode/schemas/tescene.schema.json": ["*.tescene"],
    "./.vscode/schemas/teproj.schema.json": ["*.teproj"]
  },
  "C_Cpp.intelliSenseEngine": "default",
  "C_Cpp.autocomplete": "default",
  "C_Cpp.errorSquiggles": "enabled"
}]]
    f = io.open(vscodeDir .. "/settings.json", "w")
    if f then f:write(settings) f:close() end

    -- 4. tescene.schema.json
    local tesceneSchema = [[{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "TimeEngine Scene Schema (.tescene)",
  "description": "Validation and IntelliSense schema for TimeEngine .tescene scene files",
  "type": "object",
  "required": ["Scene", "Entities"],
  "properties": {
    "Scene": { "type": "string", "description": "Name or filename of the active scene" },
    "Entities": {
      "type": "array",
      "description": "List of alive scene entities and their active components",
      "items": {
        "type": "object",
        "required": ["Entity", "Tag"],
        "properties": {
          "Entity": { "type": "integer", "description": "Unique 64-bit Entity identifier" },
          "Tag": { "type": "string", "description": "Entity display tag name" },
          "TransformComponent": {
            "type": "object",
            "properties": {
              "Position": { "type": "string" },
              "Rotation": { "type": "string" },
              "Scale": { "type": "string" }
            }
          },
          "SpriteComponent": {
            "type": "object",
            "properties": {
              "Color": { "type": "string" },
              "TexturePath": { "type": "string" },
              "TilingFactor": { "type": "string" }
            }
          },
          "BoxComponent": { "type": "object", "properties": { "Size": { "type": "string" }, "Color": { "type": "string" } } },
          "CircleComponent": { "type": "object", "properties": { "Radius": { "type": "string" }, "Thickness": { "type": "string" }, "Fade": { "type": "string" }, "Color": { "type": "string" } } },
          "TriangleComponent": { "type": "object", "properties": { "PointA": { "type": "string" }, "PointB": { "type": "string" }, "PointC": { "type": "string" }, "Color": { "type": "string" } } },
          "LightComponent": { "type": "object", "properties": { "Color": { "type": "string" }, "Intensity": { "type": "string" }, "Radius": { "type": "string" } } },
          "AmbientLightComponent": { "type": "object", "properties": { "Color": { "type": "string" }, "Intensity": { "type": "string" } } },
          "ParallaxComponent": { "type": "object", "properties": { "ParallaxFactor": { "type": "string" } } }
        },
        "additionalProperties": true
      }
    }
  }
}]]
    f = io.open(schemasDir .. "/tescene.schema.json", "w")
    if f then f:write(tesceneSchema) f:close() end

    -- 5. teproj.schema.json
    local teprojSchema = [[{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "TimeEngine Project Schema (.teproj)",
  "description": "Validation and IntelliSense schema for TimeEngine .teproj project files",
  "type": "object",
  "required": ["Project"],
  "properties": {
    "Project": {
      "type": "object",
      "required": ["Name"],
      "properties": {
        "Name": { "type": "string", "description": "Name of the TimeEngine project" },
        "StartScene": { "type": "string", "description": "Path to default startup scene (.tescene)" },
        "AssetDirectory": { "type": "string", "description": "Root assets directory" },
        "ScriptModulePath": { "type": "string", "description": "Path to script module" },
        "ThumbnailPath": { "type": "string", "description": "Project preview icon path" }
      },
      "additionalProperties": true
    }
  }
}]]
    f = io.open(schemasDir .. "/teproj.schema.json", "w")
    if f then f:write(teprojSchema) f:close() end
end

GenerateVSCodeConfig()

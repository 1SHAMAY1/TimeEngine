-- =============================================================================
-- VendorLeakageRule.lua: Premake Code Quality Rule for Strict Vendor Isolation
-- =============================================================================

local M = {}

local vendorRules = {
    {
        name = "Velox Physics",
        id = "TE_VND01",
        headerPatterns = { "#%s*include%s*[<\"]velox/" },
        symbolPatterns = { "%f[%a]vx[A-Z]%w+", "%f[%a]Velox::" },
        allowedPaths = { "Engine/src/Core/Physics/", "Engine/Include/Core/Physics/" },
        msg = "Velox physics header/symbol leaked outside Physics subsystem. Must use TimeEngine Physics abstractions."
    },
    {
        name = "GLFW",
        id = "TE_VND02",
        headerPatterns = { "#%s*include%s*[<\"]GLFW/", "#%s*include%s*[<\"]glfw3%.h" },
        symbolPatterns = { "%f[%a]glfw[A-Z]%w+" },
        allowedPaths = {
            "Engine/src/Window/", "Engine/Include/Window/",
            "Engine/src/Platform/Windows/", "Engine/src/Platform/Linux/", "Engine/src/Platform/Mac/",
            "Engine/src/Utils/Platform/", "Engine/src/Utils/TimeGUI", "Engine/src/Core/Input/Input.cpp"
        },
        msg = "GLFW header/symbol leaked outside Window/Platform backend. Must use TimeEngine Window & Input abstractions."
    },
    {
        name = "OpenGL (GLAD / Desktop GL)",
        id = "TE_VND03",
        headerPatterns = { "#%s*include%s*[<\"]glad/glad.*%.h", "#%s*include%s*[<\"]GL/gl" },
        symbolPatterns = { "%f[%a]glad_%w+", "%f[%a]gl[A-Z]%w+" },
        allowedPaths = {
            "Engine/src/Renderer/OpenGL/", "Engine/Include/Renderer/OpenGL/",
            "Engine/src/Renderer/OpenGLES/", "Engine/Include/Renderer/OpenGLES/",
            "Engine/src/Renderer/Texture.cpp", "Engine/src/Renderer/ShaderLibrary.cpp"
        },
        msg = "OpenGL/GLAD header/symbol leaked outside OpenGL/OpenGLES renderer backend. Must use TimeEngine Renderer2D/RenderCommand abstractions."
    },
    {
        name = "OpenGL ES",
        id = "TE_VND04",
        headerPatterns = { "#%s*include%s*[<\"]GLES", "#%s*include%s*[<\"]glad/glad_gles" },
        symbolPatterns = { "%f[%a]gl[A-Z]%w+" },
        allowedPaths = {
            "Engine/src/Renderer/OpenGLES/", "Engine/Include/Renderer/OpenGLES/",
            "Engine/src/Renderer/OpenGL/", "Engine/Include/Renderer/OpenGL/",
            "Engine/src/Renderer/Texture.cpp", "Engine/src/Renderer/ShaderLibrary.cpp"
        },
        msg = "OpenGL ES header/symbol leaked outside OpenGLES renderer backend. Must use TimeEngine Renderer abstractions."
    },
    {
        name = "Vulkan / Volk",
        id = "TE_VND05",
        headerPatterns = { "#%s*include%s*[<\"]vulkan/", "#%s*include%s*[<\"]volk%.h" },
        symbolPatterns = { "%f[%a]Vk[A-Z]%w+", "%f[%a]vk[A-Z]%w+" },
        allowedPaths = {
            "Engine/src/Renderer/Vulkan/", "Engine/Include/Renderer/Vulkan/"
        },
        msg = "Vulkan/Volk header/symbol leaked outside Vulkan renderer backend. Must use TimeEngine Renderer abstractions."
    },
    {
        name = "DirectX 11",
        id = "TE_VND06",
        headerPatterns = { "#%s*include%s*[<\"]d3d11%.h", "#%s*include%s*[<\"]dxgi%.h", "#%s*include%s*[<\"]d3dcompiler%.h", "#%s*include%s*[<\"]directx/" },
        symbolPatterns = { "%f[%a]ID3D11%w+", "%f[%a]IDXGI%w+", "%f[%a]D3D11_%w+" },
        allowedPaths = {
            "Engine/src/Renderer/DirectX11/", "Engine/Include/Renderer/DirectX11/",
            "Engine/src/Renderer/Texture.cpp", "Engine/src/Window/WindowsWindow.cpp",
            "Engine/src/Platform/Windows/"
        },
        msg = "DirectX 11 header/symbol leaked outside DirectX11 renderer backend. Must use TimeEngine Renderer abstractions."
    },
    {
        name = "Metal",
        id = "TE_VND07",
        headerPatterns = { "#%s*include%s*[<\"]Metal/Metal%.h", "#%s*include%s*[<\"]metal/" },
        symbolPatterns = { "%f[%a]MTL%w+" },
        allowedPaths = {
            "Engine/src/Renderer/Metal/", "Engine/Include/Renderer/Metal/"
        },
        msg = "Metal header/symbol leaked outside Metal renderer backend. Must use TimeEngine Renderer abstractions."
    },
    {
        name = "ImGui",
        id = "TE_VND08",
        headerPatterns = { "#%s*include%s*[<\"]imgui", "#%s*include%s*[<\"]backends/imgui", "#%s*include%s*[<\"]imconfig%.h" },
        symbolPatterns = { "%f[%a]ImGui::", "%f[%a]ImVec2%f[%A]", "%f[%a]ImVec4%f[%A]", "%f[%a]ImDrawList%f[%A]", "%f[%a]ImFont%f[%A]" },
        allowedPaths = {
            "Engine/src/Utils/TimeGUI", "Engine/Include/Utils/TimeGUI",
            "Engine/src/Utils/MathUtils.cpp", "Engine/src/Core/Input/Input.cpp"
        },
        msg = "ImGui header/symbol leaked outside TimeGUI. All UI, Editor, and Panels must use TimeGUI wrapper."
    },
    {
        name = "STB Libraries (stb_image, stb_truetype, stb_image_write, stb_rect_pack)",
        id = "TE_VND09",
        headerPatterns = {
            "#%s*include%s*[<\"]stb_image%.h",
            "#%s*include%s*[<\"]stb_truetype%.h",
            "#%s*include%s*[<\"]stb_image_write%.h",
            "#%s*include%s*[<\"]stb_rect_pack%.h",
            "#%s*include%s*[<\"]stb/"
        },
        symbolPatterns = { "%f[%a]stbi_%w+", "%f[%a]stbtt_%w+", "%f[%a]stbiw_%w+", "%f[%a]stbrp_%w+" },
        allowedPaths = {
            "Engine/src/Core/Asset/AssetManager.cpp",
            "Engine/src/Renderer/Texture.cpp"
        },
        msg = "STB library header/symbol leaked directly. Must use TimeEngine AssetManager / Texture abstractions."
    },
    {
        name = "Customizable_Logger",
        id = "TE_VND10",
        headerPatterns = { "#%s*include%s*[<\"]Customizable_Logger" },
        symbolPatterns = { "%f[%a]Customizable_Logger::" },
        allowedPaths = {
            "Engine/src/Utils/Logger", "Engine/Include/Utils/Logger", "Engine/src/Core/Log.cpp"
        },
        msg = "Customizable_Logger header/symbol leaked directly. Must use TimeEngine Logger and TE_LOG_* macros."
    }
}

local function isPathAllowed(normPath, allowedList)
    for _, allowed in ipairs(allowedList) do
        if normPath:find(allowed, 1, true) then
            return true
        end
    end
    return false
end

function M.CheckFile(filepath)
    local violations = {}
    local normPath = filepath:gsub("\\", "/")

    if normPath:find("/Vendor/") or normPath:match("^Vendor/") then
        return violations
    end

    local f = io.open(filepath, "r")
    if not f then return violations end

    local lineNum = 0
    local inBlockComment = false

    for line in f:lines() do
        lineNum = lineNum + 1
        local trimmed = line:match("^%s*(.-)%s*$")

        -- Block comment tracking
        if inBlockComment then
            if trimmed:find("%*/") then
                inBlockComment = false
                trimmed = trimmed:gsub(".-%*/", "")
            else
                trimmed = ""
            end
        end

        if trimmed:find("/%*") then
            if trimmed:find("%*/") then
                trimmed = trimmed:gsub("/%*.-%*/", "")
            else
                inBlockComment = true
                trimmed = trimmed:gsub("/%*.*", "")
            end
        end

        local code = trimmed:gsub("//.*", "")

        if #code > 0 and not code:find("ALLOW_VENDOR_CALL") and not code:find("NOLINT") then
            for _, rule in ipairs(vendorRules) do
                local matched = false
                local matchedPattern = ""

                -- 1. Check header includes
                if rule.headerPatterns then
                    for _, pat in ipairs(rule.headerPatterns) do
                        if code:find(pat) then
                            matched = true
                            matchedPattern = pat
                            break
                        end
                    end
                end

                -- 2. Check symbol/type usages
                if not matched and rule.symbolPatterns then
                    for _, pat in ipairs(rule.symbolPatterns) do
                        if code:find(pat) then
                            matched = true
                            matchedPattern = pat
                            break
                        end
                    end
                end

                if matched then
                    if not isPathAllowed(normPath, rule.allowedPaths) then
                        table.insert(violations, {
                            file = filepath,
                            line = lineNum,
                            code = rule.id,
                            message = rule.msg,
                            snippet = trimmed
                        })
                    end
                end
            end
        end
    end
    f:close()
    return violations
end

return M

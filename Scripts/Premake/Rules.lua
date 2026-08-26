-- =============================================================================
-- Rules.lua: Master Code Quality, Architecture & Safety Rules for TimeEngine
-- Loads modular code-level rules from Scripts/Premake/Rules/
-- =============================================================================

local rulesDir = _SCRIPT_DIR or "Scripts/Premake"
local rootDir = _MAIN_SCRIPT_DIR or path.getabsolute(rulesDir .. "/../..")

local MemorySafetyRule     = dofile(rulesDir .. "/Rules/EngineTypesAndMemorySafetyRule.lua")
local AISlopRule           = dofile(rulesDir .. "/Rules/AISlopRule.lua")
local CopyrightDetectorRule = dofile(rulesDir .. "/Rules/CopyrightDetectorRule.lua")
local VendorLeakageRule    = dofile(rulesDir .. "/Rules/VendorLeakageRule.lua")

local function getSourceFiles()
    local sourcePatterns = {
        rootDir .. "/Engine/src/**.h",
        rootDir .. "/Engine/src/**.hpp",
        rootDir .. "/Engine/src/**.cpp",
        rootDir .. "/Engine/src/**.inl",
        rootDir .. "/Engine/Include/**.h",
        rootDir .. "/Engine/Include/**.hpp",
        rootDir .. "/Engine/Include/**.inl",
        rootDir .. "/TimeEditor/src/**.h",
        rootDir .. "/TimeEditor/src/**.hpp",
        rootDir .. "/TimeEditor/src/**.cpp",
        rootDir .. "/TimeEditor/Include/**.h",
        rootDir .. "/TimeEditor/Include/**.hpp",
        rootDir .. "/Engine/Plugins/**.h",
        rootDir .. "/Engine/Plugins/**.hpp",
        rootDir .. "/Engine/Plugins/**.cpp",
        rootDir .. "/Projects/**.h",
        rootDir .. "/Projects/**.hpp",
        rootDir .. "/Projects/**.cpp"
    }

    local allFiles = {}
    for _, pattern in ipairs(sourcePatterns) do
        local matches = os.matchfiles(pattern)
        for _, f in ipairs(matches) do
            local norm = path.normalize(f)
            if not norm:find("/Vendor/") and not norm:match("^Vendor/") then
                allFiles[norm] = true
            end
        end
    end
    return allFiles
end

local function printViolations(list)
    for _, v in ipairs(list) do
        local loc = v.file
        if v.line and v.line > 0 then
            loc = string.format("%s(%d)", v.file, v.line)
        end
        print(string.format("%s: error %s: %s", loc, v.code, v.message))
        if v.snippet and #v.snippet > 0 then
            print(string.format("    -> %s", v.snippet))
        end
    end
end

-- =============================================================================
-- Premake Actions
-- =============================================================================

newaction {
    trigger     = "check-memory-safety",
    description = "Scans codebase to enforce TimeEngine smart pointers (TERef/Ref, TEScope/Scope), TEString, and zero raw new/delete",
    execute     = function ()
        print(">>> Running TimeEngine Types & Memory Safety Rule Check...")
        local files = getSourceFiles()
        local total = {}
        for f, _ in pairs(files) do
            local res = MemorySafetyRule.CheckFile(f)
            for _, v in ipairs(res) do table.insert(total, v) end
        end
        printViolations(total)
        print(string.format("Memory Safety Violations: %d", #total))
        if #total > 0 then os.exit(1) end
    end
}

newaction {
    trigger     = "check-ai-slop",
    description = "Scans codebase for LLM prompt echoes, conversational boilerplate, and lazy stubs",
    execute     = function ()
        print(">>> Running TimeEngine AI Slop & Boilerplate Rule Check...")
        local files = getSourceFiles()
        local total = {}
        for f, _ in pairs(files) do
            local res = AISlopRule.CheckFile(f)
            for _, v in ipairs(res) do table.insert(total, v) end
        end
        printViolations(total)
        print(string.format("AI Slop Violations: %d", #total))
        if #total > 0 then os.exit(1) end
    end
}

newaction {
    trigger     = "check-copyright",
    description = "Scans for foreign copyright headers, competitor engine/product mentions (UE5/Fortnite, Unity, Godot, etc.), and license anomalies",
    execute     = function ()
        print(">>> Running TimeEngine Copyright & Competitor Mention Rule Check...")
        local files = getSourceFiles()
        local total = {}
        for f, _ in pairs(files) do
            local res = CopyrightDetectorRule.CheckFile(f)
            for _, v in ipairs(res) do table.insert(total, v) end
        end
        printViolations(total)
        print(string.format("Copyright / Competitor Violations: %d", #total))
        if #total > 0 then os.exit(1) end
    end
}

newaction {
    trigger     = "check-vendor-leakage",
    description = "Scans for raw vendor header leakage (Velox, GLFW, OpenGL, OpenGLES, Vulkan, DX11, Metal, ImGui, stb_image, Logger) outside designated wrappers",
    execute     = function ()
        print(">>> Running TimeEngine Strict Vendor Isolation Rule Check...")
        local files = getSourceFiles()
        local total = {}
        for f, _ in pairs(files) do
            local res = VendorLeakageRule.CheckFile(f)
            for _, v in ipairs(res) do table.insert(total, v) end
        end
        printViolations(total)
        print(string.format("Vendor Leakage Violations: %d", #total))
        if #total > 0 then os.exit(1) end
    end
}

newaction {
    trigger     = "check-rules",
    description = "Runs all TimeEngine code quality, architectural isolation, type safety, and memory rules",
    execute     = function ()
        print("=================================================================")
        print("  TimeEngine Code Quality & Architecture Safety Check")
        print("=================================================================")

        local files = getSourceFiles()
        local totalViolations = {}
        local scannedCount = 0

        for f, _ in pairs(files) do
            scannedCount = scannedCount + 1

            -- 1. TimeEngine Types & Memory Safety (Zero raw new/delete, Smart Pointers, TEString)
            for _, v in ipairs(MemorySafetyRule.CheckFile(f)) do table.insert(totalViolations, v) end

            -- 2. AI Slop & Boilerplate
            for _, v in ipairs(AISlopRule.CheckFile(f)) do table.insert(totalViolations, v) end

            -- 3. Copyright & Competitors
            for _, v in ipairs(CopyrightDetectorRule.CheckFile(f)) do table.insert(totalViolations, v) end

            -- 4. Strict Vendor Isolation (ImGui->TimeGUI, stb->Asset, API->Backend)
            for _, v in ipairs(VendorLeakageRule.CheckFile(f)) do table.insert(totalViolations, v) end
        end

        printViolations(totalViolations)

        print("-----------------------------------------------------------------")
        print(string.format("Scanned %d files. Total code rule violations found: %d", scannedCount, #totalViolations))

        if #totalViolations > 0 then
            print("[RULE CHECK] FAILED: Please resolve all violations listed above.")
            os.exit(1)
        else
            print("[RULE CHECK] PASSED: All code complies with TimeEngine architecture and quality rules!")
        end
    end
}

local function getTestFiles()
    local testPatterns = {
        rootDir .. "/Engine/**.bak",
        rootDir .. "/TimeEditor/**.bak"
    }
    local allFiles = {}
    for _, pattern in ipairs(testPatterns) do
        local matches = os.matchfiles(pattern)
        for _, f in ipairs(matches) do
            local norm = path.normalize(f)
            allFiles[norm] = true
        end
    end
    return allFiles
end

newaction {
    trigger     = "test-rules",
    description = "Runs all TimeEngine rule checks against *.bak test files in Engine/ and TimeEditor/ (does not gate on violations)",
    execute     = function ()
        print("=================================================================")
        print("  TimeEngine Rule Tester (*.bak files only)")
        print("=================================================================")

        local files = getTestFiles()
        local totalViolations = {}
        local scannedCount = 0

        for f, _ in pairs(files) do
            scannedCount = scannedCount + 1

            for _, v in ipairs(MemorySafetyRule.CheckFile(f))     do table.insert(totalViolations, v) end
            for _, v in ipairs(AISlopRule.CheckFile(f))           do table.insert(totalViolations, v) end
            for _, v in ipairs(CopyrightDetectorRule.CheckFile(f)) do table.insert(totalViolations, v) end
            for _, v in ipairs(VendorLeakageRule.CheckFile(f))    do table.insert(totalViolations, v) end
        end

        printViolations(totalViolations)

        print("-----------------------------------------------------------------")
        print(string.format("Scanned %d .bak file(s). Total violations found: %d", scannedCount, #totalViolations))
        print("[TEST] Done - review violations above against expected counts.")
    end
}

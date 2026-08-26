-- =============================================================================
-- CopyrightDetectorRule.lua: Premake Code Quality Rule for Copyright & Competitors
-- =============================================================================

local M = {}

local competitorPatterns = {
    -- Unreal Engine & Epic Games Ecosystem
    { pattern = "unreal%s*engine", id = "TE_CPY01", msg = "Mention of competitor 'Unreal Engine'." },
    { pattern = "unrealengine", id = "TE_CPY01", msg = "Mention of competitor 'UnrealEngine'." },
    { pattern = "%f[%a]ue[45]%f[%A]", id = "TE_CPY02", msg = "Mention of Unreal Engine version ('UE4' / 'UE5')." },
    { pattern = "%f[%a]uefn%f[%A]", id = "TE_CPY02", msg = "Mention of 'UEFN' (Unreal Editor for Fortnite)." },
    { pattern = "%f[%a]fortnite%f[%A]", id = "TE_CPY03", msg = "Mention of competitor product 'Fortnite'." },
    { pattern = "%f[%a]nanite%f[%A]", id = "TE_CPY04", msg = "Mention of Unreal Engine subsystem 'Nanite'." },
    { pattern = "%f[%a]lumen%f[%A]", id = "TE_CPY04", msg = "Mention of Unreal Engine subsystem 'Lumen'." },
    { pattern = "%f[%a]niagara%s*vfx", id = "TE_CPY04", msg = "Mention of Unreal Engine subsystem 'Niagara'." },
    { pattern = "%f[%a]uobject%f[%A]", id = "TE_CPY05", msg = "Unreal Engine C++ framework class signature ('UObject')." },
    { pattern = "%f[%a]uproperty%f[%A]", id = "TE_CPY05", msg = "Unreal Engine C++ macro signature ('UPROPERTY')." },
    { pattern = "%f[%a]ufunction%f[%A]", id = "TE_CPY05", msg = "Unreal Engine C++ macro signature ('UFUNCTION')." },
    { pattern = "%f[%a]aactor%f[%A]", id = "TE_CPY05", msg = "Unreal Engine C++ framework class signature ('AActor')." },
    { pattern = "%f[%a]blueprints?%f[%A]", id = "TE_CPY06", msg = "Unreal Engine visual scripting signature ('Blueprints')." },

    -- Unity Technologies
    { pattern = "unity3d", id = "TE_CPY10", msg = "Mention of competitor 'Unity3D'." },
    { pattern = "unity%s*engine", id = "TE_CPY10", msg = "Mention of competitor 'Unity Engine'." },
    { pattern = "%f[%a]monobehaviour%f[%A]", id = "TE_CPY11", msg = "Unity framework component class ('MonoBehaviour')." },
    { pattern = "%f[%a]scriptableobject%f[%A]", id = "TE_CPY11", msg = "Unity data asset class ('ScriptableObject')." },
    { pattern = "%f[%a]gameobject%f[%A]", id = "TE_CPY11", msg = "Unity component model signature ('GameObject')." },
    { pattern = "%f[%a]prefab%f[%A]", id = "TE_CPY11", msg = "Unity scene asset signature ('Prefab')." },

    -- Godot Engine
    { pattern = "godot%s*engine", id = "TE_CPY20", msg = "Mention of competitor 'Godot Engine'." },
    { pattern = "%f[%a]gdscript%f[%A]", id = "TE_CPY21", msg = "Godot scripting language signature ('GDScript')." },
    { pattern = "%f[%a]gdextension%f[%A]", id = "TE_CPY21", msg = "Godot extension signature ('GDExtension')." },

    -- Other Engines
    { pattern = "frostbite%s*engine", id = "TE_CPY30", msg = "Mention of competitor 'EA Frostbite Engine'." },
    { pattern = "cryengine", id = "TE_CPY31", msg = "Mention of competitor 'CryEngine'." },
    { pattern = "source%s*2%s*engine", id = "TE_CPY32", msg = "Mention of competitor 'Valve Source 2 Engine'." },
    { pattern = "lumberyard", id = "TE_CPY33", msg = "Mention of competitor 'Amazon Lumberyard'." },
    { pattern = "%f[%a]o3de%f[%A]", id = "TE_CPY34", msg = "Mention of competitor 'Open 3D Engine (O3DE)'." }
}

local foreignCopyrights = {
    { pattern = "copyright.*epic%s*games", id = "TE_LIC01", msg = "Foreign copyright header detected: Epic Games." },
    { pattern = "copyright.*unity%s*technologies", id = "TE_LIC02", msg = "Foreign copyright header detected: Unity Technologies." },
    { pattern = "copyright.*valve%s*corporation", id = "TE_LIC03", msg = "Foreign copyright header detected: Valve Corporation." },
    { pattern = "copyright.*crytek", id = "TE_LIC04", msg = "Foreign copyright header detected: Crytek." },
    { pattern = "copyright.*electronic%s*arts", id = "TE_LIC05", msg = "Foreign copyright header detected: Electronic Arts." }
}

local licenseAnomalies = {
    { pattern = "gnu%s+general%s+public%s+license", id = "TE_LIC10", msg = "Incompatible GPL license header detected in TimeEngine source." },
    { pattern = "affero%s+general%s+public%s+license", id = "TE_LIC11", msg = "Incompatible AGPL license header detected in TimeEngine source." },
    { pattern = "all%s+rights%s+reserved%.%s*unauthorized%s+copying", id = "TE_LIC12", msg = "Foreign proprietary closed-source license text detected." }
}

function M.CheckFile(filepath)
    local violations = {}
    local f = io.open(filepath, "r")
    if not f then return violations end

    local lineNum = 0
    for line in f:lines() do
        lineNum = lineNum + 1
        local lower = line:lower()

        for _, rule in ipairs(foreignCopyrights) do
            if lower:find(rule.pattern) then
                table.insert(violations, {
                    file = filepath,
                    line = lineNum,
                    code = rule.id,
                    message = rule.msg,
                    snippet = line:match("^%s*(.-)%s*$")
                })
            end
        end

        for _, rule in ipairs(licenseAnomalies) do
            if lower:find(rule.pattern) then
                table.insert(violations, {
                    file = filepath,
                    line = lineNum,
                    code = rule.id,
                    message = rule.msg,
                    snippet = line:match("^%s*(.-)%s*$")
                })
            end
        end

        for _, rule in ipairs(competitorPatterns) do
            if lower:find(rule.pattern) then
                table.insert(violations, {
                    file = filepath,
                    line = lineNum,
                    code = rule.id,
                    message = rule.msg,
                    snippet = line:match("^%s*(.-)%s*$")
                })
            end
        end
    end
    f:close()
    return violations
end

return M

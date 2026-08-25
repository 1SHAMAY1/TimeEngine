-- =============================================================================
-- EngineTypesAndMemorySafetyRule.lua: TimeEngine Types, Containers, Namespaces & Memory Safety Rule
-- Enforces TimeEngine smart pointers (Ref/TERef, Scope/TEScope), containers (TEArray, TEMap, TESet),
-- TEString, TEFileSystem, zero raw new/delete/malloc/free/raw-ptr allocations,
-- and prevents TE:: / TimeEngine:: namespace qualifiers.
-- =============================================================================

local M = {}

-- Specific, fine-grained exemption tables per rule check
local exemptStdFilesystem = {
    "TEFileSystem.hpp",
    "TEFileSystem.cpp"
}

local exemptStdString = {
    "TEString.hpp",
    "TEString.cpp",
    "TEFileSystem.hpp",
    "TEFileSystem.cpp",
    "PreRequisites.h",
    "GameplayUtils.hpp",
    "GameplayUtils.cpp",
    "EngineTypes.hpp"
}

local exemptContainers = {
    "PreRequisites.h",
    "TEString.hpp",
    "TEString.cpp",
    "TEFileSystem.hpp",
    "TEFileSystem.cpp",
    "GameplayUtils.hpp",
    "GameplayUtils.cpp",
    "EngineTypes.hpp"
}

local exemptNamespace = {
    "PreRequisites.h",
    "GameplayUtils.hpp",
    "EngineTypes.hpp"
}

local exemptSmartPointers = {
    "PreRequisites.h",
    "GameplayUtils.hpp",
    "EngineTypes.hpp"
}

local exemptRawMemory = {}

local function isFileInList(filepath, list)
    local norm = filepath:gsub("\\", "/")
    for _, ex in ipairs(list) do
        if norm:find(ex, 1, true) then
            return true
        end
    end
    return false
end

function M.CheckFile(filepath)
    local violations = {}
    local f = io.open(filepath, "r")
    if not f then
        print("COULD NOT OPEN FILE: " .. tostring(filepath))
        return violations
    end

    local isStringExempt = isFileInList(filepath, exemptStdString)
    local isFilesystemExempt = isFileInList(filepath, exemptStdFilesystem)
    local isContainerExempt = isFileInList(filepath, exemptContainers)
    local isNamespaceExempt = isFileInList(filepath, exemptNamespace)
    local isSmartPointerExempt = isFileInList(filepath, exemptSmartPointers)
    local isRawMemoryExempt = isFileInList(filepath, exemptRawMemory)

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

        -- Strip single line comments
        local code = trimmed:gsub("//.*", "")

        -- Strip string literals so words like "new" or "string" in text are not flagged
        local codeWithoutStrings = code:gsub('"[^"]*"', '""')

        if #codeWithoutStrings > 0 then
            -- 1. ZERO-EXEMPTION: Check for raw 'new' / 'new[]' / raw pointer new allocation
            if not isRawMemoryExempt then
                if codeWithoutStrings:find("%f[%a]new%s+[%a_]") and not codeWithoutStrings:find("operator%s+new") and not codeWithoutStrings:find("new%s*%(") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_MEM01",
                        message = "Direct 'new' is prohibited. Use TimeEngine CreateRef<T>() or CreateScope<T>() instead.",
                        snippet = trimmed
                    })
                end

                -- Raw pointer assignment via new (e.g. Type* ptr = new Type..., int* raw = new int(...))
                if codeWithoutStrings:find("%*%s*[%a_]+%s*=%s*new%s+") or codeWithoutStrings:find("%f[%a]new%s+[%a_]+%s*%(") or codeWithoutStrings:find("%f[%a]new%s+[%a_]+%s*%[") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_MEM01B",
                        message = "Direct raw pointer allocation ('* ptr = new ...') is prohibited. Use TimeEngine CreateRef<T>() or CreateScope<T>().",
                        snippet = trimmed
                    })
                end

                -- 2. ZERO-EXEMPTION: Check for raw 'delete' / 'delete[]'
                if (code:find("%f[%a]delete%s+[%a_]") or code:find("%f[%a]delete%s*%[%s*%]%s*[%a_]") or code:find("%f[%a]delete%s+this")) and not code:find("operator%s+delete") and not code:find("SafeDelete") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_MEM02",
                        message = "Direct 'delete' is prohibited. Rely on TimeEngine smart pointers (Ref<T> / TERef<T>, Scope<T> / TEScope<T>) for automatic memory lifecycle.",
                        snippet = trimmed
                    })
                end

                -- 3. ZERO-EXEMPTION: Check for C-style memory allocations (malloc, calloc, realloc, free)
                if code:find("%f[%a]malloc%s*%(") or code:find("%f[%a]calloc%s*%(") or code:find("%f[%a]realloc%s*%(") or (code:find("%f[%a]free%s*%(") and not code:find("[%a_]+free%s*%(")) then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_MEM06",
                        message = "Direct C-style memory allocation ('malloc', 'calloc', 'realloc', 'free') is prohibited. Use TimeEngine memory systems.",
                        snippet = trimmed
                    })
                end
            end

            -- 4. Check for smart pointers (Must use TERef<T> / TEScope<T> / CreateRef<T> / CreateScope<T>)
            if not isSmartPointerExempt and not code:find("ALLOW_STD_SMART_PTR") then
                if code:find("unique_ptr%s*<") or code:find("shared_ptr%s*<") or code:find("weak_ptr%s*<") or code:find("make_unique%s*<") or code:find("make_shared%s*<") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_MEM07",
                        message = "Direct 'unique_ptr' / 'shared_ptr' / 'make_shared' is prohibited. Use TimeEngine 'TERef<T>' / 'TEScope<T>' / 'CreateRef<T>()' / 'CreateScope<T>()' instead.",
                        snippet = trimmed
                    })
                end
            end

            -- 5. Check for raw containers (Must use TEArray<T>, TEMap<K,V>, TESet<K>)
            if not isContainerExempt and not code:find("ALLOW_RAW_CONTAINER") then
                if code:find("vector%s*<") or code:find("std::vector") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_CONT01",
                        message = "Direct 'std::vector' / 'vector<T>' is prohibited. Use TimeEngine 'TEArray<T>' instead.",
                        snippet = trimmed
                    })
                elseif code:find("%f[%a]unordered_map%s*<") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_CONT02",
                        message = "Direct 'unordered_map<K, V>' is prohibited. Use TimeEngine 'TEMap<K, V>' instead.",
                        snippet = trimmed
                    })
                elseif code:find("%f[%a]unordered_set%s*<") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_CONT03",
                        message = "Direct 'unordered_set<K>' is prohibited. Use TimeEngine 'TESet<K>' instead.",
                        snippet = trimmed
                    })
                end
            end

            -- 6. Check for invalid namespace qualifiers (::TE::, TE::, ::TimeEngine::, TimeEngine::)
            if not isNamespaceExempt and not code:find("ALLOW_NAMESPACE") then
                if code:find("::TE::", 1, true) or code:find("::TimeEngine::", 1, true) then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_NS01",
                        message = "Direct '::TE::' or '::TimeEngine::' prefix is prohibited. Engine classes live in global scope with 'using namespace TE' aliases.",
                        snippet = trimmed
                    })
                elseif code:find("TimeEngine::", 1, true) or code:find("Timeengine::", 1, true) then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_NS02",
                        message = "Namespace 'TimeEngine::' is prohibited. Engine classes are in the global namespace.",
                        snippet = trimmed
                    })
                elseif code:find("%f[%a]TE::[%a_]") and not code:find("namespace%s+TE") and not code:find("using%s+namespace%s+TE") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_NS03",
                        message = "Prefix 'TE::' is prohibited for engine types. Types are accessible globally via standard aliases.",
                        snippet = trimmed
                    })
                end
            end

            -- 7. Check for std::string / string / wstring / u8string / char32_t usage (Must use TEString)
            if not isStringExempt and not code:find("ALLOW_STD_STRING") then
                if code:find("std::string", 1, true) or code:find("std::wstring", 1, true) or code:find("std::u8string", 1, true) or code:find("std::u16string", 1, true) or code:find("std::u32string", 1, true) or code:find("std::basic_string", 1, true) or code:find("#%s*include%s*<string>") then
                    if not code:find("std::string_view", 1, true) or code:find("std::string%s*[%(&<;,=]") then
                        table.insert(violations, {
                            file = filepath,
                            line = lineNum,
                            code = "TE_MEM03",
                            message = "Direct 'std::string' / '<string>' is prohibited. Use TimeEngine 'TEString' instead.",
                            snippet = trimmed
                        })
                    end
                elseif (codeWithoutStrings:find("%f[%a]string%s+[%a_]") or codeWithoutStrings:find("%f[%a]wstring%s+[%a_]") or codeWithoutStrings:find("%f[%a]u8string%s+[%a_]") or codeWithoutStrings:find("%f[%a]string%s+[%a_]+%(") or codeWithoutStrings:find("%f[%a]wstring%s+[%a_]+%(")) and not codeWithoutStrings:find("TEString") and not codeWithoutStrings:find("string_view") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_MEM03B",
                        message = "Direct 'string' variable or function return type is prohibited. Use TimeEngine 'TEString' instead.",
                        snippet = trimmed
                    })
                elseif codeWithoutStrings:find("%f[%a]char32_t%f[%A]") or codeWithoutStrings:find("%f[%a]char16_t%f[%A]") or codeWithoutStrings:find("%f[%a]char8_t%f[%A]") or codeWithoutStrings:find("%f[%a]wchar_t%f[%A]") or codeWithoutStrings:find("%f[%a]char%s+[%a_]+%s*%[%s*%d+%s*%]") or codeWithoutStrings:find("%f[%a]char%s+[%a_]+%s*%[%s*MAX_PATH%s*%]") or codeWithoutStrings:find("unsigned%s+char%s*%*%s*[%a_]+") or codeWithoutStrings:find("%f[%a]uint8_t%s*%*%s*[%a_]+") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_CHAR01",
                        message = "Direct raw 'char[]' buffer, 'unsigned char*', or 'char32_t' / 'char16_t' / 'wchar_t' is prohibited. Use TimeEngine 'TEString' or 'TEArray<uint8_t>' instead.",
                        snippet = trimmed
                    })
                end
            end

            -- 8. Check for filesystem usage (Must use TEFileSystem / TEString path operations)
            if not isFilesystemExempt and not code:find("ALLOW_STD_FILESYSTEM") then
                if code:find("filesystem::") or code:find("std::filesystem", 1, true) or code:find("std::experimental::filesystem", 1, true) or code:find("#%s*include%s*[<%\"]filesystem[>%\"]") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_MEM04",
                        message = "Direct 'filesystem' / '<filesystem>' is prohibited. Use TimeEngine 'TEFileSystem' and 'TEString' path operations instead.",
                        snippet = trimmed
                    })
                end
            end

            -- 9. Check for std::getline usage (Must use TEFileSystem::ForEachLine)
            if not isStringExempt and not code:find("ALLOW_STD_GETLINE") then
                if code:find("getline%s*%(") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_MEM05",
                        message = "Direct 'getline' is prohibited. Use 'TEFileSystem::ForEachLine' or 'TEString::Split' instead.",
                        snippet = trimmed
                    })
                end
            end

            -- 10. Check for legacy std::filesystem methods on TEString (.stem(), .filename(), .parent_path(), etc.)
            if not isFilesystemExempt and not code:find("ALLOW_LEGACY_PATH") then
                if code:find("%.stem%(%)") or code:find("%.filename%(%)") or code:find("%.parent_path%(%)") or code:find("%.has_parent_path%(%)") or code:find("%.extension%(%)") or code:find("%.replace_extension%(") then
                    table.insert(violations, {
                        file = filepath,
                        line = lineNum,
                        code = "TE_STR01",
                        message = "Legacy std::filesystem path methods (.stem(), .filename(), .parent_path(), .has_parent_path(), .extension(), .replace_extension()) on TEString are prohibited. Use GetStem(), GetFilename(), GetParentPath(), HasParentPath(), GetExtension(), ReplaceExtension() instead.",
                        snippet = trimmed
                    })
                end
            end
        end
    end
    f:close()
    return violations
end

return M

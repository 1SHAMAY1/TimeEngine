-- =============================================================================
-- AISlopRule.lua: Premake Code Quality Rule for AI Slop & Boilerplate
-- =============================================================================

local M = {}

local conversationalPatterns = {
    { pattern = "as per user request", id = "TE_SLOP01", msg = "LLM prompt echo detected ('as per user request')." },
    { pattern = "as an ai language model", id = "TE_SLOP02", msg = "LLM identity statement detected ('as an ai language model')." },
    { pattern = "certainly!? here is", id = "TE_SLOP03", msg = "Conversational AI filler detected ('certainly! here is')." },
    { pattern = "here is the (revised|updated|modified) code", id = "TE_SLOP04", msg = "LLM response header detected in source." },
    { pattern = "i hope this helps", id = "TE_SLOP05", msg = "Conversational AI sign-off detected ('i hope this helps')." },
    { pattern = "let me know if you (need|have) any (further|questions)", id = "TE_SLOP06", msg = "Conversational AI sign-off detected in code." },
    { pattern = "key changes made:", id = "TE_SLOP07", msg = "LLM summary header detected in source." },
    { pattern = "given the user context", id = "TE_SLOP08", msg = "LLM internal monologue leaked in source comment." },
    { pattern = "i'll implement", id = "TE_SLOP09", msg = "LLM first-person conversational comment detected." },
    { pattern = "let's use", id = "TE_SLOP10", msg = "LLM conversational phrasing detected in code comment." },
    { pattern = "actually,%s*let's", id = "TE_SLOP11", msg = "LLM conversational monologue detected in code comment." },
    { pattern = "without checking vendor", id = "TE_SLOP12", msg = "LLM internal reasoning comment detected." },
}

local lazyPlaceholders = {
    { pattern = "%.%.%.%s*existing code%s*%.%.%.", id = "TE_LAZY01", msg = "Lazy AI ellipsis placeholder ('... existing code ...')." },
    { pattern = "%.%.%.%s*rest of code%s*%.%.%.", id = "TE_LAZY02", msg = "Lazy AI ellipsis placeholder ('... rest of code ...')." },
    { pattern = "todo:%s*insert your logic here", id = "TE_LAZY03", msg = "Lazy AI implementation placeholder." },
    { pattern = "insert %w+ implementation here", id = "TE_LAZY04", msg = "Generic AI placeholder ('insert ... here')." },
    { pattern = "rest of the code remains unchanged", id = "TE_LAZY05", msg = "Lazy AI omission comment." },
}

local markdownLeaks = {
    { pattern = "^%s*```(cpp|c%+%+|hpp|h|lua|json|yaml|py)%s*$", id = "TE_MDLK01", msg = "Markdown code fence leaked inside source file." },
    { pattern = "^%s*###%s+explanation:?", id = "TE_MDLK02", msg = "Markdown heading leaked inside source file." },
}

function M.CheckFile(filepath)
    local violations = {}
    local ext = path.getextension(filepath):lower()
    local isSource = (ext == ".h" or ext == ".hpp" or ext == ".c" or ext == ".cpp" or ext == ".inl" or ext == ".cs" or ext == ".lua")

    local f = io.open(filepath, "r")
    if not f then return violations end

    local lineNum = 0
    for line in f:lines() do
        lineNum = lineNum + 1
        local lower = line:lower()

        for _, rule in ipairs(conversationalPatterns) do
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

        for _, rule in ipairs(lazyPlaceholders) do
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

        if isSource then
            for _, rule in ipairs(markdownLeaks) do
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
    end
    f:close()
    return violations
end

return M

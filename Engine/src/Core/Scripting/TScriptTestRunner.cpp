#include "Core/PreRequisites.h"
#include "Core/Scripting/TScriptLexer.hpp"
#include "Core/Scripting/TScriptParser.hpp"
#include "Core/Scripting/TScriptInterpreter.hpp"
#include "Core/Scripting/TScriptAsset.hpp"
#include "Core/Log.h"

#include <iostream>
#include <cassert>

void TestLexer() {
    TEString code = R"(
        // Line comment
        # Python comment
        var speed = 300.0
        float health = 100.0
        if (speed > 200.0 and health != 0.0) {
            speed += 10.0
        }
    )";

    TScriptLexer lexer(code);
    auto tokens = lexer.Tokenize();
    assert(!tokens.empty());
    std::cout << "[TestLexer] Passed. Tokens count: " << tokens.size() << std::endl;
}

void TestParserAndInterpreter() {
    TEString code = R"(
        class TestScript : TComponent
            T_REGISTER_PROPERTY(float, speed, 150.0)

            public:
                float health = 100.0
                var score = 0

            on_ready() {
                TE_CORE_INFO("Script initialized!")
                score = 50
            }

            on_update(float dt) {
                health -= 10.0 * dt
                score += 5
            }
    )";

    TScriptAsset asset(code);
    assert(asset.ASTValid);

    TScriptInterpreter interpreter;
    interpreter.LoadProgram(asset.CachedAST);

    interpreter.DispatchReady();
    assert(interpreter.GetGlobals().at("score").AsNumber() == 50.0);

    interpreter.DispatchUpdate(0.1f);
    assert(interpreter.GetGlobals().at("health").AsNumber() == 99.0);
    assert(interpreter.GetGlobals().at("score").AsNumber() == 55.0);

    std::cout << "[TestParserAndInterpreter] Passed!" << std::endl;
}

int main() {
    std::cout << "--- Running TScript Standalone Tests ---" << std::endl;
    TestLexer();
    TestParserAndInterpreter();
    std::cout << "--- All TScript Tests Succeeded ---" << std::endl;
    return 0;
}


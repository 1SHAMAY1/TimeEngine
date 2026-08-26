#include "Core/PreRequisites.h"
#include "Core/Scripting/TScriptParser.hpp"
#include <cmath>

TScriptParser::TScriptParser(TEArray<TScriptToken> tokens) : m_Tokens(std::move(tokens)) {}

const TScriptToken &TScriptParser::Peek() const
{
    if (m_Current >= m_Tokens.size())
        return m_Tokens.back();
    return m_Tokens[m_Current];
}

const TScriptToken &TScriptParser::Previous() const
{
    if (m_Current == 0)
        return m_Tokens[0];
    return m_Tokens[m_Current - 1];
}

bool TScriptParser::IsAtEnd() const { return Peek().type == TScriptTokenType::EndOfFile; }

TScriptToken TScriptParser::Advance()
{
    if (!IsAtEnd())
        m_Current++;
    return Previous();
}

bool TScriptParser::Check(TScriptTokenType type) const
{
    if (IsAtEnd())
        return false;
    return Peek().type == type;
}

bool TScriptParser::Match(TScriptTokenType type)
{
    if (Check(type))
    {
        Advance();
        return true;
    }
    return false;
}

bool TScriptParser::MatchAny(std::initializer_list<TScriptTokenType> types)
{
    for (auto t : types)
    {
        if (Check(t))
        {
            Advance();
            return true;
        }
    }
    return false;
}

TScriptToken TScriptParser::Consume(TScriptTokenType type, const TEString &message)
{
    if (Check(type))
        return Advance();
    m_Error = "Line " + TEString::FromInt(Peek().line) + ": " + message + " (got '" + Peek().lexeme + "')";
    return Peek();
}

void TScriptParser::SkipNewlinesAndSemicolons()
{
    while (Match(TScriptTokenType::Newline) || Match(TScriptTokenType::Semicolon))
    {
    }
}

TScriptProgram TScriptParser::ParseProgram()
{
    TScriptProgram program;
    SkipNewlinesAndSemicolons();

    while (!IsAtEnd() && !HasError())
    {
        if (Check(TScriptTokenType::ClassKw))
        {
            ParseClassDecl(program);
        }
        else
        {
            StmtNode stmt = ParseStatement();
            if (stmt)
                program.topLevel.push_back(stmt);
        }
        SkipNewlinesAndSemicolons();
    }
    return program;
}

void TScriptParser::ParseClassDecl(TScriptProgram &program)
{
    Consume(TScriptTokenType::ClassKw, "Expected 'class'");
    TScriptToken classNameToken = Consume(TScriptTokenType::Identifier, "Expected class name");

    TEArray<TEString> baseClasses;
    if (Match(TScriptTokenType::Colon))
    {
        do
        {
            TScriptToken baseToken = Consume(TScriptTokenType::Identifier, "Expected base class or interface name");
            baseClasses.push_back(baseToken.lexeme);
        } while (Match(TScriptTokenType::Comma));
    }

    auto classNode = CreateRef<ClassDeclNode>(classNameToken.lexeme, baseClasses);
    SkipNewlinesAndSemicolons();

    while (!IsAtEnd() && !Check(TScriptTokenType::ClassKw) && !HasError())
    {
        StmtNode member = ParseMemberDecl();
        if (member)
            classNode->members.push_back(member);
        SkipNewlinesAndSemicolons();
    }

    program.classes.push_back(classNode);
}

StmtNode TScriptParser::ParseMemberDecl()
{
    if (Match(TScriptTokenType::PublicKw))
    {
        Match(TScriptTokenType::Colon);
        return CreateRef<AccessModifierNode>(ScriptAccessModifier::Public);
    }
    if (Match(TScriptTokenType::PrivateKw))
    {
        Match(TScriptTokenType::Colon);
        return CreateRef<AccessModifierNode>(ScriptAccessModifier::Private);
    }
    if (Match(TScriptTokenType::ProtectedKw))
    {
        Match(TScriptTokenType::Colon);
        return CreateRef<AccessModifierNode>(ScriptAccessModifier::Protected);
    }

    if (Check(TScriptTokenType::TRegisterPropertyKw))
    {
        return ParsePropertyDecl();
    }

    if (Check(TScriptTokenType::OnReady) || Check(TScriptTokenType::OnUpdate) || Check(TScriptTokenType::OnCollision) ||
        Check(TScriptTokenType::OnInput) || Check(TScriptTokenType::OnTimer) || Check(TScriptTokenType::OnDestroy) ||
        (Check(TScriptTokenType::Identifier) && Peek().lexeme.find("on_") == 0))
    {
        return ParseEventFunc();
    }

    if (Check(TScriptTokenType::Var) || Check(TScriptTokenType::FloatKw) || Check(TScriptTokenType::IntKw) ||
        Check(TScriptTokenType::StringKw) || Check(TScriptTokenType::BoolKw) || Check(TScriptTokenType::Vec2Kw))
    {
        return ParseVarDecl();
    }

    return ParseStatement();
}

StmtNode TScriptParser::ParsePropertyDecl()
{
    Consume(TScriptTokenType::TRegisterPropertyKw, "Expected T_REGISTER_PROPERTY");
    Consume(TScriptTokenType::LParen, "Expected '(' after T_REGISTER_PROPERTY");

    TScriptToken typeTok = Advance();
    Consume(TScriptTokenType::Comma, "Expected ','");
    TScriptToken nameTok = Consume(TScriptTokenType::Identifier, "Expected property name");
    Consume(TScriptTokenType::Comma, "Expected ','");

    TScriptValue defVal = TScriptValue::Nil();
    if (Check(TScriptTokenType::Number))
    {
        defVal = TScriptValue::Number(std::stod(Advance().lexeme));
    }
    else if (Check(TScriptTokenType::String))
    {
        defVal = TScriptValue::String(Advance().lexeme);
    }
    else if (Check(TScriptTokenType::True) || Check(TScriptTokenType::False))
    {
        defVal = TScriptValue::Bool(Advance().type == TScriptTokenType::True);
    }
    else if (Match(TScriptTokenType::Vec2Kw) || Match(TScriptTokenType::LBrace))
    {
        if (Previous().type == TScriptTokenType::Vec2Kw)
            Consume(TScriptTokenType::LBrace, "Expected '{'");
        double x = std::stod(Consume(TScriptTokenType::Number, "Expected X value").lexeme);
        Consume(TScriptTokenType::Comma, "Expected ','");
        double y = std::stod(Consume(TScriptTokenType::Number, "Expected Y value").lexeme);
        Consume(TScriptTokenType::RBrace, "Expected '}'");
        defVal = TScriptValue::Vec2(TEVector2{static_cast<float>(x), static_cast<float>(y)});
    }

    Consume(TScriptTokenType::RParen, "Expected ')'");
    return CreateRef<PropertyDeclNode>(typeTok.lexeme, nameTok.lexeme, defVal);
}

StmtNode TScriptParser::ParseVarDecl()
{
    TScriptToken typeTok = Advance(); // var, float, int, string, etc.
    TScriptToken nameTok = Consume(TScriptTokenType::Identifier, "Expected variable name");

    ExprNode init = nullptr;
    if (Match(TScriptTokenType::Assign))
    {
        init = ParseExpression();
    }

    return CreateRef<VarDeclNode>(typeTok.lexeme, nameTok.lexeme, init);
}

StmtNode TScriptParser::ParseEventFunc()
{
    TScriptToken nameTok = Advance();
    TEArray<ScriptParam> params;

    Consume(TScriptTokenType::LParen, "Expected '(' after event name");
    if (!Check(TScriptTokenType::RParen))
    {
        do
        {
            TScriptToken firstTok = Advance();

            // Check if there is a second identifier (e.g. "float dt" or "InputActionBinding[] bindings")
            if (Check(TScriptTokenType::LBracket))
            {
                // Array type: Type[] name
                Match(TScriptTokenType::LBracket);
                Consume(TScriptTokenType::RBracket, "Expected ']'");
                TScriptToken paramNameTok = Consume(TScriptTokenType::Identifier, "Expected parameter name");
                params.push_back(ScriptParam{firstTok.lexeme + "[]", paramNameTok.lexeme});
            }
            else if (Check(TScriptTokenType::Identifier))
            {
                // Typed parameter: Type name
                TScriptToken paramNameTok = Advance();
                params.push_back(ScriptParam{firstTok.lexeme, paramNameTok.lexeme});
            }
            else
            {
                // Untyped parameter: name (default type is "var")
                params.push_back(ScriptParam{"var", firstTok.lexeme});
            }
        } while (Match(TScriptTokenType::Comma));
    }
    Consume(TScriptTokenType::RParen, "Expected ')'");

    TERef<BlockNode> body = ParseBlock();
    return CreateRef<EventFuncNode>(nameTok.lexeme, params, body);
}

TERef<BlockNode> TScriptParser::ParseBlock()
{
    Consume(TScriptTokenType::LBrace, "Expected '{' to start block");
    SkipNewlinesAndSemicolons();

    TEArray<StmtNode> stmts;
    while (!Check(TScriptTokenType::RBrace) && !IsAtEnd() && !HasError())
    {
        StmtNode stmt = ParseStatement();
        if (stmt)
            stmts.push_back(stmt);
        SkipNewlinesAndSemicolons();
    }

    Consume(TScriptTokenType::RBrace, "Expected '}' to close block");
    return CreateRef<BlockNode>(stmts);
}

StmtNode TScriptParser::ParseStatement()
{
    if (Check(TScriptTokenType::If))
        return ParseIfStatement();
    if (Check(TScriptTokenType::While))
        return ParseWhileStatement();
    if (Check(TScriptTokenType::For))
        return ParseForStatement();
    if (Check(TScriptTokenType::Return))
        return ParseReturnStatement();
    if (Check(TScriptTokenType::LBrace))
        return ParseBlock();
    if (Check(TScriptTokenType::Var) || Check(TScriptTokenType::FloatKw) || Check(TScriptTokenType::IntKw) ||
        Check(TScriptTokenType::StringKw) || Check(TScriptTokenType::BoolKw) || Check(TScriptTokenType::Vec2Kw))
    {
        return ParseVarDecl();
    }

    return ParseExpressionStatement();
}

StmtNode TScriptParser::ParseIfStatement()
{
    Consume(TScriptTokenType::If, "Expected 'if'");
    Consume(TScriptTokenType::LParen, "Expected '('");
    ExprNode condition = ParseExpression();
    Consume(TScriptTokenType::RParen, "Expected ')'");

    TERef<BlockNode> thenBlock = ParseBlock();
    TERef<BlockNode> elseBlock = nullptr;

    SkipNewlinesAndSemicolons();
    if (Match(TScriptTokenType::Else))
    {
        if (Check(TScriptTokenType::If))
        {
            auto nestedIf = ParseIfStatement();
            elseBlock = CreateRef<BlockNode>(TEArray<StmtNode>{nestedIf});
        }
        else
        {
            elseBlock = ParseBlock();
        }
    }

    return CreateRef<IfNode>(condition, thenBlock, elseBlock);
}

StmtNode TScriptParser::ParseWhileStatement()
{
    Consume(TScriptTokenType::While, "Expected 'while'");
    Consume(TScriptTokenType::LParen, "Expected '('");
    ExprNode condition = ParseExpression();
    Consume(TScriptTokenType::RParen, "Expected ')'");

    TERef<BlockNode> body = ParseBlock();
    return CreateRef<WhileNode>(condition, body);
}

StmtNode TScriptParser::ParseForStatement()
{
    Consume(TScriptTokenType::For, "Expected 'for'");
    Consume(TScriptTokenType::LParen, "Expected '('");

    // Check for range-for: for (var item : collection)
    if ((Check(TScriptTokenType::Var) || Check(TScriptTokenType::FloatKw) || Check(TScriptTokenType::IntKw)) &&
        m_Current + 2 < m_Tokens.size() && m_Tokens[m_Current + 2].type == TScriptTokenType::Colon)
    {
        Advance(); // type
        TScriptToken varTok = Consume(TScriptTokenType::Identifier, "Expected variable name");
        Consume(TScriptTokenType::Colon, "Expected ':'");
        ExprNode coll = ParseExpression();
        Consume(TScriptTokenType::RParen, "Expected ')'");
        TERef<BlockNode> body = ParseBlock();
        return CreateRef<ForRangeNode>(varTok.lexeme, coll, body);
    }

    // Classic C-style for: for (int i = 0; i < 10; i++)
    StmtNode init = nullptr;
    if (!Match(TScriptTokenType::Semicolon))
    {
        init = ParseStatement();
        Match(TScriptTokenType::Semicolon);
    }
    ExprNode cond = nullptr;
    if (!Check(TScriptTokenType::Semicolon))
    {
        cond = ParseExpression();
    }
    Consume(TScriptTokenType::Semicolon, "Expected ';'");
    StmtNode step = nullptr;
    if (!Check(TScriptTokenType::RParen))
    {
        step = ParseExpressionStatement();
    }
    Consume(TScriptTokenType::RParen, "Expected ')'");

    TERef<BlockNode> body = ParseBlock();
    return CreateRef<ForCStyleNode>(init, cond, step, body);
}

StmtNode TScriptParser::ParseReturnStatement()
{
    Consume(TScriptTokenType::Return, "Expected 'return'");
    ExprNode val = nullptr;
    if (!Check(TScriptTokenType::Newline) && !Check(TScriptTokenType::Semicolon) && !Check(TScriptTokenType::RBrace))
    {
        val = ParseExpression();
    }
    return CreateRef<ReturnNode>(val);
}

StmtNode TScriptParser::ParseExpressionStatement()
{
    ExprNode expr = ParseExpression();
    return expr;
}

ExprNode TScriptParser::ParseExpression() { return ParseAssignment(); }

ExprNode TScriptParser::ParseAssignment()
{
    ExprNode expr = ParseLogicOr();

    if (MatchAny({TScriptTokenType::Assign, TScriptTokenType::PlusEq, TScriptTokenType::MinusEq,
                  TScriptTokenType::StarEq, TScriptTokenType::SlashEq}))
    {
        TScriptToken opTok = Previous();
        ExprNode val = ParseAssignment();

        if (expr->type == ASTNodeType::Variable)
        {
            auto varNode = std::static_pointer_cast<VariableNode>(expr);
            return CreateRef<AssignNode>(varNode->name, opTok.lexeme, val);
        }
        else if (expr->type == ASTNodeType::PropertyAccess)
        {
            auto propNode = std::static_pointer_cast<PropertyAccessNode>(expr);
            return CreateRef<PropertySetNode>(propNode->object, propNode->field, opTok.lexeme, val);
        }
        m_Error = "Invalid assignment target.";
    }

    return expr;
}

ExprNode TScriptParser::ParseLogicOr()
{
    ExprNode expr = ParseLogicAnd();

    while (MatchAny({TScriptTokenType::PipePipe, TScriptTokenType::Or}))
    {
        TEString op = Previous().lexeme;
        ExprNode right = ParseLogicAnd();
        expr = CreateRef<BinaryOpNode>(op, expr, right);
    }
    return expr;
}

ExprNode TScriptParser::ParseLogicAnd()
{
    ExprNode expr = ParseEquality();

    while (MatchAny({TScriptTokenType::AndAnd, TScriptTokenType::And}))
    {
        TEString op = Previous().lexeme;
        ExprNode right = ParseEquality();
        expr = CreateRef<BinaryOpNode>(op, expr, right);
    }
    return expr;
}

ExprNode TScriptParser::ParseEquality()
{
    ExprNode expr = ParseComparison();

    while (MatchAny({TScriptTokenType::Eq, TScriptTokenType::Neq}))
    {
        TEString op = Previous().lexeme;
        ExprNode right = ParseComparison();
        expr = CreateRef<BinaryOpNode>(op, expr, right);
    }
    return expr;
}

ExprNode TScriptParser::ParseComparison()
{
    ExprNode expr = ParseTerm();

    while (MatchAny({TScriptTokenType::Lt, TScriptTokenType::Gt, TScriptTokenType::Leq, TScriptTokenType::Geq}))
    {
        TEString op = Previous().lexeme;
        ExprNode right = ParseTerm();
        expr = CreateRef<BinaryOpNode>(op, expr, right);
    }
    return expr;
}

ExprNode TScriptParser::ParseTerm()
{
    ExprNode expr = ParseFactor();

    while (MatchAny({TScriptTokenType::Plus, TScriptTokenType::Minus}))
    {
        TEString op = Previous().lexeme;
        ExprNode right = ParseFactor();
        expr = CreateRef<BinaryOpNode>(op, expr, right);
    }
    return expr;
}

ExprNode TScriptParser::ParseFactor()
{
    ExprNode expr = ParsePower();

    while (MatchAny({TScriptTokenType::Star, TScriptTokenType::Slash, TScriptTokenType::Percent}))
    {
        TEString op = Previous().lexeme;
        ExprNode right = ParsePower();
        expr = CreateRef<BinaryOpNode>(op, expr, right);
    }
    return expr;
}

ExprNode TScriptParser::ParsePower()
{
    ExprNode expr = ParseUnary();

    while (Match(TScriptTokenType::StarStar))
    {
        TEString op = Previous().lexeme;
        ExprNode right = ParseUnary();
        expr = CreateRef<BinaryOpNode>(op, expr, right);
    }
    return expr;
}

ExprNode TScriptParser::ParseUnary()
{
    if (MatchAny({TScriptTokenType::Bang, TScriptTokenType::Not, TScriptTokenType::Minus}))
    {
        TEString op = Previous().lexeme;
        ExprNode operand = ParseUnary();
        return CreateRef<UnaryOpNode>(op, operand);
    }
    return ParseCallOrAccess();
}

ExprNode TScriptParser::ParseCallOrAccess()
{
    ExprNode expr = ParsePrimary();

    while (true)
    {
        if (Match(TScriptTokenType::Dot))
        {
            TScriptToken nameTok = Consume(TScriptTokenType::Identifier, "Expected property or method name after '.'");
            if (Match(TScriptTokenType::LParen))
            {
                // Member method call: obj.method(arg1, arg2)
                TEArray<ExprNode> args;
                if (!Check(TScriptTokenType::RParen))
                {
                    do
                    {
                        args.push_back(ParseExpression());
                    } while (Match(TScriptTokenType::Comma));
                }
                Consume(TScriptTokenType::RParen, "Expected ')' after arguments");
                expr = CreateRef<CallNode>(nameTok.lexeme, args, expr);
            }
            else
            {
                expr = CreateRef<PropertyAccessNode>(expr, nameTok.lexeme);
            }
        }
        else if (Match(TScriptTokenType::LParen))
        {
            // Function call: func(arg1, arg2)
            if (expr->type == ASTNodeType::Variable)
            {
                TEString funcName = std::static_pointer_cast<VariableNode>(expr)->name;
                TEArray<ExprNode> args;
                if (!Check(TScriptTokenType::RParen))
                {
                    do
                    {
                        args.push_back(ParseExpression());
                    } while (Match(TScriptTokenType::Comma));
                }
                Consume(TScriptTokenType::RParen, "Expected ')' after arguments");
                expr = CreateRef<CallNode>(funcName, args);
            }
            else
            {
                m_Error = "Invalid call expression target.";
                break;
            }
        }
        else
        {
            break;
        }
    }

    return expr;
}

ExprNode TScriptParser::ParsePrimary()
{
    if (Match(TScriptTokenType::Number))
    {
        return CreateRef<LiteralNode>(TScriptValue::Number(std::stod(Previous().lexeme)));
    }
    if (Match(TScriptTokenType::String))
    {
        return CreateRef<LiteralNode>(TScriptValue::String(Previous().lexeme));
    }
    if (Match(TScriptTokenType::True))
    {
        return CreateRef<LiteralNode>(TScriptValue::Bool(true));
    }
    if (Match(TScriptTokenType::False))
    {
        return CreateRef<LiteralNode>(TScriptValue::Bool(false));
    }
    if (Match(TScriptTokenType::Nil))
    {
        return CreateRef<LiteralNode>(TScriptValue::Nil());
    }

    if (Match(TScriptTokenType::Identifier))
    {
        return CreateRef<VariableNode>(Previous().lexeme);
    }

    if (Match(TScriptTokenType::LParen))
    {
        ExprNode expr = ParseExpression();
        Consume(TScriptTokenType::RParen, "Expected ')' after expression");
        return expr;
    }

    m_Error = "Unexpected token '" + Peek().lexeme + "'";
    return CreateRef<LiteralNode>(TScriptValue::Nil());
}

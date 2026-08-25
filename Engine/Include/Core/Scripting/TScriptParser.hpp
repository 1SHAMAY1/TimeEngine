#pragma once

#include "Core/Scripting/TScriptLexer.hpp"
#include "Core/Scripting/TScriptAST.hpp"


class TE_API TScriptParser {
public:
    TScriptParser(TEArray<TScriptToken> tokens);

    TScriptProgram ParseProgram();
    bool HasError() const { return !m_Error.empty(); }
    const TEString& GetError() const { return m_Error; }

private:
    const TScriptToken& Peek() const;
    const TScriptToken& Previous() const;
    bool IsAtEnd() const;
    TScriptToken Advance();
    bool Check(TScriptTokenType type) const;
    bool Match(TScriptTokenType type);
    bool MatchAny(std::initializer_list<TScriptTokenType> types);
    TScriptToken Consume(TScriptTokenType type, const TEString& message);
    void SkipNewlinesAndSemicolons();

    void ParseClassDecl(TScriptProgram& program);
    StmtNode ParseMemberDecl();
    StmtNode ParseVarDecl();
    StmtNode ParsePropertyDecl();
    StmtNode ParseEventFunc();
    TERef<BlockNode> ParseBlock();

    StmtNode ParseStatement();
    StmtNode ParseIfStatement();
    StmtNode ParseWhileStatement();
    StmtNode ParseForStatement();
    StmtNode ParseReturnStatement();
    StmtNode ParseExpressionStatement();

    ExprNode ParseExpression();
    ExprNode ParseAssignment();
    ExprNode ParseLogicOr();
    ExprNode ParseLogicAnd();
    ExprNode ParseEquality();
    ExprNode ParseComparison();
    ExprNode ParseTerm();
    ExprNode ParseFactor();
    ExprNode ParsePower();
    ExprNode ParseUnary();
    ExprNode ParseCallOrAccess();
    ExprNode ParsePrimary();

    TEArray<TScriptToken> m_Tokens;
    size_t m_Current = 0;
    TEString m_Error;
};


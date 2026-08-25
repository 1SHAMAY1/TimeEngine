#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"


enum class TScriptTokenType {
    // Literals
    Number, String, True, False, Nil,

    // Keywords
    Identifier, Var, FloatKw, IntKw, StringKw, BoolKw, Vec2Kw,
    If, Else, While, For, Return, And, Or, Not,
    PublicKw, PrivateKw, ProtectedKw, ClassKw,
    TRegisterPropertyKw,

    // Events
    OnReady, OnUpdate, OnCollision, OnInput, OnTimer, OnDestroy,

    // Operators
    Plus, Minus, Star, Slash, Percent, StarStar,
    PlusEq, MinusEq, StarEq, SlashEq,
    Eq, Neq, Lt, Gt, Leq, Geq,
    Assign, AndAnd, PipePipe, Bang,

    // Punctuation
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Dot, Comma, Colon, Semicolon, Newline,

    // Meta
    EndOfFile, Error
};

struct TScriptToken {
    TScriptTokenType type;
    TEString lexeme;
    int line = 1;
    int column = 1;
};

class TE_API TScriptLexer {
public:
    TScriptLexer(TEString source);

    TEArray<TScriptToken> Tokenize();

private:
    char Peek() const;
    char PeekNext() const;
    char Advance();
    bool IsAtEnd() const;
    bool Match(char expected);

    void SkipWhitespaceAndComments();
    TScriptToken MakeToken(TScriptTokenType type);
    TScriptToken MakeErrorToken(const TEString& message);

    TScriptToken LexNumber();
    TScriptToken LexString();
    TScriptToken LexIdentifierOrKeyword();

    TEString m_Source;
    size_t m_Start = 0;
    size_t m_Current = 0;
    int m_Line = 1;
    int m_Column = 1;
};


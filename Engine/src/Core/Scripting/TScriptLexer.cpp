#include "Core/Scripting/TScriptLexer.hpp"
#include "Core/PreRequisites.h"
#include <cctype>

static const TEMap<TEString, TScriptTokenType> s_Keywords = {
    {"var", TScriptTokenType::Var},
    {"float", TScriptTokenType::FloatKw},
    {"int", TScriptTokenType::IntKw},
    {"string", TScriptTokenType::StringKw},
    {"bool", TScriptTokenType::BoolKw},
    {"TEVector2", TScriptTokenType::Vec2Kw},
    {"if", TScriptTokenType::If},
    {"else", TScriptTokenType::Else},
    {"while", TScriptTokenType::While},
    {"for", TScriptTokenType::For},
    {"return", TScriptTokenType::Return},
    {"and", TScriptTokenType::And},
    {"or", TScriptTokenType::Or},
    {"not", TScriptTokenType::Not},
    {"true", TScriptTokenType::True},
    {"false", TScriptTokenType::False},
    {"nil", TScriptTokenType::Nil},
    {"null", TScriptTokenType::Nil},
    {"public", TScriptTokenType::PublicKw},
    {"private", TScriptTokenType::PrivateKw},
    {"protected", TScriptTokenType::ProtectedKw},
    {"class", TScriptTokenType::ClassKw},
    {"T_REGISTER_PROPERTY", TScriptTokenType::TRegisterPropertyKw},
    {"on_ready", TScriptTokenType::OnReady},
    {"on_update", TScriptTokenType::OnUpdate},
    {"on_collision", TScriptTokenType::OnCollision},
    {"on_input", TScriptTokenType::OnInput},
    {"on_timer", TScriptTokenType::OnTimer},
    {"on_destroy", TScriptTokenType::OnDestroy}};

TScriptLexer::TScriptLexer(TEString source) : m_Source(std::move(source)) {}

char TScriptLexer::Peek() const
{
    if (IsAtEnd())
        return '\0';
    return m_Source[m_Current];
}

char TScriptLexer::PeekNext() const
{
    if (m_Current + 1 >= m_Source.size())
        return '\0';
    return m_Source[m_Current + 1];
}

char TScriptLexer::Advance()
{
    char c = m_Source[m_Current++];
    m_Column++;
    return c;
}

bool TScriptLexer::IsAtEnd() const { return m_Current >= m_Source.size(); }

bool TScriptLexer::Match(char expected)
{
    if (IsAtEnd())
        return false;
    if (m_Source[m_Current] != expected)
        return false;
    m_Current++;
    m_Column++;
    return true;
}

void TScriptLexer::SkipWhitespaceAndComments()
{
    while (!IsAtEnd())
    {
        char c = Peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            Advance();
            break;
        case '\n':
            return; // Let token loop handle newline as token
        case '#':   // Python comment
            while (!IsAtEnd() && Peek() != '\n')
                Advance();
            break;
        case '/':
            if (PeekNext() == '/')
            { // C comment
                while (!IsAtEnd() && Peek() != '\n')
                    Advance();
            }
            else if (PeekNext() == '*')
            { // Block comment
                Advance();
                Advance();
                while (!IsAtEnd() && !(Peek() == '*' && PeekNext() == '/'))
                {
                    if (Peek() == '\n')
                    {
                        m_Line++;
                        m_Column = 1;
                    }
                    Advance();
                }
                if (!IsAtEnd())
                {
                    Advance();
                    Advance();
                }
            }
            else
            {
                return;
            }
            break;
        default:
            return;
        }
    }
}

TScriptToken TScriptLexer::MakeToken(TScriptTokenType type)
{
    TEString text = m_Source.substr(m_Start, m_Current - m_Start);
    return TScriptToken{type, text, m_Line, m_Column - static_cast<int>(text.length())};
}

TScriptToken TScriptLexer::MakeErrorToken(const TEString &message)
{
    return TScriptToken{TScriptTokenType::Error, message, m_Line, m_Column};
}

TScriptToken TScriptLexer::LexNumber()
{
    while (std::isdigit(Peek()))
        Advance();
    if (Peek() == '.' && std::isdigit(PeekNext()))
    {
        Advance(); // Eat '.'
        while (std::isdigit(Peek()))
            Advance();
    }
    return MakeToken(TScriptTokenType::Number);
}

TScriptToken TScriptLexer::LexString()
{
    while (!IsAtEnd() && Peek() != '"')
    {
        if (Peek() == '\n')
        {
            m_Line++;
            m_Column = 1;
        }
        Advance();
    }
    if (IsAtEnd())
        return MakeErrorToken("Unterminated string.");
    Advance(); // Eat closing '"'
    TEString text = m_Source.substr(m_Start + 1, m_Current - m_Start - 2);
    return TScriptToken{TScriptTokenType::String, text, m_Line, m_Column - static_cast<int>(text.length())};
}

TScriptToken TScriptLexer::LexIdentifierOrKeyword()
{
    while (std::isalnum(Peek()) || Peek() == '_')
        Advance();
    TEString text = m_Source.substr(m_Start, m_Current - m_Start);

    auto it = s_Keywords.find(text);
    if (it != s_Keywords.end())
    {
        return MakeToken(it->second);
    }
    return MakeToken(TScriptTokenType::Identifier);
}

TEArray<TScriptToken> TScriptLexer::Tokenize()
{
    TEArray<TScriptToken> tokens;

    while (!IsAtEnd())
    {
        SkipWhitespaceAndComments();
        if (IsAtEnd())
            break;

        m_Start = m_Current;
        char c = Advance();

        if (c == '\n')
        {
            tokens.push_back(MakeToken(TScriptTokenType::Newline));
            m_Line++;
            m_Column = 1;
            continue;
        }

        if (std::isdigit(c))
        {
            tokens.push_back(LexNumber());
            continue;
        }

        if (std::isalpha(c) || c == '_')
        {
            tokens.push_back(LexIdentifierOrKeyword());
            continue;
        }

        if (c == '"')
        {
            tokens.push_back(LexString());
            continue;
        }

        switch (c)
        {
        case '(':
            tokens.push_back(MakeToken(TScriptTokenType::LParen));
            break;
        case ')':
            tokens.push_back(MakeToken(TScriptTokenType::RParen));
            break;
        case '{':
            tokens.push_back(MakeToken(TScriptTokenType::LBrace));
            break;
        case '}':
            tokens.push_back(MakeToken(TScriptTokenType::RBrace));
            break;
        case '[':
            tokens.push_back(MakeToken(TScriptTokenType::LBracket));
            break;
        case ']':
            tokens.push_back(MakeToken(TScriptTokenType::RBracket));
            break;
        case ',':
            tokens.push_back(MakeToken(TScriptTokenType::Comma));
            break;
        case '.':
            tokens.push_back(MakeToken(TScriptTokenType::Dot));
            break;
        case ';':
            tokens.push_back(MakeToken(TScriptTokenType::Semicolon));
            break;
        case ':':
            tokens.push_back(MakeToken(TScriptTokenType::Colon));
            break;
        case '%':
            tokens.push_back(MakeToken(TScriptTokenType::Percent));
            break;

        case '+':
            if (Match('='))
                tokens.push_back(MakeToken(TScriptTokenType::PlusEq));
            else
                tokens.push_back(MakeToken(TScriptTokenType::Plus));
            break;
        case '-':
            if (Match('='))
                tokens.push_back(MakeToken(TScriptTokenType::MinusEq));
            else
                tokens.push_back(MakeToken(TScriptTokenType::Minus));
            break;
        case '*':
            if (Match('*'))
                tokens.push_back(MakeToken(TScriptTokenType::StarStar));
            else if (Match('='))
                tokens.push_back(MakeToken(TScriptTokenType::StarEq));
            else
                tokens.push_back(MakeToken(TScriptTokenType::Star));
            break;
        case '/':
            if (Match('='))
                tokens.push_back(MakeToken(TScriptTokenType::SlashEq));
            else
                tokens.push_back(MakeToken(TScriptTokenType::Slash));
            break;

        case '!':
            if (Match('='))
                tokens.push_back(MakeToken(TScriptTokenType::Neq));
            else
                tokens.push_back(MakeToken(TScriptTokenType::Bang));
            break;
        case '=':
            if (Match('='))
                tokens.push_back(MakeToken(TScriptTokenType::Eq));
            else
                tokens.push_back(MakeToken(TScriptTokenType::Assign));
            break;
        case '<':
            if (Match('='))
                tokens.push_back(MakeToken(TScriptTokenType::Leq));
            else
                tokens.push_back(MakeToken(TScriptTokenType::Lt));
            break;
        case '>':
            if (Match('='))
                tokens.push_back(MakeToken(TScriptTokenType::Geq));
            else
                tokens.push_back(MakeToken(TScriptTokenType::Gt));
            break;
        case '&':
            if (Match('&'))
                tokens.push_back(MakeToken(TScriptTokenType::AndAnd));
            else
                tokens.push_back(MakeErrorToken("Unexpected character '&'"));
            break;
        case '|':
            if (Match('|'))
                tokens.push_back(MakeToken(TScriptTokenType::PipePipe));
            else
                tokens.push_back(MakeErrorToken("Unexpected character '|'"));
            break;
        default:
            tokens.push_back(MakeErrorToken(TEString("Unexpected character: ") + c));
            break;
        }
    }

    m_Start = m_Current;
    tokens.push_back(MakeToken(TScriptTokenType::EndOfFile));
    return tokens;
}

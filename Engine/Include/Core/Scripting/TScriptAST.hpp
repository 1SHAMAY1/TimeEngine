#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "Core/Scripting/TScriptValue.hpp"


enum class ASTNodeType {
    Literal,
    Variable,
    Assign,
    PropertyAccess,
    PropertySet,
    BinaryOp,
    UnaryOp,
    Call,
    If,
    While,
    ForRange,
    ForCStyle,
    Return,
    Block,
    VarDecl,
    PropertyDecl,
    AccessModifier,
    EventFunc,
    ClassDecl
};

struct ASTNode {
    ASTNodeType type;

    ASTNode(ASTNodeType t) : type(t) {}
    virtual ~ASTNode() = default;
};

using ExprNode = TERef<ASTNode>;
using StmtNode = TERef<ASTNode>;

struct LiteralNode : public ASTNode {
    TScriptValue value;
    LiteralNode(TScriptValue val) : ASTNode(ASTNodeType::Literal), value(val) {}
};

struct VariableNode : public ASTNode {
    TEString name;
    VariableNode(TEString n) : ASTNode(ASTNodeType::Variable), name(std::move(n)) {}
};

struct AssignNode : public ASTNode {
    TEString name;
    TEString op; // "=", "+=", "-=", "*=", "/="
    ExprNode value;
    AssignNode(TEString n, TEString o, ExprNode val)
        : ASTNode(ASTNodeType::Assign), name(std::move(n)), op(std::move(o)), value(std::move(val)) {}
};

struct PropertyAccessNode : public ASTNode {
    ExprNode object;
    TEString field;
    PropertyAccessNode(ExprNode obj, TEString f)
        : ASTNode(ASTNodeType::PropertyAccess), object(std::move(obj)), field(std::move(f)) {}
};

struct PropertySetNode : public ASTNode {
    ExprNode object;
    TEString field;
    TEString op;
    ExprNode value;
    PropertySetNode(ExprNode obj, TEString f, TEString o, ExprNode val)
        : ASTNode(ASTNodeType::PropertySet), object(std::move(obj)), field(std::move(f)), op(std::move(o)), value(std::move(val)) {}
};

struct BinaryOpNode : public ASTNode {
    TEString op;
    ExprNode left;
    ExprNode right;
    BinaryOpNode(TEString o, ExprNode l, ExprNode r)
        : ASTNode(ASTNodeType::BinaryOp), op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
};

struct UnaryOpNode : public ASTNode {
    TEString op;
    ExprNode operand;
    UnaryOpNode(TEString o, ExprNode expr)
        : ASTNode(ASTNodeType::UnaryOp), op(std::move(o)), operand(std::move(expr)) {}
};

struct CallNode : public ASTNode {
    TEString name;
    ExprNode callee; // Non-null if member call like obj.method()
    TEArray<ExprNode> args;
    CallNode(TEString n, TEArray<ExprNode> a, ExprNode c = nullptr)
        : ASTNode(ASTNodeType::Call), name(std::move(n)), callee(std::move(c)), args(std::move(a)) {}
};

struct BlockNode : public ASTNode {
    TEArray<StmtNode> statements;
    BlockNode(TEArray<StmtNode> stmts = {})
        : ASTNode(ASTNodeType::Block), statements(std::move(stmts)) {}
};

struct IfNode : public ASTNode {
    ExprNode condition;
    TERef<BlockNode> thenBlock;
    TERef<BlockNode> elseBlock;
    IfNode(ExprNode cond, TERef<BlockNode> t, TERef<BlockNode> e = nullptr)
        : ASTNode(ASTNodeType::If), condition(std::move(cond)), thenBlock(std::move(t)), elseBlock(std::move(e)) {}
};

struct WhileNode : public ASTNode {
    ExprNode condition;
    TERef<BlockNode> body;
    WhileNode(ExprNode cond, TERef<BlockNode> b)
        : ASTNode(ASTNodeType::While), condition(std::move(cond)), body(std::move(b)) {}
};

struct ForRangeNode : public ASTNode {
    TEString varName;
    ExprNode collection;
    TERef<BlockNode> body;
    ForRangeNode(TEString var, ExprNode coll, TERef<BlockNode> b)
        : ASTNode(ASTNodeType::ForRange), varName(std::move(var)), collection(std::move(coll)), body(std::move(b)) {}
};

struct ForCStyleNode : public ASTNode {
    StmtNode init;
    ExprNode condition;
    StmtNode step;
    TERef<BlockNode> body;
    ForCStyleNode(StmtNode i, ExprNode c, StmtNode s, TERef<BlockNode> b)
        : ASTNode(ASTNodeType::ForCStyle), init(std::move(i)), condition(std::move(c)), step(std::move(s)), body(std::move(b)) {}
};

struct ReturnNode : public ASTNode {
    ExprNode value;
    ReturnNode(ExprNode val = nullptr) : ASTNode(ASTNodeType::Return), value(std::move(val)) {}
};

struct VarDeclNode : public ASTNode {
    TEString type;
    TEString name;
    ExprNode init;
    VarDeclNode(TEString t, TEString n, ExprNode i = nullptr)
        : ASTNode(ASTNodeType::VarDecl), type(std::move(t)), name(std::move(n)), init(std::move(i)) {}
};

struct PropertyDeclNode : public ASTNode {
    TEString type;
    TEString name;
    TScriptValue defaultValue;
    PropertyDeclNode(TEString t, TEString n, TScriptValue def)
        : ASTNode(ASTNodeType::PropertyDecl), type(std::move(t)), name(std::move(n)), defaultValue(std::move(def)) {}
};

enum class ScriptAccessModifier {
    Public,
    Private,
    Protected
};

struct AccessModifierNode : public ASTNode {
    ScriptAccessModifier modifier;
    AccessModifierNode(ScriptAccessModifier m) : ASTNode(ASTNodeType::AccessModifier), modifier(m) {}
};

struct ScriptParam {
    TEString type;
    TEString name;
};

struct EventFuncNode : public ASTNode {
    TEString eventName;
    TEArray<ScriptParam> params;
    TERef<BlockNode> body;
    AccessModifierNode access{ ScriptAccessModifier::Public };
    EventFuncNode(TEString name, TEArray<ScriptParam> p, TERef<BlockNode> b)
        : ASTNode(ASTNodeType::EventFunc), eventName(std::move(name)), params(std::move(p)), body(std::move(b)) {}
};

struct ClassDeclNode : public ASTNode {
    TEString className;
    TEArray<TEString> baseClasses;
    TEArray<StmtNode> members; // VarDeclNode, PropertyDeclNode, EventFuncNode, AccessModifierNode
    ClassDeclNode(TEString name, TEArray<TEString> bases = {})
        : ASTNode(ASTNodeType::ClassDecl), className(std::move(name)), baseClasses(std::move(bases)) {}
};

struct TScriptProgram {
    TEArray<TERef<ClassDeclNode>> classes;
    TEArray<StmtNode> topLevel; // Fallback top-level statements / functions if outside class
};


#include "Core/PreRequisites.h"
#include "Core/Scripting/TScriptInterpreter.hpp"
#include "Core/Log.h"
#include <cmath>
#include <algorithm>


void TScriptInterpreter::LoadProgram(const TScriptProgram& program) {
    m_Program = program;
    m_Globals.clear();
    m_EventMap.clear();
    m_RuntimeError = "";

    // Process top-level and class elements
    for (const auto& cls : m_Program.classes) {
        for (const auto& member : cls->members) {
            if (member->type == ASTNodeType::PropertyDecl) {
                auto prop = std::static_pointer_cast<PropertyDeclNode>(member);
                m_Globals[prop->name] = prop->defaultValue;
            } else if (member->type == ASTNodeType::VarDecl) {
                auto varDecl = std::static_pointer_cast<VarDeclNode>(member);
                if (varDecl->init) {
                    m_Globals[varDecl->name] = EvalExpr(varDecl->init);
                } else {
                    m_Globals[varDecl->name] = TScriptValue::Nil();
                }
            } else if (member->type == ASTNodeType::EventFunc) {
                auto evt = std::static_pointer_cast<EventFuncNode>(member);
                m_EventMap[evt->eventName] = evt.get();
            }
        }
    }

    for (const auto& stmt : m_Program.topLevel) {
        if (stmt->type == ASTNodeType::EventFunc) {
            auto evt = std::static_pointer_cast<EventFuncNode>(stmt);
            m_EventMap[evt->eventName] = evt.get();
        } else {
            ExecStmt(stmt);
        }
    }
}

void TScriptInterpreter::BindEntity(uint64_t entityID, Ref<Scene> scene) {
    m_BoundEntityID = entityID;
    m_Scene = scene.get();
}

void TScriptInterpreter::DispatchReady() {
    auto it = m_EventMap.find("on_ready");
    if (it != m_EventMap.end() && it->second->body) {
        ExecBlock(*it->second->body);
    }
}

void TScriptInterpreter::DispatchUpdate(float dt) {
    auto it = m_EventMap.find("on_update");
    if (it != m_EventMap.end() && it->second->body) {
        // Expose parameter dt if expected
        if (!it->second->params.empty()) {
            m_Globals[it->second->params[0].name] = TScriptValue::Number(dt);
        }
        ExecBlock(*it->second->body);
    }
}

void TScriptInterpreter::DispatchCollision(uint64_t otherEntityID) {
    auto it = m_EventMap.find("on_collision");
    if (it != m_EventMap.end() && it->second->body) {
        if (!it->second->params.empty()) {
            m_Globals[it->second->params[0].name] = TScriptValue::Entity(otherEntityID);
        }
        ExecBlock(*it->second->body);
    }
}

void TScriptInterpreter::DispatchInput(const TEArray<InputActionBinding>& bindings) {
    auto it = m_EventMap.find("on_input");
    if (it != m_EventMap.end() && it->second->body) {
        ExecBlock(*it->second->body);
    }
}

void TScriptInterpreter::DispatchTimer(const TEString& name) {
    auto it = m_EventMap.find("on_timer");
    if (it != m_EventMap.end() && it->second->body) {
        if (!it->second->params.empty()) {
            m_Globals[it->second->params[0].name] = TScriptValue::String(name);
        }
        ExecBlock(*it->second->body);
    }
}

void TScriptInterpreter::DispatchDestroy() {
    auto it = m_EventMap.find("on_destroy");
    if (it != m_EventMap.end() && it->second->body) {
        ExecBlock(*it->second->body);
    }
}

TEMap<TEString, TScriptValue> TScriptInterpreter::SnapshotVariables() const {
    return m_Globals;
}

void TScriptInterpreter::RestoreVariables(const TEMap<TEString, TScriptValue>& snapshot) {
    m_Globals = snapshot;
}

TScriptValue TScriptInterpreter::CallBuiltin(const TEString& name, const TEArray<TScriptValue>& args) {
    if (name == "TE_CORE_INFO" || name == "TE_CLIENT_LOG" || name == "print") {
        TEString msg;
        for (size_t i = 0; i < args.size(); ++i) {
            msg += args[i].ToString() + (i + 1 < args.size() ? " " : "");
        }
        TE_CORE_INFO("[TScript] {0}", msg);
        return TScriptValue::Nil();
    }
    if (name == "TE_CORE_WARN") {
        TEString msg;
        for (size_t i = 0; i < args.size(); ++i) {
            msg += args[i].ToString() + (i + 1 < args.size() ? " " : "");
        }
        TE_CORE_WARN("[TScript] {0}", msg);
        return TScriptValue::Nil();
    }
    if (name == "TE_CORE_ERROR") {
        TEString msg;
        for (size_t i = 0; i < args.size(); ++i) {
            msg += args[i].ToString() + (i + 1 < args.size() ? " " : "");
        }
        TE_CORE_ERROR("[TScript] {0}", msg);
        return TScriptValue::Nil();
    }
    if (name == "min" && args.size() >= 2) {
        return TScriptValue::Number((std::min)(args[0].AsNumber(), args[1].AsNumber()));
    }
    if (name == "max" && args.size() >= 2) {
        return TScriptValue::Number((std::max)(args[0].AsNumber(), args[1].AsNumber()));
    }
    if (name == "abs" && args.size() >= 1) {
        return TScriptValue::Number(std::abs(args[0].AsNumber()));
    }
    if (name == "sqrt" && args.size() >= 1) {
        return TScriptValue::Number(std::sqrt(args[0].AsNumber()));
    }
    if (name == "lerp" && args.size() >= 3) {
        double a = args[0].AsNumber();
        double b = args[1].AsNumber();
        double t = args[2].AsNumber();
        return TScriptValue::Number(a + (b - a) * t);
    }
    return TScriptValue::Nil();
}

TScriptValue TScriptInterpreter::GetProperty(const ExprNode& target, const TEString& field) {
    TScriptValue base = EvalExpr(target);
    if (base.IsVec2()) {
        TEVector2 v = base.AsVec2();
        if (field == "x") return TScriptValue::Number(v.x);
        if (field == "y") return TScriptValue::Number(v.y);
    }
    return TScriptValue::Nil();
}

void TScriptInterpreter::SetProperty(const ExprNode& target, const TEString& field, const TEString& op, const TScriptValue& val) {
    if (target->type == ASTNodeType::Variable) {
        TEString varName = std::static_pointer_cast<VariableNode>(target)->name;
        TScriptValue base = m_Globals[varName];
        if (base.IsVec2()) {
            TEVector2 v = base.AsVec2();
            float rhs = static_cast<float>(val.AsNumber());
            if (field == "x") {
                if (op == "=") v.x = rhs;
                else if (op == "+=") v.x += rhs;
                else if (op == "-=") v.x -= rhs;
                else if (op == "*=") v.x *= rhs;
                else if (op == "/=") v.x /= rhs;
            } else if (field == "y") {
                if (op == "=") v.y = rhs;
                else if (op == "+=") v.y += rhs;
                else if (op == "-=") v.y -= rhs;
                else if (op == "*=") v.y *= rhs;
                else if (op == "/=") v.y /= rhs;
            }
            m_Globals[varName] = TScriptValue::Vec2(v);
        }
    }
}

TScriptValue TScriptInterpreter::EvalExpr(const ExprNode& node) {
    if (!node) return TScriptValue::Nil();

    switch (node->type) {
        case ASTNodeType::Literal: {
            auto lit = std::static_pointer_cast<LiteralNode>(node);
            return lit->value;
        }
        case ASTNodeType::Variable: {
            auto varNode = std::static_pointer_cast<VariableNode>(node);
            auto it = m_Globals.find(varNode->name);
            if (it != m_Globals.end()) return it->second;
            return TScriptValue::Nil();
        }
        case ASTNodeType::Assign: {
            auto assign = std::static_pointer_cast<AssignNode>(node);
            TScriptValue rhs = EvalExpr(assign->value);
            if (assign->op == "=") {
                m_Globals[assign->name] = rhs;
            } else if (assign->op == "+=") {
                m_Globals[assign->name] = TScriptValue::Number(m_Globals[assign->name].AsNumber() + rhs.AsNumber());
            } else if (assign->op == "-=") {
                m_Globals[assign->name] = TScriptValue::Number(m_Globals[assign->name].AsNumber() - rhs.AsNumber());
            } else if (assign->op == "*=") {
                m_Globals[assign->name] = TScriptValue::Number(m_Globals[assign->name].AsNumber() * rhs.AsNumber());
            } else if (assign->op == "/=") {
                m_Globals[assign->name] = TScriptValue::Number(m_Globals[assign->name].AsNumber() / rhs.AsNumber());
            }
            return m_Globals[assign->name];
        }
        case ASTNodeType::PropertyAccess: {
            auto prop = std::static_pointer_cast<PropertyAccessNode>(node);
            return GetProperty(prop->object, prop->field);
        }
        case ASTNodeType::PropertySet: {
            auto propSet = std::static_pointer_cast<PropertySetNode>(node);
            TScriptValue rhs = EvalExpr(propSet->value);
            SetProperty(propSet->object, propSet->field, propSet->op, rhs);
            return rhs;
        }
        case ASTNodeType::BinaryOp: {
            auto bin = std::static_pointer_cast<BinaryOpNode>(node);
            TScriptValue lhs = EvalExpr(bin->left);
            TScriptValue rhs = EvalExpr(bin->right);

            if (bin->op == "+") {
                if (lhs.IsString() || rhs.IsString()) {
                    return TScriptValue::String(lhs.ToString() + rhs.ToString());
                }
                return TScriptValue::Number(lhs.AsNumber() + rhs.AsNumber());
            }
            if (bin->op == "-") return TScriptValue::Number(lhs.AsNumber() - rhs.AsNumber());
            if (bin->op == "*") return TScriptValue::Number(lhs.AsNumber() * rhs.AsNumber());
            if (bin->op == "/") return TScriptValue::Number(lhs.AsNumber() / rhs.AsNumber());
            if (bin->op == "%") return TScriptValue::Number(std::fmod(lhs.AsNumber(), rhs.AsNumber()));
            if (bin->op == "**") return TScriptValue::Number(std::pow(lhs.AsNumber(), rhs.AsNumber()));
            if (bin->op == "==") return TScriptValue::Bool(lhs == rhs);
            if (bin->op == "!=") return TScriptValue::Bool(lhs != rhs);
            if (bin->op == "<")  return TScriptValue::Bool(lhs.AsNumber() < rhs.AsNumber());
            if (bin->op == ">")  return TScriptValue::Bool(lhs.AsNumber() > rhs.AsNumber());
            if (bin->op == "<=") return TScriptValue::Bool(lhs.AsNumber() <= rhs.AsNumber());
            if (bin->op == ">=") return TScriptValue::Bool(lhs.AsNumber() >= rhs.AsNumber());
            if (bin->op == "&&" || bin->op == "and") return TScriptValue::Bool(lhs.IsTruthy() && rhs.IsTruthy());
            if (bin->op == "||" || bin->op == "or") return TScriptValue::Bool(lhs.IsTruthy() || rhs.IsTruthy());

            return TScriptValue::Nil();
        }
        case ASTNodeType::UnaryOp: {
            auto un = std::static_pointer_cast<UnaryOpNode>(node);
            TScriptValue val = EvalExpr(un->operand);
            if (un->op == "-" && val.IsNumber()) return TScriptValue::Number(-val.AsNumber());
            if ((un->op == "!" || un->op == "not")) return TScriptValue::Bool(!val.IsTruthy());
            return TScriptValue::Nil();
        }
        case ASTNodeType::Call: {
            auto call = std::static_pointer_cast<CallNode>(node);
            TEArray<TScriptValue> args;
            for (const auto& a : call->args) args.push_back(EvalExpr(a));
            return CallBuiltin(call->name, args);
        }
        default:
            break;
    }
    return TScriptValue::Nil();
}

void TScriptInterpreter::ExecStmt(const StmtNode& node) {
    if (!node) return;

    switch (node->type) {
        case ASTNodeType::Block: {
            auto block = std::static_pointer_cast<BlockNode>(node);
            ExecBlock(*block);
            break;
        }
        case ASTNodeType::VarDecl: {
            auto varDecl = std::static_pointer_cast<VarDeclNode>(node);
            if (varDecl->init) {
                m_Globals[varDecl->name] = EvalExpr(varDecl->init);
            } else {
                m_Globals[varDecl->name] = TScriptValue::Nil();
            }
            break;
        }
        case ASTNodeType::If: {
            auto ifNode = std::static_pointer_cast<IfNode>(node);
            if (EvalExpr(ifNode->condition).IsTruthy()) {
                if (ifNode->thenBlock) ExecBlock(*ifNode->thenBlock);
            } else if (ifNode->elseBlock) {
                ExecBlock(*ifNode->elseBlock);
            }
            break;
        }
        case ASTNodeType::While: {
            auto whileNode = std::static_pointer_cast<WhileNode>(node);
            while (EvalExpr(whileNode->condition).IsTruthy()) {
                if (whileNode->body) ExecBlock(*whileNode->body);
            }
            break;
        }
        case ASTNodeType::ForCStyle: {
            auto forNode = std::static_pointer_cast<ForCStyleNode>(node);
            if (forNode->init) ExecStmt(forNode->init);
            while (!forNode->condition || EvalExpr(forNode->condition).IsTruthy()) {
                if (forNode->body) ExecBlock(*forNode->body);
                if (forNode->step) ExecStmt(forNode->step);
            }
            break;
        }
        default:
            EvalExpr(node);
            break;
    }
}

void TScriptInterpreter::ExecBlock(const BlockNode& block) {
    for (const auto& stmt : block.statements) {
        ExecStmt(stmt);
    }
}


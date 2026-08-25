#pragma once

#include "Core/PreRequisites.h"
#include "Core/Scripting/TScriptAST.hpp"
#include "Core/Scripting/TScriptLexer.hpp"
#include "Core/Scripting/TScriptParser.hpp"
#include "Core/Scripting/TScriptValue.hpp"
#include "Utils/TimeGUI.hpp"
#include <functional>


struct SpriteScriptContext
{
    TimeGUI::TimeGUIDrawList DrawList;
    TEVector2 Origin = TEVector2(0.0f, 0.0f);
    TEVector2 CanvasSize = TEVector2(128.0f, 128.0f);
    float Time = 0.0f;
    float DeltaTime = 0.0f;
    int FrameIndex = 0;
    int TotalFrames = 8;
    float FrameProgress = 0.0f;
    int PixelWidth = 32;
    int PixelHeight = 32;
    TEArray<TEVector4> PixelBuffer;
};

class SpriteScriptRuntime
{
public:
    using NativeFn = std::function<TScriptValue(const TEArray<TScriptValue> &)>;

    SpriteScriptRuntime();
    ~SpriteScriptRuntime() = default;

    bool Compile(const TEString &sourceCode);
    bool IsValid() const { return m_IsValid; }
    const TEString &GetCompileError() const { return m_CompileError; }
    const TEString &GetRuntimeError() const { return m_RuntimeError; }

    void Execute(TimeGUI::TimeGUIDrawList dl, const TEVector2 &origin, const TEVector2 &canvasSize, float time,
                 float dt, int frameIndex, int totalFrames, int pixelWidth = 32, int pixelHeight = 32);

    void RegisterFunction(const TEString &name, NativeFn fn);
    const TEMap<TEString, TScriptValue> &GetGlobals() const { return m_Globals; }
    const TEArray<TEVector4> &GetPixelBuffer() const { return m_Context.PixelBuffer; }

    static TEString GetApiDocumentationMarkdown();

private:
    void RegisterBuiltins();
    void ResetGlobals();

    TScriptValue EvalExpr(const ExprNode &node);
    void ExecStmt(const StmtNode &node);
    void ExecBlock(const BlockNode &block);
    TScriptValue CallBuiltin(const TEString &name, const TEArray<TScriptValue> &args);

    // Helpers
    unsigned int ValueToColorU32(const TScriptValue &val, float alpha = 1.0f);
    TEVector4 ValueToColorVec4(const TScriptValue &val);
    TEVector2 ToScreenPos(float x, float y) const;
    TEVector2 ToScreenSize(float w, float h) const;

private:
    TScriptProgram m_Program;
    bool m_IsValid = false;
    TEString m_SourceCode;
    TEString m_CompileError;
    TEString m_RuntimeError;

    SpriteScriptContext m_Context;
    TEMap<TEString, TScriptValue> m_Globals;
    TEMap<TEString, NativeFn> m_NativeFunctions;
    TEMap<TEString, EventFuncNode *> m_EventMap;
};

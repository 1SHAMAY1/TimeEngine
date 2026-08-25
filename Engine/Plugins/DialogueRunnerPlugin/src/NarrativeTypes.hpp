#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "Core/Core.h"
#include "Utils/MathUtils.hpp"
#include <cstdint>



enum class NarrativeNodeType : uint8_t
{
    Entry = 0,
    Dialogue,
    Choice,
    Condition,
    Action,
    Divert,
    Exit
};

enum class PinType : uint8_t
{
    Flow = 0,
    Condition,
    ChoiceOption,
    Value
};

enum class QuestStatus : uint8_t
{
    NotStarted = 0,
    Active,
    Completed,
    Failed
};

enum class ComparisonOp : uint8_t
{
    Equal = 0,
    NotEqual,
    GreaterThan,
    LessThan,
    GreaterEqual,
    LessEqual
};

enum class MutationOp : uint8_t
{
    Set = 0,
    Add,
    Subtract,
    Multiply,
    Toggle
};

struct StoryChoice
{
    int Index = 0;
    TEString Text;
    TEString TargetKnot;
    uint64_t TargetNodeID = 0;
    bool bIsEnabled = true;
    TEString ConditionExpression;
};

struct QuestObjective
{
    TEString ID;
    TEString Description;
    bool bIsCompleted = false;
    int CurrentProgress = 0;
    int TargetProgress = 1;
};

struct QuestData
{
    TEString ID;
    TEString Title;
    TEString Description;
    QuestStatus Status = QuestStatus::NotStarted;
    TEArray<QuestObjective> Objectives;
};


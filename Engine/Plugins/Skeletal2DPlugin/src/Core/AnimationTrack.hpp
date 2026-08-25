#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Utils/TEString.hpp"
#include "Renderer/TEColor.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/Texture.hpp"

namespace Skeletal2D {

enum class EasingCurve
{
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Stepped
};

inline float EvaluateCurve(float t, EasingCurve curve)
{
    switch (curve)
    {
    case EasingCurve::EaseIn:
        return t * t;
    case EasingCurve::EaseOut:
        return t * (2.0f - t);
    case EasingCurve::EaseInOut:
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    case EasingCurve::Stepped:
        return 0.0f;
    case EasingCurve::Linear:
    default:
        return t;
    }
}

template <typename T>
struct Keyframe
{
    float Time = 0.0f;
    T Value;
    EasingCurve Curve = EasingCurve::Linear;
};

struct BoneTimeline
{
    int BoneIndex = -1;
    TEString BoneName;

    TEArray<Keyframe<glm::vec2>> TranslationKeys;
    TEArray<Keyframe<float>> RotationKeys; // Radians
    TEArray<Keyframe<glm::vec2>> ScaleKeys;

    void Evaluate(float time, glm::vec2& outPos, float& outRot, glm::vec2& outScale,
                  const glm::vec2& defaultPos, float defaultRot, const glm::vec2& defaultScale) const
    {
        outPos = EvaluateVector2(TranslationKeys, time, defaultPos);
        outRot = EvaluateFloat(RotationKeys, time, defaultRot);
        outScale = EvaluateVector2(ScaleKeys, time, defaultScale);
    }

private:
    static float EvaluateFloat(const TEArray<Keyframe<float>>& keys, float time, float defaultVal)
    {
        if (keys.empty()) return defaultVal;
        if (keys.size() == 1 || time <= keys.front().Time) return keys.front().Value;
        if (time >= keys.back().Time) return keys.back().Value;

        for (size_t i = 0; i < keys.size() - 1; ++i)
        {
            if (time >= keys[i].Time && time <= keys[i + 1].Time)
            {
                float span = keys[i + 1].Time - keys[i].Time;
                float t = (span > 0.0001f) ? (time - keys[i].Time) / span : 0.0f;
                float curvedT = EvaluateCurve(t, keys[i].Curve);
                return glm::mix(keys[i].Value, keys[i + 1].Value, curvedT);
            }
        }
        return defaultVal;
    }

    static glm::vec2 EvaluateVector2(const TEArray<Keyframe<glm::vec2>>& keys, float time, const glm::vec2& defaultVal)
    {
        if (keys.empty()) return defaultVal;
        if (keys.size() == 1 || time <= keys.front().Time) return keys.front().Value;
        if (time >= keys.back().Time) return keys.back().Value;

        for (size_t i = 0; i < keys.size() - 1; ++i)
        {
            if (time >= keys[i].Time && time <= keys[i + 1].Time)
            {
                float span = keys[i + 1].Time - keys[i].Time;
                float t = (span > 0.0001f) ? (time - keys[i].Time) / span : 0.0f;
                float curvedT = EvaluateCurve(t, keys[i].Curve);
                return glm::mix(keys[i].Value, keys[i + 1].Value, curvedT);
            }
        }
        return defaultVal;
    }
};

struct EventKeyframe
{
    float Time = 0.0f;
    TEString EventName;
    int IntValue = 0;
    float FloatValue = 0.0f;
    TEString StringValue;
};

struct AnimationClip
{
    TEString Name;
    float Duration = 1.0f;
    bool IsLooping = true;

    TEArray<BoneTimeline> BoneTimelines;
    TEArray<EventKeyframe> EventKeys;
};

struct TrackEntry
{
    int TrackIndex = 0;
    TERef<AnimationClip> CurrentClip = nullptr;
    TERef<AnimationClip> PreviousClip = nullptr;

    float TrackTime = 0.0f;
    float TimeScale = 1.0f;
    bool Loop = true;

    float MixTime = 0.0f;
    float MixDuration = 0.0f; // Cross-fade duration
    float Alpha = 1.0f;       // Overall track blend weight

    bool IsPlaying = false;
};

} // namespace Skeletal2D

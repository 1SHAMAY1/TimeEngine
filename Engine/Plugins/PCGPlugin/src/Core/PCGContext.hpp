#pragma once

#include "Core/PCGPointData.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <atomic>
#include <glm/glm.hpp>
#include <random>

class Scene;

struct PCGExecutionContext
{
    uint32_t Seed = 1337;
    glm::vec3 BoundsMin = {-50.0f, -50.0f, 0.0f};
    glm::vec3 BoundsMax = {50.0f, 50.0f, 0.0f};
    glm::vec3 Origin = {0.0f, 0.0f, 0.0f};

    Scene *TargetScene = nullptr;
    uint32_t TargetEntityID = 0;

    std::atomic<bool> Cancelled{false};

    // Cache of output point datasets per Pin ID
    TEMap<uint64_t, PCGPointDataRef> PinDataCache;

    PCGExecutionContext() = default;
    PCGExecutionContext(uint32_t seed, const glm::vec3 &bMin, const glm::vec3 &bMax, Scene *scene = nullptr)
        : Seed(seed), BoundsMin(bMin), BoundsMax(bMax), TargetScene(scene)
    {
    }

    void SetPinData(uint64_t pinId, PCGPointDataRef data) { PinDataCache[pinId] = data; }

    PCGPointDataRef GetPinData(uint64_t pinId) const
    {
        auto it = PinDataCache.Find(pinId);
        if (it != nullptr)
            return *it;
        return nullptr;
    }

    void ClearCache() { PinDataCache.Clear(); }

    float RandomFloat(float minVal = 0.0f, float maxVal = 1.0f)
    {
        static thread_local std::mt19937 gen(Seed);
        std::uniform_real_distribution<float> dist(minVal, maxVal);
        return dist(gen);
    }

    int RandomInt(int minVal, int maxVal)
    {
        static thread_local std::mt19937 gen(Seed);
        std::uniform_int_distribution<int> dist(minVal, maxVal);
        return dist(gen);
    }
};

#pragma once
#include "GameFrameWork/GameplayUtils.hpp"
#include "ParticleTypes.hpp"
#include <cstdint>
#include <vector>

class ParticlePool
{
public:
    ParticlePool(size_t initialCapacity = 2000) { Resize(initialCapacity); }

    void Resize(size_t newCapacity)
    {
        m_Capacity = newCapacity;
        m_Particles.resize(m_Capacity);
        m_FirstFreeIndex = 0;
        m_ActiveCount = 0;
    }

    Particle *Allocate()
    {
        if (m_ActiveCount >= m_Capacity)
            return nullptr;

        // Search starting from m_FirstFreeIndex
        for (size_t i = 0; i < m_Capacity; ++i)
        {
            size_t idx = (m_FirstFreeIndex + i) % m_Capacity;
            if (!m_Particles[idx].Active)
            {
                m_FirstFreeIndex = (idx + 1) % m_Capacity;
                m_Particles[idx].Active = true;
                ++m_ActiveCount;
                return &m_Particles[idx];
            }
        }
        return nullptr;
    }

    void Free(Particle *particle)
    {
        if (particle && particle->Active)
        {
            particle->Active = false;
            if (m_ActiveCount > 0)
                --m_ActiveCount;
        }
    }

    void Clear()
    {
        for (auto &p : m_Particles)
            p.Active = false;
        m_ActiveCount = 0;
        m_FirstFreeIndex = 0;
    }

    TEArray<Particle> &GetParticles() { return m_Particles; }
    const TEArray<Particle> &GetParticles() const { return m_Particles; }

    size_t GetCapacity() const { return m_Capacity; }
    size_t GetActiveCount() const { return m_ActiveCount; }

private:
    TEArray<Particle> m_Particles;
    size_t m_Capacity = 0;
    size_t m_ActiveCount = 0;
    size_t m_FirstFreeIndex = 0;
};

#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Layer.hpp"


// Transparent iterator yielding Layer* for seamless compatibility with engine loops
template <typename IterType> class LayerIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Layer *;
    using difference_type = std::ptrdiff_t;
    using pointer = Layer **;
    using reference = Layer *&;

    LayerIterator(IterType iter) : m_Iter(iter) {}
    Layer *operator*() const { return m_Iter->get(); }
    Layer *operator->() const { return m_Iter->get(); }
    LayerIterator &operator++()
    {
        ++m_Iter;
        return *this;
    }
    LayerIterator operator++(int)
    {
        LayerIterator tmp = *this;
        ++m_Iter;
        return tmp;
    }
    LayerIterator &operator--()
    {
        --m_Iter;
        return *this;
    }
    LayerIterator operator--(int)
    {
        LayerIterator tmp = *this;
        --m_Iter;
        return tmp;
    }
    bool operator==(const LayerIterator &other) const { return m_Iter == other.m_Iter; }
    bool operator!=(const LayerIterator &other) const { return m_Iter != other.m_Iter; }

private:
    IterType m_Iter;
};

class TE_API LayerStack
{
public:
    LayerStack();
    ~LayerStack();

    void PushLayer(TERef<Layer> layer);
    void PushOverlay(TERef<Layer> overlay);
    void PopLayer(TERef<Layer> layer);
    void PopOverlay(TERef<Layer> overlay);

    // Deferred removal methods to prevent crashes during iteration
    void MarkLayerForRemoval(TERef<Layer> layer);
    void MarkOverlayForRemoval(TERef<Layer> overlay);
    void ProcessDeferredRemovals();
    void Clear();

    auto begin() { return LayerIterator(m_Layers.begin()); }
    auto end() { return LayerIterator(m_Layers.end()); }
    auto begin() const { return LayerIterator(m_Layers.begin()); }
    auto end() const { return LayerIterator(m_Layers.end()); }

    const TEArray<TERef<Layer>> &GetLayers() const { return m_Layers; }

private:
    TEArray<TERef<Layer>> m_Layers;
    TEArray<TERef<Layer>> m_LayersToRemove;
    unsigned int m_LayerInsertIndex = 0;
};


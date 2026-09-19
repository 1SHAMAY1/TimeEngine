#pragma once
#include "EngineTypes/TEArray.hpp"
#include <unordered_set>
#include <set>
#include <utility>
#include <functional>

// ==========================================
// TESet - Engine Hash Set Template
// ==========================================
// ==========================================
template <typename KeyType, typename Hasher = std::hash<KeyType>> class TESet
{
private:
    std::unordered_set<KeyType, Hasher> m_Set;

public:
    using key_type = KeyType;
    using value_type = KeyType;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using hasher = Hasher;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = typename std::unordered_set<KeyType, Hasher>::iterator;
    using const_iterator = typename std::unordered_set<KeyType, Hasher>::const_iterator;

    TESet() = default;
    TESet(std::initializer_list<KeyType> initList) : m_Set(initList) {}

    TESet(const std::unordered_set<KeyType, Hasher> &other) : m_Set(other) {}
    TESet(std::unordered_set<KeyType, Hasher> &&other) : m_Set(std::move(other)) {}
    TESet &operator=(const std::unordered_set<KeyType, Hasher> &other)
    {
        m_Set = other;
        return *this;
    }
    TESet &operator=(std::unordered_set<KeyType, Hasher> &&other)
    {
        m_Set = std::move(other);
        return *this;
    }
    operator std::unordered_set<KeyType, Hasher> &() { return m_Set; }
    operator const std::unordered_set<KeyType, Hasher> &() const { return m_Set; }

    size_t Num() const { return m_Set.size(); }
    size_t Size() const { return m_Set.size(); }
    size_t size() const { return m_Set.size(); }
    bool IsEmpty() const { return m_Set.empty(); }
    bool empty() const { return m_Set.empty(); }

    void Add(const KeyType &key) { m_Set.insert(key); }
    void Add(KeyType &&key) { m_Set.insert(std::move(key)); }
    void insert(const KeyType &key) { m_Set.insert(key); }
    void insert(KeyType &&key) { m_Set.insert(std::move(key)); }

    bool Contains(const KeyType &key) const { return m_Set.find(key) != m_Set.end(); }
    size_t count(const KeyType &key) const { return m_Set.count(key); }
    iterator find(const KeyType &key) { return m_Set.find(key); }
    const_iterator find(const KeyType &key) const { return m_Set.find(key); }

    bool Remove(const KeyType &key) { return m_Set.erase(key) > 0; }
    size_t erase(const KeyType &key) { return m_Set.erase(key); }
    iterator erase(const_iterator pos) { return m_Set.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return m_Set.erase(first, last); }

    void Empty() { m_Set.clear(); }
    void Clear() { m_Set.clear(); }
    void clear() { m_Set.clear(); }

    bool operator==(const TESet &other) const { return m_Set == other.m_Set; }
    bool operator!=(const TESet &other) const { return m_Set != other.m_Set; }

    iterator begin() { return m_Set.begin(); }
    iterator end() { return m_Set.end(); }
    const_iterator begin() const { return m_Set.begin(); }
    const_iterator end() const { return m_Set.end(); }
    const_iterator cbegin() const { return m_Set.cbegin(); }
    const_iterator cend() const { return m_Set.cend(); }
};

template <typename KeyType> using TEOrderedSet = std::set<KeyType>;

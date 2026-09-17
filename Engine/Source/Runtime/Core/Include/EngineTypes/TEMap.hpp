#pragma once
#include "EngineTypes/TEArray.hpp"
#include <unordered_map>
#include <map>
#include <utility>
#include <functional>

// ==========================================
// TEMap - Engine Hash Map Template
// ==========================================
// TEMap - Engine Hash Map Template
// ==========================================
template <typename KeyType, typename ValueType, typename Hasher = std::hash<KeyType>> class TEMap
{
private:
    std::unordered_map<KeyType, ValueType, Hasher> m_Map;

public:
    using key_type = KeyType;
    using mapped_type = ValueType;
    using value_type = std::pair<const KeyType, ValueType>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using hasher = Hasher;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = typename std::unordered_map<KeyType, ValueType, Hasher>::iterator;
    using const_iterator = typename std::unordered_map<KeyType, ValueType, Hasher>::const_iterator;

    TEMap() = default;
    TEMap(std::initializer_list<value_type> initList) : m_Map(initList) {}

    // Implicit conversion / assignment with std::unordered_map
    TEMap(const std::unordered_map<KeyType, ValueType, Hasher> &other) : m_Map(other) {}
    TEMap(std::unordered_map<KeyType, ValueType, Hasher> &&other) : m_Map(std::move(other)) {}
    TEMap &operator=(const std::unordered_map<KeyType, ValueType, Hasher> &other)
    {
        m_Map = other;
        return *this;
    }
    TEMap &operator=(std::unordered_map<KeyType, ValueType, Hasher> &&other)
    {
        m_Map = std::move(other);
        return *this;
    }
    operator std::unordered_map<KeyType, ValueType, Hasher> &() { return m_Map; }
    operator const std::unordered_map<KeyType, ValueType, Hasher> &() const { return m_Map; }

    size_t Num() const { return m_Map.size(); }
    size_t Size() const { return m_Map.size(); }
    size_t size() const { return m_Map.size(); }
    bool IsEmpty() const { return m_Map.empty(); }
    bool empty() const { return m_Map.empty(); }

    void Add(const KeyType &key, const ValueType &value) { m_Map[key] = value; }
    void Add(const KeyType &key, ValueType &&value) { m_Map[key] = std::move(value); }

    ValueType &operator[](const KeyType &key) { return m_Map[key]; }
    ValueType &operator[](KeyType &&key) { return m_Map[std::move(key)]; }

    bool Contains(const KeyType &key) const { return m_Map.find(key) != m_Map.end(); }

    size_t count(const KeyType &key) const { return m_Map.count(key); }

    ValueType &at(const KeyType &key) { return m_Map.at(key); }

    const ValueType &at(const KeyType &key) const { return m_Map.at(key); }

    ValueType *Find(const KeyType &key)
    {
        auto it = m_Map.find(key);
        return (it != m_Map.end()) ? &(it->second) : nullptr;
    }

    const ValueType *Find(const KeyType &key) const
    {
        auto it = m_Map.find(key);
        return (it != m_Map.end()) ? &(it->second) : nullptr;
    }

    iterator find(const KeyType &key) { return m_Map.find(key); }
    const_iterator find(const KeyType &key) const { return m_Map.find(key); }

    bool Remove(const KeyType &key) { return m_Map.erase(key) > 0; }

    bool Erase(const KeyType &key) { return m_Map.erase(key) > 0; }

    size_t erase(const KeyType &key) { return m_Map.erase(key); }
    iterator erase(const_iterator pos) { return m_Map.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return m_Map.erase(first, last); }

    void Empty() { m_Map.clear(); }
    void Clear() { m_Map.clear(); }
    void clear() { m_Map.clear(); }

    TEArray<KeyType> GetKeys() const
    {
        TEArray<KeyType> keys;
        keys.Reserve(m_Map.size());
        for (const auto &pair : m_Map)
            keys.Add(pair.first);
        return keys;
    }

    TEArray<ValueType> GetValues() const
    {
        TEArray<ValueType> values;
        values.Reserve(m_Map.size());
        for (const auto &pair : m_Map)
            values.Add(pair.second);
        return values;
    }

    iterator begin() { return m_Map.begin(); }
    iterator end() { return m_Map.end(); }
    const_iterator begin() const { return m_Map.begin(); }
    const_iterator end() const { return m_Map.end(); }
    const_iterator cbegin() const { return m_Map.cbegin(); }
    const_iterator cend() const { return m_Map.cend(); }
};



template <typename KeyType, typename ValueType>
using TEOrderedMap = std::map<KeyType, ValueType>;
